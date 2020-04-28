#ifndef IP_H_
#define IP_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/** @brief Get the %ip for a given hostname.
 *
	 *  @details This method gets a hostname and tries to resolve it using
 *  SOCK_STREAM and IPPROTO_TCP as hints. It returns the uint32_t value
 *  stored in 'sockaddr_in->sin_addr.s_addr' field.
 *
 *  @param hostname The hostname to be resolved in an %ip
 */
uint32_t get_ip_by_name(const char *hostname);

#ifdef __cplusplus
}
#endif

#endif // IP_H_
