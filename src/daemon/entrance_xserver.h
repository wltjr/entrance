#ifndef ENTRANCE_XSERVER_H_
#define ENTRANCE_XSERVER_H_

typedef void (*Entrance_X_Cb)(const char *data);
int entrance_xserver_init(Entrance_X_Cb start, const char *dname);
void entrance_xserver_shutdown(void);

#endif /* ENTRANCE_XSERVER_H_ */
