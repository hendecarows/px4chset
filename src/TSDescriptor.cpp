// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <vector>

#include "TSPacket.h"
#include "TSDescriptor.h"

namespace TS
{

void ServiceListDescriptor::clear()
{
	data_size_ = 0;
	descriptor_tag_ = 0;
	descriptor_length_ = 0;
	service_lists_.clear();
}

int32_t ServiceListDescriptor::parse(const uint8_t* buf)
{
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 41 15 00 97 01 00 98 01 00 99 01 02 F1 C0 02 F3
	// 0010 C0 02 F4 C0 02 F5 C0

	auto p = buf;
	if (p[0] != 0x41) { return 0; }

	descriptor_tag_ = p[0];
	descriptor_length_ = p[1];

	p += 2;
	const auto last = p + descriptor_length_;
	while (p < last)
	{
		service_lists_.emplace_back(
			(p[0] << 4) | p[1],		// ServiceID
			p[2]					// ServiceType
		);
		p += 3;
	}

	data_size_ = 2 + descriptor_length_;

	return data_size_;
}

void SatelliteDeliverySystemDescriptor::clear()
{
	data_size_ = 0;
	descriptor_tag_ = 0;
	descriptor_length_ = 0;
	frequency_ = 0;
	orbital_position_ = 0;
	west_east_flag_ = false;
	polarisation_ = 0;
	modulation_ = 0;
	symbol_rate_ = 0;
	fec_inner_ = 0;
}

int32_t SatelliteDeliverySystemDescriptor::parse(const uint8_t* buf)
{
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 43 0B 01 17 27 48 11 00 E8 02 88 60 08

	auto p = buf;
	if (p[0] != 0x43) { return 0; }

	descriptor_tag_ = p[0];
	descriptor_length_ = p[1];
	frequency_ = Packet::bcd_to_dec(p + 2, 4);
	orbital_position_ = Packet::bcd_to_dec(p + 6, 2);
	west_east_flag_ = (p[8] & 0x80) ? true : false;
	polarisation_ = (p[8] & 0x60) >> 5;
	modulation_ = (p[8] & 0x1f);
	symbol_rate_ = Packet::bcd_to_dec(p + 9, 4);
	fec_inner_ = p[0x0c] & 0x0f;
	data_size_ = 2 + descriptor_length_;

	return data_size_;
}

}
