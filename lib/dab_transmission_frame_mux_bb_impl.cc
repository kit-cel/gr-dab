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
#include <boost/format.hpp>
#include "dab_transmission_frame_mux_bb_impl.h"

namespace gr {
  namespace dab {

    dab_transmission_frame_mux_bb::sptr
    dab_transmission_frame_mux_bb::make(int transmission_mode, int num_subch,
                                        const std::vector<unsigned int> &subch_size) {
      return gnuradio::get_initial_sptr(
              new dab_transmission_frame_mux_bb_impl(transmission_mode,
                                                     num_subch, subch_size));
    }

    /*
     * The private constructor
     */
    dab_transmission_frame_mux_bb_impl::dab_transmission_frame_mux_bb_impl(
            int transmission_mode, int num_subch,
            const std::vector<unsigned int> &subch_size)
            : gr::block("dab_transmission_frame_mux_bb",
                        gr::io_signature::make(1 + num_subch, 1 + num_subch,
                                               sizeof(unsigned char)),
                        gr::io_signature::make(1, 1, sizeof(unsigned char))),
              d_transmission_mode(transmission_mode), d_subch_size(subch_size),
              d_num_subch(num_subch) {
      switch (transmission_mode) {
        case 1:
          d_num_fibs = 12;
          d_num_cifs = 4;
          break;
        case 2:
          d_num_fibs = 3;
          d_num_cifs = 1;
          break;
        case 3:
          d_num_fibs = 4;
          d_num_cifs = 1;
          break;
        case 4:
          d_num_fibs = 6;
          d_num_cifs = 2;
          break;
        default:
          throw std::invalid_argument((boost::format("Transmission mode %d doesn't exist")
                                       % transmission_mode).str());
      }
      if (subch_size.size() != num_subch) {
        GR_LOG_WARN(d_logger, "sizeof vector subch_size does not match with num_subch");
      }
      d_vlen_out = d_num_fibs * d_fib_len + d_num_cifs * d_cif_len;
      d_fic_len = d_num_fibs * d_fib_len;
      d_subch_total_size = 0;
      for (int i = 0; i < num_subch; ++i) {
        d_subch_total_size += subch_size[i];
      }
      if (d_subch_total_size * d_cu_len > d_cif_len) {
        throw std::out_of_range((boost::format("subchannels are %d bytes too long for CIF")
                                 % (d_subch_total_size * d_cu_len - d_cif_len)).str());
      }
      GR_LOG_DEBUG(d_logger, boost::format( "MUX init with: fic_len = %d, subch_total_size = %d, vlen_out = %d")
                             % d_fic_len % d_subch_total_size % d_vlen_out);
      set_output_multiple(d_vlen_out);

      // generate PRBS for padding
      generate_prbs(d_prbs, sizeof(d_prbs));
      GR_LOG_DEBUG(d_logger, boost::format("key num_subch: %d") % d_num_subch);
    }

    /*
     * Our virtual destructor.
     */
    dab_transmission_frame_mux_bb_impl::~dab_transmission_frame_mux_bb_impl() {
    }

    void
    dab_transmission_frame_mux_bb_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
      // the first input is always the FIC
      ninput_items_required[0] = d_fic_len * (noutput_items / d_vlen_out);
      for (int i = 0; i < d_num_subch; ++i) {
        // the amount of consumed data of each sub-channel depends on its size
        ninput_items_required[i + 1] = d_subch_size[i] * d_cu_len * d_num_cifs * (noutput_items / d_vlen_out);
      }
    }

    void
    dab_transmission_frame_mux_bb_impl::generate_prbs(unsigned char *out_ptr, int length) {
      char bits[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
      char newbit;
      unsigned char temp = 0;
      for (int i = 0; i < length * 8; ++i) {
        newbit = bits[8] ^ bits[4];
        memcpy(bits + 1, bits, 8);
        bits[0] = newbit;
        temp = (temp << 1) | (newbit & 01);
        if ((i + 1) % 8 == 0) {
          out_ptr[(i - 7) / 8] = temp;
          temp = 0;
        }
      }

    }

    int
    dab_transmission_frame_mux_bb_impl::general_work(int noutput_items,
                                                     gr_vector_int &ninput_items,
                                                     gr_vector_const_void_star &input_items,
                                                     gr_vector_void_star &output_items) {
      unsigned char *out = (unsigned char *) output_items[0];
      // write FIBs
      const unsigned char *in_fic = (const unsigned char *) input_items[0];
      for (int i = 0; i < noutput_items / d_vlen_out; ++i) {
        memcpy(out + i * d_vlen_out, in_fic, d_fic_len);
        in_fic += d_fic_len;
      }
      // write sub-channels
      unsigned int cu_index = 0;
      for (int j = 0; j < d_num_subch; ++j) {
        const unsigned char *in_msc = (const unsigned char *) input_items[j + 1];
        for (int i = 0; i < noutput_items / d_vlen_out; ++i) {
          for (int k = 0; k < d_num_cifs; ++k) {
            memcpy(out + i * d_vlen_out + d_fic_len + k * d_cif_len + cu_index * d_cu_len,
                   in_msc + (i * d_num_cifs + k) * d_subch_size[j] * d_cu_len,
                   d_subch_size[j] * d_cu_len);
          }
        }
        cu_index += d_subch_size[j];
      }
      // fill remaining cus with padding
      for (int i = 0; i < noutput_items / d_vlen_out; ++i) {
        for (int j = d_subch_total_size * d_cu_len; j < d_cif_len; ++j) {
          for (int k = 0; k < d_num_cifs; ++k) {
            out[i * d_vlen_out + d_fic_len + k * d_cif_len + j] = d_prbs[j];
          }
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume(0, noutput_items / d_vlen_out * d_fic_len);
      for (int j = 0; j < d_num_subch; ++j) {
        consume(j + 1, noutput_items / d_vlen_out * d_subch_size[j] * d_cu_len * d_num_cifs);
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */
