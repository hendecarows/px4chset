// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "TSPacket.h"
#include "TSHeader.h"
#include "TSDescriptor.h"

namespace TS
{

class TranspoteDescriptor
{
public:
	TranspoteDescriptor() = default;
	TranspoteDescriptor(const uint8_t* buf) { parse(buf); }
	virtual ~TranspoteDescriptor() = default;

	int32_t size() const { return data_size_; }
	uint16_t transport_stream_id() const { return transport_stream_id_; }
	uint16_t original_network_id() const { return original_network_id_; }
	uint16_t transport_descriptors_length() const { return transport_descriptors_length_; }
	const ServiceListDescriptor& service_list_descriptor() const {
		return service_list_descriptor_;
	}
	const SatelliteDeliverySystemDescriptor& satellite_delivery_system_descriptor() const {
		return satellite_delivery_system_descriptor_;
	}

	void clear();
	int32_t parse(const uint8_t* buf);

private:
	int32_t data_size_ = 0;
	uint16_t transport_stream_id_ = 0;
	uint16_t original_network_id_ = 0;
	uint16_t transport_descriptors_length_ = 0;
	ServiceListDescriptor service_list_descriptor_;
	SatelliteDeliverySystemDescriptor satellite_delivery_system_descriptor_;
};

class NITHeader : public Header
{
public:
	NITHeader() = default;
	virtual ~NITHeader() = default;

	int32_t size() const { return data_size_; }
	bool on_update() const { return on_update_; }
	uint8_t pointer_field() const { return pointer_field_; }
	uint8_t table_id() const { return table_id_; }
	bool section_syntax_indicator() const { return section_syntax_indicator_; }
	uint16_t section_length() const { return section_length_; }
	uint16_t network_id() const { return network_id_; }
	uint8_t version_number() const { return version_number_; }
	bool current_next_indicator() const { return current_next_indicator_; }
	uint8_t section_number() const { return section_number_; }
	uint8_t last_section_number() const { return last_section_number_; }
	uint16_t network_descriptors_length() const { return network_descriptors_length_; }
	uint16_t transport_stream_loop_length() const { return transport_stream_loop_length_; }

	void clear();
	bool parse_nit_header(const uint8_t* packet);
	std::string show() const;

protected:
	int32_t data_size_ = 0;
	bool on_update_ = false;
	uint8_t pointer_field_ = 0;
	uint8_t table_id_ = 0;
	bool section_syntax_indicator_ = false;
	uint16_t section_length_ = 0;
	uint16_t network_id_ = 0;
	uint8_t version_number_ = 0xff;
	bool current_next_indicator_ = false;
	uint8_t section_number_ = 0;
	uint8_t last_section_number_ = 0;
	uint16_t network_descriptors_length_ = 0;
	uint16_t transport_stream_loop_length_ = 0;
};

class NITSection : public Packet
{
public:
	NITSection() = default;
	virtual ~NITSection() = default;

	bool on_update() const { return on_update_; }
	const std::vector<Header>& headers() const { return ts_headers_; }
	const std::vector<uint8_t>& payloads() const { return payload_buf_; }
	const std::vector<TranspoteDescriptor>& transport_descriptors() const { return transport_descriptors_; }

	void clear();
	void push(const uint8_t* buf, const size_t size);
	std::string show() const;

private:
	bool on_update_ = false;
	bool has_next_packet_ = false;
	int32_t packet_counter_ = 0;
	int32_t total_packets_ = 0;
	NITHeader nit_header_;

	std::vector<Header> ts_headers_;
	std::vector<uint8_t> payload_buf_;
	std::vector<TranspoteDescriptor> transport_descriptors_;

	void parse();
};

}
