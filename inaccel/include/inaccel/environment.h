#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/** @brief Namespace for InAccel's shared memory files
 *
 *  @details Do not modify directly this value. Instead modify\
 *  accordingly INACCEL_NAMESPACE environment variable in order to
 *  override default value
 */
extern const char *INACCEL_NAMESPACE;
/** @brief Hostname of the FPGA resource manager.
 *
 *  @details You should not modify directly this value. Instead modify
 *  accordingly CORAL_HOSTNAME environment variable in order to override
 *  default value (ensure first that resource manager resides in such
 *  hostname)
 */
extern const char *CORAL_HOSTNAME;
/** @brief %ip of the FPGA resource manager in byte order format.
 *
 *  @details This value cannot be modified. It calls ip class to resolve
 *  CORAL_HOSTNAME.
 */
extern uint32_t CORAL_IP;
/** @brief Listening port of the FPGA resource manager.
 *
 *  @details Do not modify directly this value. Instead modify
 *  accordingly CORAL_PORT environment variable in order to override
 *  default value (ensure that resource manager indeed listens on the
 *  port you supplied)
 */
extern int CORAL_PORT;
/** @brief PID of the master process.
 *
 *  @details Do not modify directly this value. This value is generated
 *  automatically.
 */
extern int pid;

#ifdef __cplusplus
}
#endif

#endif // ENVIRONMENT_H_
