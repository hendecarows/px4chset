// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctime>

#include <chrono>
#include <iomanip>
#include <ios>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

#include "iconv.hpp"

#include "chset.h"
#include "convert.h"

std::string Convert::dump(const std::string& format, const ChSets& chsets)
{
	if (convert_.count(format))
	{
		return convert_.at(format).function(chsets);
	}

	throw std::runtime_error("failed to dump invalid format");
}

bool Convert::has_format(const std::string& format)
{
	return convert_.count(format) ? true : false;
}

bool Convert::is_sorting(const std::string& format)
{
	return convert_.count(format) ? convert_.at(format).sorting : false;
}

std::string Convert::get_filename(const std::string& format)
{
	const auto& filename = convert_.at(format).filename;

	if (filename.find('%') != std::string::npos)
	{
		auto now = std::chrono::system_clock::now();
		auto t = std::chrono::system_clock::to_time_t(now);
#if defined(_WIN32)
		std::tm tm;
		auto err = localtime_s(&tm, &t);
#else
		auto tm = *std::localtime(&t);
#endif
		std::ostringstream os;
		os << std::put_time(&tm, filename.c_str());
		return os.str();
	}

	return filename;
}

std::string Convert::json(const ChSets& chsets)
{
	return chsets.json().dump(4);
}

std::string Convert::libdvbv5(const ChSets& chsets)
{
	std::ostringstream os;

	if (chsets.bs().size() > 0)
	{
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "[BS" << std::setw(2) << std::setfill('0') << c.number() << "_" << tsnum << "]\n"
					<< "\tDELIVERY_SYSTEM = ISDBS\n"
					<< "\tFREQUENCY = " << c.frequency_if_khz() << '\n'
					<< "\tSTREAM_ID = " << tsid << '\n';
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsnum = 0;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "[CS" << c.number() << "]\n"
				<< "\tDELIVERY_SYSTEM = ISDBS\n"
				<< "\tFREQUENCY = " << c.frequency_if_khz() << '\n'
				<< "\tSTREAM_ID = " << tsid << '\n';
		}
	}

	return os.str();
}

std::string Convert::libdvbv5lnb(const ChSets& chsets)
{
	std::ostringstream os;

	if (chsets.bs().size() > 0)
	{
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "[BS" << std::setw(2) << std::setfill('0') << c.number() << "_" << tsnum << "]\n"
					<< "\tDELIVERY_SYSTEM = ISDBS\n"
					<< "\tLNB = 110BS\n"
					<< "\tFREQUENCY = " << c.frequency_khz() << '\n'
					<< "\tSTREAM_ID = " << tsid << '\n';
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsnum = 0;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "[CS" << c.number() << "]\n"
				<< "\tDELIVERY_SYSTEM = ISDBS\n"
				<< "\tLNB = 110BS\n"
				<< "\tFREQUENCY = " << c.frequency_khz() << '\n'
				<< "\tSTREAM_ID = " << tsid << '\n';
		}
	}

	return os.str();
}

std::string Convert::mirakurun(const ChSets& chsets)
{
	std::ostringstream os;

	if (chsets.bs().size() > 0)
	{
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "- name: BS" << std::setw(2) << std::setfill('0') << c.number() << '_' << tsnum << '\n'
					<< "  type: BS\n"
					<< "  channel: BS" << std::setw(2) << std::setfill('0') << c.number() << '_' << tsnum << '\n'
					<< "  isDisabled: false\n";
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsnum = 0;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "- name: CS" << c.number() << '\n'
				<< "  type: CS\n"
				<< "  channel: CS"  << c.number() << '\n'
				<< "  isDisabled: false\n";
		}
	}

	return os.str();
}

std::string Convert::bondriver_dvb(const ChSets& chsets)
{
	auto bonch = 0;
	auto is_start_cs = false;
	std::ostringstream os;

	os << "#ISDB_S\n";

	if (chsets.bs().size() > 0)
	{
		os << "; BS\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< '\t' << bonch
					<< '\t' << c.frequency_idx()
					<< '\t' << "0x" << std::hex << tsid << std::dec << '\n';
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto tsnum = 0;
		os << "\n; CS110\n";
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "ND"  << std::setw(2) << std::setfill('0') << c.number()
				<< '\t' << bonch
				<< '\t' << c.frequency_idx()
				<< '\t' << "0x" << std::hex << tsid << std::dec << '\n';
			bonch++;
		}
	}

	return os.str();
}

std::string Convert::bondriver_pt(const ChSets& chsets)
{
	auto bonch = 0;
	std::ostringstream os;

	os << "#ISDB_S\n";

	if (chsets.bs().size() > 0)
	{
		os << "; BS\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< '\t' << bonch
					<< '\t' << c.frequency_idx()
					<< '\t' << tsnum << '\n';
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto tsnum = 0;
		os << "\n; CS110\n";
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "ND" << std::setw(2) << std::setfill('0') << c.number()
				<< '\t' << bonch
				<< '\t' << c.frequency_idx()
				<< '\t' << tsnum << '\n';
			bonch++;
		}
	}

	return os.str();
}

std::string Convert::bondriver_ptx(const ChSets& chsets)
{
	auto space = 0;
	auto bonch = 0;
	std::ostringstream os;

	if (chsets.bs().size() > 0)
	{
		os << "[Space.BS]\n"
			"Name=BS\n"
			"System=ISDB-S\n\n"
			"[Space.BS.Channel]\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "Ch" << bonch << '='
					<< "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< ',' << c.frequency_idx()
					<< ',' << tsnum << '\n';
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto tsnum = 0;
		bonch = 0;
		os << "\n[Space.CS110]\n"
			"Name=CS110\n"
			"System=ISDB-S\n\n"
			"[Space.CS110.Channel]\n";
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "Ch" << bonch << '='
				<< "ND" << std::setw(2) << std::setfill('0') << c.number() << "/TS0"
				<< ',' << c.frequency_idx()
				<< ',' << tsnum << '\n';
			bonch++;
		}
	}

	return os.str();
}

std::string Convert::bondriver_px4(const ChSets& chsets)
{
	auto space = 0;
	auto bonch = 0;
	std::ostringstream os;

	os << ";\r\n"
		u8"; BonDriver_PX4 チャンネル定義ファイル (ISDB-S) (日本における衛星波デジタル放送用)\r\n"
		u8"; (BonDriver_PT3-STのChSet.txtと互換性あり)\r\n"
		";\r\n"
		u8"; チャンネル空間定義 ($チャンネル空間名<TAB>チャンネル空間ID)\r\n"
		"$BS\t0\r\n"
		"$CS110\t1\r\n"
		";\r\n"
		u8"; チャンネル定義 (チャンネル名<TAB>チャンネル空間ID<TAB>チャンネルID<TAB>PTX内部チャンネルID<TAB>TSID(ISDB-S用))\r\n";

	if (chsets.bs().size() > 0)
	{
		os << "; [BS]\r\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< '\t' << space
					<< '\t' << bonch
					<< '\t' << c.frequency_idx()
					<< '\t' << tsid << "\r\n";
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto tsnum = 0;
		space = 1;
		bonch = 0;
		os << "; [CS]\r\n";
		for (const ChSet& c : chsets.cs())
		{
			if (!c.has_lock()) continue;
			auto tsid = c.transport_stream_id(tsnum);
			if (tsid == 0xffff) continue;
			os << "ND" << std::setw(2) << std::setfill('0') << c.number()
				<< '\t' << space
				<< '\t' << bonch
				<< '\t' << c.frequency_idx()
				<< '\t' << tsid << "\r\n";
			bonch++;
		}
	}

	// 出力encoding: CP932
	// 入力encoding: UTF-8
	std::string output;
	iconvpp::converter conv("CP932", "UTF-8", true);
	conv.convert(os.str(), output);
	return output;
}

std::string Convert::bondriver_px3(const ChSets& chsets)
{
	const std::vector<int> Col = { 237, 100, 221, 55, 228, 260, 251, 290, 240, 307, 160, 257 };
	std::ostringstream os;

	if (chsets.bs().size() > 0)
	{
		auto bonch = 0;
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				bonch++;
			}
		}
		os << "[BS]\r\n" << "CHANNEL_NUM = " << bonch << "\r\n";
		bonch = 1;
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				// CH001 = BS01/TS0,0,11727480,0x4010
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "CH" << std::setw(3) << std::setfill('0') << bonch << " = "
					<< "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< ",0," << c.frequency_khz() << ",0x" << std::hex << tsid << std::dec << "\r\n";
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto bonch = 1;
		os << "\r\n[CS110]\r\n" << "CHANNEL_NUM = " << chsets.cs().size() << "\r\n";
		for (const ChSet& c : chsets.cs())
		{
			// CH001 = ND02,237,12291000,0x0
			if (!c.has_lock()) continue;
			os << "CH" << std::setw(3) << std::setfill('0') << bonch << " = "
				<< "ND" << std::setw(2) << std::setfill('0') << c.number() << ','
				<< std::setw(3) << std::setfill('0') << Col.at(bonch - 1) << ','
				<< c.frequency_khz() << ",0x0\r\n";
			bonch++;
		}
	}

	// 出力encoding: UTF-16LE
	// 入力encoding: UTF-8
	std::string output;
	iconvpp::converter conv("UTF-16LE", "UTF-8", true);
	conv.convert(os.str(), output);

	// BOM LE追加
	output.insert(0, "\xff\xfe");

	return output;
}

std::string Convert::bondriver_bda(const ChSets& chsets)
{
	std::ostringstream os;

	os << "[Tuner]\r\n"
		"Guid=\r\n"
		"FriendlyName0=\r\n"
		"Name=\"ISDB-S\"\r\n"
		"DVBSystemType=\"DVB-T\"\r\n"
		"DefaultNetwork=\"BS/CS110\"\r\n"
		"\r\n"
		"[BonDriver]\r\n"
		"DebugLog=NO\r\n"
		u8"; 録画用として使用する場合でCPU高負荷時等バッファーオーバーランによるDropが発生する場合は下記を調整すると効果があるかもしれません\r\n"
		";ThreadPriorityStream=\"THREAD_PRIORITY_HIGHEST\"\r\n"
		"\r\n"
		"[Channel]\r\n"
		"ReserveUnusedCh=NO\r\n"
		"\r\n";

	if (chsets.bs().size() > 0)
	{
		auto bonch = 0;
		os << "[TuningSpace00]\r\n"
			"TuningSpaceName=\"BS\"\r\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				// CH000=1,11727.480,R,0,BS01/TS0,0x0,0x4010
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "CH" << std::setw(3) << std::setfill('0') << bonch << '='
					<< "1," << std::fixed << std::setprecision(3) << c.frequency_khz() / 1000.0
					<< ",R,0," << "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< ",0x0" << ',' << "0x" << std::hex << tsid << std::dec << "\r\n";
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto bonch = 0;
		os << "\r\n[TuningSpace01]\r\n"
			"TuningSpaceName=\"CS110\"\r\n";
		for (const ChSet& c : chsets.cs())
		{
			// CH000=1,12291.000,R,0,ND02/TS0,0x0,0x0
			if (!c.has_lock()) continue;
			os << "CH" << std::setw(3) << std::setfill('0') << bonch << '='
				<< "1," << std::fixed << std::setprecision(3) << c.frequency_khz() / 1000.0
				<< ",R,0," << "ND" << std::setw(2) << std::setfill('0') << c.number() << "/TS0"
				<< ",0x0,0x0\r\n";
			bonch++;
		}
	}

	// 出力encoding: CP932
	// 入力encoding: UTF-8
	std::string output;
	iconvpp::converter conv("CP932", "UTF-8", true);
	conv.convert(os.str(), output);
	return output;
}

std::string Convert::bondriver_plex_px(const ChSets& chsets)
{
	std::ostringstream os;

	os << u8"; PX-Q3PE/W3PE/W3PEV2/W3U3/W3U3V2/W3U2/S3U2用の衛星放送iniファイル\r\n"
		"[PlexPX]\r\n"
		"UseKnownGUID=YES\r\n"
		"ISDB-S=YES\r\n"
		"M2_Dec=NO\r\n"
		"\r\n"
		"[Tuner]\r\n"
		"Guid=\r\n"
		"FriendlyName0=\"HDTV_PX_W3PE BDA Digital Tuner_0\"\r\n"
		"FriendlyName1=\"HDTV_PX_W3PE BDA Digital Tuner_1\"\r\n"
		"FriendlyName2=\"HDTV_PX_W3U2 BDA Digital Tuner_0\"\r\n"
		"FriendlyName3=\"HDTV_PX_S3U2 BDA Digital Tuner_0\"\r\n"
		"FriendlyName4=\"HDTV_PX_W3U3 BDA Digital Tuner_0\"\r\n"
		"FriendlyName5=\"HDTV_PX_Q3PE BDA Digital Tuner_0\"\r\n"
		"FriendlyName6=\"HDTV_PX_Q3PE BDA Digital Tuner_1\"\r\n"
		"FriendlyName7=\"HDTV_PX_Q3PE BDA Digital Tuner_4\"\r\n"
		"FriendlyName8=\"HDTV_PX_Q3PE BDA Digital Tuner_5\"\r\n"
		"FriendlyName9=\"HDTV_PX_W3U3_V2 BDA Digital Tuner_0\"\r\n"
		"FriendlyName10=\"HDTV_PX_W3U3_V2 BDA Digital Tuner_2\"\r\n"
		"FriendlyName11=\"HDTV_PX_W3PEV2 BDA Digital Tuner_0\"\r\n"
		"FriendlyName12=\"HDTV_PX_W3PEV2 BDA Digital Tuner_4\"\r\n"
		"CaptureFriendlyName0=\"HDTV_PX_W3PE BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName1=\"HDTV_PX_W3PE BDA Digital Capture_1\"\r\n"
		"CaptureFriendlyName2=\"HDTV_PX_W3U2 BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName3=\"HDTV_PX_S3U2 BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName4=\"HDTV_PX_W3U3 BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName5=\"HDTV_PX_Q3PE BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName6=\"HDTV_PX_Q3PE BDA Digital Capture_1\"\r\n"
		"CaptureFriendlyName7=\"HDTV_PX_Q3PE BDA Digital Capture_4\"\r\n"
		"CaptureFriendlyName8=\"HDTV_PX_Q3PE BDA Digital Capture_5\"\r\n"
		"CaptureFriendlyName9=\"HDTV_PX_W3U3_V2 BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName10=\"HDTV_PX_W3U3_V2 BDA Digital Capture_2\"\r\n"
		"CaptureFriendlyName11=\"HDTV_PX_W3PEV2 BDA Digital Capture_0\"\r\n"
		"CaptureFriendlyName12=\"HDTV_PX_W3PEV2 BDA Digital Capture_4\"\r\n"
		"Name=\"PlexPX-S\"\r\n"
		"UseSpecial=\"PlexPX\"\r\n"
		"DVBSystemType=\"DVB-T\"\r\n"
		"DefaultNetwork=\"UHF/CATV\"\r\n"
		"\r\n"
		"[BonDriver]\r\n"
		"DebugLog=NO\r\n"
		"BuffSize=1024\r\n"
		"MaxBuffCount=512\r\n"
		"WaitTsCount=1\r\n"
		"WaitTsSleep=100\r\n"
		u8"; 録画用として使用する場合でCPU高負荷時等バッファーオーバーランによるDropが発生する場合は下記を調整すると効果があるかもしれません\r\n"
		";ThreadPriorityDecode=\"THREAD_PRIORITY_ABOVE_NORMAL\"\r\n"
		";ThreadPriorityStream=\"THREAD_PRIORITY_HIGHEST\"\r\n"
		"\r\n"
		"[Channel]\r\n"
		"ReserveUnusedCh=NO\r\n"
		"\r\n";

	if (chsets.bs().size() > 0)
	{
		auto bonch = 0;
		os << "[TuningSpace00]\r\n"
			"TuningSpaceName=\"BS\"\r\n";
		for (const ChSet& c : chsets.bs())
		{
			if (!c.has_lock()) continue;
			for (size_t tsnum = 0; tsnum < c.transport_stream_id().size(); tsnum++)
			{
				// CH000=1,11727.480,R,0,BS01/TS0,0x0,0x4010
				auto tsid = c.transport_stream_id(tsnum);
				if (tsid == 0xffff) continue;
				os << "CH" << std::setw(3) << std::setfill('0') << bonch << '='
					<< "1," << std::fixed << std::setprecision(3) << c.frequency_khz() / 1000.0
					<< ",R,0," << "BS" << std::setw(2) << std::setfill('0') << c.number() << "/TS" << tsnum
					<< ",0x0" << ',' << "0x" << std::hex << tsid << std::dec << "\r\n";
				bonch++;
			}
		}
	}

	if (chsets.cs().size() > 0)
	{
		auto bonch = 0;
		os << "\r\n[TuningSpace01]\r\n"
			"TuningSpaceName=\"CS110\"\r\n";
		for (const ChSet& c : chsets.cs())
		{
			// CH000=1,12291.000,R,0,ND02/TS0,0x0,0x0
			if (!c.has_lock()) continue;
			os << "CH" << std::setw(3) << std::setfill('0') << bonch << '='
				<< "1," << std::fixed << std::setprecision(3) << c.frequency_khz() / 1000.0
				<< ",R,0," << "ND" << std::setw(2) << std::setfill('0') << c.number() << "/TS0"
				<< ",0x0,0x0\r\n";
			bonch++;
		}
	}

	// 出力encoding: CP932
	// 入力encoding: UTF-8
	std::string output;
	iconvpp::converter conv("CP932", "UTF-8", true);
	conv.convert(os.str(), output);
	return output;
}
