/* -*- c++ -*- */
/*
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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

#ifndef INCLUDED_DAB_DEMUX_CC_IMPL_H
#define INCLUDED_DAB_DEMUX_CC_IMPL_H

#include <dab/demux_cc.h>

namespace gr {
  namespace dab {
    /*! \brief Separation of FIC and MSC symbols.
     * \param symbol_length number of samples per symbol
     * \param symbols_fic number of symbols in the fic per transmission frame
     * \param symobls_mic number of symbols in the msc per transmission frame
     * \param fillval complex value to fill in if sync has been lost during frame
     */

    class demux_cc_impl : public demux_cc {
    private:
      unsigned int d_symbol_lenght;
      /*!< Length in bytes of one OFDM symbol. */
      unsigned int d_symbols_fic;
      /*!< Number of OFDM symbols with FIC content per transmission frame. */
      unsigned int d_symbols_msc;
      /*!< Number of OFDM symbols with MSC content per transmission frame. */
      gr_complex d_fillval;
      /*!< Complex value, used to fill gaps in the transmission frame
       * (e.g. caused to a loss of sync) to avoid discontinuity in data flow. */

      unsigned int d_fic_counter;
      /*!< Counts the symbols containing fic data.
       * The number of fic symbols per transmission frame does not match
       * with the number of transmitted fibs. */
      unsigned int d_msc_counter; /*!< Counts the symbols containing msc data. */

    public:
      demux_cc_impl(unsigned int symbol_length, unsigned int symbols_fic,
                    unsigned int symbol_msc, gr_complex fillval);

      ~demux_cc_impl();

      // Where all the action really happens
      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_DEMUX_CC_IMPL_H */

