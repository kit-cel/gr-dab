#!/usr/bin/env python

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import math
import dab_swig as dab


class qa_qpsk_mapper_vbvc(gr_unittest.TestCase):
    """
    @brief QA for the qpsk mapper block

    This class implements a test bench to verify the corresponding C++ class.
    """

    def setUp(self):
        self.tb = gr.top_block()

    def tearDown(self):
        self.tb = None

    def test_001_qpsk_mapper_vbvc(self):
        src_data = (31, 255)
        vector_length = 8
        SQRT2 = 1.0/math.sqrt(2)
        expected_result = (SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j,
                           -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j, -SQRT2 - SQRT2*1j)
        src = blocks.vector_source_b(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_char, vector_length/4)
        mapper = dab.qpsk_mapper_vbvc_make(vector_length)
        v2s = blocks.vector_to_stream(gr.sizeof_gr_complex, vector_length)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, s2v, mapper, v2s, dst)
        self.tb.run()
        result_data = dst.data()
        self.assertFloatTuplesAlmostEqual(expected_result, result_data, 6)

    def test_002_qpsk_mapper_vbvc(self):
        src_data = (0, 3)
        vector_length = 8
        SQRT2 = 1.0/math.sqrt(2)
        expected_result = (SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j, SQRT2 + SQRT2*1j,
                           SQRT2 + SQRT2*1j, SQRT2 - SQRT2*1j, SQRT2 - SQRT2*1j)
        src = blocks.vector_source_b(src_data)
        s2v = blocks.stream_to_vector(gr.sizeof_char, vector_length/4)
        mapper = dab.qpsk_mapper_vbvc_make(vector_length)
        v2s = blocks.vector_to_stream(gr.sizeof_gr_complex, vector_length)
        dst = blocks.vector_sink_c()
        self.tb.connect(src, s2v, mapper, v2s, dst)
        self.tb.run()
        result_data = dst.data()
        self.assertFloatTuplesAlmostEqual(expected_result, result_data, 6)


if __name__ == '__main__':
    gr_unittest.run(qa_qpsk_mapper_vbvc, "qa_qpsk_mapper_vbvc.xml")