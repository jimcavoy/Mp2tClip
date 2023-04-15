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
#include <vector>
#include <sstream>

using namespace ThetaStream;
using namespace std;
namespace fs = std::filesystem;

std::shared_ptr<istream> openInputStream(const std::string& input);
void createOutputDir(const std::string& dirpath);

int main(int argc, char* argv[])
{
	std::vector<uint8_t> memblock{};
	try
	{
		CommandLineParser cmdline;
		cmdline.parse(argc, argv, "Mp2tClip");

		shared_ptr<istream> ifile = openInputStream(cmdline.filename());
		
		createOutputDir(cmdline.outputDirectory());

		size_t N = cmdline.filename() == "-" ? 7 * 188 : 49 * 188;
		memblock.resize(N);

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

std::shared_ptr<std::istream> openInputStream(const std::string& input)
{
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

