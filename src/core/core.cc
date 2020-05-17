/**
 *
 * @file    dbcore.cc
 * @author  damianzim
 * @date    2020-04-07
 *
 */

#include "core/core.h"

#include <string>

#include "kkv/logger.h"
#include "kkv/options.h"
#include "kkv/status.h"

namespace kkv {

DBCore::DBCore(const Slice& path, const std::shared_ptr<Configuration>&& config)
    : Streamer(config),
      config_(config),
      lock_file_(nullptr),
      paths_(path) {
  status_.SetState(Paths::CreateDir(paths_.Base()));
  if (!status_.ok()) {
    status_.SetMessage("Cannot create DB directory");
    return;
  }

  lock_file_ = fopen(paths_.PathLockFile().c_str(), "w");
  status_.SetState(Paths::LockFile(lock_file_, paths_.PathLockFile().c_str()));
  if (!status_.ok()) {
    status_.SetMessage("Cannot lock file: DB::LOCK");
    return;
  }

  if (Paths::IsFile(paths_.PathConfigFile()) && config_->error_when_exists) {
    status_.Set(State::kError, "DB already exists");
    return;
  }

  status_.SetState(config_->Open(paths_.PathConfigFile()));
  if (!status_.ok()) {
    status_.SetMessage("Cannot resolve configuration (see logs)");
    return;
  }

  if (!(status_ = Init(paths_)).ok())
    return;

  status_.Set(State::kOk, "Database initialized successfully");
};

DBCore::~DBCore() { Paths::CloseFile(lock_file_); }

DB::~DB() = default;

Status DB::Open(DB** db, const Slice& path, Options options) {
  *db = nullptr;

  DBCore* dbcore = nullptr;
  Status status;

  try {
    dbcore = new DBCore(path, std::make_shared<Configuration>(
        reinterpret_cast<Configuration&>(options)));
    status = dbcore->status_;
  } catch (const std::bad_alloc& err) {
    SPDLOG_CRITICAL(err.what());
    status.Set(State::kFatal, "Allocation error (DB)");
  }

  if (status.ok())
    *db = dbcore;
  else
    delete dbcore;

  return status;
}

} // namespace kkv
