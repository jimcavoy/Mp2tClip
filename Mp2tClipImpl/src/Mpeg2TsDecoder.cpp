#include "Mpeg2TsDecoder.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <libgen.h>
#include <syslog.h>
#endif // !_WIN32

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <math.h>

using namespace std;
namespace fs = std::filesystem;

#ifdef _WIN32
#define sprintf sprintf_s
#else
#define _MAX_PATH 512
#endif

const uint64_t RESOLUTION = 27'000'000; // 27MHz

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
    , _length((cmdline.length() + 2) * RESOLUTION)
    , _offset(cmdline.offset() * RESOLUTION)
{
    createOutputDir(cmdline.outputDirectory());
}

Mpeg2TsDecoder::~Mpeg2TsDecoder()
{
    close();
}

void Mpeg2TsDecoder::onPacket(lcss::TransportPacket& pckt)
{
    const uint8_t* data = pckt.getData();

    updateClock(pckt);

    auto pcrTime = _pcrClock.time();

    if (_offset > 0 && _offset > pcrTime)
    {
        return;
    }

    if (pckt.payloadUnitStart())
    {
        onPayloadUnitStart(pckt);
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
            _currentAU.insert(pckt.data(), pckt.length());
        }

        switch (_pmtProxy.packetType(pckt.PID()))
        {
        case PmtProxy::STREAM_TYPE::$EXI:
        case PmtProxy::STREAM_TYPE::$XML:
            _nextLabelAU.insert(data, pckt.data_byte());
            _currentAU.insert(pckt.data(), pckt.length());
            break;
        default:
            _currentAU.insert(pckt.data(), pckt.length());
            break;
        }
    }
}

void Mpeg2TsDecoder::close()
{
    if (_ofile.is_open())
    {
        for (auto& au : _segment)
        {
            _ofile.write((char*)au.data(), au.length());
        }
        _segment.clear();
        _ofile.close();
    }
    else if (!_segment.empty())
    {
        createClippedFile();
        for (auto& au : _segment)
        {
            _ofile.write((char*)au.data(), au.length());
        }
        _segment.clear();
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
#ifdef linux
    syslog(LOG_NOTICE, "Created clipped file, %s", fname.c_str());
#endif
}

void Mpeg2TsDecoder::updateClock(const lcss::TransportPacket& pckt)
{
    PCRClock temp;
    const char afe = pckt.adaptationFieldExist();
    if (afe == 0x02 || afe == 0x03)
    {
        const lcss::AdaptationField* adf = pckt.getAdaptationField();
        if (adf != nullptr && adf->length() > 0 && adf->PCR_flag())
        {
            // Check for dis-continual increase in the PCR time
            uint8_t pcr[6]{};
            adf->getPCR(pcr);

            temp.setTime(pcr);

            auto pcrTime = _pcrClock.time();
            _pcrClock.setTime(pcr);
            auto newPcrTime = temp.time();

            if (pcrTime > newPcrTime)
            {
                std::cerr << "Discontinual timpstamp. Create Clip." << std::endl;
                onCreateClipWithoutKeyFrame();
                return;
            }

            if (_duration == std::numeric_limits<uint64_t>::max() && _offset < _pcrClock.time())
            {
                _duration = _pcrClock.time() + _length;
            }
        }
    }

    timeExpired();
}

bool Mpeg2TsDecoder::timeExpired()
{
    if (_duration == std::numeric_limits<uint64_t>::max())
    {
        return false;
    }

    uint64_t pcr = _pcrClock.time();
    long diff = _duration - pcr;
    diff = abs(diff);
    if (diff < 27'000'000)
    {
        onCreateClip();
        return true;
    }
    return false;
}

void Mpeg2TsDecoder::onCreateClip()
{
    if (_cmdline.keyFrame())
    {
        onCreateClipWithKeyFrame();
    }
    else
    {
        onCreateClipWithoutKeyFrame();
    }
}

void Mpeg2TsDecoder::onCreateClipWithKeyFrame()
{
    std::vector<AccessUnit> startAUs;
    bool isKey{ false };

    if (!_ofile.is_open())
    {
        createClippedFile();
    }

    // Find the last key frame which will be used to start a new clip file.
    for (std::vector<AccessUnit>::reverse_iterator it = _segment.rbegin(); it != _segment.rend();)
    {
        isKey = it->isKey();
        startAUs.insert(startAUs.begin(), *it);
        // remove the current segment;
        it = std::vector<AccessUnit>::reverse_iterator(_segment.erase((++it).base()));

        if (isKey == true)
        {
            break;
        }
    }

    // write out the current segment to the clip file
    for (auto& au : _segment)
    {
        _ofile.write((const char*)au.data(), au.length());
    }

    // Create a new clip file
    createClippedFile();

    // Add PAT and PMT add the beginning of the clip file
    _ofile.write((const char*)_patPacket.data(), _patPacket.length());
    for (const auto& p : _pmtPackets)
    {
        _ofile.write((const char*)p.data(), p.length());
    }

    // Add the key frame at the beginning of the clip file
    for (auto& au : startAUs)
    {
        _ofile.write((const char*)au.data(), au.length());
    }
    _duration = _pcrClock.time() + _length;
    _segment.clear();
}

void Mpeg2TsDecoder::onCreateClipWithoutKeyFrame()
{
    if (!_ofile.is_open())
    {
        createClippedFile();
    }

    // write out the current segment to the clip file
    for (auto& au : _segment)
    {
        _ofile.write((const char*)au.data(), au.length());
    }

    // Create a new clip file
    createClippedFile();

    // Add PAT and PMT add the beginning of the clip file
    _ofile.write((const char*)_patPacket.data(), _patPacket.length());
    for (const auto& p : _pmtPackets)
    {
        _ofile.write((const char*)p.data(), p.length());
    }

    _duration = _pcrClock.time() + _length;
    _segment.clear();
}

void Mpeg2TsDecoder::onPayloadUnitStart(lcss::TransportPacket& pckt)
{
    const uint8_t* data = pckt.getData();

    if (pckt.PID() == 0) // Program Association Table
    {
        _pat.parse(data);
        _patPacket = pckt;
        _segment.push_back(AccessUnit(pckt.data(), pckt.length()));
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
            _segment.push_back(AccessUnit(pckt.data(), pckt.length()));
        }
    }
    else
    {
        lcss::PESPacket pes;
        UINT16 bytesParsed = pes.parse(data);
        if (bytesParsed > 0)
        {
            switch (_pmtProxy.packetType(pckt.PID()))
            {
            case PmtProxy::STREAM_TYPE::$EXI:
            case PmtProxy::STREAM_TYPE::$XML:
            {
                if (_nextLabelAU.length() > 0)
                {
                    if (_previousLabelAU.length() != 0 &&
                        _previousLabelAU != _nextLabelAU &&
                        _cmdline.breakOnLabelChange())
                    {
                        onCreateClip();
                    }

                    _previousLabelAU = _nextLabelAU;
                }
                _nextLabelAU.clear();
                _nextLabelAU.insert(data + bytesParsed, pckt.data_byte() - bytesParsed);
                if (_currentAU.length() > 0)
                {
                    _segment.push_back(_currentAU);
                }
                _currentAU.clear();
                _currentAU.insert(pckt.data(), pckt.length());
                break;
            }
            case PmtProxy::STREAM_TYPE::H264:
            case PmtProxy::STREAM_TYPE::H265:
            {
                if (_currentAU.length() > 0)
                {
                    _videoDecoder.parse(_currentAU.data(), _currentAU.length());
                    if (_videoDecoder.hasKeyFrame())
                    {
                        _currentAU.toogleKey();
                    }
                    _segment.push_back(_currentAU);
                    _videoDecoder.reset();
                }
                _currentAU.clear();
                _currentAU.insert(pckt.data(), pckt.length());
                break;
            }
            default:
                if (_currentAU.length() > 0)
                {
                    _segment.push_back(_currentAU);
                }
                _currentAU.clear();
                _currentAU.insert(pckt.data(), pckt.length());
            }
        }
    }
}


