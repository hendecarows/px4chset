// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "TSPacket.h"

namespace TS
{

uint32_t Packet::bcd_to_dec(const uint8_t* p, size_t bytes)
{
	uint32_t value = 0;
	for (size_t i = 0; i < bytes; i++)
	{
		uint32_t v1 = (p[i] & 0xf0) >> 4;
		uint32_t v2 = p[i] & 0x0f;
		uint32_t v11 = 1;
		uint32_t v12 = 1;
		size_t size1 = (bytes - i) * 2 - 1;

		for (size_t j = 0; j < size1; j++)
		{
			if (j == 0)
			{
				v11 *= 10;
			}
			else
			{
				v11 *= 10;
				v12 *= 10;
			}
		}
		value += v1 * v11 + v2 * v12;
	}

	return value;
}

void Packet::clear()
{
	rest_buf_.clear();
	sync_buf_.clear();
}

size_t Packet::sync(const uint8_t* buf, const size_t size)
{
	if (size == 0) { return 0; }

	// 前回の残りに今回の分を結合
	auto rest_size = rest_buf_.size();
	rest_buf_.resize(rest_size + size);
	std::copy(buf, buf + size, rest_buf_.begin() + rest_size);

	// 2パケット未満の場合は処理しない
	if (rest_buf_.size() < PACKET_SIZE * 2) { return 0; }

	size_t sync_size = 0;
	auto src = rest_buf_.cbegin();
	auto last = rest_buf_.cend();

	sync_buf_.resize(rest_buf_.size());
	auto dst = sync_buf_.begin();
	while (src < last - PACKET_SIZE)
	{
		if ((src[0] != SYNC_BYTE) || (src[PACKET_SIZE] != SYNC_BYTE))
		{
			src++;
			continue;
		}

		std::copy(src, src + PACKET_SIZE, dst);
		src += PACKET_SIZE;
		dst += PACKET_SIZE;
		sync_size += PACKET_SIZE;
	}

	std::copy(src, last, rest_buf_.begin());
	rest_buf_.resize(std::distance(src, last));
	sync_buf_.resize(sync_size);

	return sync_size;
}

}
