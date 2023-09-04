#pragma once

#include <memory>

namespace ThetaStream
{
	class CommandLineParser;
}

namespace ThetaStream
{
	class Clipper
	{
	public:
		Clipper(const ThetaStream::CommandLineParser& cmdline);
		~Clipper();

		void operator()();

		void stop();

		void closeFile();

		int bytes();

	private:
		class Impl;
		std::unique_ptr<Impl> _pimpl;

	};
}

