#include "libmicroemu/emu_context.h"
#include <string_view>

// Write implementation of IRegAccessor
namespace libmicroemu {
namespace internal {
/**
 * @class RegAccessor
 * @brief Provides access to processor registers.
 *
 * The RegAccessor class allows reading and writing values to processor registers.
 * It provides methods to read the value of a register and write a value to a register.
 * The class also handles edge cases where the register ID is out of range (greater than or equal to
 * 16).
 *
 * @tparam TCpuAccessor The type of the cpu accessor object used by the RegAccessor object.
 * @tparam TRegOps The type of the register operations used by the RegAccessor object.
 */
template <typename TCpuAccessor> class RegAccessor : public IRegAccessor {
public:
  using CpuAccessor = TCpuAccessor;
  using Reg = typename CpuAccessor::Reg;
  using SReg = typename CpuAccessor::SReg;
  /**
   * @brief Constructs a RegAccessor object with the given processor states.
   *
   * @param CpuAccessor The processor states to be used by the RegAccessor object.
   */
  RegAccessor(CpuAccessor &cpua) : cpua_(cpua) {}

  /**
   * @brief Default destructor for the RegAccessor object.
   */
  ~RegAccessor() = default;

  /**
   * @brief Default copy constructor for the RegAccessor object.
   */
  RegAccessor(const RegAccessor &) = default;

  /**
   * @brief Default copy assignment operator for the RegAccessor object.
   */
  RegAccessor &operator=(const RegAccessor &) = default;

  /**
   * @brief Default move constructor for the RegAccessor object.
   */
  RegAccessor(RegAccessor &&) = default;

  /**
   * @brief Default move assignment operator for the RegAccessor object.
   */
  RegAccessor &operator=(RegAccessor &&) = default;

  /// @copydoc IRegAccessor::GetRegisterName
  std::string_view GetRegisterName(const RegisterId &id) const override {
    const RegisterId eid = static_cast<RegisterId>(id);
    return Reg::GetRegisterName(eid);
  }

  /// @copydoc IRegAccessor::ReadRegister
  u32 ReadRegister(const RegisterId &id) const override { return cpua_.ReadRegister(id); }

  /// @copydoc IRegAccessor::WriteRegister
  void WriteRegister(const RegisterId &id, u32 value) override { cpua_.WriteRegister(id, value); }

private:
  CpuAccessor &cpua_; /**< The processor states used by the RegAccessor object. */
};
} // namespace internal
} // namespace libmicroemu