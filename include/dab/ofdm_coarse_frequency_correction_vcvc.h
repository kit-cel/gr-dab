/* -*- c++ -*- */
/*
 * 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 * The content of this class is adopted from ODR-DabMod and written into a GNU Radio OutOfTree block.
 *
   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 Her Majesty
   the Queen in Right of Canada (Communications Research Center Canada)
   See https://github.com/Opendigitalradio/ODR-DabMod for licensing information of ODR-DabMod.
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


#ifndef INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_H
#define INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_H

#include <dab/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace dab {

    /*!
     * \brief coarse frequency correction in mulitple of the sub-carrier spacing
     * \ingroup dab
     *
     */
    class DAB_API ofdm_coarse_frequency_correction_vcvc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<ofdm_coarse_frequency_correction_vcvc> sptr;

      virtual float get_snr() = 0;

      /*!
       * \brief Return a shared_ptr to a new instance of dab::ofdm_coarse_frequency_correction_vcvc.
       *
       * To avoid accidental use of raw pointers, dab::ofdm_coarse_frequency_correction_vcvc's
       * constructor is in a private implementation
       * class. dab::ofdm_coarse_frequency_correction_vcvc::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fft_length, int num_carriers, int cyclic_prefix_length);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_OFDM_COARSE_FREQUENCY_CORRECTION_VCVC_H */

