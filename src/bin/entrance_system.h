#ifndef ENTRANCE_SYSTEM_H_
#define ENTRANCE_SYSTEM_H_

const char *entrance_system_distro_get(void);
const char *entrance_system_logo_get(void);

/**
 * @brief Shutdown system, free memory
 */
void entrance_system_shutdown();

#endif /* ENTRANCE_SYSTEM_H_ */
