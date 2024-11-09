#pragma once
#include "libmicroemu/exception_type.h"
#include "libmicroemu/internal/bus/mem_access_results.h"
#include "libmicroemu/logger.h"
#include "libmicroemu/register_details.h"
#include "libmicroemu/result.h"
#include "libmicroemu/special_register_id.h"
#include "libmicroemu/types.h"
#include <type_traits>

namespace libmicroemu::internal {

enum class BusExceptionType {
  kRaiseNoException,           // No exception has occurred.
  kRaiseStkerr,                // STKERR: A derived bus fault has occurred on exception entry.
  kRaiseUnstkerr,              // UNSTKERR: A derived bus fault has occurred on exception return.
  kRaiseImpreciseDataBusError, // IMPRECISERR: An imprecise data bus error has occurred.
  kRaisePreciseDataBusError,   // PRECISERR: A precise data bus error has occurred.
  kRaiseInstructionBusError,   // IBUSERR: An instruction bus error has occurred.
};

template <typename TProcessorStates, typename TSpecRegOps, typename TExceptionTrigger,
          typename TLogger, typename... TBusParticipant>
class Bus : public TBusParticipant... {
public:
  using SReg = TSpecRegOps;
  using ExcTrig = TExceptionTrigger;

  explicit Bus(const TBusParticipant &...participant) : TBusParticipant(participant)... {}

  /// \brief Destructor
  virtual ~Bus() = default;

  /// \brief Copy constructor for Bus.
  /// \param r_src the object to be copied
  Bus(const Bus &r_src) = default;

  /// \brief Copy assignment operator for Bus.
  /// \param r_src the object to be copied
  Bus &operator=(const Bus &r_src) = default;

  /// \brief Move constructor for Bus.
  /// \param r_src the object to be copied
  Bus(Bus &&r_src) = default;

  /// \brief Move assignment operator for Bus.
  /// \param r_src the object to be copied
  Bus &operator=(Bus &&r_src) = default;

  template <typename T> Result<T> Read(TProcessorStates &pstates, me_adr_t vadr) const {
    auto read_result = ForwardRead<T, TBusParticipant...>(pstates, vadr);
    switch (read_result.status_code) {
    case ReadStatusCode::kOk: {
      return Ok<T>(read_result.content);
    }
    case ReadStatusCode::kReadNotAllowed: {
      // ExcTrig::SetPending(pstates, ExceptionType::kSysTick);
      return Err<T>(StatusCode::kScMemInaccesible);
    }
    default: {
      return Err<T>(StatusCode::kScUnexpected);
    }
    }
    // not reachable
  }

  template <typename T>
  Result<T> ReadOrRaise(TProcessorStates &pstates, me_adr_t vadr, BusExceptionType exc_type) const {
    auto read_res = ForwardRead<T, TBusParticipant...>(pstates, vadr);

    if (read_res.status_code == ReadStatusCode::kOk) {
      return Ok(read_res.content);
    }

    switch (exc_type) {
    case BusExceptionType::kRaiseNoException: {
      return Ok(read_res.content); // No exception to be triggered
    }
    case BusExceptionType::kRaiseStkerr: {
      // Bfar will not be updated according to the ARMv7-M Architecture Reference Manual
      // SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      // cfsr |= SReg::kCfsrBusFaultBfarValidMsk;
      cfsr |= CfsrBusFault::kStkerrMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok(read_res.content);
    }
    case BusExceptionType::kRaiseUnstkerr: {
      // Bfar will not be updated according to the ARMv7-M Architecture Reference Manual
      // SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      // cfsr |= SReg::kCfsrBusFaultBfarValidMsk;
      cfsr |= CfsrBusFault::kUnstkerrMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok(read_res.content);
    }
    case BusExceptionType::kRaiseImpreciseDataBusError: {
      // SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      // cfsr |= SReg::kCfsrBusFaultBfarValidMsk;
      cfsr |= CfsrBusFault::kImpreciseErrMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok(read_res.content);
    }
    case BusExceptionType::kRaisePreciseDataBusError: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kPreciseErrMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok(read_res.content);
    }
    case BusExceptionType::kRaiseInstructionBusError: {
      // SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      // cfsr |= SReg::kCfsrBusFaultBfarValidMsk;
      cfsr |= CfsrBusFault::kIbuErrMsk;
      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok(read_res.content);
    }
    }
    // not reachable
    return Err<T>(StatusCode::kScUnexpected);
  }

  template <typename T>
  Result<void> Write(TProcessorStates &pstates, me_adr_t vadr, T value) const {

    auto write_res = ForwardWrite<T, TBusParticipant...>(pstates, vadr, value);
    switch (write_res.status_code) {
    case WriteStatusCode::kOk: {
      return Ok();
    }
    case WriteStatusCode::kWriteNotAllowed: {
      return Err(StatusCode::kScMemInaccesible);
    }
    default: {
      return Err(StatusCode::kScUnexpected);
    }
    }

    // not reachable
  }

  template <typename T>
  Result<void> WriteOrRaise(TProcessorStates &pstates, me_adr_t vadr, T value,
                            BusExceptionType exc_type) const {
    auto write_res = ForwardWrite<T, TBusParticipant...>(pstates, vadr, value);
    if (write_res.status_code == WriteStatusCode::kOk) {
      return Ok();
    }

    switch (exc_type) {
    case BusExceptionType::kRaiseNoException: {
      return Ok(); // No exception to be triggered
    }
    case BusExceptionType::kRaiseStkerr: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kStkerrMsk;

      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      return Ok();
    }
    case BusExceptionType::kRaiseUnstkerr: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kUnstkerrMsk;

      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      return Ok();
    }
    case BusExceptionType::kRaiseImpreciseDataBusError: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kImpreciseErrMsk;

      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      return Ok();
    }
    case BusExceptionType::kRaisePreciseDataBusError: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kPreciseErrMsk;

      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      return Ok();
    }
    case BusExceptionType::kRaiseInstructionBusError: {
      SReg::template WriteRegister<SpecialRegisterId::kBfar>(pstates, vadr);

      ExcTrig::SetPending(pstates, ExceptionType::kBusFault);
      auto cfsr = SReg::template ReadRegister<SpecialRegisterId::kCfsr>(pstates);
      cfsr |= CfsrBusFault::kBfarValidMsk;
      cfsr |= CfsrBusFault::kIbuErrMsk;

      SReg::template WriteRegister<SpecialRegisterId::kCfsr>(pstates, cfsr);
      return Ok();
    }
    }
    // not reachable
    return Err(StatusCode::kScUnexpected);
  }

private:
  template <typename T, typename TAct, typename... Rest>
  ReadResult<T> ForwardRead(TProcessorStates &pstates, me_adr_t vadr) const {
    if (!TAct::IsVAdrInRange(vadr)) {
      return ForwardRead<T, Rest...>(pstates, vadr);
    }
    const auto read_result = TAct::template Read<T>(pstates, vadr);
    return read_result;
  }

  template <typename T> ReadResult<T> ForwardRead(TProcessorStates &pstates, me_adr_t vadr) const {
    static_cast<void>(vadr);
    static_cast<void>(pstates);
    return ReadResult<T>{T{}, ReadStatusCode::kReadNotAllowed};
  }

  /// \brief Forward write operation to the next bus participant
  /// \details This function is called when the current bus participant is read-only
  /// \tparam T the type of the value to be written
  /// \tparam TAct the type of the bus participant
  /// \tparam Rest the rest of the bus participants
  /// \param vadr the virtual address to be written
  /// \param value the value to be written
  /// \return the result of the write operation
  template <typename T, typename TAct, typename... Rest,
            typename std::enable_if_t<TAct::kReadOnly, T> = 0>
  WriteResult<T> ForwardWrite(TProcessorStates &pstates, me_adr_t vadr, T value) const {
    if (!TAct::IsVAdrInRange(vadr)) {
      return ForwardWrite<T, Rest...>(pstates, vadr, value);
    }
    return WriteResult<T>{WriteStatusCode::kWriteNotAllowed};
  }

  /// \brief Forward write operation to the next bus participant
  /// \details This function is called when the current bus participant allows write-access
  /// \tparam T the type of the value to be written
  /// \tparam TAct the type of the bus participant
  /// \tparam Rest the rest of the bus participants
  /// \param vadr the virtual address to be written
  /// \param value the value to be written
  /// \return the result of the write operation
  template <typename T, typename TAct, typename... Rest,
            typename std::enable_if_t<!TAct::kReadOnly, T> = 0>
  WriteResult<T> ForwardWrite(TProcessorStates &pstates, me_adr_t vadr, T value) const {
    if (!TAct::IsVAdrInRange(vadr)) {
      return ForwardWrite<T, Rest...>(pstates, vadr, value);
    }
    const auto write_res = TAct::template Write<T>(pstates, vadr, value);
    return write_res;
  }

  template <typename T>
  WriteResult<T> ForwardWrite(TProcessorStates &pstates, me_adr_t vadr, T value) const {
    static_cast<void>(vadr);
    static_cast<void>(value);
    static_cast<void>(pstates);
    return WriteResult<T>{WriteStatusCode::kWriteNotAllowed};
  }
};

} // namespace libmicroemu::internal
