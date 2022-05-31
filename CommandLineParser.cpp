#include "CommandLineParser.h"

#include <exception>
#include <iostream>
#include <sstream>

using namespace std;

const char* usage = "Usage: Mp2tClip <OPTIONS>";
const char* opts = "  -s\tSource MPEG-2 TS file; otherwise, stdin.\n \
 -d\tClip duration in seconds. (default: 60 seconds)\n \
 -o\tOutput directory where application writes the clipped files. (default: clipped)\n \
 -O\tOffset time when to start clipping in seconds. (default: 0 seconds)\n \
 -n\tThe base name for output clip file; otherwise, use the source file name.\n \
 \t  If stdin, use default name.  (default: clip) \n \
 -?\tPrint this message.";


namespace ThetaStream
{
	class CommandLineParser::Impl
	{
	public:
		Impl() {}

	public:
		int _duration{ 60 };
		int _offset{};
		std::string _source{"-"};
		std::string _output{"clipped"};
		std::string _oname{"clip"};
	};
}

ThetaStream::CommandLineParser::CommandLineParser()
{
	_pimpl = std::make_unique<ThetaStream::CommandLineParser::Impl>();
}

ThetaStream::CommandLineParser::~CommandLineParser()
{
}

ThetaStream::CommandLineParser::CommandLineParser(const CommandLineParser& other)
{
	_pimpl = std::make_unique<ThetaStream::CommandLineParser::Impl>();
	
	_pimpl->_duration = other.length();
	_pimpl->_offset = other.offset();
	_pimpl->_source = other.filename();
	_pimpl->_output = other.outputDirectory();
	_pimpl->_oname = other.outputFilename();
}

ThetaStream::CommandLineParser& ThetaStream::CommandLineParser::operator=(const ThetaStream::CommandLineParser& rhs)
{
	ThetaStream::CommandLineParser temp(rhs);
	swap(temp);
	return *this;
}

void ThetaStream::CommandLineParser::parse(int argc, char** argv, const char* appname)
{
	char c{};

	while (--argc > 0 && (*++argv)[0] == '-')
	{
		c = *++argv[0];
		switch (c)
		{
		case 's':
			_pimpl->_source = *argv + 1;
			break;
		case 'o':
			_pimpl->_output = *argv + 1;
			break;
		case 'd':
			_pimpl->_duration = std::stoi(*argv + 1);
			break;
		case 'O':
			_pimpl->_offset = std::stoi(*argv + 1);
		case 'n':
			_pimpl->_oname = *argv + 1;
			break;
		case '?':
		{
			std::stringstream msg;
			msg << usage << endl;
			msg << endl << "Options: " << endl;
			msg << opts << endl;
			std::runtime_error exp(msg.str().c_str());
			throw exp;
		}
		default:
		{
			std::stringstream msg;
			msg << appname << ": illegal option " << c << endl;
			std::runtime_error exp(msg.str().c_str());
			throw exp;
		}
		}
	}
}

std::string ThetaStream::CommandLineParser::filename() const
{
	return _pimpl->_source;
}

int ThetaStream::CommandLineParser::length() const
{
	return _pimpl->_duration;
}

int ThetaStream::CommandLineParser::offset() const
{
	return _pimpl->_offset;
}

std::string ThetaStream::CommandLineParser::outputDirectory() const
{
	return _pimpl->_output;
}

std::string ThetaStream::CommandLineParser::outputFilename() const
{
	return _pimpl->_oname;
}


void ThetaStream::CommandLineParser::swap(ThetaStream::CommandLineParser& other)
{
	std::swap(_pimpl->_duration, other._pimpl->_duration);
	std::swap(_pimpl->_offset, other._pimpl->_offset);
	_pimpl->_source.swap(other._pimpl->_source);
	_pimpl->_output.swap(other._pimpl->_output);
	_pimpl->_oname.swap(other._pimpl->_oname);
}
