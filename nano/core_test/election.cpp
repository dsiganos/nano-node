#include <nano/node/election.hpp>
#include <nano/test_common/system.hpp>
#include <nano/test_common/testutil.hpp>

#include <gtest/gtest.h>

using namespace std::chrono_literals;

TEST (election, construction)
{
	nano::system system (1);
	auto & node = *system.nodes[0];
	node.block_confirm (nano::dev::genesis);
	node.scheduler.flush ();
	auto election = node.active.election (nano::dev::genesis->qualified_root ());
	election->transition_active ();
}

TEST (election, quorum_minimum_flip_success)
{
	nano::system system;
	nano::node_config node_config (nano::get_available_port (), system.logging);
	node_config.online_weight_minimum = nano::dev::constants.genesis_amount;
	node_config.frontiers_confirmation = nano::frontiers_confirmation_mode::disabled;
	auto & node1 = *system.add_node (node_config);
	nano::keypair key1;
	nano::block_builder builder;
	auto send1 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta ())
				 .link (key1.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send1);
	nano::keypair key2;
	auto send2 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta ())
				 .link (key2.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send2);
	node1.process_active (send1);
	node1.block_processor.flush ();
	node1.scheduler.flush ();
	node1.process_active (send2);
	node1.block_processor.flush ();
	node1.scheduler.flush ();
	auto election = node1.active.election (send1->qualified_root ());
	ASSERT_NE (nullptr, election);
	ASSERT_EQ (2, election->blocks ().size ());
	auto vote1 (std::make_shared<nano::vote> (nano::dev::genesis_key.pub, nano::dev::genesis_key.prv, std::numeric_limits<uint64_t>::max (), send2));
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote1));
	node1.block_processor.flush ();
	ASSERT_NE (nullptr, node1.block (send2->hash ()));
	ASSERT_TRUE (election->confirmed ());
}

TEST (election, quorum_minimum_flip_fail)
{
	nano::system system;
	nano::node_config node_config (nano::get_available_port (), system.logging);
	node_config.online_weight_minimum = nano::dev::constants.genesis_amount;
	node_config.frontiers_confirmation = nano::frontiers_confirmation_mode::disabled;
	auto & node1 = *system.add_node (node_config);
	nano::keypair key1;
	nano::block_builder builder;
	auto send1 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta () - 1)
				 .link (key1.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send1);
	nano::keypair key2;
	auto send2 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta () - 1)
				 .link (key2.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send2);
	node1.process_active (send1);
	node1.block_processor.flush ();
	node1.scheduler.flush ();
	node1.process_active (send2);
	node1.block_processor.flush ();
	node1.scheduler.flush ();
	auto election = node1.active.election (send1->qualified_root ());
	ASSERT_NE (nullptr, election);
	ASSERT_EQ (2, election->blocks ().size ());
	auto vote1 (std::make_shared<nano::vote> (nano::dev::genesis_key.pub, nano::dev::genesis_key.prv, std::numeric_limits<uint64_t>::max (), send2));
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote1));
	node1.block_processor.flush ();
	ASSERT_NE (nullptr, node1.block (send1->hash ()));
	ASSERT_FALSE (election->confirmed ());
}

TEST (election, quorum_minimum_confirm_success)
{
	nano::system system;
	nano::node_config node_config (nano::get_available_port (), system.logging);
	node_config.online_weight_minimum = nano::dev::constants.genesis_amount;
	node_config.frontiers_confirmation = nano::frontiers_confirmation_mode::disabled;
	auto & node1 = *system.add_node (node_config);
	nano::keypair key1;
	nano::block_builder builder;
	auto send1 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta ())
				 .link (key1.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send1);
	node1.process_active (send1);
	node1.block_processor.flush ();
	node1.scheduler.activate (nano::dev::genesis_key.pub, node1.store.tx_begin_read ());
	node1.scheduler.flush ();
	auto election = node1.active.election (send1->qualified_root ());
	ASSERT_NE (nullptr, election);
	ASSERT_EQ (1, election->blocks ().size ());
	auto vote1 (std::make_shared<nano::vote> (nano::dev::genesis_key.pub, nano::dev::genesis_key.prv, std::numeric_limits<uint64_t>::max (), send1));
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote1));
	node1.block_processor.flush ();
	ASSERT_NE (nullptr, node1.block (send1->hash ()));
	ASSERT_TRUE (election->confirmed ());
}

TEST (election, quorum_minimum_confirm_fail)
{
	nano::system system;
	nano::node_config node_config (nano::get_available_port (), system.logging);
	node_config.online_weight_minimum = nano::dev::constants.genesis_amount;
	node_config.frontiers_confirmation = nano::frontiers_confirmation_mode::disabled;
	auto & node1 = *system.add_node (node_config);
	nano::keypair key1;
	nano::block_builder builder;
	auto send1 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (node1.online_reps.delta () - 1)
				 .link (key1.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send1);
	node1.process_active (send1);
	node1.block_processor.flush ();
	node1.scheduler.activate (nano::dev::genesis_key.pub, node1.store.tx_begin_read ());
	node1.scheduler.flush ();
	auto election = node1.active.election (send1->qualified_root ());
	ASSERT_NE (nullptr, election);
	ASSERT_EQ (1, election->blocks ().size ());
	auto vote1 (std::make_shared<nano::vote> (nano::dev::genesis_key.pub, nano::dev::genesis_key.prv, std::numeric_limits<uint64_t>::max (), send1));
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote1));
	node1.block_processor.flush ();
	ASSERT_NE (nullptr, node1.block (send1->hash ()));
	ASSERT_FALSE (election->confirmed ());
}

namespace nano
{
TEST (election, quorum_minimum_update_weight_before_quorum_checks)
{
	nano::system system;
	nano::node_config node_config (nano::get_available_port (), system.logging);
	node_config.frontiers_confirmation = nano::frontiers_confirmation_mode::disabled;

	// create node 1, node1 is given a signing key immediately
	auto & node1 = *system.add_node (node_config);
	system.wallet (0)->insert_adhoc (nano::dev::genesis_key.prv);

	// this will be the amount remaining in genesis after "send1"
	// the amount sent is: total_amount - genesis_acc_balance
	// the genesis account balance is set to be one less than quorum,
	// meaning that the genesis account does not have enough weight to reach quorum
	// the weight of account "key1" will also be needed for the election to succeed
	auto genesis_acc_balance = ((nano::uint256_t (node_config.online_weight_minimum.number ()) * nano::online_reps::online_weight_quorum) / 100).convert_to<nano::uint128_t> () - 1;

	// create accounts key1 and key2
	nano::keypair key1, key2;

	// create send1 block, sending "total_amount - genesis_acc_balance" from genesis to key1
	nano::block_builder builder;
	auto send1 = builder.state ()
				 .account (nano::dev::genesis_key.pub)
				 .previous (nano::dev::genesis->hash ())
				 .representative (nano::dev::genesis_key.pub)
				 .balance (genesis_acc_balance)
				 .link (key1.pub)
				 .work (0)
				 .sign (nano::dev::genesis_key.prv, nano::dev::genesis_key.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send1);

	// create open block to receive "send1", it is the open block of account "key1"
	auto open1 = builder.state ()
				 .account (key1.pub)
				 .previous (0)
				 .representative (key1.pub)
				 .balance (nano::dev::constants.genesis_amount - genesis_acc_balance)
				 .link (send1->hash ())
				 .work (0)
				 .sign (key1.prv, key1.pub)
				 .build_shared ();
	node1.work_generate_blocking (*open1);

	// create send2 block, sending 3 raw from account "key1" to "key2"
	auto send2 = builder.state ()
				 .account (key1.pub)
				 .previous (open1->hash ())
				 .representative (key1.pub)
				 .balance (3)
				 .link (key2.pub)
				 .work (0)
				 .sign (key1.prv, key1.pub)
				 .build_shared ();
	node1.work_generate_blocking (*send2);

	// process the 3 blocks on node1
	node1.process_active (send1);
	node1.block_processor.flush ();
	node1.process (*open1);
	// why is there no flush here???
	node1.process (*send2);
	node1.block_processor.flush ();
	ASSERT_EQ (node1.ledger.cache.block_count, 4);

	// create node 2 without a signing key
	node_config.peering_port = nano::get_available_port ();
	auto & node2 = *system.add_node (node_config);

	// process the 3 blocks on node2, why are there no calls to node1.block_processor.flush() here, as above???
	node2.process (*send1);
	node2.process (*open1);
	node2.process (*send2);

	// node2 is given a signing key after being asked to process blocks, is this significant???
	system.wallet (1)->insert_adhoc (key1.prv);

	// check node2 has processed the 3 blocks (3 + genesis = 4)
	node2.block_processor.flush ();
	ASSERT_EQ (node2.ledger.cache.block_count, 4);

	// start elections on genesis account on node1
	node1.scheduler.activate (nano::dev::genesis_key.pub, node1.store.tx_begin_read ());
	node1.scheduler.flush ();

	// check that the election on genesis account is active
	auto election = node1.active.election (send1->qualified_root ());
	ASSERT_NE (nullptr, election);
	ASSERT_EQ (1, election->blocks ().size ());

	// create a final vote for "send1" block signed by the genesis account
	auto vote1 (std::make_shared<nano::vote> (nano::dev::genesis_key.pub, nano::dev::genesis_key.prv, std::numeric_limits<uint64_t>::max (), send1));

	// add vote1 to node1's active elections
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote1));

	// create a final vote for "send1" block signed by account "key1"
	auto vote2 (std::make_shared<nano::vote> (key1.pub, key1.prv, std::numeric_limits<uint64_t>::max (), send1));

	// find the channel that connects node1 to node2
	auto channel = node1.network.find_channel (node2.network.endpoint ());
	ASSERT_NE (channel, nullptr);

	// pretend that vote2 arrived from node2 as a rep_crawler response
	// this assert fails occasionally!
	// I think this fails because the rep_crawler does not have send1 block hash in its active container, but what puts it there???
	ASSERT_TIMELY (10s, !node1.rep_crawler.response (channel, vote2));

	// the election should be unconfirmed because vote1 alone is not enough to reach quorum
	ASSERT_FALSE (election->confirmed ());

	{
		// what does this do???
		nano::lock_guard<nano::mutex> guard (node1.online_reps.mutex);
		// Modify online_m for online_reps to more than is available, this checks that voting below updates it to current online reps.
		// where is the check described in the line above here done???
		node1.online_reps.online_m = node_config.online_weight_minimum.number () + 20;
	}

	// inject vote2 into node1 active elections container
	ASSERT_EQ (nano::vote_code::vote, node1.active.vote (vote2));
	node1.block_processor.flush ();
	// Do we need node.scheduler.flush() here???

	// check that send1 block is in the ledger
	ASSERT_NE (nullptr, node1.block (send1->hash ()));

	// check that election is confirmed, how long does this state last for???
	ASSERT_TRUE (election->confirmed ());
}
}
