/* -*- c++ -*- */
/*
 *
 * Copyright 2018, 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 *
 * GNU Radio block written for gr-dab including the following third party elements:
 * -QT-DAB: classes mp4Processor and faad-decoder except the reed-solomon class and the PAD processing
 *  Copyright (C) 2013
 *  Jan van Katwijk (J.vanKatwijk@gmail.com)
 *  Lazy Chair Computing
 * -KA9Q: the char-sized Reed-Solomon encoder and decoder of the libfec library
 *  (details on the license see /fec/LICENCE)
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

#ifndef INCLUDED_DAB_MP4_DECODE_BS_IMPL_H
#define INCLUDED_DAB_MP4_DECODE_BS_IMPL_H

#include <dab/mp4_decode_bs.h>
#include "neaacdec.h"

namespace gr {
  namespace dab {
/*! \brief DAB+ Audio frame decoder with PAD processing.
 * according to ETSI TS 102 563 and ETSI EN 300 401
 */
    class mp4_decode_bs_impl : public mp4_decode_bs {
    private:
      int d_nsamples_produced;
      int d_bit_rate_n;
      int d_sample_rate;
      int d_superframe_size;
      bool d_aacInitialized;
      int32_t baudRate;
      uint8_t d_dac_rate, d_sbr_flag, d_aac_channel_mode, d_ps_flag, d_mpeg_surround, d_num_aus;
      int16_t d_au_start[10];

      NeAACDecHandle aacHandle;

      const static uint8_t d_length_xpad_subfield_table[8];
      /*!< Lookup table for length of X-PAD data subfield.*/
      char d_dynamic_label[256];
      /*!< Character array with dynamic label. Size is maximum length of a dynamic label.*/
      uint8_t d_dyn_lab_index; /*!< Indexing the first unwritten byte of the dynamic label array*/
      uint8_t d_dyn_lab_seg_index;
      /*!< Signalizing how many bytes of the current segment are already written to the buffer. */
      bool d_last_dyn_lab_seg;
      struct fixed_pad {
        // first byte "L-1"
        uint8_t byte_l_ind : 4;
        uint8_t xpad_ind : 2;
        uint8_t type : 2;
        // second byte "L"
        uint8_t z : 1;
        uint8_t content_ind : 1;
        uint8_t byte_l_data : 6;
      }; /*!< Structure with bit fields of the 2 F-PAD bytes at the end of the PAD field. */
      struct content_ind {
        uint8_t app_type: 5;
        uint8_t length : 3;
      }; /*!< Structure with bit fields of the content idicator. */
      struct dynamic_label_header {
        uint8_t field3 : 4;
        uint8_t field2 : 4;

        uint8_t field1 : 4; // length if character field and command if command field
        uint8_t c : 1;
        uint8_t last : 1;
        uint8_t first : 1;
        uint8_t toggle : 1;
      }; /*!< Structure with bit fields of the X-PAD data group for a dynamic label segment. */
      uint8_t d_dyn_lab_curr_char_field_length;
      /*!< Length of the character field of the current dynamic label segment. */
      uint8_t d_dyn_lab_seg[64];
      /*!< Buffer for one dynamic label segment. Size is maximum length of a
       * segment (16 bytes char field + 2 bytes header + 2 bytes CRC) */


      bool crc16(const uint8_t *msg, int16_t len);

      uint16_t BinToDec(const uint8_t *data, size_t offset, size_t length);

      int get_aac_channel_configuration(int16_t m_mpeg_surround_config,
                                        uint8_t aacChannelMode);

      bool initialize(uint8_t dacRate,
                      uint8_t sbrFlag,
                      int16_t mpegSurround,
                      uint8_t aacChannelMode);

      void handle_aac_frame(const uint8_t *v,
                            int16_t frame_length,
                            uint8_t dacRate,
                            uint8_t sbrFlag,
                            uint8_t mpegSurround,
                            uint8_t aacChannelMode,
                            int16_t *out_sample1,
                            int16_t *out_sample2);

      void process_pad(uint8_t *pad, int16_t xpad_length);

      //! Processes a subfield of a dynamic label segment.
      /*!
       * @param subfield Pointer to the last logical byte (the last logical byte
       * corresponds to the first byte in the array order, caused
       * to inverted byte order) of the subfield of a dynamic label segment.
       * @param subfield_length Length of the subfield of a dynamic label segment in bytes.
       */
      void process_dynamic_label_segment_subfield(uint8_t *subfield, uint8_t subfield_length);

      int16_t MP42PCM(uint8_t dacRate,
                      uint8_t sbrFlag,
                      int16_t mpegSurround,
                      uint8_t aacChannelMode,
                      uint8_t buffer[],
                      int16_t bufferLength,
                      int16_t *out_sample1,
                      int16_t *out_sample2);

    public:
      mp4_decode_bs_impl(int bit_rate_n);

      ~mp4_decode_bs_impl();

      virtual int get_sample_rate() { return d_sample_rate; }

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_MP4_DECODE_BS_IMPL_H */
