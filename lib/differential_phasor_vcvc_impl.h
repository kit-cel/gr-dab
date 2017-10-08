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

#ifndef INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_IMPL_H
#define INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_IMPL_H

#include <dab/differential_phasor_vcvc.h>

namespace gr {
  namespace dab {

    class differential_phasor_vcvc_impl : public differential_phasor_vcvc
    {
     private:
      unsigned int d_length;
      unsigned int d_symbols_per_frame;

     public:
      differential_phasor_vcvc_impl(unsigned int length, unsigned int symbols_per_frame);
      ~differential_phasor_vcvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_IMPL_H */

