#pragma once
#include "h264prsr.h"

class VideoDecoder :
	public ThetaStream::H264Parser
{
public:
	VideoDecoder();
	virtual ~VideoDecoder();

	virtual void onNALUnit(ThetaStream::NALUnit& nalu) override;

	bool hasKeyFrame() const;
	void reset();

private:
	bool _keyFrame{ false };
};

