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

#ifndef INCLUDED_DAB_FIB_SOURCE_B_IMPL_H
#define INCLUDED_DAB_FIB_SOURCE_B_IMPL_H

#include <dab/fib_source_b.h>

namespace gr {
  namespace dab {
/*! \brief source that produces Fast Information Blocks (FIBs) according to the DAB standard
 *
 * output: unpacked byte stream with FIBs (each 256 bit) and zeros at last 16 bits for following CRC16
 *
 * produces Fast Information Blocks (FIBs) according to the DAB standard and the input parameters
 *
 * @param transmission_mode transmission mode
 * @param num_subch number of subchannels to be transmitted, each in a speparated service
 * @param ensemble_label string label of the DAB ensemble (max 16 characters)
 * @param programme_service_label string label of the DAB service (max 16 characters)
 * @param service_comp_label string label of the DAB service component (max 16 characters)
 * @param service_comp_lang language of the service component in hex according to table 9, 10 in ETSI TS 101 756
 * @param protection_mode protection profile of set A according to table 7
 * @param data_rate_n n = data_rate/8kbit/s
 */
    class fib_source_b_impl : public fib_source_b {
    private:
      int d_transmission_mode; //transmission mode
      std::vector <uint8_t> d_dabplus;
      int d_offset;
      uint16_t d_nFIBs_written; //counts totally written FIBs
      void bit_adaption(char *out_ptr, int number,
                        int num_bits); //writes an integer value to num_bits bits, beginning at (overwrites default zeros)

      //Multiplex Channel Info
      int d_num_subch; //number of subchannels
      const static char d_ensemble_info[56]; //CIF counter changes every FIB
      const static int d_size_ensemble_info = 56;

      void CIF_counter(char *out_ptr, int counter);//implementation of the mod 20 and mod 250 counter
      const static char d_service_orga_header[16]; //*const
      const static int d_size_service_orga_header = 16;
      const static char d_service_orga[40]; //*services
      const static int d_size_service_orga = 40;
      const static char d_subchannel_orga_header[16]; //const
      const static int d_size_subchannel_orga_header = 16;
      const static char d_subchannel_orga_field[32]; //*numSubCh
      const static int d_size_subchannel_orga_field = 32;
      int d_start_adress, d_subch_size;
      std::vector <uint8_t> d_protection_mode, d_data_rate_n;

      //Service Information
      int d_label_counter;
      const static int d_size_label = 176;
      static char d_ensemble_label[176]; //21*8+8, ensemble label (FIG 1/0)
      const static int d_size_ensemble_label = 176;
      static char d_programme_service_label[176]; //21*8+8, service label (FIG 1/0)
      const static int d_size_service_label = 176;
      std::string d_service_labels;

      int write_label(char *out_ptr, std::string label, int num_chars = 16);//default for 16 characters (16 byte)

    public:
      fib_source_b_impl(int transmission_mode, int num_subch, std::string ensemble_label,
                        std::string programme_service_labels, std::string service_comp_label, uint8_t service_comp_lang,
                        const std::vector <uint8_t> &protection_mode, const std::vector <uint8_t> &data_rate_n, const std::vector <uint8_t> &dabplus);

      ~fib_source_b_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_FIB_SOURCE_B_IMPL_H */
