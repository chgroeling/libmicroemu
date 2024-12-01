#pragma once

#include "libmicroemu/internal/logic/predicates.h"
#include "libmicroemu/internal/result.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_id.h"
#include "libmicroemu/special_register_id.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace libmicroemu::internal {

template <typename TCpuStates, typename RegOps, typename TSpecRegOps, typename TLogger = NullLogger>
class CpuAccessor : public TCpuStates {
public:
  using Reg = RegOps;
  using SReg = TSpecRegOps;

  // ---- General registers ----

  template <RegisterId Id> inline u32 ReadRegister() const {
    return RegOps::ReadRegister(*this, Id);
  }

  inline u32 ReadRegister(const RegisterId id) const { return RegOps::ReadRegister(*this, id); }

  template <RegisterId Id> inline void WriteRegister(const u32 &value) {
    RegOps::WriteRegister(*this, Id, value);
  }

  inline void WriteRegister(RegisterId id, u32 value) { RegOps::WriteRegister(*this, id, value); }

  // ---- Special registers ----
  template <SpecialRegisterId SId> inline u32 ReadRegister() const {
    return SReg::template ReadRegister<SId>(*this);
  }

  inline u32 ReadRegister(const SpecialRegisterId &reg_id) const {
    return SReg::ReadRegister(*this, reg_id);
  }

  template <SpecialRegisterId SId> inline void WriteRegister(u32 value) {
    SReg::template WriteRegister<SId>(*this, value);
  }

  inline void WriteRegister(const SpecialRegisterId &reg_id, u32 value) {
    SReg::WriteRegister(*this, reg_id, value);
  }
};

} // namespace libmicroemu::internal