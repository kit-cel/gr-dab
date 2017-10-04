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
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),
        d_symbol_lenght(symbol_length),
        d_symbols_fic(symbols_fic),
        d_symbols_msc(symbol_msc),
        d_fillval(fillval)
    {
      set_output_multiple(symbol_length);
      d_symbol_count = 0;
      d_fic_syms_written = 0;
      d_msc_syms_written = 0;
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

      // get tags for the beginning of a frame
      std::vector<gr::tag_t> tags;
      unsigned int tag_count = 0;
      get_tags_in_window(tags, 0, 0, noutput_items);
      // check if there are any tags between a symbol start (that must not be the case)
      for (int j = 0; j < tags.size(); ++j) {
        if((tags[j].offset-nitems_read(0))%d_symbol_lenght != 0){
          GR_LOG_WARN(d_logger, format("Tag detected on position (%d): not a multiple of %d!") %(int)(tags[j].offset-nitems_read(0)) %(int)d_symbol_lenght);
        }
      }

      for (int i = 0; i < noutput_items/d_symbol_lenght; ++i) {
        if(tag_count < tags.size() && tags[tag_count].offset-nitems_read(0)-i*d_symbol_lenght == 0) {
          // a tag on this symbol: a new frame begins here
          tag_count++;
          // check if last written channel is full before starting new frame; when not, fill it with fillval
          if(d_on_fic && d_symbol_count < d_symbols_fic){
            GR_LOG_ERROR(d_logger, format("Tag received while in FIC, fill last %d symbols with fillval") %(int)(d_symbols_fic-d_symbol_count));
            for (unsigned int j = 0; j < (d_symbols_fic-d_symbol_count)*d_symbol_lenght; ++j) {
              fic_out[d_fic_syms_written*d_symbol_lenght + j] = d_fillval;
            }
            d_fic_syms_written += d_symbols_fic-d_symbol_count;
          }
          if(!d_on_fic && d_symbol_count < d_symbols_msc){
            GR_LOG_ERROR(d_logger, format("Tag received while in MSC, fill last %d symbols with fillval") %(int)(d_symbols_msc-d_symbol_count));
            for (unsigned int j = 0; j < (d_symbols_msc-d_symbol_count)*d_symbol_lenght; ++j) {
              msc_out[d_msc_syms_written*d_symbol_lenght + j] = d_fillval;
            }
            d_msc_syms_written += d_symbols_msc-d_symbol_count;
          }
          d_symbol_count = 0;
          // now we can start with the new frame, which always starts with fic symbols
          d_on_fic = true;
        }
        if(d_on_fic){
          // copy next symbol to fic output
          memcpy(&fic_out[d_fic_syms_written++], &in[i*d_symbol_lenght], d_symbol_lenght* sizeof(gr_complex));
          // check if all fic symbols for this frame have been sent
          if(++d_symbol_count >= d_symbols_fic){
            d_on_fic = false;
            d_symbol_count = 0;
          }
        }
        else{
          // copy next symbol to msc output
          memcpy(&msc_out[d_msc_syms_written++], &in[i*d_symbol_lenght], d_symbol_lenght* sizeof(gr_complex));
          // check if all msc symbols for this frame have been sent
          if(++d_symbol_count >= d_symbols_msc){
            d_on_fic = true;
            d_symbol_count = 0;
          }
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced on each output stream separately.
      produce(0, d_fic_syms_written*d_symbol_lenght);
      produce(1, d_msc_syms_written*d_symbol_lenght);
      return WORK_CALLED_PRODUCE;
    }

  } /* namespace dab */
} /* namespace gr */

