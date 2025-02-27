// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

class Config
{
public:
	Config() = default;
	~Config();

	bool is_sorting() const { return is_sorting_; }
	const std::string& format() const { return format_; }
	const std::string& error() const { return error_; }
	int32_t buffer_size() const { return BUFFER_SIZE; }
	const std::string& input() const { return input_; }
	const std::string& output() const { return output_; }
	std::FILE* fp_input() const { return fp_input_; }
	std::FILE* fp_output() const { return fp_output_; }

	void parse(int argc, char* argv[]);

private:
	static constexpr int32_t BUFFER_SIZE = 188*1024;

	bool is_sorting_ = false;
	std::string format_ = "json";
	std::string error_;
	std::string input_ = "-";
	std::string output_ = "-";
	std::FILE* fp_input_ = stdin;
	std::FILE* fp_output_ = stdout;

	std::string usage(const std::string& argv0, const std::string& msg = "") const;
	void open_file();
	void close_file();
};
