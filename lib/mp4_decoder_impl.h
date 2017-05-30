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

#ifndef INCLUDED_DAB_MP4_DECODER_IMPL_H
#define INCLUDED_DAB_MP4_DECODER_IMPL_H

#include <dab/mp4_decoder.h>
#include "dab-constants.h"
//#include "reed-solomon.h"
//#include "neaacdec.h"
#include "firecode-checker.h"

namespace gr {
  namespace dab {

    class mp4_decoder_impl : public mp4_decoder
    {
     private:
      int d_frame_size;
      int d_bit_rate_n;
      firecode_checker fc;

//      bool      process_superframe (uint8_t [], int16_t);
//      void      handle_aacFrame (uint8_t *, int16_t, uint8_t, uint8_t, uint8_t, uint8_t, bool*);
//      uint8_t   *outVector;
//      int16_t   au_start [10];

      //faad_decoder
//      int faad_decoder_get_aac_channel_configuration(int16_t, uint8_t);
//      bool faad_decoder_initialize(uint8_t, uint8_t, int16_t, uint8_t);
//      int16_t faad_decoder_MP42PCM (uint8_t dacRate, uint8_t sbrFlag, int16_t mpegSurround, uint8_t aacChannelMode, uint8_t buffer [], int16_t bufferLength);
//      bool aacInitialized;
//      NeAACDecHandle aacHandle;
//      NeAACDecConfigurationPtr aacConf;
//      NeAACDecFrameInfo hInfo;
//      int32_t baudRate;

     public:
      mp4_decoder_impl(int bit_rate_n);
      ~mp4_decoder_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_MP4_DECODER_IMPL_H */

