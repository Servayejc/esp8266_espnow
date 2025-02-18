#ifndef TEMPERATURES_H_
#define TEMPERATURES_H_
#include <Arduino.h>
#include "global.h"

void StartConversion(uint8_t busNdx);
void StartAllConversion();
float ReadTemp(byte Ndx);
void ReadTemperatures();
void getReadings(uint8_t Ndx, struct_message *myData);
void getAddress();
String search();
void searchAll();
#endif