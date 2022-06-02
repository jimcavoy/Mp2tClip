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


using namespace ThetaStream;
using namespace std;
namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	const int N = 188 * 49;
	uint8_t memblock[N]{};
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

		if (fs::create_directory(cmdline.outputDirectory().c_str()))
		{
			cerr << "Failed to create directory " << cmdline.outputDirectory() << endl;
			return -1;
		}

		Mpeg2TsDecoder decoder(cmdline);
		while (ifile->good())
		{
			ifile->read((char*)memblock, N);
			decoder.parse(memblock, (unsigned)ifile->gcount());
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

