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

#ifndef INCLUDED_DAB_CONV_ENCODER_BB_IMPL_H
#define INCLUDED_DAB_CONV_ENCODER_BB_IMPL_H

#include <dab/conv_encoder_bb.h>

namespace gr {
  namespace dab {
/*! \brief convolutional encoding for DAB/DAB+ without puncturing
 *
 * convolutional encoder according to DAB standard ETSI EN 300 401 V1.4.1
 * code rate R=1/4, polyonoms in octal form: [133, 171, 145, 133],
 * appends 3*8 tailbits to mother codeword (flush out of state register)
 *
 * input and output are packed bytes
 *
 * @param framesize size of packed bytes in one frame; output framelenght is 4*framesize+3 in packed bytes
 *
 */
    class conv_encoder_bb_impl : public conv_encoder_bb {
    private:
      int d_framesize; /*!< size of packed bytes in one frame */
      int d_outsize; /*!< = d_framesize * 4 + 3 */
      unsigned int d_in_offset, d_out_offset; /*!< control variable for buffer read/write operations */
      uint16_t d_memory; /*!< virtual shift register for convolutional encoder */


    public:
      conv_encoder_bb_impl(int framesize);

      ~conv_encoder_bb_impl();

      // Where all the action really happens
      void
      forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };
  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_CONV_ENCODER_BB_IMPL_H */

