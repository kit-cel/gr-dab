/* -*- c++ -*- */
/*
 * Copyright 2017, 2018 Moritz Luca Schmid, Communications Engineering Lab (CEL)
 * Karlsruhe Institute of Technology (KIT).
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

#ifndef INCLUDED_DAB_OFDM_SYNCHRONIZATION_CVF_IMPL_H
#define INCLUDED_DAB_OFDM_SYNCHRONIZATION_CVF_IMPL_H

#include <dab/ofdm_synchronization_cvf.h>

namespace gr {
  namespace dab {
/*! \brief Sets tag at the beginning of each OFDM frame.
 * Lets only pass the one OFDM frames (without the NULL symbol), which
 * were detected completely.
 *
 * \param symbol_length Length of each OFDM symbol without guard intervall.
 * \param cyclic_prefix_length Length of the cyclic prefix. (= length of the guard intervall)
 * \param fft_length Length of the FFT vector.
 * \param symbols_per_frame Number of OFDM symbols without the NULL symbol.
 *
 */
    class ofdm_synchronization_cvf_impl : public ofdm_synchronization_cvf {
    private:
      int d_symbol_length; /*!< Length of each OFDM symbol without guard intervall. */
      int d_cyclic_prefix_length; /*!< Length of the cyclic prefix. (= length of the guard intervall) */
      int d_fft_length; /*!< Length of the FFT vector.*/
      int d_moving_average_counter;
      /*!< Counts the number of steps the moving average did, to reset it
       * from time to time to avoid value drifting caused by float rounding.*/
      gr_complex d_correlation;
      /*!< Fixed lag correlation (not normalized) with the lag length
       * equal to the cyclic prefix length.
       */
      gr_complex d_correlation_normalized;
      /*!< Normalized version of d_correlation. The normalization value
       * is the mean energy of the correlation sequence, averaged between the
       * two parts of the correlation.
       */
      float *d_mag_squared;
      /*!< Allocated buffer for volk function.
       * We write the calculated magnitued squared samples to this buffer to
       * accumulate them in the next step.
       */
      gr_complex *d_fixed_lag_corr;
      /*!< Allocated buffer for volk function.
       * We write the calculated correlation samples to this buffer\
       * accumulate them in the next step.
       */
      float d_correlation_normalized_magnitude;
      /*!< Magnitude of the current fixed correlation value.*/
      float d_correlation_normalized_phase;
      /*!< Phase of the current fixed correlation value. */
      float d_energy_prefix;
      /*!< Energy of the upcoming cyclic_prefix_length samples. This value
       * is used for normalization and NULL symbol detection.
       */
      float d_energy_repetition;
      /*!< Energy of cyclic_prefix_length samplex, starting symbol_length
       * samples from the current sample. This value is used for normalization
       * and NULL symbol detection. When we are at the beginning of a OFDM symbol,
       * this is equal to the energy of the cyclic prefix.
       */
      float d_frequency_offset_per_sample;
      /*!< Frequency offset, described as a phase shift per sample. (in rad/sample)*/
      bool d_NULL_detected;
      /*!< Signalizes if we recently detected a NULL symbol and
       * therefore expect the first symbol of the next frame now.
       */
      int d_symbols_per_frame;
      /*!< Number of OFDM symbols without the NULL symbol. */
      int d_symbol_count; /*!< Counts the number of detected symbols.*/
      int d_symbol_element_count;
      /*!< Counts the number of samples in each symbol. */
      bool d_wait_for_NULL;
      /*!< Signalizes if we detected the last symbol of an OFDM frame and
       * we are now expecting the NULL symbol. */
      bool d_on_triangle;
      /*!< Signalizes if we are over the correlation threshold.
       * This is used to do a kind of early late synchronization to find
       * the correalation peak.
       */
      float d_phase;
      bool d_peak_set;
      /*!< Signalizes if we already set the peak on the current
       * correlation triangle.
      */
      float d_correlation_maximum;
      /*!< Stores the max correlation value we detected so far on the current
       * correlation triangle.
       */
      int d_nwritten;
      /*!< Stores the number of items, we already wrote to the output buffer.*/

      /*! \brief Calculates a fixed lag correlation over the given sample sequence.
       *
       * @param sample Pointer to the first sample of the sequence.
       * @param new_calculation If true, calculated the correlation from scratch, else do a moving average.
       */
      void delayed_correlation(const gr_complex *sample, bool new_calculation);

      /*! \brief Checks if we reached the maximum of a correlation triangle.
       * Peak detection with a very simple, a-causal method.
       * @return True, if we found the peak and therefore are at the start of a symbol.
       */
      bool detect_start_of_symbol();

    public:
      ofdm_synchronization_cvf_impl(int symbol_length, int cyclic_prefix_length,
                                    int fft_length, int symbols_per_frame);

      ~ofdm_synchronization_cvf_impl();

      void forecast(int noutput_items, gr_vector_int &ninput_items_required);

      // Where all the action really happens
      int general_work(int noutput_items, gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
    };

  } // namespace dab
} // namespace gr

#endif /* INCLUDED_DAB_OFDM_SYNCHRONIZATION_CVF_IMPL_H */

