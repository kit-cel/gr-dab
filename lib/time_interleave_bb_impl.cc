/* -*- c++ -*- */
/*
 * Copyright 2017 by Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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
#include "time_interleave_bb_impl.h"

namespace gr {
  namespace dab {

    time_interleave_bb::sptr
    time_interleave_bb::make(int vector_length,
                             const std::vector<unsigned char> &scrambling_vector) {
      return gnuradio::get_initial_sptr
              (new time_interleave_bb_impl(vector_length, scrambling_vector));
    }

    /*
     * The private constructor
     */
    time_interleave_bb_impl::time_interleave_bb_impl(int vector_length,
                                                     const std::vector<unsigned char> &scrambling_vector)
            : gr::sync_block("time_interleave_bb",
                             gr::io_signature::make(1, 1,
                                                    sizeof(unsigned char) *
                                                    vector_length),
                             gr::io_signature::make(1, 1,
                                                    sizeof(unsigned char) *
                                                    vector_length)),
              d_vector_length(vector_length),
              d_scrambling_vector(scrambling_vector) {
      d_scrambling_length = scrambling_vector.size(); //size of the scrambling vector
      set_history(
              d_scrambling_length); //need for max delay of (scrambling_length-1) * 24ms
    }

    /*
     * Our virtual destructor.
     */
    time_interleave_bb_impl::~time_interleave_bb_impl() {
    }

    int
    time_interleave_bb_impl::work(int noutput_items,
                                  gr_vector_const_void_star &input_items,
                                  gr_vector_void_star &output_items) {
      const unsigned char *in = (const unsigned char *) input_items[0];
      unsigned char *out = (unsigned char *) output_items[0];

      // produce output vectors
      for (int i = 0;
           i < noutput_items; i++) { //iteration over produced output vectors
        for (int j = 0;
             j < d_vector_length; j++) { //iteration over elements of vector
          //*out++ = in[vec_length * (i + (scrambling_length-1) - d_scrambling_vector[j % scrambling_length]) + j];
          *out++ = in[d_vector_length * (i + (d_scrambling_length - 1) -
                                         d_scrambling_vector[j %
                                                             d_scrambling_length]) +
                      j];
        }
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */
