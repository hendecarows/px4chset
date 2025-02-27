// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <vector>

namespace TS
{

class ServiceList
{
public:
	ServiceList() = default;
	ServiceList(uint16_t service_id, uint8_t service_type) :
		service_id_(service_id), service_type_(service_type)
	{}
	virtual ~ServiceList() = default;

	uint16_t service_id() const { return service_id_; }
	uint8_t service_type() const { return service_type_; }

private:
	uint16_t service_id_ = 0;
	uint8_t service_type_ = 0;
};

class ServiceListDescriptor
{
public:
	ServiceListDescriptor() = default;
	ServiceListDescriptor(const uint8_t* buf) { parse(buf); }
	virtual ~ServiceListDescriptor() = default;

	int32_t size() const { return data_size_; }
	uint8_t descriptor_tag() const { return descriptor_tag_; }
	uint8_t descriptor_length() const { return descriptor_length_; }
	const std::vector<ServiceList> service_lists() const { return service_lists_; }

	void clear();
	int32_t parse(const uint8_t* buf);

private:
	int32_t data_size_ = 0;
	uint8_t descriptor_tag_ = 0;
	uint8_t descriptor_length_ = 0;
	std::vector<ServiceList> service_lists_;
};

class SatelliteDeliverySystemDescriptor
{
public:
	SatelliteDeliverySystemDescriptor() = default;
	SatelliteDeliverySystemDescriptor(const uint8_t* buf) { parse(buf); }
	virtual ~SatelliteDeliverySystemDescriptor() = default;

	int32_t size() const { return data_size_; }
	uint8_t descriptor_tag() const { return descriptor_tag_; }
	uint8_t descriptor_length() const { return descriptor_length_; }
	uint32_t frequency() const { return frequency_; }
	uint16_t orbital_position() const { return orbital_position_; }
	bool west_east_flag() const { return west_east_flag_; }
	uint8_t polarisation() const { return polarisation_; }
	uint8_t modulation() const { return modulation_; }
	uint32_t symbol_rate() const { return symbol_rate_; }
	uint8_t fec_inner() const { return fec_inner_; }

	void clear();
	int32_t parse(const uint8_t* buf);

private:
	int32_t data_size_ = 0;
	uint8_t descriptor_tag_ = 0;
	uint8_t descriptor_length_ = 0;
	uint32_t frequency_ = 0;
	uint16_t orbital_position_ = 0;
	bool west_east_flag_ = false;
	uint8_t polarisation_ = 0;
	uint8_t modulation_ = 0;
	uint32_t symbol_rate_ = 0;
	uint8_t fec_inner_ = 0;
};

}
