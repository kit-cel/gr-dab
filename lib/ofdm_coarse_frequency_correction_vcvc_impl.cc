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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "ofdm_coarse_frequency_correction_vcvc_impl.h"
#include <volk/volk.h>
#include <math.h>

namespace gr {
  namespace dab {

    ofdm_coarse_frequency_correction_vcvc::sptr
    ofdm_coarse_frequency_correction_vcvc::make(int fft_length,
                                                int num_carriers,
                                                int cyclic_prefix_length) {
      return gnuradio::get_initial_sptr(
              new ofdm_coarse_frequency_correction_vcvc_impl(fft_length,
                                                             num_carriers,
                                                             cyclic_prefix_length));
    }

    /*
     * The private constructor
     */
    ofdm_coarse_frequency_correction_vcvc_impl::ofdm_coarse_frequency_correction_vcvc_impl(
            int fft_length, int num_carriers, int cyclic_prefix_length)
            : gr::sync_block("ofdm_coarse_frequency_correction_vcvc",
                             gr::io_signature::make(1, 1, fft_length * sizeof(gr_complex)),
                             gr::io_signature::make(1, 1, num_carriers * sizeof(gr_complex))),
              d_fft_length(fft_length),
              d_num_carriers(num_carriers),
              d_cyclic_prefix_length(cyclic_prefix_length),
              d_freq_offset(0),
              d_snr(0) {
      unsigned int alignment = volk_get_alignment();
      d_mag_squared = (float *) volk_malloc(sizeof(float) * num_carriers + 1, alignment);
    }
    /*
     * Our virtual destructor.
     */
    ofdm_coarse_frequency_correction_vcvc_impl::~ofdm_coarse_frequency_correction_vcvc_impl() {
    }

    /*! Energy measurement over the num_carriers sub_carriers + the central carrier.
     * The energy gets a maximum when the calculation window and the occupied carriers are congruent.
     * Fine frequency synchronization in the range of one sub-carrier spacing is already done.
     */
    void
    ofdm_coarse_frequency_correction_vcvc_impl::measure_energy(const gr_complex *symbol) {
      unsigned int i, index;
      float energy = 0, max = 0;
      // first energy measurement is processed completely
      volk_32fc_magnitude_squared_32f(d_mag_squared, symbol, d_num_carriers + 1);
      volk_32f_accumulator_s32f(&energy, d_mag_squared, d_num_carriers + 1);
      // subtract the central (DC) carrier which is not occupied
      energy -= std::real(symbol[d_num_carriers]) * std::real(symbol[d_num_carriers]) +
                std::imag(symbol[d_num_carriers]) * std::imag(symbol[d_num_carriers]);
      max = energy;
      index = 0;
      /* the energy measurements with all possible carrier offsets are calculated over a moving sum,
       * searching for a maximum of energy
       */
      for (i = 1; i < d_fft_length - d_num_carriers; i++) {
        /* diff on left side */
        energy -= std::real(symbol[i - 1]) * std::real(symbol[i - 1]) +
                  std::imag(symbol[i - 1]) * std::imag(symbol[i - 1]);
        /* diff for zero carrier */
        energy += std::real(symbol[i + d_num_carriers / 2 - 1]) * std::real(symbol[i + d_num_carriers / 2 - 1]) +
                  std::imag(symbol[i + d_num_carriers / 2 - 1]) * std::imag(symbol[i + d_num_carriers / 2 - 1]);
        energy -= std::real(symbol[i + d_num_carriers / 2]) * std::real(symbol[i + d_num_carriers / 2]) +
                  std::imag(symbol[i + d_num_carriers / 2]) * std::imag(symbol[i + d_num_carriers / 2]);
        /* diff on rigth side */
        energy += std::real(symbol[i + d_num_carriers]) * std::real(symbol[i + d_num_carriers]) +
                  std::imag(symbol[i + d_num_carriers]) * std::imag(symbol[i + d_num_carriers]);
        /* new max found? */
        if (energy > max) {
          max = energy;
          index = i;
        }
      }
      d_freq_offset = index;
    }

    /*! SNR measurement by comparing the energy of occupied sub-carriers with the ones of empty sub-carriers
     * @return estimated SNR float value
     */
    void
    ofdm_coarse_frequency_correction_vcvc_impl::measure_snr(const gr_complex *symbol) {
      // measure normalized energy of occupied sub-carriers
      float energy = 0;
      volk_32fc_magnitude_squared_32f(d_mag_squared, &symbol[d_freq_offset], d_num_carriers + 1);
      volk_32f_accumulator_s32f(&energy, d_mag_squared, d_num_carriers + 1);
      // subtract the central (DC) carrier which is not assigned
      energy -= std::real(symbol[d_num_carriers + d_freq_offset]) * std::real(symbol[d_num_carriers + d_freq_offset]) +
                std::imag(symbol[d_num_carriers + d_freq_offset]) * std::imag(symbol[d_num_carriers + d_freq_offset]);

      // measure normalized energy of empty sub-carriers
      float noise_left = 0, noise_right, noise_total;
      // empty sub-carriers on left side
      volk_32fc_magnitude_squared_32f(d_mag_squared, symbol, d_freq_offset);
      volk_32f_accumulator_s32f(&noise_left, d_mag_squared, d_freq_offset);
      // empty sub-carriers on right side
      volk_32fc_magnitude_squared_32f(d_mag_squared,
                                      &symbol[d_freq_offset + d_num_carriers + 1],
                                      d_fft_length - d_num_carriers - d_freq_offset - 1);
      volk_32f_accumulator_s32f(&noise_right,
                                d_mag_squared,
                                d_fft_length - d_num_carriers - d_freq_offset - 1);
      // add noise energies from both sides to total noise
      noise_total = noise_left + noise_right;
      noise_total += std::real(symbol[d_freq_offset + d_num_carriers / 2]) * std::real(symbol[d_freq_offset + d_num_carriers / 2]) +
                     std::imag(symbol[d_freq_offset + d_num_carriers / 2]) * std::imag(symbol[d_freq_offset + d_num_carriers / 2]);

      // normalize
      energy = energy / d_num_carriers;
      noise_total = noise_total / (d_fft_length - d_num_carriers);
      // check if ratio is in the definition range of the log
      if (energy > noise_total) {
        // now we can calculate the SNR in dB
        d_snr = 10 * log10((energy - noise_total) / noise_total);
      }
    }

    int
    ofdm_coarse_frequency_correction_vcvc_impl::work(int noutput_items,
                                                     gr_vector_const_void_star &input_items,
                                                     gr_vector_void_star &output_items) {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      std::vector <gr::tag_t> tags;
      unsigned int tag_count = 0;
      // get tags for the beginning of a frame
      get_tags_in_window(tags, 0, 0, noutput_items);
      for (int i = 0; i < noutput_items; ++i) {
        /* new calculation for each new frame, measured at the pilot symbol and frequency correction
         * applied for all symbols of this frame
         */
        if (tag_count < tags.size() &&
            tags[tag_count].offset - nitems_read(0) - i == 0) {
          measure_energy(&in[i * d_fft_length]);
          measure_snr(&in[i * d_fft_length]);
          tag_count++;
        }
        // copy the first half (left of central sub-carrier) of the sub-carriers to the output
        memcpy(out, &in[i * d_fft_length + d_freq_offset], d_num_carriers / 2 * sizeof(gr_complex));
        // copy the second half (right of central sub-carrier) of the sub-carriers to the output
        memcpy(out + d_num_carriers / 2,
               &in[i * d_fft_length + d_freq_offset + d_num_carriers / 2 + 1],
               d_num_carriers / 2 * sizeof(gr_complex));
        out += d_num_carriers;
      }

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace dab */
} /* namespace gr */

