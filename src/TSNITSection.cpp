// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

#include "TSPacket.h"
#include "TSHeader.h"
#include "TSDescriptor.h"
#include "TSNITSection.h"

namespace TS
{

void TranspoteDescriptor::clear()
{
	data_size_ = 0;
	transport_stream_id_ = 0;
	original_network_id_ = 0;
	transport_descriptors_length_ = 0;
	service_list_descriptor_.clear();
	satellite_delivery_system_descriptor_.clear();
}

int32_t TranspoteDescriptor::parse(const uint8_t* buf)
{
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 40 10 00 04 F0 24 41 15 00 97 01 00 98 01 00 99
	// 0010 01 02 F1 C0 02 F3 C0 02 F4 C0 02 F5 C0 43 0B 01
	// 0020 17 27 48 11 00 E8 02 88 60 08

	auto p = buf;
	transport_stream_id_ = (p[0] << 8) | p[1];
	original_network_id_ = (p[2] << 8) | p[3];
	transport_descriptors_length_ = ((p[4] & 0x0f) << 8) | p[5];

	p += 6;
	auto code = service_list_descriptor_.parse(p);
	if (code == 0)
	{
		clear();
		return 0;
	}

	p += code;
	code = satellite_delivery_system_descriptor_.parse(p);
	if (code == 0)
	{
		clear();
		return 0;
	}

	data_size_ = transport_descriptors_length_ + 6;

	return data_size_;
}

void NITHeader::clear()
{
	data_size_ = 0;
	on_update_ = false;
	pointer_field_ = 0;
	table_id_ = 0;
	section_syntax_indicator_ = false;
	section_length_ = 0;
	network_id_ = 0;
	version_number_ = 0xff;
	current_next_indicator_ = false;
	section_number_ = 0;
	last_section_number_ = 0;
	network_descriptors_length_ = 0;
	transport_stream_loop_length_ = 0;
}

bool NITHeader::parse_nit_header(const uint8_t* packet)
{
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 47 60 10 17 00 40 F3 07 00 04 E7 00 00 F0 12 40
	// 0010 0C 0E 89 42 53 20 44 69 67 69 74 61 6C FE 02 02
	// 0020 01 F2 E8 40 10 00 04 F0 24 41 15 00 97 01 00 98
	// 0030 01 00 99 01 02 F1 C0 02 F3 C0 02 F4 C0 02 F5 C0
	// 0040 43 0B 01 17 27 48 11 00 E8 02 88 60 08 40 11 00
	// 0050 04 F0 21 41 12 00 A1 01 00 A2 01 00 A3 01 00 A9
	// 0060 A1 02 FE C0 03 00 C0 43 0B 01 17 27 48 11 00 E8
	// 0070 02 88 60 08 40 12 00 04 F0 21 41 12 00 AB 01 00
	// 0080 AC 01 00 AD 01 00 B3 A1 03 09 C0 03 0A C0 43 0B
	// 0090 01 17 27 48 11 00 E8 02 88 60 08 40 30 00 04 F0
	// 00A0 18 41 09 00 BF 01 03 17 C0 03 18 C0 43 0B 01 17
	// 00B0 65 84 11 00 E8 02 88 60 08 40 90 00

	if (!parse_ts_header(packet)
		|| (pid() != 0x0010)
		|| transport_error_indicator()
		|| adaptation_field_control()
		)
	{
		return false;
	}

	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 40 F3 07 00 04 E7 00 00 F0 12 40 0C 0E 89 42 53
	// 0010 20 44 69 67 69 74 61 6C FE 02 02 01 F2 E8

	if (payload_start_indicator())
	{
		pointer_field_ = packet[4];
		auto p = &packet[5] + pointer_field_;
		table_id_ = p[0];

		section_syntax_indicator_ = (p[1] & 0x08) ? true : false;
		section_length_ = ((p[1] & 0x0f) << 8) | p[2];
		network_id_ = (p[3] << 8) | p[4];
		version_number_ = (p[5] & 0x3e) >> 1;
		current_next_indicator_ = (p[5] & 0x01) ? true : false;
		section_number_ = p[6];
		last_section_number_ = p[7];
		network_descriptors_length_ = ((p[8] & 0x0f) << 8) | p[9];

		auto i = 0x0a + network_descriptors_length_;
		transport_stream_loop_length_ = ((p[i] & 0x0f) << 8) | p[i + 1];
		on_update_ = true;

		data_size_ = 1 + pointer_field_ + 10 + network_descriptors_length_ + 2;
	}

	return true;
}

std::string NITHeader::show() const
{
	std::ostringstream os;
	os
		<< "sync_byte = 0x" << std::hex << static_cast<int>(sync_byte()) << '\n'
		<< "transport_error_indicator = " << std::boolalpha << transport_error_indicator() << '\n'
		<< "payload_start_indicator = " << std::boolalpha << payload_start_indicator() << '\n'
		<< "pid = 0x" << std::hex << std::setw(4) << std::setfill('0') << pid() << '\n'
		<< "adaptation_field_control = " << std::boolalpha << adaptation_field_control() << '\n'
		<< "continuity_counter = " << std::dec << static_cast<int>(continuity_counter())
		<< " (0x" << std::hex << static_cast<int>(continuity_counter()) << ')' << '\n'
		<< "pointer_field = " << std::dec << static_cast<int>(pointer_field())
		<< " (0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(pointer_field()) << ')' << '\n'
		<< "table_id = 0x" << std::hex << static_cast<int>(table_id()) << '\n'
		<< "section_syntax_indicator = " << std::boolalpha << section_syntax_indicator() << '\n'
		<< "section_length = " << std::dec << section_length()
		<< " (0x" << std::hex << static_cast<int>(section_length()) << ')' << '\n'
		<< "network_id = 0x" << std::hex << std::setw(4) << std::setfill('0') << network_id() << '\n'
		<< "version_number = " << std::dec << static_cast<int>(version_number())
		<< " (0x" << std::hex << static_cast<int>(version_number()) << ')' << '\n'
		<< "current_next_indicator = " << std::boolalpha << current_next_indicator() << '\n'
		<< "section_number = 0x" << std::hex << static_cast<int>(section_number()) << '\n'
		<< "last_section_number = 0x" << std::dec << static_cast<int>(last_section_number()) << '\n'
		<< "network_descriptors_length = " << std::dec << network_descriptors_length()
		<< " (0x" << std::hex << std::setw(3) << std::setfill('0')
		<< static_cast<int>(network_descriptors_length()) << ')' << '\n'
		<< "transport_stream_loop_length = " << std::dec << transport_stream_loop_length()
		<< " (0x" << std::hex << static_cast<int>(transport_stream_loop_length()) << ')' << '\n'
		<< '\n';

	return os.str();
}

void NITSection::clear()
{
	on_update_ = false;
	has_next_packet_ = false;
	packet_counter_ = 0;
	total_packets_ = 0;
	ts_headers_.clear();
	nit_header_.clear();
	payload_buf_.clear();
}

void NITSection::push(const uint8_t* buf, const size_t size)
{
	if (size == 0) { return; }

	auto sync_size = sync(buf, size);
	if (sync_size == 0) { return; }

	NITHeader nit;
	auto last = buffer().data() + sync_size;
	for (auto p = buffer().data(); p < last; p += Packet::size())
	{
		if (!nit.parse_nit_header(p)) { continue; }

		if (nit.payload_start_indicator()
			&& nit.table_id() == 0x40
			&& nit.current_next_indicator()
			&& nit.network_id() == 0x0004
			)
		{
			if (nit_header_.version_number() != nit.version_number())
			{
				clear();
				total_packets_ = static_cast<int32_t>((nit.section_length() + 4 + 183) / Packet::payload_size());
				packet_counter_ = 1;
				nit_header_ = nit;
				on_update_ = false;
				ts_headers_.reserve(total_packets_);
				ts_headers_.emplace_back(nit);
				payload_buf_.resize(total_packets_ * Packet::payload_size());
				std::copy(p + Header::size(), p + Packet::size(), payload_buf_.data());
				if (total_packets_ > 1)
				{
					has_next_packet_ = true;
				}
			}
			else
			{
				has_next_packet_ = false;
				on_update_ = true;
			}
		}
		else if (has_next_packet_ && packet_counter_ < total_packets_)
		{
			uint8_t require_continuity_counter = (nit_header_.continuity_counter() + packet_counter_) & 0x0f;
			if (nit.continuity_counter() == require_continuity_counter)
			{
				ts_headers_.emplace_back(nit);
				std::copy(p + Header::size(), p + Packet::size(), payload_buf_.data() + packet_counter_ * Packet::payload_size());
				packet_counter_++;
				if (packet_counter_ == total_packets_)
				{
					has_next_packet_ = false;
					on_update_ = true;
					parse();
				}
			}
			else
			{
				clear();
			}
		}
		else
		{
			clear();
		}
	}
}

void NITSection::parse()
{
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 00 40 F3 07 00 04 E7 00 00 F0 12 40 0C 0E 89 42
	// 0010 53 20 44 69 67 69 74 61 6C FE 02 02 01 F2 E8
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 40 10 00 04 F0 24 41 15 00 97 01 00 98 01 00 99
	// 0010 01 02 F1 C0 02 F3 C0 02 F4 C0 02 F5 C0 43 0B 01
	// 0020 17 27 48 11 00 E8 02 88 60 08
	//      +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F
	// 0000 40 11 00 04 F0 21 41 12 00 A1 01 00 A2 01 00 A3
	// 0010 01 00 A9 A1 02 FE C0 03 00 C0 43 0B 01 17 27 48
	// 0020 11 00 E8 02 88 60 08

	const auto* p = payload_buf_.data() + nit_header_.size();
	const auto last = p + nit_header_.transport_stream_loop_length();
	transport_descriptors_.clear();

	while (p < last)
	{
		auto& t = transport_descriptors_.emplace_back(p);
		p += t.size();
	}
}

std::string NITSection::show() const
{
	std::ostringstream os;
	for (const auto& d : transport_descriptors_)
	{
		os
			<< "transport stream id = 0x"
			<< std::hex << std::setw(4) << std::setfill('0') << d.transport_stream_id() << '\n'
			<< "original_network_id = 0x"
			<< d.original_network_id() << '\n'
			<< "transport_descriptors_length = "
			<< std::dec << d.transport_descriptors_length()
			<< " (0x" << std::hex << d.transport_descriptors_length() << ')' << '\n';
		for (const auto& s : d.service_list_descriptor().service_lists())
		{
			os
				<< "service id = 0x"
				<< std::hex << std::setw(4) << std::setfill('0') << s.service_id()
				<< " service type = 0x"
				<< static_cast<int>(s.service_type()) << '\n';
		}
		os
			<< "frequency = " << std::dec << d.satellite_delivery_system_descriptor().frequency() << '\n';
	}

	return os.str();
}

}
