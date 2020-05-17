/**
 *
 * @file    configuration.h
 * @author  damianzim
 * @date    2020-04-20
 *
 */

#ifndef KKV_CORE_CONFIGURATION_H_
#define KKV_CORE_CONFIGURATION_H_

#include <stdio.h>

#include <memory>
#include <utility>

#include "kkv/options.h"
#include "kkv/status.h"
#include "utility/filesystem.h"

namespace kkv {

class Configuration : public Options {
 public:
  Configuration() : config_file_(nullptr), path_(nullptr) {};

  ~Configuration();

  [[nodiscard]] uint64_t GetAllSectorsSize() const;

  const uint16_t &GetPartitionsCountRef() const;
  const uint16_t &GetSlotsCountRef() const;

  Result Open(const fs::path& config_path);

 private:
  bool Dump();
  bool Load();

  const fs::path* path_;

  FILE* config_file_;
  bool error_dump_{false};
  bool is_dumped_{false};
};

} // namespace kkv

#endif  // KKV_CORE_CONFIGURATION_H_
