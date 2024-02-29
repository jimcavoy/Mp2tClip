#include "Mpeg2TsDecoder.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <libgen.h>
#endif // !_WIN32

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <cmath>

using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
#define sprintf sprintf_s
#else
#define _MAX_PATH 512
#endif

namespace
{
	const uint8_t PTS_DTS_MASK = 0xC0;
	char TAG_HDMV[] = { (char)0x48, (char)0x44, (char)0x4D, (char)0x56, (char)0xFF, (char)0x1B, (char)0x44, (char)0x3F, 0 };
	char TAG_HDPR[] = { (char)0x48, (char)0x44, (char)0x50, (char)0x52, (char)0xFF, (char)0x1B, (char)0x67, (char)0x3F, 0 };

	void PrintTimestamp(const lcss::PESPacket& pes)
	{
		UINT16 pts_dts_flag = (pes.flags2() & PTS_DTS_MASK);

		if (pts_dts_flag == 0xC0)
		{
			cout.precision(12);
			std::stringstream pts;
			std::stringstream dts;
			pts.precision(12);
			dts.precision(12);

			pts << pes.pts() << "/90-kHz = " << pes.ptsInSeconds() << " seconds";
			dts << pes.dts() << "/90-kHz = " << pes.dtsInSeconds() << " seconds";
			cout << pts.str() << endl << dts.str() << endl;
		}
		else if (pts_dts_flag == 0x80)
		{
			cout.precision(12);
			std::stringstream pts;
			pts.precision(12);

			pts << pes.pts() << "/90-kHz = " << pes.ptsInSeconds() << " seconds";
			cout << pts.str() << endl;
		}
	}

	std::string create_timestamp()
	{
		char buffer[128]{};
#ifdef _WIN32
		SYSTEMTIME lt;
		GetLocalTime(&lt);

		sprintf(buffer, "%d%02d%02d%02d%02d%02d%03d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);
#else
		struct timespec spec;
		clock_gettime(CLOCK_REALTIME, &spec);
		struct tm timeinfo;
		localtime_r(&spec.tv_sec, &timeinfo);
		long ms = round(spec.tv_nsec / 1.0e6);

		sprintf(buffer, "%d%02d%02d%02d%02d%02d%03ld",
			timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, ms);
#endif
		return std::string(buffer);
	}

	string MakeFilename(const string& path)
	{
		string ret;
#ifdef _WIN32
		// Extract name
		char drive[_MAX_DRIVE]{};
		char dir[_MAX_DIR]{};
		char fname[_MAX_FNAME]{};
		char ext[_MAX_EXT]{};
		char newfname[_MAX_PATH]{};

		_splitpath_s(path.c_str(), drive, dir, fname, ext);

		string ts = create_timestamp();
		sprintf_s(newfname, "%s_%s%s", fname, ts.c_str(), ext);
		ret = newfname;
#else
		char newfname[512]{};
		char* bname = basename((char*)path.c_str());
		char fname[128]{};
		int i = 0;
		char c = bname[i];
		while (c != '.')
		{
			fname[i] = c;
			c = bname[i++];
		}

		string ts = create_timestamp();
		sprintf(newfname, "%s_%s%s", fname, ts.c_str(), ".ts");
		ret = newfname;
#endif
		return ret;
	}

	void createOutputDir(const std::string& strDirname)
	{
		std::error_code errc{};
		auto curdir = fs::current_path();
		fs::path dirname(strDirname);
		fs::path dirpath = curdir / dirname;
		if (!fs::exists(dirpath))
		{
			if (!fs::create_directory(dirpath, errc))
			{
				std::stringstream msg;
				msg << "Fail to create output directory, " << strDirname;
				std::runtime_error ex(msg.str().c_str());
				throw ex;
			}
		}
	}
}

Mpeg2TsDecoder::Mpeg2TsDecoder(const ThetaStream::CommandLineParser& cmdline)
	: _cmdline(cmdline)
{
	createOutputDir(cmdline.outputDirectory());
	createClippedFile();
}

Mpeg2TsDecoder::~Mpeg2TsDecoder()
{
	close();
}

void Mpeg2TsDecoder::onPacket(lcss::TransportPacket& pckt)
{
	const uint8_t* data = pckt.getData();

	updateClock(pckt);

	if (pckt.payloadUnitStart())
	{
		if (pckt.PID() == 0) // Program Association Table
		{
			_pat.parse(data);
			_patPacket = pckt;
		}
		else if (_pat.find(pckt.PID()) != _pat.end()) // Program Specific Information Table, chapter 2.4.4
		{
			auto it = _pat.find(pckt.PID());
			if (it->second > 0)
			{
				_pmt = lcss::ProgramMapTable(data, pckt.data_byte());
				if (_pmt.parse())
				{
					_pmtProxy.update(_pmt);
				}
				_pmtPackets.clear();
				_pmtPackets.push_back(pckt);
			}
		}
		else
		{
			lcss::PESPacket pes;
			UINT16 bytesParsed = pes.parse(data);
			if (bytesParsed > 0)
			{
				if (_pmtProxy.packetType(pckt.PID()) == PmtProxy::STREAM_TYPE::$EXI)
				{
					if (_nextAU.length() > 0)
					{
						if (_previousAU.length() != 0 &&
							_previousAU != _nextAU &&
							_cmdline.breakOnLabelChange())
						{
							_labelChanged = true;
						}

						_previousAU = _nextAU;
					}
					_nextAU.clear();
					_nextAU.insert(data + bytesParsed, pckt.data_byte() - bytesParsed);
				}
			}
		}
		_ofile.flush();
	}
	else
	{
		auto it = _pat.find(pckt.PID());
		if (it != _pat.end() && it->second > 0)
		{
			_pmt.add(data, pckt.data_byte());
			if (_pmt.parse())
			{
				_pmtProxy.update(_pmt);
			}
			_pmtPackets.push_back(pckt);
		}

		if (_pmtProxy.packetType(pckt.PID()) == PmtProxy::STREAM_TYPE::$EXI)
		{
			_nextAU.insert(data, pckt.data_byte());
		}
	}

	switch (_pmtProxy.packetType(pckt.PID()))
	{
	case PmtProxy::STREAM_TYPE::H264:
		_videoDecoder.parse(pckt.data(), (uint32_t)pckt.length());
		break;
	}

	writePacket(pckt);
}

void Mpeg2TsDecoder::close()
{
	if (_ofile.is_open())
	{
		_ofile.close();
	}
}

void Mpeg2TsDecoder::createClippedFile()
{
	string path = _cmdline.outputDirectory();
	string fname = _cmdline.outputFilename();
	if (_cmdline.filename() != "-")
	{
		fname = MakeFilename(_cmdline.filename());
	}
	else
	{
		char newfname[_MAX_PATH]{};
		std::string ts = create_timestamp();
		sprintf(newfname, "%s_%s%s", _cmdline.outputFilename().c_str(), ts.c_str(), ".ts");
		fname = newfname;
	}
#ifdef _WIN32
	path += "\\";
#else
	path += "/";
#endif
	path += fname;

	if (_ofile.is_open())
	{
		_ofile.close();
	}

	_ofile.open(path, std::ios::out | std::ios::binary);
	if (!_ofile.is_open())
	{
		char szErr[_MAX_PATH]{};
		sprintf(szErr, "Failed to open output file %s", path.c_str());
		std::runtime_error exp(szErr);
		throw exp;
	}
	cerr << "Created clipped file " << fname << endl;
}

void Mpeg2TsDecoder::writePacket(lcss::TransportPacket& pckt)
{
	const size_t size = pckt.length();
	const UINT32 offset = _cmdline.offset();
	const double pcrTime = _pcrClock.timeInSeconds();

	if (offset > 0 && offset > pcrTime)
	{
		return;
	}

	if (!_ofile.is_open())
	{
		createClippedFile();
	}

	if ((timeExpired() || _labelChanged) && _videoDecoder.hasKeyFrame())
	{
		createClippedFile();
		_ofile.write((const char*)_patPacket.data(), _patPacket.length());
		for (const auto& p : _pmtPackets)
		{
			_ofile.write((const char*)p.data(), p.length());
		}
		_duration = pcrTime + _cmdline.length();
		_videoDecoder.reset();
		_labelChanged = false;
	}

	if (size > 0)
	{
		_ofile.write((const char*)pckt.data(), size);
	}
}

void Mpeg2TsDecoder::updateClock(const lcss::TransportPacket& pckt)
{
	const char afe = pckt.adaptationFieldExist();
	if (afe == 0x02 || afe == 0x03)
	{
		const lcss::AdaptationField* adf = pckt.getAdaptationField();
		if (adf != nullptr && adf->length() > 0 && adf->PCR_flag())
		{
			uint8_t pcr[6]{};
			adf->getPCR(pcr);
			_pcrClock.setTime(pcr);

			if (_duration == std::numeric_limits<double>::max() && _cmdline.offset() < _pcrClock.timeInSeconds())
			{
				_duration = _pcrClock.timeInSeconds() + _cmdline.length();
			}
		}
	}
}

bool Mpeg2TsDecoder::timeExpired() const
{
	if (_duration == std::numeric_limits<double>::max())
	{
		return false;
	}

	double diff = _duration - _pcrClock.timeInSeconds();
	if (diff < 0.5)
	{
		return true;
	}
	return false;
}

