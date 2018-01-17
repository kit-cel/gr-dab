/* -*- c++ -*- */
/*
 * 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 * The content of this class is adopted from ODR-DabMod and written into a GNU Radio OutOfTree block.
 *
   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Her Majesty
   the Queen in Right of Canada (Communications Research Center Canada)
   See https://github.com/Opendigitalradio/ODR-DabMod for licensing information of ODR-DabMod.
 */
/*
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
#include "conv_encoder_bb_impl.h"

namespace gr {
  namespace dab {

    conv_encoder_bb::sptr
    conv_encoder_bb::make(int framesize) {
      return gnuradio::get_initial_sptr
              (new conv_encoder_bb_impl(framesize));
    }

    const static uint8_t PARITY[] = {
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
            0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
    };

    /*
     * The private constructor
     */
    conv_encoder_bb_impl::conv_encoder_bb_impl(int framesize)
            : gr::block("conv_encoder_bb",
                        gr::io_signature::make(1, 1, sizeof(unsigned char)),
                        gr::io_signature::make(1, 1,
                                               sizeof(unsigned char))),
              d_framesize(framesize) {
      d_outsize = (d_framesize * 4) + 3;
      set_output_multiple(framesize * 4 + 3);
    }

    /*
     * Our virtual destructor.
     */
    conv_encoder_bb_impl::~conv_encoder_bb_impl() {
    }

    void
    conv_encoder_bb_impl::forecast(int noutput_items,
                                   gr_vector_int &ninput_items_required) {
      ninput_items_required[0] =
              (noutput_items / (d_framesize * 4 + 3)) * d_framesize;
    }

    int
    conv_encoder_bb_impl::general_work(int noutput_items,
                                       gr_vector_int &ninput_items,
                                       gr_vector_const_void_star &input_items,
                                       gr_vector_void_star &output_items) {
      const unsigned char *in = (const unsigned char *) input_items[0];
      unsigned char *out = (unsigned char *) output_items[0];
      d_memory = 0;
      d_in_offset = 0;
      d_out_offset = 0;
      uint8_t data;

      for (int n = 0; n < (noutput_items / (d_framesize * 4 + 3)); n++) {
        // for each input byte
        for (int in_count = 0; in_count < d_framesize; ++in_count) {
          data = in[d_in_offset];
          // For next 4 output bytes
          for (unsigned out_count = 0; out_count < 4; ++out_count) {
            out[d_out_offset] = 0;
            // For each 4-bit output word (2 4-bit output words in 1 byte)
            for (unsigned j = 0; j < 2; ++j) {
              d_memory >>= 1;
              d_memory |= (data >> 7) << 6;
              data <<= 1;
              uint8_t poly[4] = {
                      (uint8_t)(d_memory & 0x5b),
                      (uint8_t)(d_memory & 0x79),
                      (uint8_t)(d_memory & 0x65),
                      (uint8_t)(d_memory & 0x5b)
              };
              // For each polynome
              for (unsigned k = 0; k < 4; ++k) {
                out[d_out_offset] <<= 1;
                out[d_out_offset] |= PARITY[poly[k]];
              }
            }
            ++d_out_offset;
          }
          ++d_in_offset;
        }
        if (d_in_offset % d_framesize != 0)
          GR_LOG_DEBUG(d_logger, "processing wrong input frame size");
        for (unsigned pad_count = 0; pad_count < 3; ++pad_count) {
          out[d_out_offset] = 0;
          // For each 4-bit output word
          for (unsigned j = 0; j < 2; ++j) {
            d_memory >>= 1;
            //PDEBUG("Memory: 0x%x\n", memory);
            uint8_t poly[4] = {
                    (uint8_t)(d_memory & 0x5b),
                    (uint8_t)(d_memory & 0x79),
                    (uint8_t)(d_memory & 0x65),
                    (uint8_t)(d_memory & 0x5b)
            };
            // For each poly
            for (unsigned k = 0; k < 4; ++k) {
              out[d_out_offset] <<= 1;
              out[d_out_offset] |= PARITY[poly[k]];
              //PDEBUG("Out bit: %i\n", out[no] >> 7);
            }
          }
          ++d_out_offset;
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each((noutput_items / (d_framesize * 4 + 3)) * d_framesize);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */

