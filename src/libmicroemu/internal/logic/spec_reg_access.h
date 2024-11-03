#include "libmicroemu/emu_context.h"

namespace microemu {
namespace internal {

/**
 * @class SpecialRegAccess
 * @brief Provides access to special registers.
 *
 * The SpecialRegAccess class allows reading and writing values to special registers.
 * It is templated on the types TProcessorStates and TSpecRegOps, which represent the processor
 * states and special register operations respectively.
 *
 * @tparam TProcessorStates The type representing the processor states.
 * @tparam TSpecRegOps The type representing the operations on special registers.
 */
template <typename TProcessorStates, typename TSpecRegOps>
class SpecialRegAccess : public ISpecialRegAccess {
public:
  using PState = TProcessorStates;
  using SReg = TSpecRegOps;

  /**
   * @brief Constructs a SpecialRegAccess object with the given processor states.
   *
   * @param pstate The processor states to be used by the SpecialRegAccess object.
   */
  SpecialRegAccess(PState &pstates) : pstates_(pstates) {}

  /**
   * @brief Default destructor for the SpecialRegAccess object.
   */
  ~SpecialRegAccess() = default;

  /**
   * @brief Default copy constructor for the SpecialRegAccess object.
   */
  SpecialRegAccess(const SpecialRegAccess &) = default;

  /**
   * @brief Default copy assignment operator for the SpecialRegAccess object.
   */
  SpecialRegAccess &operator=(const SpecialRegAccess &) = default;

  /**
   * @brief Default move constructor for the SpecialRegAccess object.
   */
  SpecialRegAccess(SpecialRegAccess &&) = default;

  /**
   * @brief Default move assignment operator for the SpecialRegAccess object.
   */
  SpecialRegAccess &operator=(SpecialRegAccess &&) = default;

  const char *GetRegisterName(const uint8_t &reg_id) const override {
    return SReg::GetRegisterName(static_cast<SpecialRegisterId>(reg_id));
  }

  const char *GetRegisterName(const SpecialRegisterId &reg_id) const override {
    return SReg::GetRegisterName(reg_id);
  }

  /**
   * @brief Reads the value of the specified special register.
   *
   * @param reg_id The ID of the special register to be read.
   * @return The value of the special register.
   */
  u32 ReadRegister(const SpecialRegisterId &reg_id) const override {
    return SReg::ReadRegister(pstates_, reg_id);
  }

  /**
   * @brief Writes the value to the specified special register.
   *
   * @param reg_id The ID of the special register to be written.
   * @param value The value to be written to the special register.
   */
  void WriteRegister(const SpecialRegisterId &reg_id, u32 value) override {
    return SReg::WriteRegister(pstates_, reg_id, value);
  }

private:
  PState &pstates_; /**< The processor states used by the SpecialRegAccess object. */
};

} // namespace internal
} // namespace microemu