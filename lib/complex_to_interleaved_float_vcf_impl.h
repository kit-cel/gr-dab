/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_VCF_IMPL_H
#define INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_VCF_IMPL_H

#include <dab/complex_to_interleaved_float_vcf.h>
/*! \brief transforms a complex vector into a real vector
 *  The first half of the output vector includes the real parts of the
 *  complex input vector, followed by the imaginary parts.
 *
 *  @param length length of the complex input vector
 */
namespace gr {
  namespace dab {

    class complex_to_interleaved_float_vcf_impl
            : public complex_to_interleaved_float_vcf {

    private:
      unsigned int d_length; /*!< length of the complex input vector */

    public:
      complex_to_interleaved_float_vcf_impl(unsigned int length);

      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };
  }
}

#endif /* INCLUDED_DAB_COMPLEX_TO_INTERLEAVED_FLOAT_VCF_H */
