#ifndef GIMX_API_H
#define GIMX_API_H

void setAimSpeed(int x, int y, int shoot = 0);
void pressX(bool release = false);
void pressButton(int b_id, bool press = true);
void startGimxServer();

#endif
