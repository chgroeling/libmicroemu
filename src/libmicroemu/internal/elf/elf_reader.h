#pragma once

#include "libmicroemu/internal/result.h"
#include "libmicroemu/types.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>

namespace libmicroemu::internal {

class NullBuffer : public std::streambuf {
public:
  int overflow(int c) { return c; }
};

static NullBuffer null_buffer;
static std::istream null_stream(&null_buffer);

// ELF Header Strukturen
struct Elf32_Ehdr {
  u8 e_ident[16];  // Magic number and other info
  u16 e_type;      // Object file type
  u16 e_machine;   // Architecture
  u32 e_version;   // Object file version
  u32 e_entry;     // Entry point virtual address
  u32 e_phoff;     // Program header table file offset
  u32 e_shoff;     // Section header table file offset
  u32 e_flags;     // Processor-specific flags
  u16 e_ehsize;    // ELF header size in bytes
  u16 e_phentsize; // Program header table entry size
  u16 e_phnum;     // Program header table entry count
  u16 e_shentsize; // Section header table entry size
  u16 e_shnum;     // Section header table entry count
  u16 e_shstrndx;  // Section header string table index
};

struct Elf32_Phdr {
  u32 p_type;   // Segment type
  u32 p_offset; // Segment file offset
  u32 p_vaddr;  // Segment virtual address
  u32 p_paddr;  // Segment physical address
  u32 p_filesz; // Segment size in file
  u32 p_memsz;  // Segment size in memory
  u32 p_flags;  // Segment flags
  u32 p_align;  // Segment alignment
};

// Segmenttypen für ELF-Dateien
enum SegmentType {
  PT_NULL = 0,              // Program header table entry unused
  PT_LOAD = 1,              // Loadable program segment
  PT_DYNAMIC = 2,           // Dynamic linking information
  PT_INTERP = 3,            // Program interpreter
  PT_NOTE = 4,              // Auxiliary information
  PT_PHDR = 6,              // Entry for header table itself
  PT_ARM_EXIDX = 0x70000001 // Exception unwind tables
};

// Flags für ELF-Segmente
enum SegmentFlags {
  PF_X = 1 << 0, // Execute
  PF_W = 1 << 1, // Write
  PF_R = 1 << 2  // Read
};

class ElfReader {
public:
  class SegmentIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = Elf32_Phdr;
    using difference_type = decltype(static_cast<int *>(nullptr) - static_cast<int *>(nullptr));
    using pointer = const Elf32_Phdr *;
    using reference = const Elf32_Phdr &;

    SegmentIterator(std::istream *file, std::streamoff pos, std::size_t index, std::size_t count)
        : stream_(file), pos_(pos), index_(index), count_(count), phdr_() {
      if (index_ < count_) {
        stream_->seekg(pos_ + index_ * static_cast<std::streamoff>(sizeof(Elf32_Phdr)),
                       std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&phdr_), sizeof(Elf32_Phdr));
      }
    }

    reference operator*() const { return phdr_; }
    pointer operator->() const { return &phdr_; }

    SegmentIterator &operator++() {
      if (++index_ < count_) {
        stream_->seekg(pos_ + index_ * static_cast<std::streamoff>(sizeof(Elf32_Phdr)),
                       std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&phdr_), sizeof(Elf32_Phdr));
      }
      return *this;
    }

    SegmentIterator operator++(int) {
      SegmentIterator tmp(*this);
      ++(*this);
      return tmp;
    }

    bool operator==(const SegmentIterator &other) const { return index_ == other.index_; }
    bool operator!=(const SegmentIterator &other) const { return index_ != other.index_; }

  private:
    std::istream *stream_;
    std::streamoff pos_;
    std::size_t index_;
    std::size_t count_;
    Elf32_Phdr phdr_;
  };

  static Result<ElfReader> ReadElf(std::istream &stream) {
    // Read ELF header
    Elf32_Ehdr ehdr;
    stream.read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr));
    if (!stream) {
      // Failed to read ELF header.
      return Err<ElfReader>(StatusCode::kElfWrongHeader);
    }

    // Check if it's a valid ELF file
    if (ehdr.e_ident[0] != 0x7f || ehdr.e_ident[1] != 'E' || ehdr.e_ident[2] != 'L' ||
        ehdr.e_ident[3] != 'F') {
      // Not a valid ELF file."
      return Err<ElfReader>(StatusCode::kElfNotValid);
    }

    // Check if it's a 32-bit ELF file
    if (ehdr.e_ident[4] != 1) {
      // Not a 32-bit ELF file.
      return Err<ElfReader>(StatusCode::kElfNotValid);
    }

    // Store program header offset, count, and entry point
    return Ok(ElfReader(stream, ehdr.e_phoff, ehdr.e_phnum, ehdr.e_entry));
  }

  ~ElfReader() {}

  SegmentIterator begin() { return SegmentIterator(&stream_, phoff_, 0, phnum_); }
  SegmentIterator end() { return SegmentIterator(&stream_, phoff_, phnum_, phnum_); }

  u32 GetEntryPoint() const { return entry_point_; }

  Result<void> GetSegmentData(const Elf32_Phdr &phdr, u8 *buffer, u32 buffer_size, u32 buffer_vadr,
                              u32 segment_vadr) {
    if (buffer_size < phdr.p_filesz) {
      // Buffer size is too small to hold the segment data.
      return Err(StatusCode::kBufferTooSmall);
    }
    stream_.seekg(phdr.p_offset, std::ios::beg);
    stream_.read(reinterpret_cast<char *>(buffer) + (segment_vadr - buffer_vadr), buffer_size);
    if (!stream_.good()) {
      return Err(StatusCode::kError);
    }
    return Ok();
  }

  // Default Constructor uses null stream
  ElfReader() : stream_(null_stream), phoff_(0), phnum_(0), entry_point_(0) {};

  ElfReader(std::istream &stream, std::streamoff phoff, std::size_t phnum, u32 entry_point)
      : stream_(stream), phoff_(phoff), phnum_(phnum), entry_point_(entry_point) {}

private:
  std::istream &stream_;
  std::streamoff phoff_;
  std::size_t phnum_;
  u32 entry_point_;
};

} // namespace libmicroemu::internal