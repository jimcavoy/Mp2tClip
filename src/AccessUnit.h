#pragma once
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// AccessUnit
class AccessUnit
{
public:
	typedef std::vector<BYTE> sodb_type; // string of data bits collection type
	typedef sodb_type::iterator iterator;
	typedef sodb_type::const_iterator const_iterator;
public:
	AccessUnit();
	AccessUnit(const BYTE* sodb, unsigned int len);
	AccessUnit(const AccessUnit& cp);
	AccessUnit& operator=(const AccessUnit& rhs);
	AccessUnit(AccessUnit&& cp) noexcept;
	AccessUnit& operator=(AccessUnit&& rhs) noexcept;
	~AccessUnit();

	void swap(AccessUnit& src);

	void insert(const BYTE* sodb, unsigned int len);

	void clear();

	size_t length() const { return sodb_.size(); }

	iterator begin() { return sodb_.begin(); }
	iterator end() { return sodb_.end(); }

	const_iterator begin() const { return sodb_.begin(); }
	const_iterator end() const { return sodb_.end(); }

	const BYTE* data() const { return sodb_.data(); };

	bool isKey() const { return isKey_; }
	void toogleKey() { isKey_ = isKey_ ? false : true; }

	bool isEqual(const AccessUnit& rhs) const;
private:
	sodb_type   sodb_{};
	bool        isKey_{false};

public:
	UINT64 pts_{0};  // units of 90 kHz clock
	UINT64 dts_{0};  // units of 90 kHz clock
	BYTE PTS[5]{};
	BYTE DTS[5]{};
	UINT16 PES_packet_length_{0};
};

inline bool operator==(const AccessUnit& lhs, const AccessUnit& rhs)
{
	return lhs.isEqual(rhs);
}

inline bool operator!=(const AccessUnit& lhs, const AccessUnit& rhs)
{
	return !lhs.isEqual(rhs);
}