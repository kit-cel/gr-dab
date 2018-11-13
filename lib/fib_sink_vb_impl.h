/* -*- c++ -*- */
/*
 * Copyright 2017 by Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
 * Copyright 2008 by Andreas Mueller
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
#ifndef INCLUDED_DAB_FIB_SINK_VB_IMPL_H
#define INCLUDED_DAB_FIB_SINK_VB_IMPL_H

#include <dab/fib_sink_vb.h>

namespace gr {
  namespace dab {
/*! \brief sink for DAB/DAB+ FIBs, interprets MSC and SI
 * CRC16 check of incoming fibs.
 * Reads correct fibs.
 * Generates json objects with service and multiplex information.
 */
    class fib_sink_vb_impl : public fib_sink_vb {

    private:
      /*! \brief Processes an incoming FIB.
       * If CRC16 fails, dump the FIB,
       * otherwise extract Fast Information Groups (FIGs) and read header information.
       *
       * @param fib Pointer to the first byte of the 30 byte FIB.
       * @return 0 if CRC failed, 1 if CRC succeeded.
       */
      int process_fib(const char *fib);
      /*! \brief Processes a FIG.
       * Switch between FIG types, extract information and write it to logger.
       * Write and collect information in JSON objects for transport to GUI.
       * @param type Type of the FIG. See ETSI EN 300 401 chapter 5.2.2.
       * @param data Pointer to the FIG data buffer.
       * @param length Length of the FIG data buffer in bytes.
       */
      void process_fig(uint8_t type, const char *data, uint8_t length);

      bool d_crc_passed;

      std::string d_json_ensemble_info;
      // service info
      std::string d_json_service_info;
      std::string d_service_info_current;
      int d_service_info_written_trigger;
      // service labels
      std::string d_json_service_labels;
      std::string d_service_labels_current;
      int d_service_labels_written_trigger;
      // service subch
      std::string d_json_subch_info;
      std::string d_subch_info_current;
      int d_subch_info_written_trigger;
      // programme type
      std::string d_json_programme_type;
      std::string d_programme_type_current;
      int d_programme_type_written_trigger;

    public:
      fib_sink_vb_impl();

      virtual std::string get_ensemble_info() { return d_json_ensemble_info; }

      virtual std::string get_service_info() { return d_json_service_info; }

      virtual std::string get_service_labels() { return d_json_service_labels; }

      virtual std::string get_subch_info() { return d_json_subch_info; }

      virtual std::string get_programme_type() { return d_json_programme_type; }

      virtual bool get_crc_passed() { return d_crc_passed; }

      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };
  }
}

#endif /* INCLUDED_DAB_FIB_SINK_B_H */
