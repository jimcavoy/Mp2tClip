#include "AccessUnit.h"


#include <iterator>

/////////////////////////////////////////////////////////////////////////////
// AccessUnit
AccessUnit::AccessUnit()
{
}

AccessUnit::AccessUnit(const uint8_t* sodb, unsigned int len)
{
	std::copy(sodb, sodb+len, std::back_inserter(sodb_));
}

AccessUnit::AccessUnit(const AccessUnit& cp)
	: isKey_(cp.isKey_)
	, pts_(cp.pts_)
	, dts_(cp.dts_)
{
	sodb_.clear();
	std::copy(cp.begin(), cp.end(), std::back_inserter(sodb_));
	memcpy(PTS,cp.PTS,5);
	memcpy(DTS,cp.DTS,5);
}

AccessUnit& AccessUnit::operator=(const AccessUnit& rhs)
{
	AccessUnit temp(rhs);
	swap(temp);

	return *this;
}

AccessUnit::AccessUnit(AccessUnit && cp) noexcept
{
	*this = std::move(cp);
}

AccessUnit & AccessUnit::operator=(AccessUnit && rhs) noexcept
{
	if (this != &rhs)
	{
		sodb_ = std::move(rhs.sodb_);
		isKey_ = rhs.isKey_;
		pts_ = rhs.pts_;
		dts_ = rhs.dts_;
		PES_packet_length_ = rhs.PES_packet_length_;

		memcpy(PTS, rhs.PTS, 5);
		memcpy(DTS, rhs.DTS, 5);

		rhs.isKey_ = 0;
		rhs.pts_ = 0;
		rhs.dts_ = 0;
		memset(rhs.PTS, 0, 5);
		memset(rhs.DTS, 0, 5);
		rhs.PES_packet_length_ = 0;
	}
	return *this;
}

AccessUnit::~AccessUnit()
{

}

void AccessUnit::insert(const uint8_t* sodb, unsigned int len)
{
	sodb_.insert(sodb_.end(), &sodb[0], &sodb[len]);
}

void AccessUnit::swap(AccessUnit& src)
{
	sodb_.clear();
	sodb_.swap(src.sodb_);
	std::swap(isKey_, src.isKey_);
	std::swap(pts_, src.pts_);
	std::swap(dts_, src.dts_);
	std::swap(PES_packet_length_, src.PES_packet_length_);
}

void AccessUnit::clear()
{
	sodb_.clear();
	isKey_ = false;
	pts_ = 0;
	dts_ = 0;
}

bool AccessUnit::isEqual(const AccessUnit& rhs) const
{
	return this->sodb_ == rhs.sodb_;
}
