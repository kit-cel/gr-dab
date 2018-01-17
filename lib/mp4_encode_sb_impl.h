/* -*- c++ -*- */
/* 
 * Copyright 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 *
 * Code from the following third party modules is used:
 * - ODR-AudioEnc, Copyright (C) 2011 Martin Storsjo, (C) 2017 Matthias P. Braendli; Licensed under the Apache License, Version 2.0 (the "License")
 * - the FDK AAC Codec Library for Android is used as a shared library but not modified or used in source and binary form; © Copyright  1995 - 2012 Fraunhofer-Gesellschaft zur Förderung der angewandten Forschung e.V.
  All rights reserved.
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

#ifndef INCLUDED_DAB_MP4_ENCODE_SB_IMPL_H
#define INCLUDED_DAB_MP4_ENCODE_SB_IMPL_H

#include <dab/mp4_encode_sb.h>
#include "fdk-aac-dab/FDK_audio.h"
#include "fdk-aac-dab/aacenc_lib.h"

namespace gr {
  namespace dab {
/*! \brief block to encode a PCM stream in HE-AAC with DAB+ specific parameters (960 granule length)
 *
 * @param bit_rate_n data rate in multiples of 8kbit/s
 * @param channels number of input audio channels
 */
    class mp4_encode_sb_impl : public mp4_encode_sb {
    private:
      int d_bit_rate_n, d_channels, d_samp_rate, d_afterburner, d_aot;
      int nconsumed, nproduced;
      int d_input_size, d_output_size;
      HANDLE_AACENCODER d_aac_encoder;
      AACENC_InfoStruct info = {0};

      bool init_aac_encoder(HANDLE_AACENCODER *encoder,
                            int channels,
                            int sample_rate,
                            int afterburner,
                            int *aot);

      bool encode(int16_t *input_buffer, int size_input_buffer,
                  unsigned char *output_buffer, int size_output_buffer);

    public:
      mp4_encode_sb_impl(int bit_rate_n, int channels, int samp_rate,
                         int afterburner);

      ~mp4_encode_sb_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_MP4_ENCODE_SB_IMPL_H */

