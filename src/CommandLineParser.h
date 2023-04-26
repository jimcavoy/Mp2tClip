#pragma once

#include <memory>
#include <string>

namespace ThetaStream
{
	class CommandLineParser
	{
	public:
		CommandLineParser();
		~CommandLineParser();
		CommandLineParser(const CommandLineParser& other);
		CommandLineParser& operator=(const CommandLineParser& rhs);

		void parse(int argc, char** argv, const char* appname);

		/// <summary>
		/// Input filename and path to clip.
		/// </summary>
		/// <returns>The path and filename.</returns>
		std::string filename() const;

		/// <summary>
		/// The duration of the output file to clip.
		/// </summary>
		/// <returns>The number of seconds.</returns>
		int length() const;

		/// <summary>
		/// The offset time to start clipping.
		/// </summary>
		/// <returns>The number of seconds.</returns>
		int offset() const;

		/// <summary>
		/// The output directory where to place the clipped files.
		/// </summary>
		/// <returns>The directory path.</returns>
		std::string outputDirectory() const;

		/// <summary>
		/// The base name for each clipped file.  By default, it
		/// uses the input file name but if reading from stdin
		/// the default base name is "clip".
		/// </summary>
		/// <returns>The base name of the clip file.</returns>
		std::string outputFilename() const;

		/// <summary>
		/// A flag to tell the app to break when a confidentiality 
		/// label changes.  The app will clip based on duration or
		/// label change whenever comes first.
		/// </summary>
		/// <returns>Returns true to clip on a confidentiality label
		/// change; otherwise false.</returns>
		bool breakOnLabelChange() const;

	private:
		void swap(CommandLineParser& other);

		class Impl;
		std::unique_ptr<Impl> _pimpl;
	};

}
