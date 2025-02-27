// SPDX-License-Identifier: GPL-3.0-or-later

#include <getopt.h>

#include <cstdio>
#include <string>
#include <stdexcept>
#include <sstream>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#endif

#include "convert.h"
#include "config.h"

Config::~Config()
{
	close_file();
}

void Config::parse(int argc, char* argv[])
{
	const option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"format", required_argument, 0, 'f'},
		{0,0,0,0},
	};

	while(true)
	{
		auto option_index = 0;
		auto c = getopt_long(argc, argv, "hf:", long_options, &option_index);
		if (c == -1) { break; }

		switch (c)
		{
		case 'f':
		{
			format_ = optarg;
			break;
		}
		case 'h':
		default:
			error_ = usage(argv[0]);
			throw std::runtime_error(error_);
		}
	}

	if (!Convert::has_format(format_))
	{
		error_ = usage(argv[0], "unknown format");
		throw std::runtime_error(error_);
	}

	is_sorting_ = Convert::is_sorting(format_);

	argc -= optind;
	if (argc == 1)
	{
		input_ = argv[optind];
		output_ = Convert::get_filename(format_);
	}
	else if (argc == 2)
	{
		input_ = argv[optind];
		output_ = argv[optind + 1];
	}
	else
	{
		error_ = usage(argv[0], "invalid number of arguments");
		throw std::runtime_error(error_);
	}

	open_file();
}

std::string Config::usage(const std::string& argv0, const std::string& msg) const
{
	std::ostringstream os;

	os << "\n"
		<< "usage: " << argv0
		<< " [options] input [output]\n"
		<< "\n"
		<< "options:\n"
		<< "  --help          show this help message\n"
		<< "  --format=str    output format (json,dvbv5,dvbv5lnb,mirakurun,bondvb,bonpt,bonptx,bonpx4,bonpx3,bonbda,bonplexpx)\n"
		<< "  input           input filename, '-': stdin\n"
		<< "  output          output filename, '-': stdout\n"
		<< "                  if 'output' is omitted, default filename is used\n";

	if (!msg.empty())
	{
		os << "\n"
			<< msg << "\n";
	}

	return os.str();
}

void Config::open_file()
{
#if defined(_WIN32)
	if (input_ == "-")
	{
		auto ec = _setmode(_fileno(stdin), _O_BINARY);
		if (ec == -1)
		{
			throw std::runtime_error("failed to _setmode(_fileno(stdin), _O_BINARY)");
		}
		fp_input_ = stdin;
	}
	else
	{
		auto ec = ::fopen_s(&fp_input_, input_.c_str(), "rb");
		if (ec)
		{
			error_ = "failed to open " + input_;
			throw std::runtime_error(error_);
		}
	}

	if (output_ == "-")
	{
		auto ec = _setmode(_fileno(stdout), _O_BINARY);
		if (ec == -1)
		{
			throw std::runtime_error("failed to _setmode(_fileno(stdout), _O_BINARY)");
		}
		fp_output_ = stdout;
	}
	else
	{
		auto ec = ::fopen_s(&fp_output_, output_.c_str(), "wb");
		if (ec)
		{
			error_ = "failed to open " + output_;
			throw std::runtime_error(error_);
		}
	}
#else
	if (input_ == "-")
	{
		fp_input_ = stdin;
	}
	else
	{
		fp_input_ = std::fopen(input_.c_str(), "rb");
		if (!fp_input_)
		{
			error_ = "failed to open " + input_;
			throw std::runtime_error(error_);
		}
	}

	if (output_ == "-")
	{
		fp_output_ = stdout;
	}
	else
	{
		fp_output_ = std::fopen(output_.c_str(), "wb");
		if (!fp_output_)
		{
			error_ = "failed to open " + output_;
			throw std::runtime_error(error_);
		}
	}
#endif
}

void Config::close_file()
{
	if (input_ != "-" && fp_input_)
	{
		std::fclose(fp_input_);
	}

	if (output_ != "-" && fp_output_)
	{
		std::fclose(fp_output_);
	}
}
