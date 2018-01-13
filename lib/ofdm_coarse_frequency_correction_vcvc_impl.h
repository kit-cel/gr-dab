/* -*- c++ -*- */
/*
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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

#ifndef INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_IMPL_H
#define INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_IMPL_H

#include <dab/ofdm_coarse_frequency_correction_vcvc.h>

namespace gr {
  namespace dab {

    class ofdm_coarse_frequency_correction_vcvc_impl : public ofdm_coarse_frequency_correction_vcvc
    {
     private:
      int d_fft_length;
      int d_num_carriers;
      int d_cyclic_prefix_length;
      float *d_mag_squared;
      unsigned int d_freq_offset;
      float d_snr;

     public:
      ofdm_coarse_frequency_correction_vcvc_impl(int fft_length, int num_carriers, int cyclic_prefix_length);
      ~ofdm_coarse_frequency_correction_vcvc_impl();

      void measure_energy(const gr_complex*);
      void measure_snr(const gr_complex*);

      virtual float get_snr()
      { return d_snr;}

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_IMPL_H */

