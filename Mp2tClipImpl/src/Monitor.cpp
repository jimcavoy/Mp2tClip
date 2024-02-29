#include <Mp2tClipImpl/Monitor.h>

#include <Mp2tClipImpl/Clipper.h>

#include <chrono>
#include <thread>

namespace ThetaStream
{
	class Monitor::Impl
	{
	public:
		Impl(ThetaStream::Clipper& clipper, int duration)
			:_clipper(clipper)
			, _duration(duration)
		{

		}

		~Impl()
		{

		}

	public:
		ThetaStream::Clipper& _clipper;
		int _duration{};
		int _count{};
		bool _run{ true };
	};
}

ThetaStream::Monitor::Monitor(ThetaStream::Clipper& clipper, int duration)
	:_pimpl(std::make_unique<ThetaStream::Monitor::Impl>(clipper, duration))
{

}

ThetaStream::Monitor::~Monitor()
{
	stop();
}

void ThetaStream::Monitor::operator() ()
{
	while (_pimpl->_run)
	{
		int bytes = _pimpl->_clipper.bytes();
		if (bytes == 0)
		{
			_pimpl->_count++;
			if (_pimpl->_count == _pimpl->_duration)
			{
				_pimpl->_clipper.closeFile();
				_pimpl->_count = 0;
			}
		}
		else if (bytes < 0)
		{
			stop();
		}
		else
		{
			_pimpl->_count = 0;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void ThetaStream::Monitor::stop()
{
	_pimpl->_run = false;
}
