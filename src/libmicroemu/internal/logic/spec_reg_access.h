#pragma once

#include "libmicroemu/emu_context.h"

namespace libmicroemu::internal {

/**
 * @class SpecialRegAccessor
 * @brief Provides access to special registers.
 *
 * The SpecialRegAccessor class allows reading and writing values to special registers.
 * It is templated on the type TCpuAccessor , which gives internal access to the registers.
 * @tparam TCpuAccessor The type of the cpu accessor object.
 * @tparam TSpecRegOps The type representing the operations on special registers.
 */
template <typename TCpuAccessor> class SpecialRegAccessor : public ISpecialRegAccessor {
public:
  using CpuAccessor = TCpuAccessor;
  using SReg = typename CpuAccessor::SReg;

  /**
   * @brief Constructs a SpecialRegAccessor object with the given processor states.
   *
   * @param CpuAccessor The processor states to be used by the SpecialRegAccessor object.
   */
  SpecialRegAccessor(CpuAccessor &cpua) : cpua_(cpua) {}

  /**
   * @brief Default destructor for the SpecialRegAccessor object.
   */
  ~SpecialRegAccessor() = default;

  /**
   * @brief Default copy constructor for the SpecialRegAccessor object.
   */
  SpecialRegAccessor(const SpecialRegAccessor &) = default;

  /**
   * @brief Default copy assignment operator for the SpecialRegAccessor object.
   */
  SpecialRegAccessor &operator=(const SpecialRegAccessor &) = default;

  /**
   * @brief Default move constructor for the SpecialRegAccessor object.
   */
  SpecialRegAccessor(SpecialRegAccessor &&) = default;

  /**
   * @brief Default move assignment operator for the SpecialRegAccessor object.
   */
  SpecialRegAccessor &operator=(SpecialRegAccessor &&) = default;

  /// @copydoc ISpecialRegAccessor::GetRegisterName
  std::string_view GetRegisterName(const SpecialRegisterId &reg_id) const override {
    return SReg::GetRegisterName(reg_id);
  }

  /// @copydoc ISpecialRegAccessor::ReadRegister
  u32 ReadRegister(const SpecialRegisterId &reg_id) const override {
    return cpua_.ReadSpecialRegister(reg_id);
  }

  /// @copydoc ISpecialRegAccessor::WriteRegister
  void WriteRegister(const SpecialRegisterId &reg_id, u32 value) override {
    return cpua_.WriteSpecialRegister(reg_id, value);
  }

private:
  CpuAccessor &cpua_; /**< The cpu accessor used by the SpecialRegAccessor object. */
};

} // namespace libmicroemu::internal