/* -*- c++ -*- */
/*
 * Copyright 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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

#include <stdio.h>
#include <sstream>
#include <string>
#include <boost/format.hpp>
#include <gnuradio/io_signature.h>
#include "ofdm_synchronization_cvf_impl.h"
#include <gnuradio/fxpt.h>
#include <cmath>

using namespace boost;

namespace gr {
  namespace dab {

    ofdm_synchronization_cvf::sptr
    ofdm_synchronization_cvf::make(int symbol_length, int cyclic_prefix_length, int fft_length, int symbols_per_frame)
    {
      return gnuradio::get_initial_sptr
              (new ofdm_synchronization_cvf_impl(symbol_length, cyclic_prefix_length, fft_length, symbols_per_frame));
    }

    /*
     * The private constructor
     */
    ofdm_synchronization_cvf_impl::ofdm_synchronization_cvf_impl(int symbol_length, int cyclic_prefix_length,
                                                                 int fft_length, int symbols_per_frame)
            : gr::block("ofdm_synchronization_cvf",
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
                        gr::io_signature::make(1, 1, sizeof(gr_complex))),
              d_symbol_length(symbol_length),
              d_cyclic_prefix_length(cyclic_prefix_length),
              d_symbols_per_frame(symbols_per_frame),
              d_fft_length(fft_length),
              d_correlation (0),
              d_energy_prefix (1),
              d_energy_repetition (1),
              d_NULL_symbol_energy (1),
              d_frequency_offset_per_sample (0),
              d_NULL_detected (false),
              d_moving_average_counter (0),
              d_symbol_count (0),
              d_symbol_element_count (0),
              d_wait_for_NULL (true),
              d_on_triangle (false),
              d_control_counter (0),
              d_phase (0),
              d_correlation_maximum (0),
              d_peak_set (false)
    {}

    /*
     * Our virtual destructor.
     */
    ofdm_synchronization_cvf_impl::~ofdm_synchronization_cvf_impl()
    {
    }

    void
    ofdm_synchronization_cvf_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items + d_symbol_length + d_cyclic_prefix_length + 1;
    }

    void
    ofdm_synchronization_cvf_impl::delayed_correlation(const gr_complex *sample, bool new_calculation)
    {
      if (d_moving_average_counter > 100000 || d_moving_average_counter == 0 || new_calculation) {
        if (d_moving_average_counter == 0 && (!new_calculation)) {
          // first value is calculated completely, next values are calculated with moving average
          d_moving_average_counter++;
        } else {
          // reset counter
          d_moving_average_counter = 0;
        }
        // calculate delayed correlation for this sample completely
        d_correlation = 0;
        for (int j = 0; j < d_cyclic_prefix_length; j++) {
          d_correlation += sample[j] * conj(sample[d_symbol_length + j]);
        }
        // calculate energy of cyclic prefix for this sample completely
        d_energy_prefix = 0;
        for (int j = 0; j < d_cyclic_prefix_length; j++) {
          d_energy_prefix += std::real(sample[j] * conj(sample[j]));
        }
        // calculate energy of its repetition for this sample completely
        d_energy_repetition = 0;
        for (int j = 0; j < d_cyclic_prefix_length; j++) {
          d_energy_repetition += std::real(sample[j + d_symbol_length] * conj(sample[j + d_symbol_length]));
        }
      } else {
        // calculate next step for moving average
        d_correlation +=
                sample[d_cyclic_prefix_length - 1] * conj(sample[d_symbol_length + d_cyclic_prefix_length - 1]);
        d_energy_prefix += std::real(sample[d_cyclic_prefix_length - 1] * conj(sample[d_cyclic_prefix_length - 1]));
        d_energy_repetition += std::real(sample[d_symbol_length + d_cyclic_prefix_length - 1] *
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
    ofdm_synchronization_cvf_impl::detect_start_of_symbol()
    {
      if(d_on_triangle){
        if(d_correlation_normalized_magnitude > d_correlation_maximum){
          d_correlation_maximum = d_correlation_normalized_magnitude;

        }
        if(d_correlation_normalized_magnitude < d_correlation_maximum-0.05 && !d_peak_set){
          // we are right behind the peak
          d_peak_set = true;
          return true;
        }
        if(d_correlation_normalized_magnitude < 0.25){
          d_peak_set = false;
          d_on_triangle = false;
          d_correlation_maximum = 0;
        }
        else{
          // we are still on the triangle but have not reached the end
          return false;
        }
      }
      else{
        // not on a correlation triangle yet
        if(d_correlation_normalized_magnitude > 0.35){
          // no we are on the triangle
          d_on_triangle = true;
          return false;
        }
        else{
          // no triangle here
          return false;
        }
      }
    }

    int
    ofdm_synchronization_cvf_impl::general_work(int noutput_items,
                                                gr_vector_int &ninput_items,
                                                gr_vector_const_void_star &input_items,
                                                gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      d_nwritten = 0;

      for (int i = 0; i < noutput_items; ++i) {
        // just for measurement reasons
        if (d_wait_for_NULL) {
          // acquisition mode: search for next correlation peak after a NULL symbol
          delayed_correlation(&in[i], false);
          if (detect_start_of_symbol()) {
            if (d_NULL_detected /*&& (d_energy_prefix > d_NULL_symbol_energy * 2)*/) {
              // calculate new frequency offset
              d_frequency_offset_per_sample = std::arg(d_correlation) / d_fft_length; // in rad/sample
              //GR_LOG_DEBUG(d_logger, format("Start of frame, abs offset %d")%(nitems_read(0)+i));
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
              // NULL symbol detection, if energy is < 0.1 * energy a symbol time later
              d_NULL_symbol_energy = d_energy_prefix;
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
                d_frequency_offset_per_sample = std::arg(d_correlation) / d_fft_length; // in rad/s
              } else {
                // no peak found -> out of track; search for next NULL symbol
                d_wait_for_NULL = true;
                GR_LOG_DEBUG(d_logger, format("Lost track at %d, switching ot acquisition mode (%d)") %d_symbol_count %
                                       d_correlation_normalized_magnitude);
              }
            }
          } else if (d_cyclic_prefix_length*0.75 <= d_symbol_element_count && d_symbol_element_count < d_cyclic_prefix_length*0.75 + d_symbol_length) {
            // calculate the complex frequency correction value
            float oi, oq;
            // fixed point sine and cosine
            int32_t angle = gr::fxpt::float_to_fixed(d_phase);
            gr::fxpt::sincos(angle, &oq, &oi);
            gr_complex fine_frequency_correction = gr_complex(oi, oq);
            // set tag at next item if it is the first element of the first symbol
            if (d_symbol_count == 0 && d_symbol_element_count == d_cyclic_prefix_length) {
              add_item_tag(0, nitems_written(0) + d_nwritten, pmt::mp("Start"),
                           pmt::from_float(std::arg(d_correlation)));
            }
            // now we start copying one symbol length to the output
            out[d_nwritten++] = in[i] * fine_frequency_correction;
          }
          d_symbol_element_count++;
        }
        /* fine frequency correction:
         * The frequency offset estimation is done above with the correlation.
         * The modulated frequency runs parralel to the whole input stream, including the cyclic prefix, and is mixed
         * to the output symbol samples.
         */
        d_phase += d_frequency_offset_per_sample;
        //place phase in [-pi, +pi[
        d_phase = std::fmod(d_phase + M_PI, 2.0f * M_PI) - M_PI;
      }

      consume_each(noutput_items);
      return d_nwritten;
    }

  } /* namespace dab */
} /* namespace gr */

