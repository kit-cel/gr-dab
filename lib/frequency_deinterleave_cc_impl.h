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

#ifndef INCLUDED_DAB_FREQUENCY_DEINTERLEAVE_CC_IMPL_H
#define INCLUDED_DAB_FREQUENCY_DEINTERLEAVE_CC_IMPL_H

#include <dab/frequency_deinterleave_cc.h>

namespace gr {
  namespace dab {
    /*! \brief DAB frequency deinterleaving
     * \param interleaving_sequence vector with scrambling info
     */

    class frequency_deinterleave_cc_impl : public frequency_deinterleave_cc
    {
     private:
      std::vector<short> d_interleaving_sequence;
      unsigned int d_length;

     public:
      frequency_deinterleave_cc_impl(const std::vector<short> &interleaving_sequence);
      ~frequency_deinterleave_cc_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_FREQUENCY_DEINTERLEAVE_CC_IMPL_H */

