/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

#ifndef INCLUDED_DAB_PHASE_CORRECTION_CC_IMPL_H
#define INCLUDED_DAB_PHASE_CORRECTION_CC_IMPL_H

#include <dab/phase_correction_cc.h>

namespace gr {
  namespace dab {
    /*! \brief phase correction with phase reference symbol
     * The first symbol after each tag (=beginning of new frame) is the phase reference symbol.
     * The phase of the samples of this symbol is known. The average phase difference from the incoming samples
     * to the ideal phase is the value used for the phase correction.
     *
     * \param num_carriers number of used OFDM sub-carriers and therefore number of samples per symbol
     */

    class phase_correction_cc_impl : public phase_correction_cc
    {
     private:
      int d_num_carriers;
      int d_pilot_counter;
      float d_phase_difference;
      const static uint8_t d_lookup_n[48];
      const static uint8_t d_lookup_h[4][32];

     public:
      phase_correction_cc_impl(int num_carriers);
      ~phase_correction_cc_impl();

      float calc_phase_diff(const gr_complex, int);

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_PHASE_CORRECTION_CC_IMPL_H */

