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
#include <gnuradio/expj.h>

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
    ofdm_synchronization_cvf_impl::ofdm_synchronization_cvf_impl(int symbol_length, int cyclic_prefix_length, int fft_length, int symbols_per_frame)
            : gr::block("ofdm_synchronization_cvf",
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
            //gr::io_signature::make(1, 1, sizeof(gr_complex))),
                        gr::io_signature::make(1, 1, sizeof(gr_complex))),
              d_symbol_length(symbol_length),
              d_cyclic_prefix_length(cyclic_prefix_length),
              d_symbols_per_frame(symbols_per_frame),
              d_fft_length(fft_length)
    {
      d_correlation = 0;
      d_energy_prefix = 1;
      d_energy_repetition = 1;
      d_NULL_symbol_energy = 1;
      d_frequency_offset = 0;
      d_NULL_detected = false;
      d_moving_average_counter = 0;
      d_symbol_count = 0;
      d_symbol_element_count = 0;
      d_wait_for_NULL = true;
      d_on_triangle = false;
      d_control_counter = 0;
    }

    /*
     * Our virtual destructor.
     */
    ofdm_synchronization_cvf_impl::~ofdm_synchronization_cvf_impl()
    {
    }

    void
    ofdm_synchronization_cvf_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items + d_symbol_length + d_cyclic_prefix_length+1;
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
        if(d_correlation_normalized_magnitude < 0.5){
          // we left the triangle
          d_on_triangle = false;
          return false;
        }
        else{
          // we are still on the triangle but we already picked our pre-peak value
          return false;
        }
      }
      else{
        // not on a correlation triangle yet
        if(d_correlation_normalized_magnitude > 0.85){
          // no we are on the triangle
          d_on_triangle = true;
          return true;
        }
        else{
          // no triangle here
          return false;
        }
      }
    }
/*
    bool
    ofdm_synchronization_cvf_impl::detect_peak() // on triangle used for detect_start_of_symbol()
    {
      if (!d_on_triangle && d_correlation_normalized_magnitude > 0.85) {
        // over threshold, peak is coming soon
        d_on_triangle = true;
        return false;
      } else if (d_on_triangle && d_correlation_normalized_magnitude < 0.8) {
        // peak found
        d_on_triangle = false;
        return true;
      } else {
        return false;
      }
    }*/

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
            if (d_NULL_detected && (d_energy_prefix > d_NULL_symbol_energy * 2)) {
              // calculate new frequency offset
              d_frequency_offset = std::arg(d_correlation)/0.001246; // in rad/s
              // set tag at beginning of new frame (first symbol after null symbol)
              add_item_tag(0, nitems_written(0) + i+1, pmt::mp("Start"),
                           pmt::from_float(std::arg(d_correlation)));
              GR_LOG_DEBUG(d_logger, format("Start of frame, freq offset %d")%d_frequency_offset);
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
            }
          } else {
            if (((!d_NULL_detected) && (d_energy_prefix / d_energy_repetition < 0.1))) {
              // NULL symbol detection, if energy is < 0.1 * energy a symbol time later
              d_NULL_symbol_energy = d_energy_prefix;
              d_NULL_detected = true;
            }
          }
        }
        else if (++d_symbol_element_count >= (d_symbol_length + d_cyclic_prefix_length)) {
          d_symbol_count++;
          // next symbol expecting
          // reset symbol_element_count because we arrived at the start of the next symbol
          d_symbol_element_count = 0;
          // check if we arrived at the next NULL symbol
          if (d_symbol_count >= d_symbols_per_frame) {
            //GR_LOG_DEBUG(d_logger, format("Sucessfully finished!!!"));
            d_symbol_count = 0;
            //TODO: skip forward more to safe computing many computing steps
            // switch to acquisition mode again to get the start of the next frame exactly
            d_wait_for_NULL = true;
          }
          else {
            // correlation has to be calculated completely new, because of skipping samples before
            delayed_correlation(&in[i], true);
            //GR_LOG_DEBUG(d_logger, format("  New possible peak %d (i=%d)") % d_correlation_normalized_magnitude % i);
            // check if there is really a peak
            if (d_correlation_normalized_magnitude > 0.5) { //TODO: check if we are on right edge
              d_frequency_offset = std::arg(d_correlation) / 0.001246; // in rad/s
              //add_item_tag(0, nitems_written(0) + i, pmt::mp("on track"), pmt::from_float(d_frequency_offset));
              //GR_LOG_DEBUG(d_logger, format("in track %d") % d_frequency_offset);
            } else {
              // no peak found -> out of track; search for next NULL symbol
              d_wait_for_NULL = true;
              GR_LOG_DEBUG(d_logger, format("Lost track, switching ot acquisition mode (%d)")%d_correlation_normalized_magnitude);
              /*add_item_tag(0, nitems_written(0) + i, pmt::mp("Lost"),
                           pmt::from_float(d_correlation_normalized_magnitude));*/
            }
          }
        }
        else if(d_cyclic_prefix_length <= d_symbol_element_count){
          // now we start copying one symbol length to the output
          out[d_nwritten++] = in[i] * gr_expj(-d_frequency_offset);
        }
        // copy sample correcting the frequency offset
        //out[i] = d_correlation_normalized_magnitude;;
      }

      // pass iq samples through with set tags
      // memcpy(out, in, (noutput_items - d_cyclic_prefix_length - d_symbol_length)* sizeof(gr_complex));

      // debug output to check the delayed correlation
      /*d_moving_average_counter = 0;
      for (int j = 0; j < noutput_items - d_cyclic_prefix_length - d_symbol_length; ++j) {
        delayed_correlation(&in[j], false);
        out[j] = d_correlation_normalized;
      }*/
      consume_each(noutput_items);
      return d_nwritten;
    }

  } /* namespace dab */
} /* namespace gr */

