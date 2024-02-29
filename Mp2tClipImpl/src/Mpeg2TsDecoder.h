#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Mp2tClipImpl/CommandLineParser.h>
#include "VideoDecoder.h"

#include <memory>
#include <fstream>
#include <limits>

#include <mp2tp/libmp2tp.h>

#include "PmtProxy.h"
#include "PCRClock.h"
#include "AccessUnit.h"

class Mpeg2TsDecoder : public lcss::TSParser
{
public:
	Mpeg2TsDecoder(const ThetaStream::CommandLineParser& cmdline);
	virtual ~Mpeg2TsDecoder();

	void onPacket(lcss::TransportPacket& pckt) override;

	void close();

private:
	void createClippedFile();
	void writePacket(lcss::TransportPacket& pckt);
	void updateClock(const lcss::TransportPacket& pckt);
	bool timeExpired() const;

private:
	const ThetaStream::CommandLineParser& _cmdline;
	lcss::ProgramAssociationTable _pat{};
	lcss::ProgramMapTable _pmt{};
	std::ofstream _ofile{};
	int _fileCount{1};
	double _duration{std::numeric_limits<double>::max()};
	lcss::TransportPacket _patPacket{};
	std::vector<lcss::TransportPacket> _pmtPackets{};
	PmtProxy _pmtProxy{};
	VideoDecoder _videoDecoder{};
	PCRClock _pcrClock;
	AccessUnit _previousAU;
	AccessUnit _nextAU;
	bool _labelChanged{ false };
};

