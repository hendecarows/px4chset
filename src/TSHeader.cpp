// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>

#include "TSHeader.h"

namespace TS
{

void Header::clear()
{
	sync_byte_ = 0;
	transport_error_indicator_ = false;
	payload_start_indicator_ = false;
	pid_ = 0;
	adaptation_field_control_ = false;
	continuity_counter_ = 0;
}

int32_t Header::parse_ts_header(const uint8_t* packet)
{
	sync_byte_ = packet[0];
	if (sync_byte_ != 0x47) { return 0; }

	transport_error_indicator_ = (packet[1] & 0x80) ? true : false;
	payload_start_indicator_ = (packet[1] & 0x40) ? true : false;
	pid_ = ((packet[1] & 0x1f) << 8) | packet[2];
	adaptation_field_control_ = (packet[3] & 0x20) ? true : false;
	continuity_counter_ = packet[3] & 0x0f;

	return HEADER_SIZE;
}

}
