/**
 *
 * @file    streamer.cc
 * @author  damianzim
 * @date    2020-04-14
 *
 */

#include "core/streamer.h"

#include <stdexcept>

#include <spdlog/sinks/stdout_color_sinks.h>
#include "infrastructure/pair.h"

namespace kkv {

namespace {

using constants::SectorSize;

constexpr auto kMinEmptyBufferSize = static_cast<uint32_t>(SectorSize::k4KiB);
constexpr auto kMaxEmptyBufferSize = static_cast<uint32_t>(SectorSize::k256MiB);

} // namespace anonymous

// Stream

const std::string& Stream::Name() const {
  static const auto name = path_.filename().string();
  return name;
}

const fs::path& Stream::Path() const { return path_; }

int64_t Stream::Size() const { return size_; }

bool Stream::Open() {
  file_ = fopen(path_.c_str(), "r+b");
  return file_ != nullptr;
}

bool Stream::UpdateSize() {
  if (fseeko(file_, 0, SEEK_END)) {
    SPDLOG_WARN("Could not change the position in the stream ({})", Name());
    return false;
  }

  if ((size_ = ftello(file_)) < 0) {
    spdlog::warn("Could not check stream size ({})", Name());
    return false;
  } else {
    return true;
  }
}

// Streamer

Status Streamer::Init(const Paths& paths) {
  size_t i = 0;
  try {
    streams_ = new std::unique_ptr<Stream>[streams_no_];

    for (i = 0; i < streams_no_; ++i)
      streams_[i] = std::make_unique<Stream>(paths.PathPartitionFile(i));
  } catch (const std::bad_alloc& err) {
    SPDLOG_CRITICAL(err.what());
    return Status(State::kFatal, "Cannot allocate memory for Streams objects.");
  }

  Result s;
  for (i = 0; i < streams_no_; ++i) {
    s = FileSystemUtils::CreateFile(streams_[i]->Path());
    if (s == Result::kError)
      return Status(State::kFatal, "Could not create partition file");

    if (!streams_[i]->Open()) {
      spdlog::critical("Cannot open file ({}) {}", i,
                       paths.PathPartitionFile(i));
      return Status(State::kFatal, "Could not open partition file");
    } else {
      if (s == Result::kOk && !AllocateEmptySpace(streams_[i].get())) {
        spdlog::critical("Allocating empty space in partition ({}) failed", i);
        return Status(State::kFatal,
                      "Could not allocate space in partition file");
      }
    }

    if (streams_[i]->UpdateSize())
      SPDLOG_DEBUG("Stream {} has size ({})", streams_[i]->Name(),
                                              streams_[i]->Size());

  }

  delete[] buffer_;
  buffer_ = nullptr;

  try {
    InitLogger();
    log_->debug("Streamer initialized successfully");
  } catch (const spdlog::spdlog_ex& err) {
    SPDLOG_WARN(err.what());
    spdlog::warn("Cannot initializer streamer logger");
  }

  return Status();
}

const std::vector<size_t>& Streamer::CalcEmptyBufferSizes() const {
  static const std::vector<size_t> sizes = [](const size_t &max_buffer_size) {
    std::vector<size_t> temp_sizes;
    for (auto i = max_buffer_size; i >= kMinEmptyBufferSize; i >>= 1U)
        temp_sizes.push_back(i);

    return temp_sizes;
  }(CalcMaxEmptyBufferSize());

  return sizes;
}

const size_t &Streamer::CalcMaxEmptyBufferSize() const {
  static const size_t size = [](uint64_t all_sectors_size) {
    for (auto i = kMaxEmptyBufferSize; i >= kMinEmptyBufferSize; i >>= 1U) {
      if (i <= all_sectors_size)
        return i;
    }

    return kMinEmptyBufferSize;
  }(config_->GetAllSectorsSize());

  return size;
}

Stream* Streamer::operator[](const size_t index) {
  if (index >= streams_no_) {
    SPDLOG_DEBUG("Stream {} is out of range, the number of available streams "
                 "is {}", index, streams_no_);
    throw std::out_of_range("There is not stream to get");
  }

  return streams_[index].get();
}

bool Streamer::AllocateEmptySpace(const Stream* stream) {
  static size_t selected_buffer_size = 0;

  if (buffer_ == nullptr) {
    for (const auto& i : CalcEmptyBufferSizes()) {
      try {
        buffer_ = new TByte[i];
      } catch (const std::bad_alloc& err) {
        SPDLOG_DEBUG(err.what());
        SPDLOG_WARN("Cannot allocate empty buffer with size {}", i);
        continue;
      }

      selected_buffer_size = i;
      break;
    }

    if (buffer_ == nullptr) {
      SPDLOG_CRITICAL("Cannot allocate any empty buffer");
      return false;
    }
  }

  if (!FileUtils::Rewind(stream->file_, stream->Name()))
    return false;

  uint64_t saved = 0;
  while (saved < config_->GetAllSectorsSize()) {
    if (!fwrite(buffer_, selected_buffer_size, 1, stream->file_)) {
      spdlog::error("Could not write to the file {}", stream->Name());
      break;
    } else {
      saved += selected_buffer_size;
      SPDLOG_TRACE("{} saved: {}, total: {}", stream->Name(),
                   selected_buffer_size, saved);
    }
  }

  return saved == config_->GetAllSectorsSize();
}

void Streamer::InitLogger() { log_ = spdlog::stdout_color_mt("streamer"); }

} // namespace kkv
