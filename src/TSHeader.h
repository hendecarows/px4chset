// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace TS
{

class Header
{
public:
	Header() = default;
	Header(const uint8_t* packet) { parse_ts_header(packet); }
	virtual ~Header() = default;

	static int32_t size() { return HEADER_SIZE; }

	uint8_t sync_byte() const { return sync_byte_; }
	bool transport_error_indicator() const { return transport_error_indicator_; }
	bool payload_start_indicator() const { return payload_start_indicator_; }
	bool adaptation_field_control() const { return adaptation_field_control_; }
	uint8_t continuity_counter() const { return continuity_counter_; }
	uint16_t pid() const { return pid_; }
	bool has_sync_byte() const { return (sync_byte_ == 0x47) ? true : false; }

	void clear();
	int32_t parse_ts_header(const uint8_t* packet);

protected:
	static constexpr int32_t HEADER_SIZE = 4;

	uint8_t sync_byte_ = 0;
	bool transport_error_indicator_ = false;
	bool payload_start_indicator_ = false;
	uint16_t pid_ = 0;
	bool adaptation_field_control_ = false;
	uint8_t continuity_counter_ = 0;
};

}
