/* -*- c++ -*- */
/*
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "mp4_decode_bs_impl.h"
#include <stdexcept>
#include <stdio.h>
#include <sstream>
#include <string>
#include <boost/format.hpp>
#include "neaacdec.h"

using namespace boost;

namespace gr {
  namespace dab {

    // lookup table for length of X-PAD data subfield
    const uint8_t mp4_decode_bs_impl::d_length_xpad_data_subfield_table[8] = {4, 6, 8, 12, 16, 24, 32, 48};

    mp4_decode_bs::sptr
    mp4_decode_bs::make(int bit_rate_n)
    {
      return gnuradio::get_initial_sptr
              (new mp4_decode_bs_impl(bit_rate_n));
    }

    /*
     * The private constructor
     */
    mp4_decode_bs_impl::mp4_decode_bs_impl(int bit_rate_n)
            : gr::block("mp4_decode_bs",
                        gr::io_signature::make(1, 1, sizeof(unsigned char)),
                        gr::io_signature::make(2, 2, sizeof(int16_t))),
              d_bit_rate_n(bit_rate_n)
    {
      d_superframe_size = bit_rate_n * 110;
      d_aacInitialized = false;
      baudRate = 48000;
      set_output_multiple(960 *
                          4); //TODO: right? baudRate*0.12 for output of one superframe
      aacHandle = NeAACDecOpen();
      //memset(d_aac_frame, 0, 960);
      d_sample_rate = -1;
      d_dynamic_label_index = 0;
    }

    /*
     * Our virtual destructor.
     */
    mp4_decode_bs_impl::~mp4_decode_bs_impl()
    {
    }

    void
    mp4_decode_bs_impl::forecast(int noutput_items,
                                 gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items; //TODO: how to calculate actual rate?
    }

    // returns aac channel configuration
    int mp4_decode_bs_impl::get_aac_channel_configuration(
            int16_t m_mpeg_surround_config, uint8_t aacChannelMode)
    {
      switch (m_mpeg_surround_config) {
        case 0:     // no surround
          return aacChannelMode ? 2 : 1;
        case 1:     // 5.1
          return 6;
        case 2:     // 7.1
          return 7;
        default:
          return -1;
      }
    }

    bool mp4_decode_bs_impl::initialize(uint8_t dacRate,
                                        uint8_t sbrFlag,
                                        int16_t mpegSurround,
                                        uint8_t aacChannelMode)
    {
      long unsigned int sample_rate;
      uint8_t channels;
      /* AudioSpecificConfig structure (the only way to select 960 transform here!)
      *
      *  00010 = AudioObjectType 2 (AAC LC)
      *  xxxx  = (core) sample rate index
      *  xxxx  = (core) channel config
      *  100   = GASpecificConfig with 960 transform
      *
      * SBR: implicit signaling sufficient - libfaad2
      * automatically assumes SBR on sample rates <= 24 kHz
      * => explicit signaling works, too, but is not necessary here
      *
      * PS:  implicit signaling sufficient - libfaad2
      * therefore always uses stereo output (if PS support was enabled)
      * => explicit signaling not possible, as libfaad2 does not
      * support AudioObjectType 29 (PS)
      */

      int core_sr_index =
              dacRate ? (sbrFlag ? 6 : 3) :
              (sbrFlag ? 8 : 5);   // 24/48/16/32 kHz
      int core_ch_config = get_aac_channel_configuration(mpegSurround,
                                                         aacChannelMode);
      if (core_ch_config == -1) {
        GR_LOG_ERROR(d_logger, "Unrecognized mpeg surround config (ignored)");
        return false;
      }
      uint8_t asc[2];
      asc[0] = 0b00010 << 3 | core_sr_index >> 1;
      asc[1] = (core_sr_index & 0x01) << 7 | core_ch_config << 3 | 0b100;
      long int init_result = NeAACDecInit2(aacHandle,
                                           asc,
                                           sizeof(asc),
                                           &sample_rate,
                                           &channels);
      if (init_result != 0) {
/*      If some error initializing occured, skip the file */
        GR_LOG_ERROR(d_logger, "Error initializing decoding library");
        NeAACDecClose(aacHandle);
        return false;
      }
      return true;
    }

    void mp4_decode_bs_impl::handle_aac_frame(const uint8_t *v,
                                              int16_t frame_length,
                                              uint8_t dacRate,
                                              uint8_t sbrFlag,
                                              uint8_t mpegSurround,
                                              uint8_t aacChannelMode,
                                              int16_t *out_sample1,
                                              int16_t *out_sample2)
    {
      // copy AU to process it
      uint8_t au[2 * 960 + 10]; // sure, large enough
      memcpy(au, v, frame_length);
      memset(&au[frame_length], 0, 10);

      // if AU contents PAD, process it
      if (((au[0] >> 5) & 07) == 4) {
        int16_t count = au[1];
        uint8_t buffer[count];
        memcpy(buffer, &au[2], count);
        process_pad(buffer, count);
      }

      int tmp = MP42PCM(dacRate,
                        sbrFlag,
                        mpegSurround,
                        aacChannelMode,
                        au,
                        frame_length,
                        out_sample1,
                        out_sample2);
    }

    void mp4_decode_bs_impl::process_pad(uint8_t *pad, int16_t length)
    {
      // read F-PAD field (header of X-PAD)
      uint8_t fpad_type = (uint8_t)(pad[length - 2] & 0xc0) >> 6;
      uint8_t xpad_indicator = (uint8_t)(pad[length - 2] & 0x30) >> 4;
      uint8_t byte_L_indicator = (uint8_t)(pad[length - 2] & 0x0f);
      uint8_t content_indicator_flag = (uint8_t)(pad[length - 1] & 0x02) >> 1;
      GR_LOG_DEBUG(d_logger, "PAD START ########################################################################################");
      GR_LOG_DEBUG(d_logger,
                   format("F-PAD: length %d, type %d, xpad indicator %d, byte L indicator %d, content indicator flag %d") %
                   (int) length % (int) fpad_type % (int) xpad_indicator % (int) byte_L_indicator %
                   (int) content_indicator_flag);
      for (int k = 0; k <length; ++k) {
        GR_LOG_DEBUG(d_logger, format("%s") %pad[length-1-k]);
      }
      // check if the X-PAD contains one or multiple content indicators
      if(content_indicator_flag == 0){
        // no content indicators: the X-PAD content is a continuation of a data group and the length is like in the previous data sub-field
        GR_LOG_DEBUG(d_logger, format("no CI flag set"));
      }else {
        // switch to signalled X-PAD type (short, variable or no X-PAD)
        if (xpad_indicator == 1) { // short X-PAD -> 4 bytes
          uint8_t app_type = (uint8_t)(pad[length-3] & 0x1f);
          GR_LOG_DEBUG(d_logger, format("short X-PAD: app type %d") %(int)app_type);
          // TODO implement short X-PAD handler
        } else if (xpad_indicator == 2) { // variable X-PAD size
          // check the number of content indicators (CIs) and the length of the CI list (these can differ due to the end marker!)
          uint8_t ci_list_length = 0;
          uint8_t n_ci_elements = 0;
          while(ci_list_length < 4){ // there are max 4 content indicators (CIs) in a X-PAD of var length
            // check if the CI is an end marker
            if((uint8_t)(pad[length-3-ci_list_length] & 0x1f) == 0){
              // found end marker of CI list, this CI increases the ci_list_length but does not count as a valid CI element
              ci_list_length++;
              break;
            }
            ci_list_length++;
            n_ci_elements++;
          }
          GR_LOG_DEBUG(d_logger, format("variable X-PAD with %d CI elements (length %d)") %(int)n_ci_elements %(int)ci_list_length);
          // iterate over CIs processing the associated X-PAD data sub-fields after now knowing the end of the CIs
          uint8_t pad_subfield_index = 3 + ci_list_length;
          for (int i = 0; i < n_ci_elements; ++i) {
            uint8_t data_subfield_length_indicator = (uint8_t)(pad[length-3-i] & 0xe0) >> 6;
            uint8_t data_subfield_length = d_length_xpad_data_subfield_table[data_subfield_length_indicator];
            uint8_t app_type = (uint8_t)(pad[length-3-i] & 0x1f);
            // Write the X-PAD data sub-field into a buffer and reverse the order of the bytes to the logical byte order.
            uint8_t xpad_subfield[data_subfield_length];
            GR_LOG_DEBUG(d_logger, format("%d. subfield: subfield_index %d, size %d, pad_sufield_index %d, type %d") %(int)(i+1) %(int)pad_subfield_index %(int)data_subfield_length %(int)pad_subfield_index %(int)app_type);

            for (int j = 0; j < data_subfield_length; ++j) {
              xpad_subfield[j] = pad[length-pad_subfield_index-j];
              //GR_LOG_DEBUG(d_logger, format("%s") %xpad_subfield[j]);
            }
            // process the X-PAD data sub-field according to its application type
            switch (app_type) {
              case 1: { // Data group length indicator; this indicates the start of a new data group
                uint16_t data_group_length = (uint16_t)(xpad_subfield[0]&0x3f)<<8 || xpad_subfield[1];
                GR_LOG_DEBUG(d_logger, format("Data group length indicator: length %d") %(int)data_group_length);
                if(crc16(xpad_subfield, data_subfield_length-2)){
                  GR_LOG_DEBUG(d_logger, format("CRC succeeded") );
                }
                else{
                  GR_LOG_DEBUG(d_logger, format("CRC failed"));
                }
                break;
              }
              /*case 2: { // Dynamic label segment, start of X-PAD data group
                // start of a new label, we don't delete the old label but overwrite it by resetting the dynamic_label_index
                d_dynamic_label_index = 0;
                //process dynamic label
                if(crc16(const_cast<const uint8_t*>(xpad_subfield), (int16_t)(d_length_xpad_data_subfield_table[data_subfield_length_indicator]))) {
                  GR_LOG_DEBUG(d_logger, format("CRC ok, START of xpad data group"));
                  process_dynamic_label_segment(const_cast<const uint8_t*>(xpad_subfield), d_length_xpad_data_subfield_table[data_subfield_length_indicator]);
                } else{
                  GR_LOG_DEBUG(d_logger, format("CRC failed"));
                  // the CRC failed, so we lost one dynamic label segment and have to dump the whole dynamic label
                  d_dynamic_label_index = 0;
                }
                break;
              }*/
              /*case 3: { // Dynamic label segment, continuation of X-PAD data group
                // we append the dynamic label segment to the existing segments
                // TODO what if our first xpad apptype is a continuation, should we wait for the first start?
                if(crc16(const_cast<const uint8_t*>(xpad_subfield), (int16_t)(d_length_xpad_data_subfield_table[data_subfield_length_indicator]-2))) {
                  GR_LOG_DEBUG(d_logger, format("CRC ok, CONTINUATION of xpad data group"));
                  process_dynamic_label_segment(const_cast<const uint8_t*>(xpad_subfield), d_length_xpad_data_subfield_table[data_subfield_length_indicator]);
                } else{
                  // the CRC failed, so we lost one dynamic label segment and have to dump the whole dynamic label
                  GR_LOG_DEBUG(d_logger, format("CRC dynamic label segment failed, dumping whole dynamic label"));
                }
                break;
              }*/
              default:
                GR_LOG_DEBUG(d_logger, format("unsupported application type (%d)") %
                                       (int) app_type);
                break;
            }
            // push index to the start of the next data subfield
            pad_subfield_index += d_length_xpad_data_subfield_table[data_subfield_length_indicator];
          }
        }
      }
    }

    void mp4_decode_bs_impl::process_dynamic_label_segment(const uint8_t *segment,
                                                           uint8_t length) {
      uint8_t toggle = (uint8_t)(segment[0]&0x80)>>7;
      uint8_t first_segment_flag = (uint8_t)(segment[0]&0x40)>>6;
      uint8_t last_segment_flag = (uint8_t)(segment[0]&0x20)>>5;
      uint8_t c_flag = (uint8_t)(segment[0]&0x10)>>4;
      uint8_t length_char_field = (uint8_t)(segment[0]&0x0f);

      // process dynamic label message and dynamic label command separately
      if(c_flag == 0){ // message segment (character data)
        //GR_LOG_DEBUG(d_logger, format("Message segment, first/last %d %d") %(int)first_segment_flag %(int)last_segment_flag);
        if(first_segment_flag){
          uint8_t charset = (uint8_t)(segment[1]&0xf0)>>4;
          //GR_LOG_DEBUG(d_logger, format("Charset: %d") %(int)charset );
        }else{
          uint8_t segnum = (uint8_t)(segment[1]&0x70)>>4;
          //GR_LOG_DEBUG(d_logger, format("SegNum: %d") %(int)segnum );
        }
        char label[length_char_field+1];
        label[length_char_field] = '\0';
        memcpy(label, &segment[2], length_char_field);
        for (int i = 0; i < length_char_field; ++i) {
          //GR_LOG_DEBUG(d_logger, format("Text: %s") % label[i]);
        }
        // check toggle bit for change of dynamic message
        if(toggle == d_dynamic_label_message_toggle){
          // This segment belongs to the current dynamic label message or the message is repeated.
          //memcpy(&d_dynamic_label[d_dynamic_label_index], &segment[2], length_char_field);

        }
        else{
          // A different dynamic label message is sent.
        }
      }
      else{ // command segment
        // check toggle bit for change of dynamic label command
        if(toggle == d_dynamic_label_command_toggle){
          // This segment belongs to the current dynamic label command or the command is repeated.

        }
        else{
          // A different dynamic label message is sent.
        }
      }
    }

    int16_t mp4_decode_bs_impl::MP42PCM(uint8_t dacRate,
                                        uint8_t sbrFlag,
                                        int16_t mpegSurround,
                                        uint8_t aacChannelMode,
                                        uint8_t buffer[],
                                        int16_t bufferLength,
                                        int16_t *out_sample1,
                                        int16_t *out_sample2)
    {
      int16_t samples;
      long unsigned int sample_rate;
      int16_t *outBuffer;
      NeAACDecFrameInfo hInfo;
      uint8_t dummy[10000];
      uint8_t channels;

      // initialize AAC decoder at the beginning
      if (!d_aacInitialized) {
        if (!initialize(dacRate, sbrFlag, mpegSurround, aacChannelMode))
          return 0;
        d_aacInitialized = true;
        GR_LOG_DEBUG(d_logger, "AAC initialized");
      }

      outBuffer = (int16_t *) NeAACDecDecode(aacHandle, &hInfo, buffer,
                                             bufferLength);
      sample_rate = hInfo.samplerate;

      samples = hInfo.samples;
      if ((sample_rate == 24000) ||
          (sample_rate == 32000) ||
          (sample_rate == 48000) ||
          (sample_rate != (long unsigned) baudRate)) {
        baudRate = sample_rate;
      }
      d_sample_rate = sample_rate;
      channels = hInfo.channels;
      if (hInfo.error != 0) {
        GR_LOG_ERROR(d_logger, format("Warning:  %s") %
                               faacDecGetErrorMessage(hInfo.error));
        return 0;
      }

      // write samples to output buffer
      if (channels == 2) {
        // the 2 channels are transmitted intereleaved; each channel gets samples/2 PCM samples
        for (int n = 0; n < samples / 2; n++) {
          out_sample1[n + d_nsamples_produced] = (int16_t) outBuffer[n * 2];
          out_sample2[n + d_nsamples_produced] = (int16_t) outBuffer[n * 2 + 1];
        }
      } else if (channels == 1) {
        int16_t *buffer = (int16_t *) alloca(2 * samples);
        int16_t i;
        for (int n = 0; n < samples / 2; n++) {
          // only 1 channel -> reproduce each sample to send it to a stereo output anyway
          out_sample1[n + d_nsamples_produced] = (int16_t) outBuffer[n * 2];
          out_sample2[n + d_nsamples_produced] = (int16_t) outBuffer[n * 2 + 1];
        }
      } else
        GR_LOG_ERROR(d_logger, "Cannot handle these channels -> dump samples");

      d_nsamples_produced += samples / 2;
      return samples / 2;
    }

/*! \brief CRC16 check
 * CRC16 check according to ETSI EN 300 401
 * @param msg data to check
 * @param len length of dataword without the 2 bytes crc at the end
 * @return true if CRC passed
 */
    bool mp4_decode_bs_impl::crc16(const uint8_t *msg, int16_t len)
    {
      int i, j;
      uint16_t accumulator = 0xFFFF;
      uint16_t crc;
      uint16_t genpoly = 0x1021;

      for (i = 0; i < len; i++) {
        int16_t data = msg[i] << 8;
        for (j = 8; j > 0; j--) {
          if ((data ^ accumulator) & 0x8000)
            accumulator = ((accumulator << 1) ^ genpoly) & 0xFFFF;
          else
            accumulator = (accumulator << 1) & 0xFFFF;
          data = (data << 1) & 0xFFFF;
        }
      }
      // compare calculated CRC with CRC in the AU
      crc = ~((msg[len] << 8) | msg[len + 1]) & 0xFFFF;
      return (crc ^ accumulator) == 0;
    }

    uint16_t mp4_decode_bs_impl::BinToDec(const uint8_t *data, size_t offset,
                                          size_t length)
    {
      uint32_t output =
              (*(data + offset / 8) << 16) | ((*(data + offset / 8 + 1)) << 8) |
              (*(data + offset / 8 +
                 2));      // should be big/little endian save
      output >>= 24 - length - offset % 8;
      output &= (0xFFFF >> (16 - length));
      return static_cast<uint16_t>(output);
    }

    int
    mp4_decode_bs_impl::general_work(int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      const unsigned char *in =
              (const unsigned char *) input_items[0] + d_superframe_size;
      int16_t *out1 = (int16_t *) output_items[0];
      int16_t *out2 = (int16_t *) output_items[1];
      d_nsamples_produced = 0;

      for (int n = 0; n < noutput_items / (960 * 4); n++) {
        // process superframe header
        // bits 0 .. 15 is firecode
        // bit 16 is unused
        d_dac_rate = (in[n * d_superframe_size + 2] >> 6) & 01; // bit 17
        d_sbr_flag = (in[n * d_superframe_size + 2] >> 5) & 01; // bit 18
        d_aac_channel_mode =
                (in[n * d_superframe_size + 2] >> 4) & 01; // bit 19
        d_ps_flag = (in[n * d_superframe_size + 2] >> 3) & 01; // bit 20
        d_mpeg_surround = (in[n * d_superframe_size + 2] & 07); // bits 21 .. 23
        // log header information
        GR_LOG_DEBUG(d_logger,
                     format("superframe header: dac_rate %d, sbr_flag %d, aac_mode %d, ps_flag %d, surround %d") %
                     (int) d_dac_rate % (int) d_sbr_flag %
                     (int) d_aac_channel_mode % (int) d_ps_flag %
                     (int) d_mpeg_surround);

        switch (2 * d_dac_rate + d_sbr_flag) {
          default:    // cannot happen
          case 0:
            d_num_aus = 4;
            d_au_start[0] = 8;
            d_au_start[1] = in[n * d_superframe_size + 3] * 16 +
                            (in[n * d_superframe_size + 4] >> 4);
            d_au_start[2] = (in[n * d_superframe_size + 4] & 0xf) * 256 +
                            in[n * d_superframe_size + 5];
            d_au_start[3] = in[n * d_superframe_size + 6] * 16 +
                            (in[n * d_superframe_size + 7] >> 4);
            d_au_start[4] = d_superframe_size;
            break;

          case 1:
            d_num_aus = 2;
            d_au_start[n * d_superframe_size + 0] = 5;
            d_au_start[1] = in[n * d_superframe_size + 3] * 16 +
                            (in[n * d_superframe_size + 4] >> 4);
            d_au_start[2] = d_superframe_size;
            break;

          case 2:
            d_num_aus = 6;
            d_au_start[0] = 11;
            d_au_start[1] = in[n * d_superframe_size + 3] * 16 +
                            (in[n * d_superframe_size + 4] >> 4);
            d_au_start[2] = (in[n * d_superframe_size + 4] & 0xf) * 256 +
                            in[n * d_superframe_size + 5];
            d_au_start[3] = in[n * d_superframe_size + 6] * 16 +
                            (in[n * d_superframe_size + 7] >> 4);
            d_au_start[4] = (in[n * d_superframe_size + 7] & 0xf) * 256 + in[8];
            d_au_start[5] = in[n * d_superframe_size + 9] * 16 +
                            (in[n * d_superframe_size + 10] >> 4);
            d_au_start[6] = d_superframe_size;
            break;

          case 3:
            d_num_aus = 3;
            d_au_start[0] = 6;
            d_au_start[1] = in[n * d_superframe_size + 3] * 16 +
                            (in[n * d_superframe_size + 4] >> 4);
            d_au_start[2] = (in[n * d_superframe_size + 4] & 0xf) * 256 +
                            in[n * d_superframe_size + 5];
            d_au_start[3] = d_superframe_size;
            break;
        }

        // each of the d_num_aus AUs of each superframe (110 * d_bit_rate_n packed bytes) is now processed separately

        for (int i = 0; i < d_num_aus; i++) {
          int16_t aac_frame_length;

          // sanity check for the address
          if (d_au_start[i + 1] < d_au_start[i]) {
            throw std::runtime_error("AU start address invalid");
            // should not happen, the header is firecode checked
          }
          aac_frame_length = d_au_start[i + 1] - d_au_start[i] - 2;

          // sanity check for the aac_frame_length
          if ((aac_frame_length >= 960) || (aac_frame_length < 0)) {
            throw std::out_of_range(
                    (boost::format("aac frame length not in range (%d)") %
                     aac_frame_length).str());
          }

          // CRC check of each AU (the 2 byte (16 bit) CRC word is excluded in aac_frame_length)
          if (crc16(&in[n * d_superframe_size + d_au_start[i]],
                    aac_frame_length)) {
            //GR_LOG_DEBUG(d_logger, format("CRC check of AU %d successful") % i);
            // handle proper AU
            handle_aac_frame(&in[n * d_superframe_size + d_au_start[i]],
                             aac_frame_length,
                             d_dac_rate,
                             d_sbr_flag,
                             d_mpeg_surround,
                             d_aac_channel_mode,
                             out1,
                             out2);
          } else {
            // dump corrupted AU
            GR_LOG_DEBUG(d_logger, format("CRC failure with dab+ frame"));
          }
        }
      }

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each(noutput_items * d_superframe_size / (960 * 4));

      // Tell runtime system how many output items we produced.
      return d_nsamples_produced;
    }

  } /* namespace dab */
} /* namespace gr */
