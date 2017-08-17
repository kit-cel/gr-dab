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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "mp4_encode_sb_impl.h"
#include <stdexcept>
#include <stdio.h>
#include <sstream>
#include <boost/format.hpp>
#include "aacenc_lib.h"

using namespace boost;

namespace gr {
  namespace dab {

    mp4_encode_sb::sptr
    mp4_encode_sb::make(int bit_rate_n, int channels, int samp_rate, int afterburner)
    {
      return gnuradio::get_initial_sptr
              (new mp4_encode_sb_impl(bit_rate_n, channels, samp_rate, afterburner));
    }

    /*
     * The private constructor
     */
    mp4_encode_sb_impl::mp4_encode_sb_impl(int bit_rate_n, int channels, int samp_rate, int afterburner)
            : gr::block("mp4_encode_sb",
                        gr::io_signature::make(channels, channels, sizeof(int16_t)),
                        gr::io_signature::make(1, 1, sizeof(unsigned char))),
              d_bit_rate_n(bit_rate_n), d_channels(channels), d_samp_rate(samp_rate), d_afterburner(afterburner)
    {
      // check input arguments
      if (d_bit_rate_n < 1 || d_bit_rate_n > 24) {
        throw std::out_of_range((format("bit_rate_n out of range (%d)") % d_bit_rate_n).str());
      }
      if (!(d_samp_rate == 32000 || d_samp_rate == 48000)) {
        throw std::invalid_argument((format("samp_rate must be 32kHz or 48kHz, not %d") % d_samp_rate).str());
      }
      // initialize AAC encoder
      d_aot = AOT_NONE;
      if (init_aac_encoder(&d_aac_encoder, d_channels, d_samp_rate, d_afterburner, &d_aot)) {
        GR_LOG_INFO(d_logger, "AAC enc init succeeded");
      } else {
        GR_LOG_ERROR(d_logger, "AAC enc init failed");
	throw std::runtime_error("AAC enc init failed");
      }
      // check encoder status
      if (aacEncInfo(d_aac_encoder, &info) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to get the encoder info");
      }
      // set input size (number of items per channel(in this case one item is a int16_t))
      d_input_size = info.frameLength;
      GR_LOG_INFO(d_logger, format("AAC Encoding: framelen = %d") % info.frameLength);

      // set output size to the superframe size (without Reed Solomon parity check words)
      d_output_size = d_bit_rate_n * 110;
      set_output_multiple(d_output_size);
    }

    /*
     * Our virtual destructor.
     */
    mp4_encode_sb_impl::~mp4_encode_sb_impl()
    {
      aacEncClose(&d_aac_encoder);
    }

    /*! \brief initialization of the fdk-aac encoder
     *
     * @param encoder fdk-aac structure which stores the initialization parameters
     * @param channels number of audio channels (1 or 2)
     * @param sample_rate audio sample rate (for DAB+, only 32000 or 48000 is provided)
     * @param afterburner 0: disable afterburner (default), 1: enable afterburner
     * @param aot audio optimization tool: (fdk-aac spec {AAC-LC (AOT_AAC_LC): 1.5 bits per sample, HE-AAC (AOT_SBR): 0.625 bits per sample (dualrate sbr), HE-AAC (AOT_SBR): 1.125 bits per sample (downsampled sbr), HE-AAC v2 (AOT_PS): 0.5 bits per sample
     * @return true if init succeeded
     */
    bool mp4_encode_sb_impl::init_aac_encoder(
            HANDLE_AACENCODER *encoder,
            int channels,
            int sample_rate,
            int afterburner,
            int *aot)
    {
      // set number of channels
      CHANNEL_MODE mode;
      switch (channels) {
        case 1:
          mode = MODE_1;
          break;
        case 2:
          mode = MODE_2;
          break;
        default:
          GR_LOG_ERROR(d_logger, format("Unsupported channels number %d") % channels);
          return false;
      }

      // allocate encoder instance with required \ref encOpen "configuration"
      if (aacEncOpen(encoder, 0, 0) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to open encoder");
        return false;
      }

      // set Audio Optimization Tool (AOT)
      if (*aot == AOT_NONE) {
        if (channels == 2 && d_bit_rate_n <= 6) {
          *aot = AOT_DABPLUS_PS;
          GR_LOG_INFO(d_logger, "AOT set to AAC Parametric Stereo");
        } else if ((channels == 1 && d_bit_rate_n <= 8) || (channels == 2 && d_bit_rate_n <= 10)) {
          *aot = AOT_DABPLUS_SBR;
          GR_LOG_INFO(d_logger, "AOT set to AAC SBR (Spectral Band Replication)");
        } else {
          *aot = AOT_DABPLUS_AAC_LC;
          GR_LOG_INFO(d_logger, "AOT set to AAC LC (Low Complexity)");
        }
      }

      GR_LOG_INFO(d_logger, format("Using %d subchannels. channels = %d, sample_rate = %d")
                            % d_bit_rate_n
                            % channels
                            % sample_rate);

      // set AAC parameters
      if (aacEncoder_SetParam(*encoder, AACENC_AOT, *aot) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the AOT");
        return false;
      }
      // set aac samplerate
      if (aacEncoder_SetParam(*encoder, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the sample rate");
        return false;
      }
      // set aac channel mode
      if (aacEncoder_SetParam(*encoder, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the channel mode");
        return false;
      }
      // set aac channel order (default is WAVE file format channel ordering (e. g. 5.1: L, R, C, LFE, SL, SR))
      if (aacEncoder_SetParam(*encoder, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the wav channel order");
        return false;
      }
      // set aac granule length (in samples) to 960 (DRM/DAB+)
      if (aacEncoder_SetParam(*encoder, AACENC_GRANULE_LENGTH, 960) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the granule length");
        return false;
      }
      // set aac transport type
      if (aacEncoder_SetParam(*encoder, AACENC_TRANSMUX, TT_DABPLUS) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the RAW transmux");
        return false;
      }
      // set aac bit rate
      GR_LOG_INFO(d_logger, format("AAC bitrate set to: %d") % (d_bit_rate_n * 8000));
      if (aacEncoder_SetParam(*encoder, AACENC_BITRATE, d_bit_rate_n * 8000) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the bitrate");
        return false;
      }
      // set aac afterburner tool
      if (aacEncoder_SetParam(*encoder, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to set the afterburner mode");
        return false;
      }
      if (!afterburner) {
        GR_LOG_WARN(d_logger, "Afterburned disabled");
      }
      // Call aacEncEncode() with NULL parameters to "initialize" encoder instance with present parameter set
      if (aacEncEncode(*encoder, NULL, NULL, NULL, NULL) != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Unable to initialize the encoder");
        return false;
      }
      return true;
    }

    /*! \brief encodes 5 frames of size_input_buffer of each channel to an MPEG4 AAC frame (corresponds to a DAB+ superframe)
     *
     * @param input_buffer buffer with input elements (stereo channels already interleaved after AACENC_CHANNELORDER rules) with minimum size of channels*AACENC_GRANULE_LENGTH items
     * @param size_input_buffer size of input buffer in items
     * @param output_buffer output buffer where the aac frame is written
     * @param size_output_buffer size of output buffer
     * @return true if no errors occurred
     */
    bool mp4_encode_sb_impl::encode(int16_t *input_buffer, int size_input_buffer, unsigned char *output_buffer,
                                    int size_output_buffer)
    {
      AACENC_ERROR err;

      // prepare input buffer
      AACENC_BufDesc in_buf = {0};
      void *in_ptr = input_buffer;
      in_buf.numBufs = 1; // set number of input buffers
      in_buf.bufs = &in_ptr; // point to input buffer
      int in_identifier = IN_AUDIO_DATA; // define content of input buffer
      in_buf.bufferIdentifiers = &in_identifier;
      int in_size = size_input_buffer; // set input size
      in_buf.bufSizes = &in_size;
      int in_elem_size = 2; // size of each buffer element in bytes (int16_t = 2 bytes)
      in_buf.bufElSizes = &in_elem_size;
      AACENC_InArgs in_args = {0};
      in_args.numInSamples = in_size;


      // prepare output buffer
      AACENC_BufDesc out_buf = {0};
      void *out_ptr = output_buffer;
      out_buf.numBufs = 1; // set number of output buffers
      out_buf.bufs = &out_ptr; // point to input buffer
      int out_identifier = OUT_BITSTREAM_DATA; // define content of output buffer
      out_buf.bufferIdentifiers = &out_identifier;
      int out_size = d_bit_rate_n * 110;
      out_buf.bufSizes = &out_size;
      int out_elem_size = 2; // size of each buffer element in bytes (int16_t = 2 bytes)
      out_buf.bufElSizes = &out_elem_size;
      AACENC_OutArgs out_args = {0}; // output arguments of encoder

      // encode input frame
      err = aacEncEncode(d_aac_encoder, &in_buf, &out_buf, &in_args, &out_args);
      if (err != AACENC_OK) {
        GR_LOG_ERROR(d_logger, "Encoding failed");
        return false;
      }
      // if no error occurred we assume everything is fine and dump input frame
      nconsumed += out_args.numInSamples / 2; //(each half of numInSamples came from one of the 2 channels)
      GR_LOG_DEBUG(d_logger,
                   format("Encoder: consumed %d, produced %d,") % out_args.numInSamples % out_args.numOutBytes);
      // buffer check
      if (out_args.numInSamples > size_input_buffer || out_args.numOutBytes > size_output_buffer) {
        throw std::runtime_error((format("too much samples (%d) to write in ouput buffer (%d samples left)") %out_args.numOutBytes %size_output_buffer).str());
      }
      nproduced += out_args.numOutBytes;
      return true;
    }

    void
    mp4_encode_sb_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items * d_input_size / d_output_size;
      ninput_items_required[1] = noutput_items * d_input_size / d_output_size;
    }

    int
    mp4_encode_sb_impl::general_work(int noutput_items,
                                     gr_vector_int &ninput_items,
                                     gr_vector_const_void_star &input_items,
                                     gr_vector_void_star &output_items)
    {
      const int16_t *in_ch1 = (const int16_t *) input_items[0];
      unsigned char *out = (unsigned char *) output_items[0];
      int16_t input_buffer[d_input_size * 2];
      GR_LOG_DEBUG(d_logger, format("New buffer with %d samples") % noutput_items);

      nconsumed = 0;
      nproduced = 0;
      do {
        // copy frame to buffer
        if (d_channels == 1){
          memcpy(input_buffer, &in_ch1[nconsumed], d_input_size * sizeof(int16_t));
        }
        else if(d_channels == 2) { // merge channels if stereo
          const int16_t *in_ch2 = (const int16_t *) input_items[1];
          for (int i = 0; i < d_input_size; ++i) {
            input_buffer[2 * i] = in_ch1[nconsumed + i];
            input_buffer[2 * i + 1] = in_ch2[nconsumed + i];
          }
        }
        // send filled buffer to encoder
        encode(input_buffer, d_input_size * d_channels, &out[nproduced], noutput_items - nproduced); // encode input stream
      } while (nproduced < noutput_items / d_output_size);

      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each(nconsumed);

      // Tell runtime system how many output items we produced.
      return nproduced;
    }

  } /* namespace dab */
} /* namespace gr */

