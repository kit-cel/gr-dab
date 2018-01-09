#!/usr/bin/env python
#
# Copyright 2008 Free Software Foundation, Inc.
# 
# This file is part of GNU Radio
# 
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

# the code in this file is partially adapted from ofdm.py from the gnuradio
# trunk (actually, only frequency synchronisation is done the same way, as that
# implementation otherwise is not suited for DAB)
#
# Andreas Mueller, 2008
# andrmuel@ee.ethz.ch

from gnuradio import gr, blocks, fft, filter, digital
import dab
from threading import Timer
from time import sleep
from math import pi
from math import sqrt

"""
modulator and demodulator for the DAB physical layer 
"""

class ofdm_mod(gr.hier_block2):
	"""
	@brief Block to create a DAB signal from bits.

	Takes a data stream and performs OFDM modulation according to the DAB standard.
	The output sample rate is 2.048 MSPS.
	"""
	
	def __init__(self, dab_params, verbose=False, debug=False):
		"""
		Hierarchical block for OFDM modulation

		@param dab_params DAB parameter object (dab.parameters.dab_parameters)
		@param debug enables debug output to files
		"""

		dp = dab_params

		gr.hier_block2.__init__(self,"ofdm_mod",
		                        gr.io_signature2(2, 2, gr.sizeof_char*dp.num_carriers/4, gr.sizeof_char), # input signature
					gr.io_signature (1, 1, gr.sizeof_gr_complex)) # output signature


		# symbol mapping
		#self.mapper_v2s = blocks.vector_to_stream_make(gr.sizeof_char, 384)
		#self.mapper_unpack = blocks.packed_to_unpacked_bb_make(1, gr.GR_MSB_FIRST)
		#self.mapper = dab.mapper_bc_make(dp.num_carriers)
		#self.mapper_s2v = blocks.stream_to_vector_make(gr.sizeof_gr_complex, 1536)
		self.mapper = dab.qpsk_mapper_vbvc(dp.num_carriers)

		# add pilot symbol
		self.insert_pilot = dab.ofdm_insert_pilot_vcc(dp.prn)

		# phase sum
		self.sum_phase = dab.sum_phasor_trig_vcc(dp.num_carriers)

		# frequency interleaving
		self.interleave = dab.frequency_interleaver_vcc(dp.frequency_interleaving_sequence_array)

		# add central carrier & move to middle
		self.move_and_insert_carrier = dab.ofdm_move_and_insert_zero(dp.fft_length, dp.num_carriers)

		# ifft
		self.ifft = fft.fft_vcc(dp.fft_length, False, [], True)

		# cyclic prefixer
		self.prefixer = digital.ofdm_cyclic_prefixer(dp.fft_length, dp.symbol_length)

		# normalize to energy = 1 after IFFT
		self.multiply_const = blocks.multiply_const_cc(1.0/sqrt(2048))

		# convert back to vectors
		self.s2v = blocks.stream_to_vector(gr.sizeof_gr_complex, dp.symbol_length)

		# add null symbol
		self.insert_null = dab.insert_null_symbol(dp.ns_length, dp.symbol_length)

		#
		# connect it all
		#

		# data
		#self.connect((self,0), self.mapper_v2s, self.mapper_unpack, self.mapper, self.mapper_s2v, (self.insert_pilot,0), (self.sum_phase,0), self.interleave, self.move_and_insert_carrier, self.ifft, self.prefixer, self.s2v, (self.insert_null,0))
		self.connect((self,0), self.mapper, (self.insert_pilot,0), (self.sum_phase,0), self.interleave, self.move_and_insert_carrier, self.ifft, self.prefixer, self.multiply_const, self.s2v, (self.insert_null,0))
		self.connect(self.insert_null, self)

		# control signal (frame start)
		self.connect((self,1), (self.insert_pilot,1), (self.sum_phase,1), (self.insert_null,1))

		if False:
			self.connect(self.mapper, blocks.file_sink(gr.sizeof_gr_complex*dp.num_carriers, "debug/generated_signal_mapper.dat"))
			self.connect(self.insert_pilot, blocks.file_sink(gr.sizeof_gr_complex*dp.num_carriers, "debug/generated_signal_pilot_inserted.dat"))
			self.connect(self.sum_phase, blocks.file_sink(gr.sizeof_gr_complex*dp.num_carriers, "debug/generated_signal_sum_phase.dat"))
			self.connect(self.interleave, blocks.file_sink(gr.sizeof_gr_complex*dp.num_carriers, "debug/generated_signal_interleave.dat"))
			self.connect(self.move_and_insert_carrier, blocks.file_sink(gr.sizeof_gr_complex*dp.fft_length, "debug/generated_signal_move_and_insert_carrier.dat"))
			self.connect(self.ifft, blocks.file_sink(gr.sizeof_gr_complex*dp.fft_length, "debug/generated_signal_ifft.dat"))
			self.connect(self.prefixer, blocks.file_sink(gr.sizeof_gr_complex, "debug/generated_signal_prefixer.dat"))
			self.connect(self.insert_null, blocks.file_sink(gr.sizeof_gr_complex, "debug/generated_signal.dat"))
