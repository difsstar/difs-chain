#pragma once

#include "Common.h"
#include "RLP.h"

namespace dev
{

inline byte nibble(bytesConstRef _data, unsigned _i)
{
	return (_i & 1) ? (_data[_i / 2] & 15) : (_data[_i / 2] >> 4);
}

/// Interprets @a _first and @a _second as vectors of nibbles and returns the length of the longest common
/// prefix of _first[_beginFirst..._endFirst] and _second[_beginSecond..._endSecond].
inline unsigned sharedNibbles(bytesConstRef _first, unsigned _beginFirst, unsigned _endFirst, bytesConstRef _second, unsigned _beginSecond, unsigned _endSecond)
{
	unsigned ret = 0;
	while (_beginFirst < _endFirst && _beginSecond < _endSecond && nibble(_first, _beginFirst) == nibble(_second, _beginSecond))
	{
		++_beginFirst;
		++_beginSecond;
		++ret;
	}
	return ret;
}

/**
 * Nibble-based view on a bytesConstRef.
 */
struct NibbleSlice
{
	bytesConstRef data;
	unsigned offset;

	NibbleSlice(bytesConstRef _data = bytesConstRef(), unsigned _offset = 0): data(_data), offset(_offset) {}
	byte operator[](unsigned _index) const { return nibble(data, offset + _index); }
	unsigned size() const { return data.size() * 2 - offset; }
	bool empty() const { return !size(); }
	NibbleSlice mid(unsigned _index) const { return NibbleSlice(data, offset + _index); }
	void clear() { data.reset(); offset = 0; }

	/// @returns true iff _k is a prefix of this.
	bool contains(NibbleSlice _k) const { return shared(_k) == _k.size(); }
	/// @returns the number of shared nibbles at the beginning of this and _k.
	unsigned shared(NibbleSlice _k) const { return sharedNibbles(data, offset, offset + size(), _k.data, _k.offset, _k.offset + _k.size()); }
	/**
	 * @brief Determine if we, a full key, are situated prior to a particular key-prefix.
	 * @param _k The prefix.
	 * @return true if we are strictly prior to the prefix.
	 */
	bool isEarlierThan(NibbleSlice _k) const
	{
		unsigned i = 0;
		for (; i < _k.size() && i < size(); ++i)
			if (operator[](i) < _k[i])		// Byte is lower - we're earlier..
				return true;
			else if (operator[](i) > _k[i])	// Byte is higher - we're not earlier.
				return false;
		if (i >= _k.size())					// Ran past the end of the prefix - we're == for the entire prefix - we're not earlier.
			return false;
		return true;						// Ran out before the prefix had finished - we're earlier.
	}
	bool operator==(NibbleSlice _k) const { return _k.size() == size() && shared(_k) == _k.size(); }
	bool operator!=(NibbleSlice _s) const { return !operator==(_s); }
};

}