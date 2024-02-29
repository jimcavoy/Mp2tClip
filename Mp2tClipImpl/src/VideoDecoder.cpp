#include "VideoDecoder.h"

#include <loki/Visitor.h>

#include <h264p/nalu.h>
#include <h264p/naluimpl.h>

class IsKeyVisitor
	:public Loki::BaseVisitor,
	public Loki::Visitor<ThetaStream::NALUnitSPS>
{
public:
	void Visit(ThetaStream::NALUnitSPS& nalu)
	{
		_isKey = true;
	}

	bool _isKey{ false };
};

VideoDecoder::VideoDecoder()
{
}

VideoDecoder::~VideoDecoder()
{
}

void VideoDecoder::onNALUnit(ThetaStream::NALUnit& nalu)
{
	IsKeyVisitor vis;
	nalu.Accept(vis);

	if (vis._isKey)
	{
		_keyFrame = true;
	}
}

bool VideoDecoder::hasKeyFrame() const
{
	return _keyFrame;
}

void VideoDecoder::reset()
{
	_keyFrame = false;
}
