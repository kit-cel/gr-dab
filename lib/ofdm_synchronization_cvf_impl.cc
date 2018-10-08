/* -*- c++ -*- */
/*
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL)
 * Karlsruhe Institute of Technology (KIT).
 * Copyright 2018 Communications Engineering Lab (CEL) Karlsruhe Institute of Technology (KIT).
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ofdm_synchronization_cvf_impl.h"
#include <gnuradio/io_signature.h>
#include <boost/format.hpp>
#include <volk/volk.h>
#include <complex>
#include <sstream>
#include <string>
#include <cstdio>
#include <cmath>

using namespace boost;

namespace gr {
  namespace dab {

    ofdm_synchronization_cvf::sptr
    ofdm_synchronization_cvf::make(int symbol_length, int cyclic_prefix_length,
                                   int fft_length, int symbols_per_frame) {
      return gnuradio::get_initial_sptr(new ofdm_synchronization_cvf_impl(symbol_length,
                                                                          cyclic_prefix_length,
                                                                          fft_length,
                                                                          symbols_per_frame));
    }

    /*
     * The private constructor
     */
    ofdm_synchronization_cvf_impl::ofdm_synchronization_cvf_impl(
            int symbol_length, int cyclic_prefix_length,
            int fft_length, int symbols_per_frame)
            : gr::block("ofdm_synchronization_cvf",
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
                        gr::io_signature::make(1, 1, sizeof(gr_complex))),
              d_symbol_length(symbol_length),
              d_cyclic_prefix_length(cyclic_prefix_length),
              d_symbols_per_frame(symbols_per_frame),
              d_fft_length(fft_length),
              d_correlation(0),
              d_energy_prefix(1),
              d_energy_repetition(1),
              d_frequency_offset_per_sample(0),
              d_NULL_detected(false),
              d_moving_average_counter(0),
              d_symbol_count(0),
              d_symbol_element_count(0),
              d_wait_for_NULL(true),
              d_on_triangle(false),
              d_phase(gr_complex(1,0)),
              d_peak_set(false),
              d_correlation_maximum(0){
      //allocation for repeating energy measurements
      unsigned int alignment = volk_get_alignment();
      d_mag_squared = (float *) volk_malloc(sizeof(float) * d_cyclic_prefix_length, alignment);
      d_fixed_lag_corr = (gr_complex *) volk_malloc(sizeof(gr_complex) * d_cyclic_prefix_length, alignment);
      this->set_output_multiple(d_symbol_length);
    }

    /*
     * Our virtual destructor.
     */
    ofdm_synchronization_cvf_impl::~ofdm_synchronization_cvf_impl() {
    }

    void
    ofdm_synchronization_cvf_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      ninput_items_required[0] = noutput_items + d_symbol_length + d_cyclic_prefix_length + 1;
    }

    void
    ofdm_synchronization_cvf_impl::delayed_correlation(const gr_complex *sample, bool new_calculation) {
      if (d_moving_average_counter > 100000 || d_moving_average_counter == 0 || new_calculation) {
        if (d_moving_average_counter == 0 && (!new_calculation)) {
          // first value is calculated completely, next values are calculated with moving average
          d_moving_average_counter++;
        } else {
          // reset counter
          d_moving_average_counter = 0;
        }
        // calculate delayed correlation for this sample completely
        volk_32fc_x2_conjugate_dot_prod_32fc(d_fixed_lag_corr, sample,
                                             &sample[d_symbol_length],
                                             d_cyclic_prefix_length);
        d_correlation = 0;
        for (int i = 0; i < d_cyclic_prefix_length; ++i) {
          d_correlation += d_fixed_lag_corr[i];
        }
        // calculate energy of cyclic prefix for this sample completely
        volk_32fc_magnitude_squared_32f(d_mag_squared, sample, d_cyclic_prefix_length);
        volk_32f_accumulator_s32f(&d_energy_prefix, d_mag_squared, d_cyclic_prefix_length);
        // calculate energy of its repetition for this sample completely
        volk_32fc_magnitude_squared_32f(d_mag_squared, &sample[d_symbol_length], d_cyclic_prefix_length);
        volk_32f_accumulator_s32f(&d_energy_repetition, d_mag_squared, d_cyclic_prefix_length);
      } else {
        // calculate next step for moving average
        d_correlation += sample[d_cyclic_prefix_length - 1] * conj(sample[d_symbol_length + d_cyclic_prefix_length - 1]);
        d_energy_prefix += std::real(sample[d_cyclic_prefix_length - 1] * conj(sample[d_cyclic_prefix_length - 1]));
        d_energy_repetition += std::real( sample[d_symbol_length + d_cyclic_prefix_length - 1] *
                                          conj(sample[d_symbol_length + d_cyclic_prefix_length - 1]));
        d_correlation -= sample[0] * conj(sample[d_symbol_length]);
        d_energy_prefix -= std::real(sample[0] * conj(sample[0]));
        d_energy_repetition -= std::real(sample[d_symbol_length] * conj(sample[d_symbol_length]));
        d_moving_average_counter++;
      }
      // normalize
      d_correlation_normalized = d_correlation / std::sqrt(d_energy_prefix * d_energy_repetition);
      // calculate magnitude
      d_correlation_normalized_magnitude = d_correlation_normalized.real() * d_correlation_normalized.real() +
                                           d_correlation_normalized.imag() * d_correlation_normalized.imag();
    }

    /*! \brief returns true at a point with a little space before the peak of a correlation triangular
     *
     */
    bool
    ofdm_synchronization_cvf_impl::detect_start_of_symbol() {
      if (d_on_triangle) {
        if (d_correlation_normalized_magnitude > d_correlation_maximum) {
          d_correlation_maximum = d_correlation_normalized_magnitude;
        }
        if (d_correlation_normalized_magnitude < d_correlation_maximum - 0.05 && !d_peak_set) {
          // we are right behind the peak
          d_peak_set = true;
          return true;
        }
        if (d_correlation_normalized_magnitude < 0.25) {
          d_peak_set = false;
          d_on_triangle = false;
          d_correlation_maximum = 0;
        } else {
          // we are still on the triangle but have not reached the end
          return false;
        }
      } else {
        // not on a correlation triangle yet
        if (d_correlation_normalized_magnitude > 0.35) {
          // now we are on the triangle
          d_on_triangle = true;
          return false;
        } else {
          // no triangle here
          return false;
        }
      }
    }

    int
    ofdm_synchronization_cvf_impl::general_work(int noutput_items,
                                                gr_vector_int &ninput_items,
                                                gr_vector_const_void_star &input_items,
                                                gr_vector_void_star &output_items) {
      // Initialize this run of work
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      d_nwritten = 0;

      for (int i = 0; i < noutput_items; ++i) {
        // just for measurement reasons
        if (d_wait_for_NULL) {
          // acquisition mode: search for next correlation peak after a NULL symbol
          delayed_correlation(&in[i], false);
          if (detect_start_of_symbol()) {
            if (d_NULL_detected) {
              // calculate new frequency offset
              d_frequency_offset_per_sample = std::arg(d_correlation) / d_fft_length; // in rad/sample
              /* The start of the first symbol after the NULL symbol has been detected. The ideal start to copy
               * the symbol is &in[i+d_cyclic_prefix_length] to minimize ISI.
               */
              //reset the symbol element counter
              d_symbol_element_count = 0;
              d_symbol_count = 0;
              // reset NULL detector
              d_NULL_detected = false;
              // switch to tracking mode
              d_wait_for_NULL = false;
            } else {
              //peak but not after NULL symbol
              d_NULL_detected = false;
            }
          } else {
            if (((!d_NULL_detected) && (d_energy_prefix / d_energy_repetition < 0.4))) {
              // NULL symbol detection, if energy is < 0.4 * energy a symbol time later
              d_NULL_detected = true;
            }
          }
        } else { // tracking mode
          if (d_symbol_element_count >= (d_symbol_length + d_cyclic_prefix_length)) {
            // we expect the start of a new symbol here or the a NULL symbol if we arrived at the end of the frame
            d_symbol_count++;
            // next symbol expecting
            // reset symbol_element_count because we arrived at the start of the next symbol
            d_symbol_element_count = 0;
            // check if we arrived at the next NULL symbol
            if (d_symbol_count >= d_symbols_per_frame) {
              d_symbol_count = 0;
              //TODO: skip forward more to safe computing many computing steps
              // switch to acquisition mode again to get the start of the next frame exactly
              d_wait_for_NULL = true;
            } else {
              // we expect the start of a new symbol here
              // correlation has to be calculated completely new, because of skipping samples before
              delayed_correlation(&in[i], true);
              // check if there is really a peak
              if (d_correlation_normalized_magnitude > 0.3) { //TODO: check if we are on right edge
                d_frequency_offset_per_sample = std::arg(d_correlation) / d_fft_length; // in rad/sample
              } else {
                // no peak found -> out of track; search for next NULL symbol
                d_wait_for_NULL = true;
                GR_LOG_DEBUG(d_logger,
                             format("Lost track at %d, switching ot acquisition mode (%d)") %
                             d_symbol_count %
                             d_correlation_normalized_magnitude);
              }
            }
          } else if (d_symbol_element_count == (3 * d_cyclic_prefix_length)/4){
            // No full OFDM symbol in the input left
            if (noutput_items - i < d_symbol_length || ninput_items[0] - i < d_symbol_length) {
              this->consume_each(i);
              return d_nwritten;
            } else {
              d_phase *=
                  std::polar(float(1.0),
                             static_cast<float>(d_cyclic_prefix_length *
                                                d_frequency_offset_per_sample));
              if (d_symbol_count == 0) {
                this->add_item_tag(0, this->nitems_written(0) + d_nwritten + d_cyclic_prefix_length - d_symbol_element_count,
                                   pmt::mp("Start"),
                                   pmt::from_float(std::arg(d_correlation)));
              }
              volk_32fc_s32fc_x2_rotator_32fc(
                  &out[d_nwritten], &in[i],
                  std::polar(float(1.0), d_frequency_offset_per_sample),
                  &d_phase, d_symbol_length);
              d_nwritten += d_symbol_length;
              d_symbol_element_count += d_symbol_length - 1;
              i += d_symbol_length - 1;
            }
          }
          d_symbol_element_count++;
        }
        /* fine frequency correction:
         * The frequency offset estimation is done above with the correlation.
         * The modulated frequency runs parallel to the whole input stream,
         * including the cyclic prefix, and is mixed
         * to the output symbol samples.
         */
    }
      consume_each(noutput_items);
      return d_nwritten;
    }

  } /* namespace dab */
} /* namespace gr */
