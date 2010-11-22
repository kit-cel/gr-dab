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

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dab_sum_elements_vff.h>
#include <gr_io_signature.h>

/*
 * Create a new instance of dab_sum_elements_vff and return
 * a boost shared_ptr.  This is effectively the public constructor.
 */
dab_sum_elements_vff_sptr 
dab_make_sum_elements_vff (unsigned int length)
{
  return gnuradio::get_initial_sptr (new dab_sum_elements_vff (length));
}

dab_sum_elements_vff::dab_sum_elements_vff (unsigned int length) : 
  gr_sync_block ("sum_elements_vff",
             gr_make_io_signature (1, 1, sizeof(float)*length),
             gr_make_io_signature (1, 1, sizeof(float))),
  d_length(length)
{
}


int 
dab_sum_elements_vff::work (int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items)
{
  const float *in = (const float *) input_items[0];
  float *out = (float *) output_items[0];

  double sum;

  for (int i = 0; i < noutput_items; i++){
    sum = 0;
    for (unsigned int j = 0; j < d_length; j++)
      sum += *in++;
    *out++ = (float)sum;
  }
    
  return noutput_items;
}
