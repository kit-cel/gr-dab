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

#ifndef INCLUDED_DAB_SELECT_CUS_VFVF_IMPL_H
#define INCLUDED_DAB_SELECT_CUS_VFVF_IMPL_H

#include <dab/select_cus_vfvf.h>

namespace gr {
  namespace dab {
/*! \brief Selects items out of a stream, defined by start address and size.
 * This block is used to select the data of one MSC sub-channel
 * out of a transmission frame.
 *
 * @param vlen Vector size of input and output vectors,
 * defining the item size on witch the address and size variables base on.
 * @param frame_length Length in items of a frame.
 * (each item is a vector with size vlen)
 * @param address Number of the first item in each frame to be copied.
 * @param size Number of items to copy in each frame.
 */
    class select_cus_vfvf_impl : public select_cus_vfvf {
    private:
      unsigned int d_vlen;
      unsigned int d_frame_len;
      unsigned int d_address;
      unsigned int d_size;

    public:
      select_cus_vfvf_impl(unsigned int vlen, unsigned int frame_len,
                           unsigned int address, unsigned int size);

      ~select_cus_vfvf_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_SELECT_CUS_VFVF_IMPL_H */

