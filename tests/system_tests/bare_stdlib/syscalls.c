
/// The newlibc and stdlibc do need some basic functions which the user must
/// supply. Without these functions there are undefined symbols after linking.
/// Most of the listed functions make no sense in an embedded bare metal
/// application, but must be defined nonetheless.

/// @brief Terminate process.
///
/// @param status User supplied argument to exit() call.
///
/// @returns This function does not return.
/// @ingroup SystemCalls
void _exit(int status) {
  __asm__("MOV r0, %[exit_code] \n"
          "SVC #0x01           \n"
          :
          : [exit_code] "r"(status)
          : "r0");
  while (1) {
  }
}
