// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include <string>
#include <vector>

#include "json.hpp"

class ChSet
{
public:
	ChSet() :
		number_(0),
		frequency_idx_(0),
		frequency_khz_(0),
		transport_stream_id_(8, 0xffff)
	{}
	virtual ~ChSet() = default;

	enum class Satellite
	{
		BS,
		CS
	};

	const std::string& transponder() const { return transponder_; }
	int32_t number() const { return number_; }
	int32_t frequency_idx() const { return frequency_idx_; }
	uint32_t frequency_khz() const { return frequency_khz_; }
	uint32_t frequency_if_khz() const { return frequency_khz_ - 10678000; }
	bool has_lock() const { return has_lock_; }
	const std::vector<uint16_t>& transport_stream_id() const { return transport_stream_id_; }
	uint16_t transport_stream_id(size_t tsnum) const { return transport_stream_id_.at(tsnum); }

	void init(Satellite kind, int32_t idx);
	bool set_transport_stream_id(uint16_t tsid);
	void sort_relative_ts_number(bool is_sorting);
	void set_from_json(const nlohmann::json& j);

protected:
	std::string transponder_;
	int32_t number_ = 0;
	int32_t frequency_idx_ = 0;
	uint32_t frequency_khz_ = 0;
	bool has_lock_ = false;
	std::vector<uint16_t> transport_stream_id_;
};

class ChSets
{
public:
	ChSets() :
		chsets_bs_(12),
		chsets_cs_(12)
	{
		init();
	}
	~ChSets() = default;

	void clear();
	bool set_transport_stream_id(uint16_t tsid);
	void sort_relative_ts_number(bool is_sorting);
	nlohmann::json json() const;
	const std::vector<ChSet>& bs() const { return chsets_bs_; };
	const std::vector<ChSet>& cs() const { return chsets_cs_; };

private:
	std::vector<ChSet> chsets_bs_;
	std::vector<ChSet> chsets_cs_;

	void init();
};

void to_json(nlohmann::json& j, const ChSet& p);
void from_json(const nlohmann::json& j, ChSet& p);
