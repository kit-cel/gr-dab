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

#ifndef INCLUDED_DAB_SYNCHRONIZATION_FF_IMPL_H
#define INCLUDED_DAB_SYNCHRONIZATION_FF_IMPL_H

#include <dab/synchronization_ff.h>

namespace gr {
  namespace dab {
/*! \brief sets tag at the beginning of each OFDM frame
 *
 * \param symbol_length length of each OFDM symbol without guard intervall
 * \param cyclic_prefix_length length of the cyclic prefix (= length of the guard intervall)
 * \param num_ofdm_symbols number of OFDM symbols without the NULL symbol
 *
 */
    class synchronization_ff_impl : public synchronization_ff
    {
     private:
      int d_symbol_length;
      int d_cyclic_prefix_length;
      int d_moving_average_counter;
      gr_complex d_correlation;
      gr_complex d_correlation_normalized;
      float d_correlation_normalized_magnitude;
      float d_correlation_normalized_phase;
      float d_energy_prefix;
      float d_energy_repetition;
      float d_NULL_symbol_energy;
      float d_frequency_offset;
      bool d_NULL_detected;
      int d_num_ofdm_symbols;
      int d_frame_count;
      int d_frame_length_count;
      bool d_wait_for_NULL;
      bool d_on_triangle;
      bool d_acquisition;

     public:
      synchronization_ff_impl(int symbol_length, int cyclic_prefix_length, int num_ofdm_symbols);
      ~synchronization_ff_impl();

      void delayed_correlation(const gr_complex *sample, bool new_calculation);
      bool detect_peak();
      bool detect_start_of_symbol();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_SYNCHRONIZATION_FF_IMPL_H */

