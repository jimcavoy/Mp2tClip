#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Mp2tClipImpl/CommandLineParser.h>
#include "VideoDecoder.h"

#include <fstream>
#include <limits>
#include <vector>

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
    void updateClock(const lcss::TransportPacket& pckt);
    bool timeExpired();
    void onCreateClip();
    void onCreateClipWithKeyFrame();
    void onCreateClipWithoutKeyFrame();
    void onPayloadUnitStart(lcss::TransportPacket& pckt);

private:
    const ThetaStream::CommandLineParser& _cmdline;
    lcss::ProgramAssociationTable _pat{};
    lcss::ProgramMapTable _pmt{};
    std::ofstream _ofile{};
    int _fileCount{ 1 };
    uint64_t _duration{ std::numeric_limits<uint64_t>::max() };
    lcss::TransportPacket _patPacket{};
    std::vector<lcss::TransportPacket> _pmtPackets{};
    PmtProxy _pmtProxy{};
    VideoDecoder _videoDecoder{};
    PCRClock _pcrClock;
    AccessUnit _previousLabelAU;
    AccessUnit _nextLabelAU;
    AccessUnit _currentAU;
    uint64_t _length{};
    uint64_t _offset{};
    std::vector<AccessUnit> _segment;
};

