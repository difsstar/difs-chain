/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file BasicAuthority.cpp
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#include "BasicAuthority.h"
#include <libdevcore/CommonJS.h>
#include <libdevcore/Log.h>
#include <libethereum/Interface.h>
#include "Exceptions.h"
#include "BlockHeader.h"
using namespace std;
using namespace dev;
using namespace eth;

void BasicAuthority::init()
{
	ETH_REGISTER_SEAL_ENGINE(BasicAuthority);
}

StringHashMap BasicAuthority::jsInfo(BlockHeader const& _bi) const
{
	return { { "sig", toJS(sig(_bi)) } };

}

bool BasicAuthority::shouldSeal(Interface* _i)
{
//	cdebug << "Comparing: " << _i->pendingInfo().timestamp() << " to " << utcTime();
	return _i->pendingInfo().timestamp() + 5 <= utcTime() || (_i->pendingInfo().timestamp() <= utcTime() && !_i->pending().empty());
}

void BasicAuthority::generateSeal(BlockHeader const& _bi, bytes const& _block_data)
{
	//(void)_block_data;
	BlockHeader bi = _bi;
	h256 h = bi.hash(WithoutSeal);
	Signature s = sign(m_secret, h);
	setSig(bi, s);
	SealEngineBase::generateSeal(bi, _block_data);
}

bool BasicAuthority::onOptionChanging(std::string const& _name, bytes const& _value)
{
	RLP rlp(_value);
	if (_name == "authorities")
		m_authorities = rlp.toUnorderedSet<Address>();
	else if (_name == "authority")
		m_secret = Secret(rlp.toHash<h256>());
	else
		return false;
	return true;
}

