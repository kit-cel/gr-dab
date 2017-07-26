/* -*- c++ -*- */
/*
 * Copyright belongs to Andreas Mueller
 * Modified 2017 by Moritz Luca Schmid, Communications Engineering Lab (CEL) / Karlsruhe Institute of Technology (KIT).
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <gnuradio/io_signature.h>
#include "fib_sink_vb_impl.h"
#include <stdexcept>
#include <stdio.h>
#include <sstream>
#include <boost/format.hpp>
#include "crc16.h"
#include "FIC.h"


using namespace boost;

namespace gr {
  namespace dab {

    fib_sink_vb::sptr
    fib_sink_vb::make()
    {
      return gnuradio::get_initial_sptr
              (new fib_sink_vb_impl());
    }


    fib_sink_vb_impl::fib_sink_vb_impl()
            : gr::sync_block("fib_sink_vb",
                             gr::io_signature::make(1, 1, sizeof(char) * 32),
                             gr::io_signature::make(0, 0, 0))
    {
      d_json = "{\"SWR1_BW\":{\"reference\":769,\"type\":\"DAB+\",\"primary\":true,\"ID\":2,\"address\":54,\"size\":84,\"protection\":3,\"bitrate\":112},\"SWR2\":{\"reference\":123,\"type\":\"DAB+\",\"primary\":true,\"ID\":3,\"address\":138,\"size\":96,\"protection\":3,\"bitrate\":128}}";
    }

    int
    fib_sink_vb_impl::process_fib(const char *fib)
    {
      uint8_t type, length, pos;
      if (crc16(fib, FIB_LENGTH, FIB_CRC_POLY, FIB_CRC_INITSTATE) != 0) {
        GR_LOG_DEBUG(d_logger, "FIB CRC error");
        return 1;
      }
      GR_LOG_DEBUG(d_logger, "FIB correct");
      pos = 0;
      while (pos < FIB_LENGTH - FIB_CRC_LENGTH && (uint8_t) fib[pos] != FIB_ENDMARKER &&
             (uint8_t) fib[pos] != 0) { //TODO correct?
        type = fib[pos] >> 5;
        length = fib[pos] & 0x1f;
        assert(pos + length <= FIB_LENGTH - FIB_CRC_LENGTH);
        assert(length != 0);
        process_fig(type, &fib[pos], length);
        pos += length + 1;
      }
      return 0;
    }

    int
    fib_sink_vb_impl::process_fig(uint8_t type, const char *data, uint8_t length)
    {
      uint8_t cn, oe, pd, extension;
      switch (type) {
        case FIB_FIG_TYPE_MCI:
          GR_LOG_DEBUG(d_logger, "FIG type 0");
          extension = (uint8_t)(data[1] & 0x1f);
          cn = (uint8_t)(data[1] & 0x80);
          if (cn == 1) GR_LOG_DEBUG(d_logger, "[WARNING, INFO FOR FUTURE CONFIGURATION]: ");
          oe = (uint8_t)(data[1] & 0x40);
          if (cn == 1) GR_LOG_DEBUG(d_logger, "[WARNING, INFO FOR OTHER ENSEMBLE]");
          pd = (uint8_t)(data[1] & 0x20);
          if (pd == 1) GR_LOG_DEBUG(d_logger, "[WARNING, LONG IDENTIFIER");

          switch (extension) {
            case FIB_MCI_EXTENSION_ENSEMBLE_INFO: {
              uint16_t ensemble_reference = (uint16_t)(data[2] & 0x0f) << 8 | (uint8_t) data[3];
              uint8_t change_flag = (uint8_t)((data[4] & 0xc0) >> 6);
              uint8_t occurrence_change = data[6];
              if (change_flag != 0)
                GR_LOG_DEBUG(d_logger,
                             format(", [CHANGE OF SUBCHANNEL OR SERVICE ORGA (%d)](at CIF %d) ") % (int) change_flag %
                             (int) occurrence_change);
              uint8_t alarm_flag = (uint8_t)((data[4] & 0x20) >> 5);
              if (alarm_flag == 1) GR_LOG_DEBUG(d_logger, ", [ALARM MESSAGE ACCESSIBLE] ");
              uint16_t CIF_counter = (uint16_t)((data[4] & 0x1f) * 250 + (data[5]));
              GR_LOG_DEBUG(d_logger,
                           format("ensemble info: reference %d, CIF counter = %d") % ensemble_reference % CIF_counter);
              break;
            }
            case FIB_MCI_EXTENSION_SUBCHANNEL_ORGA: {
              uint8_t subch_counter = 0;
              GR_LOG_DEBUG(d_logger, "subchannel orga: ");
              do {
                uint8_t subchID = (uint8_t)((data[2 + subch_counter] & 0xfc) >> 2);
                uint16_t start_adress = (uint16_t)((data[2 + subch_counter] & 0x03) << 8) |
                                        (uint8_t)(data[3 + subch_counter]);
                uint8_t sl_form = (uint8_t)(data[4] & 0x80);
                if (sl_form == 0) {
                  uint8_t table_switch = (uint8_t)(data[4 + subch_counter] & 0x40);
                  if (table_switch != 0) GR_LOG_DEBUG(d_logger, " [WARNING: OTHER TABLE USED] ");
                  uint8_t table_index = (uint8_t)(data[4 + subch_counter] & 0x3f);
                  GR_LOG_DEBUG(d_logger, format("subchID = %d , start adress = %d, index %d") % (int) subchID %
                                         (int) start_adress % (int) table_index);
                  subch_counter += 3;
                } else {
                  uint8_t option = (uint8_t)(data[4 + subch_counter] & 0x70);
                  uint8_t protect_level = (uint8_t)((data[4 + subch_counter] & 0x0c) >> 2);
                  uint16_t subch_size = (uint16_t)((data[4 + subch_counter] & 0x03) << 8) |
                                        (uint8_t)(data[5 + subch_counter]);
                  GR_LOG_DEBUG(d_logger,
                               format("subchID = %d , start adress = %d, option %d, protect level %d, subch size %d") %
                               (int) subchID % (int) start_adress % (int) option % (int) protect_level %
                               (int) subch_size);
                  subch_counter += 4;
                }
              } while (1 + subch_counter < length);
              break;
            }
            case FIB_MCI_EXTENSION_SERVICE_ORGA: {
              uint8_t service_counter = 1;
              do { //iterate over services
                uint16_t service_reference = (uint16_t)(data[service_counter + 1] & 0x0f) << 8 |
                                             (uint8_t) data[service_counter + 2];
                GR_LOG_DEBUG(d_logger, format("service orga: reference %d ") % service_reference);
                uint8_t local_flag = (uint8_t)((data[service_counter + 3] & 0x80) >> 7);
                if (local_flag == 1) GR_LOG_DEBUG(d_logger, "[LOCAL FLAG SET] ");
                uint8_t ca = (uint8_t)((data[service_counter + 3] & 0x70) >> 4);
                if (ca != 0) GR_LOG_DEBUG(d_logger, "[CONDITIONAL ACCESS USED] ");
                uint8_t num_service_comps = (uint8_t)(data[service_counter + 3] & 0x0f);
                GR_LOG_DEBUG(d_logger, format("(%d components):") % (int) num_service_comps);
                for (int i = 0; i < num_service_comps; i++) { //iterate over service components
                  uint8_t TMID = (uint8_t)((data[service_counter + 4 + i * 2] & 0xc0) >> 6);
                  uint8_t comp_type = (uint8_t)(data[service_counter + 4 + i * 2] & 0x3f);
                  uint8_t subchID = (uint8_t)((data[service_counter + 5 + i * 2] & 0xfc) >> 2);
                  uint8_t ps = (uint8_t)((data[service_counter + 5 + i * 2 + 1] & 0x02) >> 1);
                  if (TMID == 0) {
                    GR_LOG_DEBUG(d_logger,
                                 format("(audio stream, type %d, subchID %d, primary %d)") % (int) comp_type %
                                 (int) subchID % (int) ps);
                  } else if (TMID == 1) {
                    GR_LOG_DEBUG(d_logger,
                                 format("(data stream, type %d, subchID %d, primary %d)") % (int) comp_type %
                                 (int) subchID % (int) ps);
                  } else if (TMID == 2) {
                    GR_LOG_DEBUG(d_logger,
                                 format("(FIDC, type %d, subchID %d, primary %d)") % (int) comp_type % (int) subchID %
                                 (int) ps);
                  } else {
                    GR_LOG_DEBUG(d_logger, "[packed data]");
                  }
                }
                service_counter += 3 + 2 * num_service_comps;
              } while (service_counter < length);
              break;
            }
            case FIB_MCI_EXTENSION_SERVICE_ORGA_PACKET_MODE:
              GR_LOG_DEBUG(d_logger, "service orga packet mode");
              break;
            case FIB_MCI_EXTENSION_SERVICE_ORGA_CA:
              GR_LOG_DEBUG(d_logger, "service orga conditional access");
              break;
            case FIB_SI_EXTENSION_SERVICE_COMP_LANGUAGE:
              GR_LOG_DEBUG(d_logger, "service comp language");
              break;
            case FIB_MCI_EXTENSION_SERVICE_COMP_GLOBAL_DEFINITION: {
              uint8_t service_comp_counter = 0;
              do {
                uint16_t service_reference = (uint16_t)(data[service_comp_counter + 2] & 0x0f) << 8 |
                                             (uint8_t) data[service_comp_counter + 3];
                uint8_t SCIdS = (uint8_t)(data[service_comp_counter + 4] & 0x0f);
                if ((data[service_comp_counter + 5] & 0x80) == 0) {
                  uint8_t subchID = (uint8_t)(data[service_comp_counter + 5] & 0x3f);
                  GR_LOG_DEBUG(d_logger,
                               format("service component global definition: reference %d, SCIdS %d, subchID %d") %
                               service_reference % (int) SCIdS % (int) subchID);
                  service_comp_counter += 5;
                } else {
                  uint16_t subchID = (uint16_t)(data[service_comp_counter + 5] & 0x0f) << 8 |
                                     (uint8_t) data[service_comp_counter + 6];
                  GR_LOG_DEBUG(d_logger,
                               format("service component global definition: reference %d, SCIdS %d, subchID %d") %
                               service_reference % (int) SCIdS % (int) subchID);
                  service_comp_counter += 6;
                }
              } while (1 + service_comp_counter < length);
              break;
            }
            case FIB_SI_EXTENSION_COUNTRY_LTO:
              GR_LOG_DEBUG(d_logger, "country LTO");
              break;
            case FIB_SI_EXTENSION_USER_APPLICATION_INFO:
              GR_LOG_DEBUG(d_logger, "user application info");
              break;
            case FIB_MCI_EXTENSION_SUBCHANNEL_PACKET_MODE_FEC:
              GR_LOG_DEBUG(d_logger, "subchannel orga packet mode fec");
              break;
            case FIB_SI_EXTENSION_PROGRAMME_NUMBER:
              GR_LOG_DEBUG(d_logger, "programme number");
              break;
            case FIB_SI_EXTENSION_PROGRAMME_TYPE:
              GR_LOG_DEBUG(d_logger, "programme type");
              break;
            case FIB_SI_EXTENSION_ANNOUNCEMENT_SUPPORT:
              GR_LOG_DEBUG(d_logger, "announcement support");
              break;
            case FIB_SI_EXTENSION_ANNOUNCEMENT_SWITCHING:
              GR_LOG_DEBUG(d_logger, "announcement switching");
              break;
            default:
              GR_LOG_DEBUG(d_logger, format("unsupported extension (%d)") % (int) extension);
              break;
          }
          break;
        case FIB_FIG_TYPE_LABEL1:
        case FIB_FIG_TYPE_LABEL2:
          GR_LOG_DEBUG(d_logger, "FIG type 2");
          char label[16];
          extension = (uint8_t)(data[1] & 0x07);
          switch (extension) {
            case FIB_SI_EXTENSION_ENSEMBLE_LABEL:
              memcpy(label, &data[4], 16);
              memcpy(label, &data[4], 16);
              GR_LOG_DEBUG(d_logger, format("[ensemble label]: %s ") % label);
              break;
            case FIB_SI_EXTENSION_PROGRAMME_SERVICE_LABEL: {
              uint16_t service_reference = (uint16_t)(data[2] & 0x0f) << 8 | (uint8_t) data[3];
              memcpy(label, &data[4], 16);
              GR_LOG_DEBUG(d_logger,
                           format("[programme service label] (reference %d): %s ") % service_reference % label);
              break;
            }
            case FIB_SI_EXTENSION_SERVICE_COMP_LABEL:
              memcpy(label, &data[5], 16);
              GR_LOG_DEBUG(d_logger, format("[service component label] %s") % label);
              break;
            case FIB_SI_EXTENSION_DATA_SERVICE_LABEL:
              memcpy(label, &data[5], 16);
              GR_LOG_DEBUG(d_logger, format("[data service label]: %s ") % label);
              break;
            default:
              GR_LOG_DEBUG(d_logger, format("[unknown extension (%d) ") % (int) extension);
              break;
          }
          break;
        case FIB_FIG_TYPE_FIDC:
          GR_LOG_DEBUG(d_logger, "FIG type 5");
          extension = (uint8_t)(data[1] & 0x07);
          switch (extension) {
            case FIB_FIDC_EXTENSION_PAGING:
              GR_LOG_DEBUG(d_logger, "paging - not supported yet");
              break;
            case FIB_FIDC_EXTENSION_TMC:
              GR_LOG_DEBUG(d_logger, "TMC (traffic message channel) - not supported yet");
              break;
            case FIB_FIDC_EXTENSION_EWS:
              GR_LOG_DEBUG(d_logger, "EWS (emergency warning service) - not supported yet");
              break;
            default:
              GR_LOG_DEBUG(d_logger, format("unsupported extension (%d)") % (int) extension);
          }
          break;
        case FIB_FIG_TYPE_CA:
          GR_LOG_DEBUG(d_logger, "FIB type CA (conditional access) not supported yet");
          break;
        default:
          GR_LOG_DEBUG(d_logger, "unsupported FIG type");
          break;
      }
      return 0;
    }

    int
    fib_sink_vb_impl::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
    {
      const char *in = (const char *) input_items[0];

      for (int i = 0; i < noutput_items; i++) {
        process_fib(in);
        in += 32;
      }



      return noutput_items;
    }
  }
}
