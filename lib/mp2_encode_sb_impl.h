/* -*- c++ -*- */
/* 
 * Copyright 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 *
 * Code from the following third party modules is used:
 * - ODR-AudioEnc, Copyright (C) 2011 Martin Storsjo, (C) 2017 Matthias P. Braendli; Licensed under the Apache License, Version 2.0 (the "License")
 * - libtoolame-dab taken from ODR-AudioEnc, derived from TooLAME, licensed under LGPL v2.1 or later. See libtoolame-dab/LGPL.txt. This is built into a shared library.
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

#ifndef INCLUDED_DAB_MP2_ENCODE_SB_IMPL_H
#define INCLUDED_DAB_MP2_ENCODE_SB_IMPL_H

#include <dab/mp2_encode_sb.h>

extern "C" {
#include <libtoolame-dab/toolame.h>
}

namespace gr {
  namespace dab {
/*! \brief block to encode a 16bit PCM stream to MPEG2 for DAB
 *
 * @param bit_rate_n data rate in multiples of 8kbit/s
 * @param channels number of input audio channels
 * @param sample_rate sample rate of the PCM audio stream
 *
 */
    class mp2_encode_sb_impl : public mp2_encode_sb {
    private:
      int d_bit_rate_n, d_channels, d_samp_rate;
      int d_output_size, d_input_size;
      int d_nproduced, d_nconsumed;

      bool init_encoder();

    public:
      mp2_encode_sb_impl(int bit_rate_n, int channels, int sample_rate);

      ~mp2_encode_sb_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_MP2_ENCODE_SB_IMPL_H */

