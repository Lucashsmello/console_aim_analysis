#ifndef GIMX_API_H
#define GIMX_API_H

void setAimSpeed(int x, int y, int shoot = 0, unsigned int* poolTime_usec_OUT = 0);
void pressX(bool release = false);
void startGimxServer();

void initializeGIMXConnetion();

#endif
