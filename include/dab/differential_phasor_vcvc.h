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


#ifndef INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_H
#define INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_H

#include <dab/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace dab {

    /*!
     * \brief <+description of block+>
     * \ingroup dab
     *
     */
    class DAB_API differential_phasor_vcvc : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<differential_phasor_vcvc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of dab::differential_phasor_vcvc.
       *
       * To avoid accidental use of raw pointers, dab::differential_phasor_vcvc's
       * constructor is in a private implementation
       * class. dab::differential_phasor_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(unsigned int length, unsigned int symbols_per_frame);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_DIFFERENTIAL_PHASOR_VCVC_H */

