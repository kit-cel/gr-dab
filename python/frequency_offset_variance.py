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

import numpy as np
from gnuradio import gr
from gnuradio import blocks
from gnuradio import filter
import dab

'''
Messung der Varianz des Frequenzoffsets bei verschiedenen SNR Szenarien
'''

class measure_freq_offset(gr.top_block):
    def __init__(self):
        gr.top_block.__init__(self)

        #data_source = blocks.vector_source_c_make(signal)
        data_source = blocks.file_source_make(gr.sizeof_gr_complex, "data/gen_iq_dab_normalized_resampled.dat")
        measure = dab.synchronization_ff_make(2048, 504, 76)
        head = blocks.head_make(gr.sizeof_float, 2)
        sink = blocks.vector_sink_f_make()
        self.connect(data_source, measure, head, sink)
        self.run()
        self.sink_data = sink.data()
        #self.data = np.asarray(self.sink_data)
        print self.sink_data

flowgraph = measure_freq_offset()
# input original iq data
#iq_gen = np.fromfile('data/gen_iq_dab_normalized_resampled.dat', dtype=complex)
#len = len(iq_gen)
# SNR vector
# noise_range = np.asarray((-10.0, 0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0))
# # calculate frequency offset for each SNR
# for SNR in noise_range:
#     print "Calculating freq offset for SNR: " + str(SNR)
#     # generate noise signal
#     #n = (np.sqrt(np.power(10.0, -SNR/10.0)/2.0) * (np.random.normal(0, 1, len) + 1j * np.random.normal(0, 1, len)))
#     # add noise to signal
#     #rx = np.add(iq_gen, n)
#     # feed noisy signal in flowgraph
#     flowgraph = measure_freq_offset()
#     #f_offset = flowgraph.data
#     # convert to Hz
#     #f_offset = np.multiply(f_offset, 1.0/(2.0*np.pi))
#     #print f_offset
#     # calculate variance
#     # var = np.var(f_offset)
#     # print var