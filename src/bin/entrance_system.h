#ifndef ENTRANCE_SYSTEM_H_
#define ENTRANCE_SYSTEM_H_

/**
 * @brief Get system distro name, detect distro from /etc/os-release (lazy init)
 *
 * @return const char* system distro name
 */
const char *entrance_system_distro_get(void);

/**
 * @brief Get distro logo path resolved from standard icon names (lazy init)
 *
 * @return const char* distro logo path
 */
const char *entrance_system_logo_get(void);

/**
 * @brief Shutdown system, free memory
 */
void entrance_system_shutdown(void);

#endif /* ENTRANCE_SYSTEM_H_ */
