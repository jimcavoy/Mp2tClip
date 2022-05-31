// mp2tclip.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "CommandLineParser.h"
#include "Mpeg2TsDecoder.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>


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
		cmdline.parse(argc, argv, "mp2tclip");

		std::ifstream ifile;
		ifile.open(cmdline.filename(), std::ios::in | std::ios::binary);
		if (!ifile.is_open())
		{
			cerr << "Error: Fail to open input file %s" << argv[1] << endl;
			return -1;
		}

		if (fs::create_directory(cmdline.outputDirectory().c_str()))
		{
			cerr << "Failed to create directory " << cmdline.outputDirectory() << endl;
			return -1;
		}

		Mpeg2TsDecoder decoder(cmdline);
		while (ifile.good())
		{
			ifile.read((char*)memblock, N);
			decoder.parse(memblock, (unsigned)ifile.gcount());
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

