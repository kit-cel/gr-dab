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

#include <gnuradio/io_signature.h>
#include "complex_to_interleaved_float_cf_impl.h"

namespace gr {
  namespace dab {

    complex_to_interleaved_float_cf::sptr
    complex_to_interleaved_float_cf::make(unsigned int symbol_length)
    {
      return gnuradio::get_initial_sptr
        (new complex_to_interleaved_float_cf_impl(symbol_length));
    }

    /*
     * The private constructor
     */
    complex_to_interleaved_float_cf_impl::complex_to_interleaved_float_cf_impl(unsigned int symbol_length)
      : gr::sync_interpolator("complex_to_interleaved_float_cf",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)), 2),
        d_symbol_length(symbol_length)
    {
      set_output_multiple(2*symbol_length);
    }

    /*
     * Our virtual destructor.
     */
    complex_to_interleaved_float_cf_impl::~complex_to_interleaved_float_cf_impl()
    {
    }

    int
    complex_to_interleaved_float_cf_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      for (int i = 0; i < noutput_items/2; ++i) {
        // write the real parts of the symbol first
        out[i] = in[i].real();
        // write the imaginary parts of the symbol second
        out[i+d_symbol_length] = in[i].imag();
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */

