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
#include "demux_cc_impl.h"
#include <stdio.h>
#include <sstream>
#include <boost/format.hpp>

using namespace boost;

namespace gr {
  namespace dab {

    demux_cc::sptr
    demux_cc::make(unsigned int symbol_length, unsigned int symbols_fic, unsigned int symbol_msc, gr_complex fillval)
    {
      return gnuradio::get_initial_sptr
        (new demux_cc_impl(symbol_length, symbols_fic, symbol_msc, fillval));
    }

    /*
     * The private constructor
     */
    demux_cc_impl::demux_cc_impl(unsigned int symbol_length, unsigned int symbols_fic, unsigned int symbol_msc, gr_complex fillval)
      : gr::block("demux_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*symbol_length),
              gr::io_signature::make(2, 2, sizeof(gr_complex)*symbol_length)),
        d_symbol_lenght(symbol_length),
        d_symbols_fic(symbols_fic),
        d_symbols_msc(symbol_msc),
        d_fillval(fillval)
    {
      set_tag_propagation_policy(TPP_DONT);
      d_symbol_count = 0;
      d_fic_counter = 0;
      d_msc_counter = 0;
      d_on_fic = true;
    }

    /*
     * Our virtual destructor.
     */
    demux_cc_impl::~demux_cc_impl()
    {
    }

    void
    demux_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    int
    demux_cc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *fic_out = (gr_complex *) output_items[0];
      gr_complex *msc_out = (gr_complex *) output_items[1];
      unsigned int nconsumed = 0;
      unsigned int fic_syms_written = 0;
      unsigned int msc_syms_written = 0;

      // get tags for the beginning of a frame
      std::vector<gr::tag_t> tags;
      const std::string s = "Start";
      pmt::pmt_t d_key = pmt::string_to_symbol(s);
      unsigned int tag_count = 0;
      get_tags_in_window(tags, 0, 0, noutput_items, d_key);

      /*fprintf(stderr, "Work call ####################################\n");
      fprintf(stderr, "nitems_read %d\n", nitems_read(0));
      fprintf(stderr, "noutput_items %d\n", noutput_items);
      fprintf(stderr, "Tags: %d\n", tags.size());
      for(int i = 0; i < tags.size(); i++){
        fprintf(stderr, "Tag offset %d\n", tags[i].offset);
      }*/
      for (int i = 0; i < noutput_items; ++i) {
        if(tag_count < tags.size() && tags[tag_count].offset-nitems_read(0) - nconsumed == 0) {
          //fprintf(stderr, "Tag detected\n");
          // this input symbol is tagged: a new frame begins here
          if(d_fic_counter%d_symbols_fic == 0 && d_msc_counter%d_symbols_msc == 0){
            //fprintf(stderr, "Tag is at beginning of frame\n");
            // we are at the beginning of a frame and also finished writing the last frame
            // we can remove this first symbol of the frame (phase reference symbol) and copy the other symbols
            tag_count++;
            nconsumed++;
            d_fic_counter = 0;
            d_msc_counter = 0;
          }
          else{
            // we did not finish the last frame, maybe we lost track in sync
            // lets fill the remaining symbols with fillval before continuing with the new input frame
            if(d_fic_counter%d_symbols_fic != 0){
              memset(&fic_out[fic_syms_written++*d_symbol_lenght], 0, d_symbol_lenght * sizeof(gr_complex));
              d_fic_counter++;
            }
            else{
              memset(&msc_out[msc_syms_written++*d_symbol_lenght], 0, d_symbol_lenght * sizeof(gr_complex));
              d_msc_counter++;
            }
          }
        }
        else if (d_fic_counter < d_symbols_fic){
          // copy this symbol to fic output
          memcpy(&fic_out[fic_syms_written++*d_symbol_lenght], &in[nconsumed++*d_symbol_lenght], d_symbol_lenght * sizeof(gr_complex));
          d_fic_counter++;
        }
        else if (d_msc_counter < d_symbols_msc){
          // copy this output to msc output
          memcpy(&msc_out[msc_syms_written++*d_symbol_lenght], &in[nconsumed++*d_symbol_lenght], d_symbol_lenght * sizeof(gr_complex));
          d_msc_counter++;
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (nconsumed);

      // Tell runtime system how many output items we produced on each output stream separately.
      produce(0, fic_syms_written);
      produce(1, msc_syms_written);
      /*fprintf(stderr, "fic_syms_written %d\n", fic_syms_written);
      fprintf(stderr, "msc_syms_written %d\n", msc_syms_written);
      fprintf(stderr, "nconsumed %d\n", nconsumed);*/
      return WORK_CALLED_PRODUCE;
    }

  } /* namespace dab */
} /* namespace gr */

