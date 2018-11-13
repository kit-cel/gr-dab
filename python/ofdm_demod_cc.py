#
# Copyright 2017 Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, blocks
from gnuradio import fft
from gnuradio import digital
from math import sqrt
import dab

class ofdm_demod_cc(gr.hier_block2):
    """
    @brief block to demodulate the DAB OFDM

    - fine time and frequency synchronization
    - FFT
    - coarse frequency correction (sub-carrier assignment)
    - differential phasor
    - phase correction with phase reference symbol
    - frequency deinterleaving
    - demux into FIC and MSC
    - output of complex qpsk symbols, separated after FIC and MSC
    """
    def __init__(self, dab_params):
        gr.hier_block2.__init__(self,
            "ofdm_demod_cc",
            gr.io_signature(1, 1, gr.sizeof_gr_complex),  # Input signature
            gr.io_signature(2, 2, gr.sizeof_gr_complex*1536))  # Output signature

        self.dp = dab_params

        # fine time and frequency synchronization
        self.sync = dab.ofdm_synchronization_cvf_make(self.dp.fft_length,
                                                      self.dp.cp_length,
                                                      self.dp.fft_length,
                                                      self.dp.symbols_per_frame)

        # FFT
        self.s2v_fft = blocks.stream_to_vector_make(gr.sizeof_gr_complex, self.dp.fft_length)
        self.fft = fft.fft_vcc(self.dp.fft_length, True, [], True)
        self.multiply = blocks.multiply_const_vcc([1.0/sqrt(2048)]*self.dp.fft_length)

        # coarse frequency correction (sub-carrier assignment)
        self.coarse_freq_corr = dab.ofdm_coarse_frequency_correction_vcvc_make(self.dp.fft_length,
                                                                               self.dp.num_carriers,
                                                                               self.dp.cp_length)

        # differential phasor
        self.differential_phasor = dab.diff_phasor_vcc_make(1536)

        # frequency deinterleaving
        self.frequency_deinterleaver = dab.frequency_interleaver_vcc_make(self.dp.frequency_deinterleaving_sequence_array)

        # demux into FIC and MSC
        self.demux = dab.demux_cc_make(self.dp.num_carriers, self.dp.num_fic_syms, self.dp.num_msc_syms)

        self.connect(
            self,
            self.sync,
            self.s2v_fft,
            self.fft,
            self.multiply,
            self.coarse_freq_corr,
            self.differential_phasor,
            self.frequency_deinterleaver,
            self.demux,
            (self, 0)
        )
        self.connect((self.demux, 1), (self, 1))

    def get_snr(self):
        return self.coarse_freq_corr.get_snr()
