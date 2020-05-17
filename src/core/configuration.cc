/**
 *
 * @file    configuration.cc
 * @author  damianzim
 * @date    2020-04-20
 *
 */

#include "core/configuration.h"

#include <string.h>

#include "common/types.h"
#include "kkv/logger.h"
#include "utility/utils.h"

namespace kkv {

Configuration::~Configuration() {
  if (!is_dumped_ && !error_dump_) {
    if (!Dump())
      spdlog::warn("Cannot dump configuration");
  }
  FileSystemUtils::CloseFile(config_file_);
}

uint64_t Configuration::GetAllSectorsSize() const {
	return 1ULL * config_.slots_count * config_.sector_size;
}

const uint16_t &Configuration::GetPartitionsCountRef() const {
  return config_.partitions_count;
}

const uint16_t &Configuration::GetSlotsCountRef() const {
  return config_.slots_count;
}

Result Configuration::Open(const fs::path& config_path) {
  path_ = &config_path;

  if (path_ == nullptr) {
    spdlog::critical("There is no path to the config file");
    return Result::kFatal;
  }

  Result s;
  if ((s = FileSystemUtils::CreateFile(*path_)) == Result::kError)
    return s;

  config_file_ = fopen(path_->c_str(), "r+b");

  if (FileSystemUtils::LockFile(config_file_, path_->c_str()) != Result::kOk)
    return Result::kError;

  if (s == Result::kAlreadyExists) {
    if (!Load()) {
      spdlog::critical("Cannot load configuration");
      return Result::kFatal;
    }
  } else {
    if ((error_dump_ = !Dump())) {
      spdlog::critical("Cannot dump initial configuration");
      return Result::kFatal;
    } else {
      is_dumped_ = true;
    }
  }

  return Result::kOk;
}

bool Configuration::Dump() {
  using namespace utils;

  constexpr auto config_size = Config::TotalSize();
  TByte buffer[config_size];

  if (!IsLittleEndian()) {
    Config config{ChangeEndianness16(GetPartitionsCount()),
                  ChangeEndianness16(GetSlotsCount()),
                  ChangeEndianness32(GetSectorSize())};

    memcpy(&buffer, static_cast<const void *>(&config), config_size);
  } else {
    memcpy(&buffer, static_cast<const void *>(&config_), config_size);
  }


  if (!FileUtils::Rewind(config_file_, path_->c_str()))
    return false;

  return (config_size == fwrite(static_cast<const void *>(buffer), 1,
                                config_size, config_file_));
}

bool Configuration::Load() {
  using namespace constants;

  if (!FileUtils::Rewind(config_file_, path_->c_str()))
    return false;

  constexpr auto config_size = Config::TotalSize();
  TByte buffer[config_size];

  auto read = fread(static_cast<void *>(buffer), 1, config_size, config_file_);
  if (read != config_size) {
    spdlog::error("Cannot read configuration from a file");
    return false;
  }

  size_t offset = 0;

  uint16_t p_count;
  memcpy(&p_count, buffer, sizeof(p_count));
  offset += sizeof(p_count);

  if (!utils::IsLittleEndian())
    p_count = utils::ChangeEndianness16(p_count);

  if (!Config::IsValidPartitionsCount(p_count)) {
    spdlog::critical("Illegal number of partitions (config file)");
    return false;
  }

  if (p_count != GetPartitionsCount()) {
    if (is_config_touched_) {
      spdlog::warn("The new number of partitions ({}) doesn't match the "
                   "configuration file ({})", GetPartitionsCount(), p_count);
    }

    config_.partitions_count = p_count;
    spdlog::info("Partitions count has been restored to ({})", p_count);
  }

  uint16_t s_count;
  memcpy(&s_count, buffer + offset, sizeof(s_count));
  offset += sizeof(s_count);

  if (!utils::IsLittleEndian())
    s_count = utils::ChangeEndianness16(s_count);

  if (!Config::IsValidSlotsCount(s_count)) {
    spdlog::critical("Illegal number of slots (config file)");
    return false;
  }

  if (s_count != GetSlotsCount()) {
    if (is_config_touched_) {
      spdlog::warn("The new number of slots ({}) doesn't match the "
                   "configuration file ({})", GetSlotsCount(), s_count);
    }

    config_.slots_count = s_count;
    spdlog::info("Slots count has been restored to ({})", s_count);
  }

  uint32_t s_size;
  memcpy(&s_size, buffer + offset, sizeof(s_size));

  if (!utils::IsLittleEndian())
    s_size = utils::ChangeEndianness32(s_size);

  if (!Config::IsValidSectorSize({ p_count, s_count }, s_size)) {
    spdlog::critical("Illegal sector size (config file)");
    return false;
  }

  if (s_size != GetSectorSize()) {
    if (is_config_touched_) {
      spdlog::warn("The new sector size ({}) doesn't match the configuration "
                   "file ({})", GetSectorSize(), s_size);
    }

    config_.sector_size = s_size;
    spdlog::info("Sector size has been restored to ({}) Bytes", s_size);
  }

  return true;
}

} // namespace kkv
