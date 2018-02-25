/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <gnuradio/io_signature.h>
#include "diff_phasor_vcc_impl.h"

namespace gr {
  namespace dab {

    diff_phasor_vcc::sptr
    diff_phasor_vcc::make(unsigned int length) {
      return gnuradio::get_initial_sptr
              (new diff_phasor_vcc_impl(length));
    }

    diff_phasor_vcc_impl::diff_phasor_vcc_impl(unsigned int length)
            : gr::sync_block("diff_phasor_vcc",
                             gr::io_signature::make(1, 1, sizeof(gr_complex) * length),
                             gr::io_signature::make(1, 1, sizeof(gr_complex) * length)),
              d_length(length) {
      set_history(2);
    }


    int
    diff_phasor_vcc_impl::work(int noutput_items,
                               gr_vector_const_void_star &input_items,
                               gr_vector_void_star &output_items) {
      gr_complex const *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for (unsigned int i = 0; i < noutput_items * d_length; i++) {
        out[i] = in[i + d_length] * conj(in[i]);
      }

      return noutput_items;
    }

  }
}
