#include <nano/lib/logging.hpp>
#include <nano/node/make_store.hpp>
#include <nano/store/lmdb/lmdb.hpp>
#include <nano/store/rocksdb/rocksdb.hpp>

std::unique_ptr<nano::store::component> nano::make_store (nano::logger & logger, std::filesystem::path const & path, nano::ledger_constants & constants, bool read_only, bool add_db_postfix, nano::rocksdb_config const & rocksdb_config, nano::txn_tracking_config const & txn_tracking_config_a, std::chrono::milliseconds block_processor_batch_max_time_a, nano::lmdb_config const & lmdb_config_a, bool backup_before_upgrade, bool force_use_write_queue)
{
	if (rocksdb_config.enable)
	{
		return std::make_unique<nano::store::rocksdb::component> (logger, add_db_postfix ? path / "rocksdb" : path, constants, rocksdb_config, read_only, force_use_write_queue);
	}

	return std::make_unique<nano::store::lmdb::component> (logger, add_db_postfix ? path / "data.ldb" : path, constants, txn_tracking_config_a, block_processor_batch_max_time_a, lmdb_config_a, backup_before_upgrade);
}
