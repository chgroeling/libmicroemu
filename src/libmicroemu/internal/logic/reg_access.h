#include "libmicroemu/emu_context.h"
// Write implementation of IRegAccess
namespace microemu {
namespace internal {
/**
 * @class RegAccess
 * @brief Provides access to processor registers.
 *
 * The RegAccess class allows reading and writing values to processor registers.
 * It provides methods to read the value of a register and write a value to a register.
 * The class also handles edge cases where the register ID is out of range (greater than or equal to
 * 16).
 *
 * @tparam TProcessorStates The type of the processor states used by the RegAccess object.
 * @tparam TRegOps The type of the register operations used by the RegAccess object.
 */
template <typename TProcessorStates, typename TRegOps, typename TSpecRegOps>
class RegAccess : public IRegAccess {
public:
  using PState = TProcessorStates;
  using Reg = TRegOps;
  using SReg = TSpecRegOps;
  /**
   * @brief Constructs a RegAccess object with the given processor states.
   *
   * @param pstate The processor states to be used by the RegAccess object.
   */
  RegAccess(PState &pstates) : pstates_(pstates) {}

  /**
   * @brief Default destructor for the RegAccess object.
   */
  ~RegAccess() = default;

  /**
   * @brief Default copy constructor for the RegAccess object.
   */
  RegAccess(const RegAccess &) = default;

  /**
   * @brief Default copy assignment operator for the RegAccess object.
   */
  RegAccess &operator=(const RegAccess &) = default;

  /**
   * @brief Default move constructor for the RegAccess object.
   */
  RegAccess(RegAccess &&) = default;

  /**
   * @brief Default move assignment operator for the RegAccess object.
   */
  RegAccess &operator=(RegAccess &&) = default;

  /// @copydoc IRegAccess::GetRegisterName
  const char *GetRegisterName(const RegisterId &id) const override {
    const RegisterId eid = static_cast<RegisterId>(id);
    return Reg::GetRegisterName(eid);
  }

  /// @copydoc IRegAccess::ReadRegister
  u32 ReadRegister(const RegisterId &id) const override {
    using enum_type = std::underlying_type<RegisterId>::type;
    const enum_type rid = static_cast<enum_type>(id);
    if (rid < 16u) {
      return Reg::ReadRegister(pstates_, rid);
    } else {
      return 0;
    }
  }

  /// @copydoc IRegAccess::WriteRegister
  void WriteRegister(const RegisterId &id, u32 value) override {
    using enum_type = std::underlying_type<RegisterId>::type;
    const enum_type rid = static_cast<enum_type>(id);
    if (rid < 16u) {
      Reg::WriteRegister(pstates_, rid, value);
    } else {
      // Edge case: Register ID is greater than or equal to 16.
      // Do nothing.
    }
  }

private:
  PState &pstates_; /**< The processor states used by the RegAccess object. */
};
} // namespace internal
} // namespace microemu