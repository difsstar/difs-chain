#pragma once

#include <array>
#include <cstdint>
#include <algorithm>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/functional/hash.hpp>
#include "CommonData.h"

namespace dev
{

/// Compile-time calculation of Log2 of constant values.
template <unsigned N> struct StaticLog2 { enum { result = 1 + StaticLog2<N/2>::result }; };
template <> struct StaticLog2<1> { enum { result = 0 }; };

extern boost::random_device s_fixedHashEngine;

/// Fixed-size raw-byte array container type, with an API optimised for storing hashes.
/// Transparently converts to/from the corresponding arithmetic type; this will
/// assume the data contained in the hash is big-endian.
template <unsigned N>
class FixedHash
{

public:
	/// The corresponding arithmetic type.
	using Arith = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<N * 8, N * 8, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

	/// The size of the container.
	enum { size = N };

	/// A dummy flag to avoid accidental construction from pointer.
	enum ConstructFromPointerType { ConstructFromPointer };

	/// Method to convert from a string.
	enum ConstructFromStringType { FromHex, FromBinary };

	/// Method to convert from a string.
	enum ConstructFromHashType { AlignLeft, AlignRight, FailIfDifferent };

	/// Construct an empty hash.
	FixedHash() { m_data.fill(0); }

	/// Construct from another hash, filling with zeroes or cropping as necessary.
	template <unsigned M> explicit FixedHash(FixedHash<M> const& _h, ConstructFromHashType _t = AlignLeft) { m_data.fill(0); unsigned c = std::min(M, N); for (unsigned i = 0; i < c; ++i) m_data[_t == AlignRight ? N - 1 - i : i] = _h[_t == AlignRight ? M - 1 - i : i]; }

	/// Convert from the corresponding arithmetic type.
	FixedHash(Arith const& _arith) { toBigEndian(_arith, m_data); }

	/// Convert from unsigned
	explicit FixedHash(unsigned _u) { toBigEndian(_u, m_data); }


};

}

