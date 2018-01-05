/* -*- c++ -*- */

#define DAB_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "dab_swig_doc.i"

%{
#include "dab/diff_phasor_vcc.h"
#include "dab/frequency_interleaver_vcc.h"
#include "dab/complex_to_interleaved_float_vcf.h"
#include "dab/unpuncture_vff.h"
#include "dab/fib_sink_vb.h"
#include "dab/ofdm_insert_pilot_vcc.h"
#include "dab/sum_phasor_trig_vcc.h"
#include "dab/ofdm_move_and_insert_zero.h"
#include "dab/insert_null_symbol.h"
#include "dab/time_interleave_bb.h"
#include "dab/time_deinterleave_ff.h"
#include "dab/crc16_bb.h"
#include "dab/fib_source_b.h"
#include "dab/select_subch_vfvf.h"
#include "dab/unpuncture_ff.h"
#include "dab/prune.h"
#include "dab/firecode_check_bb.h"
#include "dab/puncture_bb.h"
#include "dab/dab_transmission_frame_mux_bb.h"
#include "dab/conv_encoder_bb.h"
#include "dab/mapper_bc.h"
#include "dab/mp2_decode_bs.h"
#include "dab/mp4_decode_bs.h"
#include "dab/reed_solomon_decode_bb.h"
#include "dab/reed_solomon_encode_bb.h"
#include "dab/mp4_encode_sb.h"
#include "dab/mp2_encode_sb.h"
#include "dab/valve_ff.h"
#include "dab/synchronization_ff.h"
#include "dab/ofdm_synchronization_cvf.h"
#include "dab/ofdm_coarse_frequency_correction_vcvc.h"
#include "dab/frequency_deinterleave_cc.h"
#include "dab/demux_cc.h"
#include "dab/select_cus_vcvc.h"
#include "dab/qpsk_mapper_vbvc.h"
%}


%include "dab/diff_phasor_vcc.h"
GR_SWIG_BLOCK_MAGIC2(dab, diff_phasor_vcc);

%include "dab/frequency_interleaver_vcc.h"
GR_SWIG_BLOCK_MAGIC2(dab, frequency_interleaver_vcc);

%include "dab/complex_to_interleaved_float_vcf.h"
GR_SWIG_BLOCK_MAGIC2(dab, complex_to_interleaved_float_vcf);


%include "dab/unpuncture_vff.h"
GR_SWIG_BLOCK_MAGIC2(dab, unpuncture_vff);

%include "dab/fib_sink_vb.h"
GR_SWIG_BLOCK_MAGIC2(dab, fib_sink_vb);

%include "dab/ofdm_insert_pilot_vcc.h"
GR_SWIG_BLOCK_MAGIC2(dab, ofdm_insert_pilot_vcc);
%include "dab/sum_phasor_trig_vcc.h"
GR_SWIG_BLOCK_MAGIC2(dab, sum_phasor_trig_vcc);
%include "dab/ofdm_move_and_insert_zero.h"
GR_SWIG_BLOCK_MAGIC2(dab, ofdm_move_and_insert_zero);
%include "dab/insert_null_symbol.h"
GR_SWIG_BLOCK_MAGIC2(dab, insert_null_symbol);
%include "dab/time_interleave_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, time_interleave_bb);
%include "dab/time_deinterleave_ff.h"
GR_SWIG_BLOCK_MAGIC2(dab, time_deinterleave_ff);
%include "dab/crc16_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, crc16_bb);
%include "dab/fib_source_b.h"
GR_SWIG_BLOCK_MAGIC2(dab, fib_source_b);
%include "dab/select_subch_vfvf.h"
GR_SWIG_BLOCK_MAGIC2(dab, select_subch_vfvf);
%include "dab/unpuncture_ff.h"
GR_SWIG_BLOCK_MAGIC2(dab, unpuncture_ff);
%include "dab/prune.h"
GR_SWIG_BLOCK_MAGIC2(dab, prune);
%include "dab/firecode_check_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, firecode_check_bb);
%include "dab/puncture_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, puncture_bb);
%include "dab/dab_transmission_frame_mux_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, dab_transmission_frame_mux_bb);
%include "dab/conv_encoder_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, conv_encoder_bb);
%include "dab/mapper_bc.h"
GR_SWIG_BLOCK_MAGIC2(dab, mapper_bc);

%include "dab/mp2_decode_bs.h"
GR_SWIG_BLOCK_MAGIC2(dab, mp2_decode_bs);

%include "dab/mp4_decode_bs.h"
GR_SWIG_BLOCK_MAGIC2(dab, mp4_decode_bs);
%include "dab/reed_solomon_decode_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, reed_solomon_decode_bb);
%include "dab/reed_solomon_encode_bb.h"
GR_SWIG_BLOCK_MAGIC2(dab, reed_solomon_encode_bb);
%include "dab/mp4_encode_sb.h"
GR_SWIG_BLOCK_MAGIC2(dab, mp4_encode_sb);
%include "dab/mp2_encode_sb.h"
GR_SWIG_BLOCK_MAGIC2(dab, mp2_encode_sb);

%include "dab/valve_ff.h"
GR_SWIG_BLOCK_MAGIC2(dab, valve_ff);


%include "dab/synchronization_ff.h"
GR_SWIG_BLOCK_MAGIC2(dab, synchronization_ff);
%include "dab/ofdm_synchronization_cvf.h"
GR_SWIG_BLOCK_MAGIC2(dab, ofdm_synchronization_cvf);
%include "dab/ofdm_coarse_frequency_correction_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(dab, ofdm_coarse_frequency_correction_vcvc);

%include "dab/frequency_deinterleave_cc.h"
GR_SWIG_BLOCK_MAGIC2(dab, frequency_deinterleave_cc);

%include "dab/demux_cc.h"
GR_SWIG_BLOCK_MAGIC2(dab, demux_cc);

%include "dab/select_cus_vcvc.h"
GR_SWIG_BLOCK_MAGIC2(dab, select_cus_vcvc);
%include "dab/qpsk_mapper_vbvc.h"
GR_SWIG_BLOCK_MAGIC2(dab, qpsk_mapper_vbvc);
