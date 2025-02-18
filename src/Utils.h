#ifndef UTILS_H_
#define Utils_H_

#include "global.h"




void saveRTCdata(bool init);
void readRTCdata();

void printSetpoint();
void printData(struct_message myData);

void updateSimulation();

#endif