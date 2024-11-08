#include "libmicroemu/emu_context.h"

namespace microemu {
namespace internal {

/**
 * @class SpecialRegAccessor
 * @brief Provides access to special registers.
 *
 * The SpecialRegAccessor class allows reading and writing values to special registers.
 * It is templated on the types TProcessorStates and TSpecRegOps, which represent the processor
 * states and special register operations respectively.
 *
 * @tparam TProcessorStates The type representing the processor states.
 * @tparam TSpecRegOps The type representing the operations on special registers.
 */
template <typename TProcessorStates, typename TSpecRegOps>
class SpecialRegAccessor : public ISpecialRegAccessor {
public:
  using PState = TProcessorStates;
  using SReg = TSpecRegOps;

  /**
   * @brief Constructs a SpecialRegAccessor object with the given processor states.
   *
   * @param pstate The processor states to be used by the SpecialRegAccessor object.
   */
  SpecialRegAccessor(PState &pstates) : pstates_(pstates) {}

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
  const char *GetRegisterName(const SpecialRegisterId &reg_id) const override {
    return SReg::GetRegisterName(reg_id);
  }

  /// @copydoc ISpecialRegAccessor::ReadRegister
  u32 ReadRegister(const SpecialRegisterId &reg_id) const override {
    return SReg::ReadRegister(pstates_, reg_id);
  }

  /// @copydoc ISpecialRegAccessor::WriteRegister
  void WriteRegister(const SpecialRegisterId &reg_id, u32 value) override {
    return SReg::WriteRegister(pstates_, reg_id, value);
  }

private:
  PState &pstates_; /**< The processor states used by the SpecialRegAccessor object. */
};

} // namespace internal
} // namespace microemu