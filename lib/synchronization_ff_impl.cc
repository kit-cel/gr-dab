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
#include "synchronization_ff_impl.h"

namespace gr {
  namespace dab {

    synchronization_ff::sptr
    synchronization_ff::make(int symbol_length, int cyclic_prefix_length)
    {
      return gnuradio::get_initial_sptr
        (new synchronization_ff_impl(symbol_length, cyclic_prefix_length));
    }

    /*
     * The private constructor
     */
    synchronization_ff_impl::synchronization_ff_impl(int symbol_length, int cyclic_prefix_length)
      : gr::sync_block("synchronization_ff",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_symbol_length(symbol_length),
        d_cyclic_prefix_length(cyclic_prefix_length)
    {
      //set_history(cyclic_prefix_length);
      set_min_noutput_items(symbol_length+cyclic_prefix_length);
      d_correlation = 0;
      d_energy_prefix = 1;
      d_energy_repetition = 1;
      d_recalculate = 0;
    }

    /*
     * Our virtual destructor.
     */
    synchronization_ff_impl::~synchronization_ff_impl()
    {
    }

    int
    synchronization_ff_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];


      for (int i = 0; i < noutput_items-d_symbol_length-d_cyclic_prefix_length; i++) {
        if(d_recalculate > 100000){
          d_correlation = 0;
          for (int j = 0; j < d_cyclic_prefix_length; j++) {
            d_correlation += in[i+j] * conj(in[d_symbol_length+i+j]);
          }

          d_energy_prefix = 0;
          for (int j = 0; j < d_cyclic_prefix_length; j++) {
            d_energy_prefix += std::real(in[i+j] * conj(in[i+j]));
          }

          d_energy_repetition = 0;
          for (int j = 0; j < d_cyclic_prefix_length; j++) {
            d_energy_repetition += std::real(in[i+j+d_symbol_length] * conj(in[i+j+d_symbol_length]));
          }

          d_recalculate = 0;
        }
        else {
          d_correlation += in[i + d_cyclic_prefix_length - 1] * conj(in[d_symbol_length+i + d_cyclic_prefix_length - 1]);
          d_energy_prefix += std::real(in[i + d_cyclic_prefix_length - 1] * conj(in[i + d_cyclic_prefix_length - 1]));
          d_energy_repetition += std::real(in[d_symbol_length + i + d_cyclic_prefix_length - 1]*conj(in[d_symbol_length+i + d_cyclic_prefix_length - 1]));
          out[i] = d_energy_repetition;
          d_correlation -= in[i] * conj(in[d_symbol_length+i]);
          d_energy_prefix -= std::real(in[i] * conj(in[i]));
          d_energy_repetition -= std::real(in[d_symbol_length+i]*conj(in[d_symbol_length+i]));
        }

        out[i] = d_correlation/std::sqrt(d_energy_prefix*d_energy_repetition);

        d_recalculate ++;
      }

        //d_delay_correlation = d_delay_correlation - in[i]*conj(in[i+d_symbol_length]) + in[i+d_cyclic_prefix_length]*conj(in[i+d_cyclic_prefix_length+d_symbol_length]);
        //d_energy_prefix = d_energy_prefix - std::real(in[i]*conj(in[i])) + std::real(in[i+d_cyclic_prefix_length]*conj(in[i+d_cyclic_prefix_length]));
        //d_energy_repetition = d_energy_repetition - in[i+d_symbol_length]*conj(in[i+d_symbol_length]) + in[i+d_cyclic_prefix_length+d_symbol_length]*conj(in[i+d_cyclic_prefix_length+d_symbol_length]);
        //out[i] = d_energy_prefix.real();

        /*for (int j = 0; j < d_cyclic_prefix_length; ++j) {
          //d_delay_correlation += in[i+j] * conj(in[i+j+d_symbol_length]);
          d_energy_prefix += std::real(in[i+j] * conj(in[i+j]));
          //d_energy_repetition += std::real(in[i+j+d_symbol_length] * conj(in[i+j+d_symbol_length]));
        }
        out[i] = d_energy_prefix;*/

      // Tell runtime system how many output items we produced.
      return noutput_items-d_symbol_length-d_cyclic_prefix_length;
    }

  } /* namespace dab */
} /* namespace gr */

