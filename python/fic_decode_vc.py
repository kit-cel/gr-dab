#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

from gnuradio import gr, trellis, blocks
import dab
from math import sqrt

class fic_decode_vc(gr.hier_block2):
    """
    @brief block to decode FIBs (fast information blocks) from the FIC (fast information channel) of a demodulated DAB signal

    - get demodulated FIC OFDM symbols from transmission frame
    - do convolutional decoding
    - undo energy dispersal
    - get FIC information
    """
    def __init__(self, dab_params):
        gr.hier_block2.__init__(self,
            "fic_decode_vc",
            gr.io_signature(1, 1, gr.sizeof_gr_complex * dab_params.num_carriers),  # Input signature
            gr.io_signature(0, 0, gr.sizeof_char)) # Output signature

        self.dp = dab_params

        # complex to intereaved float for convolutional decoding
        self.c2f = dab.complex_to_interleaved_float_vcf_make(self.dp.num_carriers)

        # FIB block partitioning
        self.v2s = blocks.vector_to_stream_make(gr.sizeof_float, self.dp.num_carriers * 2)
        self.s2v = blocks.stream_to_vector_make(gr.sizeof_float, self.dp.fic_punctured_codeword_length)

        # unpuncturing
        self.unpuncture = dab.unpuncture_vff(self.dp.assembled_fic_puncturing_sequence, 0)

        # convolutional coding
        # self.fsm = trellis.fsm(self.dp.conv_code_in_bits, self.dp.conv_code_out_bits, self.dp.conv_code_generator_polynomials)
        self.fsm = trellis.fsm(1, 4, [0133, 0171, 0145, 0133])  # OK (dumped to text and verified partially)
        self.conv_v2s = blocks.vector_to_stream(gr.sizeof_float, self.dp.fic_conv_codeword_length)
        # self.conv_decode = trellis.viterbi_combined_fb(self.fsm, 20, 0, 0, 1, [1./sqrt(2),-1/sqrt(2)] , trellis.TRELLIS_EUCLIDEAN)
        table = [
            0, 0, 0, 0,
            0, 0, 0, 1,
            0, 0, 1, 0,
            0, 0, 1, 1,
            0, 1, 0, 0,
            0, 1, 0, 1,
            0, 1, 1, 0,
            0, 1, 1, 1,
            1, 0, 0, 0,
            1, 0, 0, 1,
            1, 0, 1, 0,
            1, 0, 1, 1,
            1, 1, 0, 0,
            1, 1, 0, 1,
            1, 1, 1, 0,
            1, 1, 1, 1
        ]
        assert (len(table) / 4 == self.fsm.O())
        table = [(1 - 2 * x) / sqrt(2) for x in table]
        self.conv_decode = trellis.viterbi_combined_fb(self.fsm, 774, 0, 0, 4, table, trellis.TRELLIS_EUCLIDEAN)
        # self.conv_s2v = blocks.stream_to_vector(gr.sizeof_char, 774)
        self.conv_prune = dab.prune(gr.sizeof_char, self.dp.fic_conv_codeword_length / 4, 0,
                                    self.dp.conv_code_add_bits_input)

        # energy dispersal
        self.prbs_src = blocks.vector_source_b(self.dp.prbs(self.dp.energy_dispersal_fic_vector_length), True)
        # self.energy_v2s = blocks.vector_to_stream(gr.sizeof_char, self.dp.energy_dispersal_fic_vector_length)
        self.add_mod_2 = blocks.xor_bb()
        self.energy_s2v = blocks.stream_to_vector(gr.sizeof_char, self.dp.energy_dispersal_fic_vector_length)
        self.cut_into_fibs = dab.repartition_vectors(gr.sizeof_char, self.dp.energy_dispersal_fic_vector_length,
                                                     self.dp.fib_bits, 1, self.dp.energy_dispersal_fic_fibs_per_vector)

        # connect all
        self.nullsink = blocks.null_sink(gr.sizeof_char)
        self.pack = blocks.unpacked_to_packed_bb(1, gr.GR_MSB_FIRST)
        self.fibout = blocks.stream_to_vector(1, 32)
        # self.filesink = gr.file_sink(gr.sizeof_char, "debug/fic.dat")
        self.fibsink = dab.fib_sink_vb()

        # self.connect((self,0), (self.select_fic_syms,0), (self.repartition_fic,0), self.unpuncture, self.conv_v2s, self.conv_decode, self.conv_s2v, self.conv_prune, self.energy_v2s, self.add_mod_2, self.energy_s2v, (self.cut_into_fibs,0), gr.vector_to_stream(1,256), gr.unpacked_to_packed_bb(1,gr.GR_MSB_FIRST), self.filesink)
        self.connect((self, 0),
                     self.c2f,
                     self.v2s,
                     self.s2v,
                     self.unpuncture,
                     self.conv_v2s,
                     self.conv_decode,
                     # self.conv_s2v,
                     self.conv_prune,
                     # self.energy_v2s,
                     self.add_mod_2,
                     #self.energy_s2v,
                     #(self.cut_into_fibs, 0),
                     #blocks.vector_to_stream(1, 256),
                     self.pack,
                     self.fibout,
                     self.fibsink)
        self.connect(self.prbs_src, (self.add_mod_2, 1))
