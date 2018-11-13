/* -*- c++ -*- */
/* 
 * Copyright 2017, 2018 by Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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

#ifndef INCLUDED_DAB_QPSK_MAPPER_VBVC_IMPL_H
#define INCLUDED_DAB_QPSK_MAPPER_VBVC_IMPL_H

#define I_SQRT2 0.707106781187

#include <dab/qpsk_mapper_vbvc.h>

namespace gr {
  namespace dab {
/*! \brief vector wise working qpsk mapper
 * QPSK mapper according to the DAB/DAB+ standard ETSI EN 300 401
 *
 * @ingroup packed byte vectors of length symbol_length/4
 * @param symbol_length size of the complex output vectors
 */
    class qpsk_mapper_vbvc_impl : public qpsk_mapper_vbvc {
    private:
      unsigned int d_symbol_length;

    public:
      qpsk_mapper_vbvc_impl(unsigned int symbol_length);

      ~qpsk_mapper_vbvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_QPSK_MAPPER_VBVC_IMPL_H */

