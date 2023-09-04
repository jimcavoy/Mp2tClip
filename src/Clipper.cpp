#include "Clipper.h"

#include "CommandLineParser.h"
#include "Mpeg2TsDecoder.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#endif

#include <atomic>
#include <cstdint>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

namespace 
{
	std::shared_ptr<std::istream> openInputStream(const std::string& input)
	{
		using namespace std;
		std::shared_ptr<std::istream> ifile;
		if (input == "-")
		{
#ifdef _WIN32
			_setmode(_fileno(stdin), _O_BINARY);
#endif
			ifile.reset(&cin, [](...) {});
		}
		else
		{
			ifstream* tsfile = new ifstream(input, ios::binary);
			if (!tsfile->is_open())
			{
				std::stringstream msg;
				msg << "Fail to open input file, " << input;
				std::runtime_error ex(msg.str().c_str());
				throw ex;
			}
			ifile.reset(tsfile);
		}
		return ifile;
	}
}


class ThetaStream::Clipper::Impl
{
public:
	Impl(const ThetaStream::CommandLineParser& cmdline)
		:_decoder(std::make_unique<Mpeg2TsDecoder>(cmdline))
	{
		_ifile = openInputStream(cmdline.filename());
		
		_memblockSize = cmdline.filename() == "-" ? 7 * 188 : 49 * 188;
		_memblock.resize(_memblockSize);
	}

public:
	bool _run{ true };
	std::unique_ptr<Mpeg2TsDecoder> _decoder;
	std::shared_ptr<std::istream> _ifile;
	std::vector<uint8_t> _memblock{};
	size_t _memblockSize{};
	std::atomic_int _bytes{};
};

ThetaStream::Clipper::Clipper(const ThetaStream::CommandLineParser& cmdline)
	:_pimpl(std::make_unique<ThetaStream::Clipper::Impl>(cmdline))
{
}

ThetaStream::Clipper::~Clipper()
{
	stop();
}

void ThetaStream::Clipper::operator()()
{
	while (_pimpl->_ifile->good() && _pimpl->_run)
	{
		_pimpl->_ifile->read((char*)_pimpl->_memblock.data(), _pimpl->_memblockSize);
		const std::streamsize read = _pimpl->_ifile->gcount();
		if (read > 0)
		{
			_pimpl->_decoder->parse(_pimpl->_memblock.data(), (unsigned)read);
			_pimpl->_bytes += read;
		}
	}
	_pimpl->_bytes = -1;
}

void ThetaStream::Clipper::stop()
{
	_pimpl->_run = false;
	closeFile();
}

void ThetaStream::Clipper::closeFile()
{
	_pimpl->_decoder->close();
}

int ThetaStream::Clipper::bytes()
{
	int ret = _pimpl->_bytes;
	if (ret >= 0)
	{
		_pimpl->_bytes = 0;
	}
	return ret;
}
