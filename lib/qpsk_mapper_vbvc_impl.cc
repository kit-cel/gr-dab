/* -*- c++ -*- */
/* 
 * Copyright 2017, 2018 by Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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
#include "qpsk_mapper_vbvc_impl.h"

namespace gr {
  namespace dab {

    qpsk_mapper_vbvc::sptr
    qpsk_mapper_vbvc::make(unsigned int symbol_length)
    {
      return gnuradio::get_initial_sptr
        (new qpsk_mapper_vbvc_impl(symbol_length));
    }

    /*
     * The private constructor
     */
    qpsk_mapper_vbvc_impl::qpsk_mapper_vbvc_impl(unsigned int symbol_length)
      : gr::sync_block("qpsk_mapper_vbvc",
              gr::io_signature::make(1, 1, sizeof(char)*symbol_length/4),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*symbol_length)),
        d_symbol_length(symbol_length)
    {}

    /*
     * Our virtual destructor.
     */
    qpsk_mapper_vbvc_impl::~qpsk_mapper_vbvc_impl()
    {
    }

    int
    qpsk_mapper_vbvc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const char *in = (const char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for (int i = 0; i < noutput_items; ++i) {
        // iterate over each symbol vector
        for (int j = 0; j < d_symbol_length/8; ++j) {
          // iterate over the symbol vector, but each byte has 8 bit, which are accessed manually
          for (int k = 0; k < 8; k++) {
            out[i*d_symbol_length + j*8 + k] =
                    gr_complex((in[i*(d_symbol_length/4) + j]&(0x80>>k))>0?-I_SQRT2:I_SQRT2,
                               (in[i*(d_symbol_length/4) + d_symbol_length/8 + j]&(0x80>>k))>0?-I_SQRT2:I_SQRT2);
          }
        }
      }
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */

