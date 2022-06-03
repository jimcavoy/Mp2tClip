// mp2tclip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "CommandLineParser.h"
#include "Mpeg2TsDecoder.h"

#ifdef _WIN32
#include <io.h>
#endif
#include <cstdint>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <array>


using namespace ThetaStream;
using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	const int N = 188 * 49;
	std::array<uint8_t, N> memblock{};
	try
	{
		CommandLineParser cmdline;
		cmdline.parse(argc, argv, "Mp2tClip");

		shared_ptr<istream> ifile;
		if (cmdline.filename() == "-")
		{
#ifdef _WIN32
			_setmode(_fileno(stdin), _O_BINARY);
#endif
			ifile.reset(&cin, [](...) {});
		}
		else
		{
			ifstream* tsfile = new ifstream(cmdline.filename(), ios::binary);
			if (!tsfile->is_open())
			{
				cerr << "Failed to open input file " << cmdline.filename() << endl;
				return -1;
			}
			ifile.reset(tsfile);
		}

		std::error_code errc{};
		auto curdir = fs::current_path();
		fs::path dirname(cmdline.outputDirectory().c_str());
		fs::path dirpath = curdir / dirname;
		if (!fs::exists(dirpath))
		{
			if (!fs::create_directory(dirpath, errc))
			{
				cerr << "Failed to create directory " << cmdline.outputDirectory() << endl;
				return -1;
			}
		}

		Mpeg2TsDecoder decoder(cmdline);
		while (ifile->good())
		{
			ifile->read((char*)memblock.data(), N);
			const streamsize read = ifile->gcount();
			if (read > 0)
			{
				decoder.parse(memblock.data(), (unsigned)read);
			}
		}
	}
	catch (std::exception & ex)
	{
		cerr << "Exception thrown: " << ex.what() << endl;
		return -1;
	}
	catch (...)
	{
		cerr << "Unknown exception thrown" << endl;
		return -1;
	}

	return 0;
}

