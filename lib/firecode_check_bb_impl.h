/* -*- c++ -*- */
/* 
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 * The class firecode_checker is adapted from the Qt-DAB software, Copyright Jan van Katwijk (Lazy Chair Computing J.vanKatwijk@gmail.com)
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

#ifndef INCLUDED_DAB_FIRECODE_CHECK_BB_IMPL_H
#define INCLUDED_DAB_FIRECODE_CHECK_BB_IMPL_H

#include <dab/firecode_check_bb.h>
#include "firecode-checker.h"

namespace gr {
  namespace dab {
/*! \brief checks firecode of logical frames
 * Checks firecode of each logical frame as a qa test for the msc_decoder.
 * According to ETSI TS 102 563 every fifth logical frame starts with a 16 bit firecode word.
 * @param bit_rate_n data rate in multiples of 8kbit/s
 */
    class firecode_check_bb_impl : public firecode_check_bb {
    private:
      int d_bit_rate_n; /*!< Byte rate. */
      int d_frame_size; /*!< Size in bytes of one transmission frame (depending on bit_rate_n).*/
      int d_nproduced, d_nconsumed; /*!< Control variable for buffer read/write operations. */
      bool d_firecode_passed; /*!< Boolean variable for displaying firecode fails. */
      firecode_checker fc; /*!< Instance of the class firecode_checker. */

    public:
      firecode_check_bb_impl(int bit_rate_n);

      ~firecode_check_bb_impl();

      virtual bool get_firecode_passed() { return d_firecode_passed; }

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_FIRECODE_CHECK_BB_IMPL_H */
