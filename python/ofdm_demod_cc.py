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

        # coarse frequency correction (sub-carrier assignment)
        self.coarse_freq_corr = dab.ofdm_coarse_frequency_correction_vcvc_make(self.dp.fft_length,
                                                                               self.dp.num_carriers,
                                                                               self.dp.cp_length)
        self.v2s_fft = blocks.vector_to_stream_make(gr.sizeof_gr_complex, self.dp.num_carriers)

        # differential phasor
        self.differential_phasor = dab.diff_phasor_vcc_make(1536)

        # phase correction with phase reference symbol
        self.phase_correction = dab.phase_correction_cc_make(self.dp.num_carriers)

        # frequency deinterleaving
        self.frequency_deinterleaver = dab.frequency_interleaver_vcc_make(self.dp.frequency_deinterleaving_sequence_array)

        # demux into FIC and MSC
        self.demux = dab.demux_cc_make(self.dp.num_carriers,
                                       3, 72, 0+0j)

        self.connect(
            self,
            self.sync,
            self.s2v_fft,
            self.fft,
            self.coarse_freq_corr,
            #self.v2s_fft,
            self.differential_phasor,
            self.frequency_deinterleaver,
            self.demux,
            (self, 0)
        )
        self.connect((self.demux, 1), (self, 1))
