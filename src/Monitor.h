#pragma once

#include <memory>

namespace ThetaStream
{
	class Clipper;
}

namespace ThetaStream
{
	class Monitor
	{
	public:
		Monitor(Clipper& clipper, int duration);
		~Monitor();

		void operator () ();

		void stop();

	private:
		class Impl;
		std::unique_ptr<Impl> _pimpl;
	};
}

