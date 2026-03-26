#ifndef ENTRANCE_XSERVER_H_
#define ENTRANCE_XSERVER_H_

typedef void (*Entrance_X_Cb)(const char *data);
int entrance_xserver_init(Entrance_X_Cb start, const char *dname);

/**
 * @brief Allocate memory for Entrance_Xserver structs, one per X server
 *
 * @param count the number of servers to allocate memory
 */
void entrance_xservers_init(int count);

void entrance_xserver_shutdown(void);

#endif /* ENTRANCE_XSERVER_H_ */
