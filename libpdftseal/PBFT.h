#pragma once

#include <set>
#include <libdevcore/concurrent_queue.h>
#include <libdevcore/db.h>
#include <libdevcore/Worker.h>
#include <libdevcrypto/Common.h>
#include <libethcore/BlockHeader.h>
#include <libethcore/SealEngine.h>
#include <libethereum/CommonNet.h>
#include "Common.h"
#include "PBFTHost.h"

namespace dev
{

namespace eth
{

DEV_SIMPLE_EXCEPTION(PbftInitFailed);
DEV_SIMPLE_EXCEPTION(UnexpectError);

class PBFT: public SealEngineFace, Worker
{
public:
	PBFT();
	virtual ~PBFT();

	static void init();

	std::string name() const override { return "PBFT"; }
	StringHashMap jsInfo(BlockHeader const& _bi) const override;

	strings sealers() const override { return {m_sealer}; }
	std::string sealer() const override { return m_sealer; }
	void setSealer(std::string const& _sealer) override { m_sealer = _sealer; }
	u256 nodeIdx() const { /*Guard l(m_mutex);*/ return m_node_idx; }
	uint64_t lastConsensusTime() const { /*Guard l(m_mutex);*/ return m_last_consensus_time;}
	unsigned accountType() const { return m_account_type; }
	u256 view() const { return m_view; }
	u256 to_view() const {return m_to_view; }
	u256 quorum() const { return m_node_num - m_f; }
	const BlockHeader& getHighestBlock() const { return m_highest_block; } 
	bool isLeader() {
		auto ret = getLeader();
		bool isLeader = false;
		if (ret.first) // if errer, isLeader is False isLeaderfalse
			isLeader = ret.second == m_node_idx;
		return isLeader;
	}

};

}
}
