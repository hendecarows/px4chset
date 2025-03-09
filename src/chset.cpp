// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <sstream>

#include <iostream>

#include "json.hpp"
#include "chset.h"

void ChSet::init(Satellite kind, int32_t tpnum)
{
	if (kind == Satellite::BS)
	{
		if ((tpnum >= 1) && (tpnum <= 23) && ((tpnum % 2) == 1))
		{
			// BS
			std::ostringstream os;
			os << "BS" << tpnum;
			transponder_ = os.str();
			number_ = tpnum;
			frequency_idx_ = (tpnum - 1) / 2;
			frequency_khz_ = 11727480 + frequency_idx_ * 38360;
			has_lock_ = false;
		}
	}
	else
	{
		if ((tpnum >= 2) && (tpnum <= 24) && ((tpnum % 2) == 0))
		{
			// CS
			std::ostringstream os;
			os << "ND" << tpnum;
			transponder_ = os.str();
			number_ = tpnum;
			frequency_idx_ = (tpnum - 2) / 2 + 12;
			frequency_khz_ = 12291000 + (frequency_idx_ - 12) * 40000;
			has_lock_ = false;
		}
	}
}

bool ChSet::set_transport_stream_id(uint16_t tsid)
{
	if (transport_stream_id_.size() == 0)
	{
		transport_stream_id_.resize(8, 0xffff);
	}

	if (tsid == 0 || tsid == 0xffff)
	{
		return false;
	}

	// BS1 to BS23
	// ND2 to ND24
	auto tpnum = (tsid & 0x01f0) >> 4;
	if (tpnum < 1 || tpnum > 24)
	{
		return false;
	}

	has_lock_ = true;

	auto tsnum = (tsid & 0x07);
	transport_stream_id_.at(tsnum) = tsid;
	auto result = std::find(transport_stream_id_.begin(), transport_stream_id_.end(), tsid);
	if (result == transport_stream_id_.end())
	{
		transport_stream_id_.at(tsnum) = tsid;
	}

	return true;
}

void ChSet::sort_relative_ts_number(int32_t method)
{
	switch (method)
	{
	case 1:
	{
		// TMCC内のTS相対番号順にTSIDを並べ替える。
		// NITから得られる情報ではTMCC内の相対TS番号がわからないため、有効なTSIDがゼロから始まる様に並べ替える。
		// 並べ替え前：[0xffff, 0x40f1, 0x40f2, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
		// 並べ替え後：[0x40f1, 0x40f2, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
		auto tsids = transport_stream_id_;
		auto result = std::find_if(tsids.begin(), tsids.end(), [](uint16_t x) { return x != 0xffff; });
		if (result == tsids.begin() || result == tsids.end())
		{
			// TSIDの相対TS番号がゼロから始まる場合、有効なTSIDが存在しない場合は何もしない。
			// [0x40f1, 0x40f2, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
			// [0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
		}
		else
		{
			// TSIDの相対TS番号がゼロ以外から始まる場合はゼロから始まる様に並べ替える。
			// 有効なTSID間に無効なTSIDがある場合は間を詰めずそのままずらす。
			// 並べ替え前：[0xffff, 0x40f1, 0xffff, 0x40f3, 0xffff, 0xffff, 0xffff, 0xffff]
			// 並べ替え後：[0x40f1, 0xffff, 0x40f3, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
			std::fill(transport_stream_id_.begin(), transport_stream_id_.end(), 0xffff);
			std::copy(result, tsids.end(), transport_stream_id_.begin());
		}
		break;
	}
	case 2:
	{
		// TMCC内のTS相対番号順にTSIDを並べ替える。
		// NITから得られる情報ではTMCC内の相対TS番号がわからないため、有効なTSIDがゼロから始まる様に並べ替える。
		// 並べ替え前：[0xffff, 0x40f1, 0xffff, 0x40f3, 0xffff, 0xffff, 0xffff, 0xffff]
		// 並べ替え後：[0x40f1, 0x40f3, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff]
		std::vector<uint16_t> tsids;
		for (auto tsid : transport_stream_id_)
		{
			if (tsid != 0xffff)
			{
				tsids.emplace_back(tsid);
			}
		}
		tsids.resize(transport_stream_id_.size(), 0xffff);
		transport_stream_id_ = tsids;
		break;
	}
	default:
	{
		// TSIDの下位3ビットから求まる値をTMCC内の相対TS番号として設定。
		// BS15の例外があるため、相対TS番号に意味がある場合は使用しない。
		auto tsids = transport_stream_id_;
		std::fill(transport_stream_id_.begin(), transport_stream_id_.end(), 0xffff);
		for (auto tsid : tsids)
		{
			auto tsnum = (tsid & 0x07);
			transport_stream_id_.at(tsnum) = tsid;
		}
	}
	}
}

void ChSet::set_from_json(const nlohmann::json& j)
{
	transponder_ = j.at("transponder").get<std::string>();
	number_ = j.at("number").get<int32_t>();
	frequency_idx_ = j.at("frequency_idx").get<int32_t>();
	frequency_khz_ = j.at("frequency_khz").get<uint32_t>();
	has_lock_ =  j.at("has_lock").get<bool>();
	transport_stream_id_ = j.at("transport_stream_id").get<std::vector<uint16_t>>();
}


void ChSets::init()
{
	static const std::vector<uint16_t> TSIDS_CS{
		0x6020,
		0x7040,
		0x7060,
		0x6080,
		0x60a0,
		0x70c0,
		0x70e0,
		0x7100,
		0x7120,
		0x7140,
		0x7160,
		0x7180,
	};

	// BS
	auto tpnum = 1;
	for (auto& p : chsets_bs_)
	{
		p.init(ChSet::Satellite::BS, tpnum);
		tpnum += 2;
	}

	// CS
	// BSのTSファイルからはCSの情報は得られないのでプリセット情報を追加
	tpnum = 2;
	for (auto& p : chsets_cs_)
	{
		p.init(ChSet::Satellite::CS, tpnum);
		tpnum += 2;
	}

	for (const auto& t : TSIDS_CS)
	{
		set_transport_stream_id(t);
	}
}

void ChSets::clear()
{
	chsets_bs_.clear();
	chsets_cs_.clear();
}

bool ChSets::set_transport_stream_id(uint16_t tsid)
{
	if (tsid == 0 || tsid == 0xffff)
	{
		return false;
	}

	// BS1 to BS23
	// ND2 to ND 24
	auto tpnum = (tsid & 0x01f0) >> 4;
	if (tpnum < 1 || tpnum > 24)
	{
		return false;
	}

	if (tpnum % 2 != 0)
	{
		// BS
		auto idx = (tpnum - 1) / 2;
		auto& c = chsets_bs_.at(idx);
		c.set_transport_stream_id(tsid);
	}
	else
	{
		// CS
		auto idx = (tpnum - 2) / 2;
		auto& c = chsets_cs_.at(idx);
		c.set_transport_stream_id(tsid);
	}

	return true;
}

void ChSets::sort_relative_ts_number(int32_t method)
{
	for (auto& c : chsets_bs_)
	{
		c.sort_relative_ts_number(method);
	}
}

nlohmann::json ChSets::json() const
{
	auto j = nlohmann::json::object();
	j["BS"] = nlohmann::json::array();
	j["CS"] = nlohmann::json::array();

	for (const auto& p : chsets_bs_)
	{
		j.at("BS").emplace_back(p);
	}

	for (const auto& p : chsets_cs_)
	{
		j.at("CS").emplace_back(p);
	}

	return j;
}

void to_json(nlohmann::json& j, const ChSet& p)
{
	j = nlohmann::json{
		{"transponder", p.transponder()},
		{"number", p.number()},
		{"frequency_idx", p.frequency_idx()},
		{"frequency_khz", p.frequency_khz()},
		{"frequency_if_khz", p.frequency_if_khz()},
		{"has_lock", p.has_lock()},
		{"transport_stream_id", p.transport_stream_id()},
	};
}

void from_json(const nlohmann::json& j, ChSet& p)
{
	p.set_from_json(j);
}

