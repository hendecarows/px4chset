// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <vector>

namespace TS
{

class Packet
{
public:
	Packet() = default;
	virtual ~Packet() = default;

	static int32_t size() { return PACKET_SIZE; }
	static int32_t header_size() { return HEADER_SIZE; }
	static int32_t payload_size() { return PAYLOAD_SIZE; }
	static uint32_t bcd_to_dec(const uint8_t* p, size_t bytes);

	const std::vector<uint8_t>& buffer() const { return sync_buf_; }
	void clear();
	size_t sync(const uint8_t* buf, const size_t size);

protected:
	static constexpr int32_t PACKET_SIZE = 188;
	static constexpr int32_t HEADER_SIZE = 4;
	static constexpr int32_t PAYLOAD_SIZE = PACKET_SIZE - HEADER_SIZE;
	static constexpr int32_t SYNC_SIZE = PACKET_SIZE * 5;
	static constexpr uint8_t SYNC_BYTE = 0x47;

private:
	std::vector<uint8_t> sync_buf_;
	std::vector<uint8_t> rest_buf_;
};

}
