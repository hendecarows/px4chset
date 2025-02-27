// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "chset.h"

class Convert
{
public:
	Convert() = delete;
	~Convert() = delete;

	static std::string dump(const std::string& format, const ChSets& chsets);
	static bool has_format(const std::string& format);
	static bool is_sorting(const std::string& format);
	static std::string get_filename(const std::string& format);

private:
	static std::string json(const ChSets& chsets);
	static std::string libdvbv5(const ChSets& chsets);
	static std::string libdvbv5lnb(const ChSets& chsets);
	static std::string mirakurun(const ChSets& chsets);
	static std::string bondriver_dvb(const ChSets& chsets);
	static std::string bondriver_pt(const ChSets& chsets);
	static std::string bondriver_ptx(const ChSets& chsets);
	static std::string bondriver_px4(const ChSets& chsets);
	static std::string bondriver_px3(const ChSets& chsets);
	static std::string bondriver_bda(const ChSets& chsets);
	static std::string bondriver_plex_px(const ChSets& chsets);

	struct Items
	{
		std::function<std::string(const ChSets&)> function;		// 変換関数
		std::string filename;									// 出力ファイル名
		bool sorting;											// 相対TS番号による並び替え
	};

	static const inline std::unordered_map<std::string, Items> convert_
	{
		{"json", {Convert::json, "tsids%y%m%d.json", true}},
		{"dvbv5", {Convert::libdvbv5, "dvbv5_channels_isdbs.conf", true}},
		{"dvbv5lnb", {Convert::libdvbv5lnb, "dvbv5_channels_isdbs_lnb.conf", true}},
		{"mirakurun", {Convert::mirakurun, "channels%y%m%d.yml", true}},
		{"bondvb", {Convert::bondriver_dvb, "BonDriver_DVB-S.conf", true}},
		{"bonpt", {Convert::bondriver_pt, "BonDriver_LinuxPT-S.conf", true}},
		{"bonptx", {Convert::bondriver_ptx, "BonDriver_LinuxPTX-S.ini", true}},
		{"bonpx4", {Convert::bondriver_px4, "BonDriver_PX4-S.ChSet.txt", false}},
		{"bonpx3", {Convert::bondriver_px3, "BonDriver_PX3_S.ini", false}},
		{"bonbda", {Convert::bondriver_bda, "BonDriver_BDA_S.ini", false}},
		{"bonplexpx", {Convert::bondriver_plex_px, "BonDriver_PlexPX_S.ini", false}},
	};
};
