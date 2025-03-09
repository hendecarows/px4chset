// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

#include "chset.h"
#include "config.h"
#include "convert.h"
#include "TSNITSection.h"


int main(int argc, char* argv[])
{
	try
	{
		Config config;
		TS::NITSection nit;
		ChSets chsets;
		std::vector<uint8_t> buf;

		config.parse(argc, argv);
		buf.resize(config.buffer_size());

		while (true)
		{
			auto size = std::fread(buf.data(), 1, buf.size(), config.fp_input());
			if (size == 0) { break; }
			nit.push(buf.data(), size);
			if (nit.on_update()) { break; }
		}

		if (nit.on_update())
		{
			for (const auto& t : nit.transport_descriptors())
			{
				chsets.set_transport_stream_id(t.transport_stream_id());
			}
			if (config.sorting())
			{
				chsets.sort_relative_ts_number(config.sorting());
			}
			auto data = Convert::dump(config.format(), chsets);
			std::fwrite(data.c_str(), data.size(), 1, config.fp_output());
		}
		else
		{
			throw std::runtime_error("NIT packets not found. Check recorded channel or time.");
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	return 0;
}
