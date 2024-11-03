#include "libmicroemu/emu_context.h"
// Write implementation of IRegisterAccess
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
class RegAccess : public IRegisterAccess {
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

  const char *GetRegisterName(const uint8_t &reg_id) const override {
    return Reg::GetRegisterName(reg_id);
  }
  /**
   * @brief Reads the value of the specified register.
   *
   * @param reg_id The ID of the register to be read. Must be in the range of 0-15.
   * @return The value of the register.
   *
   * @note If the given register ID is greater than or equal to 16, the method returns 0.
   */
  u32 ReadRegister(const uint8_t &reg_id) const override {
    if (reg_id < 16u) {
      return Reg::ReadRegister(pstates_, reg_id);
    } else {
      return 0;
    }
  }

  /**
   * @brief Writes the value to the specified register.
   *
   * @param reg_id The ID of the register to be written. Must be in the range of 0-15.
   * @param value The value to be written to the register.
   *
   * @note If the given register ID is greater than or equal to 16, the method does nothing.
   */
  void WriteRegister(const uint8_t &reg_id, u32 value) override {
    if (reg_id < 16u) {
      Reg::WriteRegister(pstates_, reg_id, value);
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