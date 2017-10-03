/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

#include <gnuradio/io_signature.h>
#include "phase_correction_cc_impl.h"
#include <gnuradio/expj.h>
#include <gnuradio/fxpt.h>
#include <cmath>

namespace gr {
  namespace dab {

    const uint8_t phase_correction_cc_impl::d_lookup_n[48] = {1,2,0,1,3,2,2,3,2,1,2,3,1,2,3,3,2,2,2,1,1,3,1,2,3,1,1,1,2,2,1,0,2,2,3,3,0,2,1,3,3,3,3,0,3,0,1,1};
    const uint8_t phase_correction_cc_impl::d_lookup_h[4][32] = {{0,2,0,0,0,0,1,1,2,0,0,0,2,2,1,1,0,2,0,0,0,0,1,1,2,0,0,0,2,2,1,1},
                                                                 {0,3,2,3,0,1,3,0,2,1,2,3,2,3,3,0,0,3,2,3,0,1,3,0,2,1,2,3,2,3,3,0},
                                                                 {0,0,0,2,0,2,1,3,2,2,0,2,2,0,1,3,0,0,0,2,0,2,1,3,2,2,0,2,2,0,1,3},
                                                                 {0,1,2,1,0,3,3,2,2,3,2,1,2,1,3,2,0,1,2,1,0,3,3,2,2,3,2,1,2,1,3,2}};

    phase_correction_cc::sptr
    phase_correction_cc::make(int num_carriers)
    {
      return gnuradio::get_initial_sptr
        (new phase_correction_cc_impl(num_carriers));
    }

    /*
     * The private constructor
     */
    phase_correction_cc_impl::phase_correction_cc_impl(int num_carriers)
      : gr::block("phase_correction_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_num_carriers(num_carriers)
    {}

    /*
     * Our virtual destructor.
     */
    phase_correction_cc_impl::~phase_correction_cc_impl()
    {
    }

    void
    phase_correction_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    float
    phase_correction_cc_impl::calc_phase_diff(const gr_complex sample, int k){
      // calculate ideal phase of this phase reference sample
      // k - k'
      uint8_t block_position = k%32;
      // i
      uint8_t i = ((k-block_position)/32)%4;
      // n over lookup table
      uint8_t n = d_lookup_n[(k-block_position)/32];
      // h over lookup table
      uint8_t h = d_lookup_h[i][block_position];
      // ideal phase over formula
      #define F_PI ((float)(M_PI))
      float ideal_phase = (F_PI/2)*(h + n);
      // phase of sample
      float real_phase = std::arg(sample);
      // phase difference
      float diff = ideal_phase - real_phase;
      return std::fmod(diff + F_PI, 2.0f * F_PI) - F_PI;
    }

    int
    phase_correction_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      unsigned int offset = 0;
      std::vector<gr::tag_t> tags;
      unsigned int tag_count = 0;
      // get tags for the beginning of a frame
      get_tags_in_window(tags, 0, 0, noutput_items);

      for (int i = 0; i < noutput_items; ++i) {
        if(tag_count < tags.size() && tags[tag_count].offset-nitems_read(0)-i == 0) {
          d_pilot_counter = 0;
          d_phase_difference = 0;
          tag_count++;
        }
        if(d_pilot_counter < d_num_carriers){
          // add phase difference of current sample to d_phase_difference
          d_phase_difference += calc_phase_diff(in[i], d_pilot_counter++);
        }
        else {
          if (d_pilot_counter == d_num_carriers) {
            // phase reference symobl completely read, calculate average phase difference
            d_phase_difference /= d_num_carriers;
            //fprintf(stderr, "phase difference: %f\n", d_phase_difference);
            d_pilot_counter++;
          }
          // calculate the complex frequency correction value
          float oi, oq;
          // fixed point sine and cosine
          //int32_t angle = gr::fxpt::float_to_fixed (d_phase_difference);
          //gr::fxpt::sincos(angle, &oq, &oi);
          //gr_complex phase_correction = gr_complex(oi, oq);
          gr_complex phase_correction = gr_expj(F_PI/4);
          // copy samples from all non phase reference symbols to output
          out[offset++] = in[i] * phase_correction;
        }
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return offset;
    }

  } /* namespace dab */
} /* namespace gr */

