#include "Mpeg2TsDecoder.h"

#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdlib.h>

using namespace std;

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

	string MakeFilename(const string& basename, int count)
	{
		string ret;
		// Extract name
		char drive[_MAX_DRIVE]{};
		char dir[_MAX_DIR]{};
		char fname[_MAX_FNAME]{};
		char ext[_MAX_EXT]{};
		char newfname[_MAX_PATH]{};

		_splitpath_s(basename.c_str(), drive, dir, fname, ext);

		sprintf_s(newfname, "%s_%03d%s", fname, count, ext);
		ret = newfname;
		return ret;
	}
}

Mpeg2TsDecoder::Mpeg2TsDecoder(const ThetaStream::CommandLineParser& cmdline)
	: _cmdline(cmdline)
{
	createClippedFile();
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
				_pmt.clear();
				_pmt.add(data, pckt.data_byte());
				if (_pmt.parse())
				{
					_pmtProxy.update(_pmt);
				}
				_pmtPackets.clear();
				_pmtPackets.push_back(pckt);
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
	}

	switch (_pmtProxy.packetType(pckt.PID()))
	{
	case PmtProxy::STREAM_TYPE::H264:
		_videoDecoder.parse((const char*)pckt.data(), (UINT32)pckt.length());
		break;
	}

	writePacket(pckt);
}

void Mpeg2TsDecoder::createClippedFile()
{
	string path = _cmdline.outputDirectory();
	string fname = _cmdline.outputFilename();
	if (_cmdline.filename() != "-")
	{
		fname = MakeFilename(_cmdline.filename(), _fileCount++);
	}
	else
	{
		char newfname[_MAX_PATH]{};
		sprintf_s(newfname, "%s_%03d%s", _cmdline.outputFilename().c_str(), _fileCount++, ".ts");
		fname = newfname;
	}
	path += "\\";
	path += fname;

	if (_ofile.is_open())
	{
		_ofile.close();
	}

	_ofile.open(path, std::ios::out | std::ios::binary);
	if (!_ofile.is_open())
	{
		char szErr[BUFSIZ]{};
		sprintf_s(szErr, "Failed to open output file %s", path.c_str());
		std::exception exp(szErr);
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

	if (timeExpired() && _videoDecoder.hasKeyFrame())
	{
		createClippedFile();
		_ofile.write((const char*)_patPacket.data(), _patPacket.length());
		for (const auto& p : _pmtPackets)
		{
			_ofile.write((const char*)p.data(), p.length());
		}
		_duration = pcrTime + _cmdline.length();
		_videoDecoder.reset();
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
