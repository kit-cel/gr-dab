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
#include "select_cus_vcvc_impl.h"

namespace gr {
  namespace dab {

    select_cus_vcvc::sptr
    select_cus_vcvc::make(unsigned int vlen, unsigned int frame_len, unsigned int address, unsigned int size)
    {
      return gnuradio::get_initial_sptr
        (new select_cus_vcvc_impl(vlen, frame_len, address, size));
    }

    /*
     * The private constructor
     */
    select_cus_vcvc_impl::select_cus_vcvc_impl(unsigned int vlen, unsigned int frame_len, unsigned int address, unsigned int size)
      : gr::block("select_cus_vcvc",
              gr::io_signature::make(1, 1, vlen * sizeof(float)),
              gr::io_signature::make(1, 1, vlen * sizeof(float))),
        d_vlen(vlen),
        d_frame_len(frame_len),
        d_address(address),
        d_size(size)
    {}

    /*
     * Our virtual destructor.
     */
    select_cus_vcvc_impl::~select_cus_vcvc_impl()
    {
    }

    void
    select_cus_vcvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    int
    select_cus_vcvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const float *in = (const float *) input_items[0];
      float *out = (float *) output_items[0];
      unsigned int nwritten = 0;

      for (int i = 0; i < noutput_items; ++i) {
        if(d_address <= (nitems_read(0)+i)%d_frame_len && (nitems_read(0)+i)%d_frame_len < d_address + d_size){
          //this cu is one of the selected subchannel -> copy it to ouput buffer
          memcpy(&out[nwritten++*d_vlen], &in[i*d_vlen], d_vlen * sizeof(float));
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return nwritten;
    }

  } /* namespace dab */
} /* namespace gr */

