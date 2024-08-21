#ifndef TEMPERATURES_H_
#define TEMPERATURES_H_
#include <Arduino.h>

void StartConversion(uint8_t busNdx);
void StartAllConversion();
float ReadTemp(byte Ndx);
void ReadTemperatures();
void getAddress();
String search();
void searchAll();
#endif