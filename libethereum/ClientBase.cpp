#include "ClientBase.h"
#include <algorithm>
#include "BlockChain.h"
#include "Executive.h"
#include "State.h"

using namespace std;
using namespace dev;
using namespace dev::eth;

const char* WatchChannel::name() { return EthBlue "ℹ" EthWhite "  "; }
const char* WorkInChannel::name() { return EthOrange "⚒" EthGreen "▬▶"; }
const char* WorkOutChannel::name() { return EthOrange "⚒" EthNavy "◀▬"; }
const char* WorkChannel::name() { return EthOrange "⚒" EthWhite "  "; }

static const int64_t c_maxGasEstimate = 50000000;

pair<h256, Address> ClientBase::submitTransaction(TransactionSkeleton const& _t, Secret const& _secret)
{
	prepareForTransaction();
	
	TransactionSkeleton ts(_t);
	ts.from = toAddress(_secret);
	if (_t.nonce == Invalid256)
		ts.nonce = max<u256>(postSeal().transactionsFrom(ts.from), m_tq.maxNonce(ts.from));
	if (ts.gasPrice == Invalid256)
		ts.gasPrice = gasBidPrice();
	if (ts.gas == Invalid256)
		ts.gas = min<u256>(gasLimitRemaining() / 5, balanceAt(ts.from) / ts.gasPrice);

	Transaction t(ts, _secret);
	m_tq.import(t.rlp());
	
	return make_pair(t.sha3(), toAddress(ts.from, ts.nonce));
}

// TODO: remove try/catch, allow exceptions
ExecutionResult ClientBase::call(Address const& _from, u256 _value, Address _dest, bytes const& _data, u256 _gas, u256 _gasPrice, BlockNumber _blockNumber, FudgeFactor _ff)
{
	ExecutionResult ret;
	try
	{
		Block temp = block(_blockNumber);
		u256 nonce = max<u256>(temp.transactionsFrom(_from), m_tq.maxNonce(_from));
		u256 gas = _gas == Invalid256 ? gasLimitRemaining() : _gas;
		u256 gasPrice = _gasPrice == Invalid256 ? gasBidPrice() : _gasPrice;
		Transaction t(_value, gasPrice, gas, _dest, _data, nonce);
		t.forceSender(_from);
		if (_ff == FudgeFactor::Lenient)
			temp.mutableState().addBalance(_from, (u256)(t.gas() * t.gasPrice() + t.value()));
		ret = temp.execute(bc().lastHashes(), t, Permanence::Reverted);
	}
	catch (...)
	{
		// TODO: Some sort of notification of failure.
	}
	return ret;
}

ExecutionResult ClientBase::create(Address const& _from, u256 _value, bytes const& _data, u256 _gas, u256 _gasPrice, BlockNumber _blockNumber, FudgeFactor _ff)
{
	ExecutionResult ret;
	try
	{
		Block temp = block(_blockNumber);
		u256 n = temp.transactionsFrom(_from);
		//	cdebug << "Nonce at " << toAddress(_secret) << " pre:" << m_preSeal.transactionsFrom(toAddress(_secret)) << " post:" << m_postSeal.transactionsFrom(toAddress(_secret));
		Transaction t(_value, _gasPrice, _gas, _data, n);
		t.forceSender(_from);
		if (_ff == FudgeFactor::Lenient)
			temp.mutableState().addBalance(_from, (u256)(t.gas() * t.gasPrice() + t.value()));
		ret = temp.execute(bc().lastHashes(), t, Permanence::Reverted);
	}
	catch (...)
	{
		// TODO: Some sort of notification of failure.
	}
	return ret;
}

std::pair<u256, ExecutionResult> ClientBase::estimateGas(Address const& _from, u256 _value, Address _dest, bytes const& _data, int64_t _maxGas, u256 _gasPrice, BlockNumber _blockNumber, GasEstimationCallback const& _callback)
{
	try
	{
		int64_t upperBound = _maxGas;
		if (upperBound == Invalid256 || upperBound > c_maxGasEstimate)
			upperBound = c_maxGasEstimate;
		int64_t lowerBound = Transaction::baseGasRequired(!_dest, &_data, EVMSchedule());
		Block bk = block(_blockNumber);
		u256 gasPrice = _gasPrice == Invalid256 ? gasBidPrice() : _gasPrice;
		ExecutionResult er;
		ExecutionResult lastGood;
		bool good = false;
		while (upperBound != lowerBound)
		{
			int64_t mid = (lowerBound + upperBound) / 2;
			u256 n = bk.transactionsFrom(_from);
			Transaction t;
			if (_dest)
				t = Transaction(_value, gasPrice, mid, _dest, _data, n);
			else
				t = Transaction(_value, gasPrice, mid, _data, n);
			t.forceSender(_from);
			EnvInfo env(bk.info(), bc().lastHashes(), 0);
			env.setGasLimit(mid);
			State tempState(bk.state());
			tempState.addBalance(_from, (u256)(t.gas() * t.gasPrice() + t.value()));
			er = tempState.execute(env, *bc().sealEngine(), t, Permanence::Reverted).first;
			if (er.excepted == TransactionException::OutOfGas ||
				er.excepted == TransactionException::OutOfGasBase ||
				er.excepted == TransactionException::OutOfGasIntrinsic ||
				er.codeDeposit == CodeDeposit::Failed ||
				er.excepted == TransactionException::BadJumpDestination)
					lowerBound = lowerBound == mid ? upperBound : mid;
			else
			{
				lastGood = er;
				upperBound = upperBound == mid ? lowerBound : mid;
				good = true;
			}

			if (_callback)
				_callback(GasEstimationProgress { lowerBound, upperBound });
		}
		if (_callback)
			_callback(GasEstimationProgress { lowerBound, upperBound });
		return make_pair(upperBound, good ? lastGood : er);
	}
	catch (...)
	{
		// TODO: Some sort of notification of failure.
		return make_pair(u256(), ExecutionResult());
	}


ImportResult ClientBase::injectBlock(bytes const& _block)
{
	return bc().attemptImport(_block, preSeal().db()).first;
}

u256 ClientBase::balanceAt(Address _a, BlockNumber _block) const
{
	return block(_block).balance(_a);
}

u256 ClientBase::countAt(Address _a, BlockNumber _block) const
{
	return block(_block).transactionsFrom(_a);
}

u256 ClientBase::stateAt(Address _a, u256 _l, BlockNumber _block) const
{
	return block(_block).storage(_a, _l);
}

h256 ClientBase::stateRootAt(Address _a, BlockNumber _block) const
{
	return block(_block).storageRoot(_a);
}

bytes ClientBase::codeAt(Address _a, BlockNumber _block) const
{
	return block(_block).code(_a);
}

}