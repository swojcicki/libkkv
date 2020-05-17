/**
 *
 * @file    dbcore.h
 * @author  damianzim
 * @date    2020-04-07
 *
 */

#ifndef KKV_CORE_CORE_H_
#define KKV_CORE_CORE_H_

#include <stdio.h>

#include <memory>

#include "core/configuration.h"
#include "core/streamer.h"
#include "kkv/db.h"
#include "kkv/slice.h"

namespace kkv {

class DBCore final : public DB, protected Streamer {
 public:
  DBCore(const Slice& path, const std::shared_ptr<Configuration>&& config);

  DBCore(const DBCore&) = delete;
  DBCore& operator=(const DBCore&) = delete;

  ~DBCore() override;

  Status status_;

 private:
  friend class DB;

  // Is this necessary? Streamer basically has such a pointer.
  const std::shared_ptr<Configuration> config_;

  FILE* lock_file_;
  Paths paths_;
};

} // namespace kkv

#endif  // KKV_CORE_CORE_H_
