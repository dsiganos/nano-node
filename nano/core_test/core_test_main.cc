#include "gtest/gtest.h"

#include <nano/node/common.hpp>
#include <nano/node/logging.hpp>
#include <nano/secure/utility.hpp>

#include <boost/filesystem/path.hpp>

namespace nano
{
void cleanup_dev_directories_on_exit ();
void force_nano_dev_network ();
}

GTEST_API_ int main (int argc, char ** argv)
{
	printf ("Running main() from core_test_main.cc\n");
	nano::force_nano_dev_network ();
	nano::node_singleton_memory_pool_purge_guard memory_pool_cleanup_guard;
	// Setting up logging so that there aren't any piped to standard output.
	nano::logging logging;
	logging.log_to_cerr_value = true;
	logging.min_time_between_log_output = std::chrono::milliseconds {0};
	logging.rep_crawler_logging_value = true;
	//logging.network_logging_value = true;
	//logging.vote_logging_value = true;
	logging.flush = true;
	logging.init (nano::unique_path ());
	testing::InitGoogleTest (&argc, argv);
	auto res = RUN_ALL_TESTS ();
	nano::cleanup_dev_directories_on_exit ();
	return res;
}
