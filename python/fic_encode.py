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

from gnuradio import gr, blocks
from gnuradio import fec
from . import dab_swig as dab

class fic_encode(gr.hier_block2):
    """
    @brief block to encode the FIBs produced by FIB_source

    -get unpacked bytes from FIB_source
    -crc16
    -energy dispersal
    -convolutional encoding
    -puncturing
    -output packed bytes
    """
    def __init__(self, dab_params):
        gr.hier_block2.__init__(self, "fic_encode",
                                gr.io_signature(1, 1, gr.sizeof_char),
                                # Input signature
                                gr.io_signature(1, 1, gr.sizeof_char))
                                # Output signature
        self.dp = dab_params

        # crc
        self.unpacked_to_packed_crc = blocks.unpacked_to_packed_bb_make(1, gr.GR_MSB_FIRST)
        self.s2v_crc = blocks.stream_to_vector(gr.sizeof_char, 32)
        self.crc16 = dab.crc16_bb(32, 0x1021, 0xffff)
        self.v2s_crc = blocks.vector_to_stream(gr.sizeof_char, 32)
        self.packed_to_unpacked_crc = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)

        # energy dispersal
        self.prbs_src = blocks.vector_source_b(self.dp.prbs(self.dp.energy_dispersal_fic_vector_length), True)
        self.add_mod_2 = blocks.xor_bb()

        # convolutional encoder
        self.conv_pack = blocks.unpacked_to_packed_bb_make(1, gr.GR_MSB_FIRST)
        self.conv_encoder = dab.conv_encoder_bb_make(self.dp.energy_dispersal_fic_vector_length/8)
        self.conv_unpack = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)

        # puncturing
        self.puncture = dab.puncture_bb_make(self.dp.assembled_fic_puncturing_sequence)

        # pack bits
        self.unpacked_to_packed_encoded = blocks.unpacked_to_packed_bb_make(1, gr.GR_MSB_FIRST)


        # connect everything
        self.connect((self, 0),
                     self.unpacked_to_packed_crc,
                     self.s2v_crc,
                     self.crc16,
                     self.v2s_crc,
                     self.packed_to_unpacked_crc,
                     (self.add_mod_2, 0),
                     self.conv_pack,
                     self.conv_encoder,
                     self.conv_unpack,
                     self.puncture,
                     self.unpacked_to_packed_encoded,
                     self)

        #connect prbs
        self.connect(self.prbs_src, (self.add_mod_2, 1))
