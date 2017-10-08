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
#include "differential_phasor_vcvc_impl.h"

namespace gr {
  namespace dab {

    differential_phasor_vcvc::sptr
    differential_phasor_vcvc::make(unsigned int length, unsigned int symbols_per_frame)
    {
      return gnuradio::get_initial_sptr
        (new differential_phasor_vcvc_impl(length, symbols_per_frame));
    }

    /*
     * The private constructor
     */
    differential_phasor_vcvc_impl::differential_phasor_vcvc_impl(unsigned int length, unsigned int symbols_per_frame)
      : gr::block("differential_phasor_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*length),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*length)),
        d_length(length),
        d_symbols_per_frame(symbols_per_frame)
    {
      set_min_noutput_items(3);
    }

    /*
     * Our virtual destructor.
     */
    differential_phasor_vcvc_impl::~differential_phasor_vcvc_impl()
    {
    }

    void
    differential_phasor_vcvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    int
    differential_phasor_vcvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      fprintf(stderr, "Work call ####################################\n");
      fprintf(stderr, "nitems_read %d\n", nitems_read(0));
      fprintf(stderr, "noutput_items %d\n", noutput_items);
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      unsigned int symbols_written = 0;

      // get tags for the beginning of a frame
      std::vector<gr::tag_t> tags;
      unsigned int tag_count = 0;
      //get_tags_in_window(tags, 0, , noutput_items);
      get_tags_in_range(tags, 0, nitems_read(0), nitems_read(0) + noutput_items);
      fprintf(stderr, "number of tags: %d\n", tags.size());
      if(tags[tag_count].offset-nitems_read(0) == 0){
        fprintf(stderr, "Tag at beginning, pos %d\n", tags[tag_count].offset);
        tag_count++;
      }

      for (int i = 1; i < noutput_items; ++i) {
        // reset symbol counter if tag is coming in
        if(tag_count < tags.size() && tags[tag_count].offset-nitems_read(0)-i == 0){
          fprintf(stderr, "Tag at pos %d\n", tags[tag_count].offset);
          /* start of frame -> this symbol is a phase reference symbol
           * we need it to calculate the phase diff of the next symbol, so we just consume it now
           */
          tag_count++;
        }
        else{
          // this is any other symbol than the phase reference symbol, and we calculate the diff to its predecessor
          for(unsigned int j = 0; j < d_length; j++){
            //out[symbols_written*d_length+j] = in[(i+1)*d_length+j] * conj(in[i*d_length+j]);
            out[symbols_written*d_length+j] = in[i*d_length+j] * conj(in[(i-1)*d_length+j]);
          }
          ++symbols_written;
        }
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items-1);
      fprintf(stderr, "consume each %d\n", noutput_items-1);

      // Tell runtime system how many output items we produced.
      return symbols_written;
    }

  } /* namespace dab */
} /* namespace gr */

