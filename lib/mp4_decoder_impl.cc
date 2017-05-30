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
#include "mp4_decoder_impl.h"

#include <stdio.h>
#include <stdexcept>
#include <sstream>
#include <boost/format.hpp>
using namespace boost;

namespace gr {
    namespace dab {

        mp4_decoder::sptr
        mp4_decoder::make(int bit_rate_n)
        {
            return gnuradio::get_initial_sptr
                    (new mp4_decoder_impl(bit_rate_n));
        }

        /*
         * The private constructor
         */
        mp4_decoder_impl::mp4_decoder_impl(int bit_rate_n)
                : gr::block("mp4_decoder",
                            gr::io_signature::make(1, 1, sizeof(char)*24*bit_rate_n), //input = frame vector (I/8 = 24*bit_rate_n packet bits)
                            gr::io_signature::make(1, 1, sizeof(char))), //output = PCM stream
                  d_bit_rate_n(bit_rate_n)/*, my_rsDecoder(8, 0435, 0, 1, 10)*/
        {
            d_frame_size = 24 * bit_rate_n;
            set_output_multiple(5*d_frame_size); //superframe
            //outVector = new uint8_t[RSDims * 110];
            //aacErrors = 0;
            //aacFrames = 0;
            //faad_decoder
            //aacHandle = NeAACDecOpen();
            //aacInitialized  = false;
            //baudRate        = 48000;
        }

        /*
         * Our virtual destructor.
         */
        mp4_decoder_impl::~mp4_decoder_impl()
        {
        }

        void
        mp4_decoder_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
        {
            ninput_items_required[0] = (noutput_items / d_frame_size); //sync
        }



        int
        mp4_decoder_impl::general_work(int noutput_items,
                                       gr_vector_int &ninput_items,
                                       gr_vector_const_void_star &input_items,
                                       gr_vector_void_star &output_items)
        {
            const unsigned char *in = (const unsigned char *) input_items[0];
            unsigned char *out = (unsigned char *) output_items[0];
            int n = 0;
            int nsuperframes_produced = 0;
            while(n < noutput_items) { //TODO: noutput_items is stream?? while-grenze anders setzten
                //check fire code and if the firecode is OK, handle the frame
                if (fc.check(&in[n * d_frame_size]) /*&& (process_superframe(in + n * d_frame_size))*/)
                {
                    GR_LOG_DEBUG(d_logger, format("fire code OK at frame %d") %n);
                    for(int i = 0; i < 5 * d_frame_size; i++) {
                        out[nsuperframes_produced * d_frame_size * 5 + i] = in[n*d_frame_size + i];
                    }
                    n += 5;
                    nsuperframes_produced++;
                }
                else
                {
                    GR_LOG_DEBUG(d_logger, format("fire code failed at frame %d - shift to left") %n);
                    n++;
                }


            }
            // Tell runtime system how many input items we consumed on
            // each input stream.
            consume_each(n);

            // Tell runtime system how many output items we produced.
            return nsuperframes_produced * 5 *d_frame_size;
        }

    } /* namespace dab */
} /* namespace gr */

