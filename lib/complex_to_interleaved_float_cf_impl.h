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

#ifndef INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_CF_IMPL_H
#define INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_CF_IMPL_H

#include <dab/complex_to_interleaved_float_cf.h>

namespace gr {
  namespace dab {
    /*! \brief separates the real and imaginary parts of a symbol
     * Separates the real and imaginary parts of a symbol by first writing all real parts as floats of the symbol
     * and then consecutively the imaginary parts as floats.
     *
     * \param symbol_length Number of samples per symbol. (the output is symbol_length real parts followed by symbol_length imag parts)
     *
     */

    class complex_to_interleaved_float_cf_impl : public complex_to_interleaved_float_cf
    {
     private:
      unsigned int d_symbol_length;

     public:
      complex_to_interleaved_float_cf_impl(unsigned int symbol_length);
      ~complex_to_interleaved_float_cf_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_CF_IMPL_H */

