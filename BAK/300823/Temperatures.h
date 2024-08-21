//2020-01-12
#ifndef TEMPERATURES_H_
#define TEMPERATURES_H_
#include <Arduino.h>

void StartConversion();
float ReadTemp(byte Ndx);
void ReadTemperatures();
void getAddress();
String search();
#endif