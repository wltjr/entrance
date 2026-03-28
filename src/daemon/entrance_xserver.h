#ifndef ENTRANCE_XSERVER_H_
#define ENTRANCE_XSERVER_H_

typedef void (*Entrance_X_Cb)(const char *data);

/**
 * @brief Allocate memory for Entrance_Xserver structs, one per X server
 *
 * @param count the number of servers to allocate memory
 */
void entrance_xservers_init(int count);

/**
 * @brief Start a X server
 * 
 * @param id the id of the X server, array index for now, could be seat id later
 * @param start callback function to invoke when the X server is started
 * @param display the display number with colon
 * @return int PID of the X server process
 */
int entrance_xserver_start(int id, Entrance_X_Cb start, const char *display);

/**
 * @brief Shutdown a specific X server
 *
 * @param id of X server in array, index value for now, could be seat id later
 */
void entrance_xserver_shutdown(int id);

#endif /* ENTRANCE_XSERVER_H_ */
