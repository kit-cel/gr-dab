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

from gnuradio import gr, gr_unittest
from gnuradio import blocks
from gnuradio import fec
import dab_swig as dab
from parameters import dab_parameters
from fic_encode import fic_encode
from fic import fic_decode

class qa_fic_encode (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

# loopback test: fib_source - fic_encoder - mapper - demapper - fic_decoder - fib_sink
# manual check, if fibs are correctly processed through the coding and mapping blocks
    def test_001_t (self):
        self.symbol_length = 32*4
        self.dab_params = dab_parameters(1, 208.064e6, True)

        # source
        self.dp = dab_parameters(1, 208.064e6, True)
        self.fib_src = dab.fib_source_b_make(1, 1, 1, "ensemble1", "service1        ", "musicmix", 4, [2], [15], [1])

        # encoder
        self.fib_enc = fic_encode(self.dab_params)
        self.unpack = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)

        # mapper
        self.map = dab.mapper_bc_make(self.dp.num_carriers)

        # demapper
        self.s2v = blocks.stream_to_vector_make(gr.sizeof_gr_complex, self.dp.num_carriers)
        self.soft_interleaver = dab.complex_to_interleaved_float_vcf_make(self.dp.num_carriers)

        # decode
        self.fic_decoder = fic_decode(self.dab_params)

        # control stream
        self.trigger_src = blocks.vector_source_b([1] + [0]*74, True)

        self.tb.connect(self.fib_src,
                        blocks.head_make(gr.sizeof_char, 100000),
                        self.fib_enc,
                        self.unpack,
                        self.map,
                        self.s2v,
                        self.soft_interleaver,
                        self.fic_decoder
                        )
        self.tb.connect(self.trigger_src, (self.fic_decoder, 1))


        self.tb.run ()
        pass


if __name__ == '__main__':
    gr_unittest.run(qa_fic_encode, "qa_fic_encode.xml")
