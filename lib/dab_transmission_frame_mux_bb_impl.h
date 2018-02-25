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

#ifndef INCLUDED_DAB_DAB_TRANSMISSION_FRAME_MUX_BB_IMPL_H
#define INCLUDED_DAB_DAB_TRANSMISSION_FRAME_MUX_BB_IMPL_H

#include <dab/dab_transmission_frame_mux_bb.h>

namespace gr {
  namespace dab {
/*! \brief multiplex to DAB transmission frames
 *
 * block multiplexes the FIBs of the FIC and all subchannels of the MCI to a
 * transmission frame according to ETSI EN 300 401
 * the number of FIBs per CIF and the number of CIFs per transmission frame
 * depends on the transmission mode
 *
 * @param transmission_mode Transmission mode 1-4 after DAB standard.
 * @param subch_size Vector with size of each sub-channel.
 */
    class dab_transmission_frame_mux_bb_impl
            : public dab_transmission_frame_mux_bb {
    private:
      int d_transmission_mode;
      /*!< Transmission_mode transmission mode 1-4 after DAB standard. */
      int d_num_subch;
      /*!< Number of sub-channels in the transmission frame and ensemble. */
      std::vector<unsigned int> d_subch_size;
      /*!< Vector containing the sizes in CUs of each subchannel. */
      const static unsigned int d_fib_len = 32 * 3;
      /*!< Length of a fib in bytes. (*3 because bit rate = 1/3) */
      const static unsigned int d_cif_len = 6912;
      /*!< Length of a Common Interleaved Frame (CIF) in bytes. */
      const static unsigned int d_cu_len = 8;
      /*!< Length of a capacity unit in bytes. */
      unsigned int d_vlen_out, d_num_cifs, d_num_fibs, d_subch_total_size;
      unsigned int d_fic_len;

      unsigned char d_prbs[d_cif_len];
      /*!< Vector for the PRBS, used for padding at the end of each frame. */

      void generate_prbs(unsigned char *out_ptr, int length);
      /*!< Generates a PRBS after the rules of ETSI EN 300 401 and writes it to output buffer out_ptr.
       * @param length Required length of the PRBS and number of bytes that are written to out_ptr.
       * @param out_ptr Pointer to the first element of the buffer to write the PRBS.
       */

    public:
      dab_transmission_frame_mux_bb_impl(int transmission_mode, int num_subch,
                                         const std::vector<unsigned int> &subch_size);

      ~dab_transmission_frame_mux_bb_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_DAB_TRANSMISSION_FRAME_MUX_BB_IMPL_H */

