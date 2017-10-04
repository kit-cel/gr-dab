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
#include "frequency_deinterleave_cc_impl.h"

namespace gr {
  namespace dab {

    frequency_deinterleave_cc::sptr
    frequency_deinterleave_cc::make(const std::vector<short> &interleaving_sequence)
    {
      return gnuradio::get_initial_sptr
        (new frequency_deinterleave_cc_impl(interleaving_sequence));
    }

    /*
     * The private constructor
     */
    frequency_deinterleave_cc_impl::frequency_deinterleave_cc_impl(const std::vector<short> &interleaving_sequence)
      : gr::sync_block("frequency_deinterleave_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_interleaving_sequence(interleaving_sequence),
        d_length(interleaving_sequence.size())
    {
      // check if interleaving sequency matches with its size
      for (int i = 0; i < d_length; ++i) {
        if (d_interleaving_sequence[i] >= d_length) {
          throw std::invalid_argument((boost::format("size of interleaving element (%d) exceeds length of symbol (%d)") %(int)d_interleaving_sequence[i] %(int)d_length).str());
        }
      }
      set_output_multiple(d_length);
    }

    /*
     * Our virtual destructor.
     */
    frequency_deinterleave_cc_impl::~frequency_deinterleave_cc_impl()
    {
    }

    int
    frequency_deinterleave_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for (int i = 0; i < noutput_items; ++i) {
        out[d_interleaving_sequence[i]] = in[i];
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */

