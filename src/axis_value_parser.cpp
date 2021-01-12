/*
  axis_value_parser.cpp - Library for SPI message deserialization
  with zcor slave (https://github.com/padand/zcor)
  Created by Andrei Paduraru, January 11, 2021.
  Released into the public domain, MIT license.
*/

#include<axis_value_parser.h>
#include<stdlib.h>

AxisValueParser::AxisValueParser() {
}

void AxisValueParser::init() {
  for(unsigned int i=0; i<AXIS_VALUE_SIZE; i++) {
    axisValue[i] = 0;
    axisValueVerify[i] = 0;
  }
}

void AxisValueParser::shift() {
  if(axisValueIndex < AXIS_VALUE_SIZE-1){
    axisValueIndex++;
  } else {
    axisValueIndex=0;
  }
}

void AxisValueParser::add(char value) {
  if(axisValue[axisValueIndex] != value) {
    axisValue[axisValueIndex] = value;
    axisValueVerify[axisValueIndex] = 0;
  } else {
    axisValueVerify[axisValueIndex] ++;
  }
  shift();
}

bool AxisValueParser::verify() {
  bool hasCycles = false;
  for(unsigned int i=0; i<AXIS_VALUE_SIZE && !hasCycles; i++) {
    if(axisValueVerify[i]<AXIS_VALUE_VERIFY_CYCLES) hasCycles = true;
  }
  return !hasCycles;
}

void AxisValueParser::shiftAll(){
  while(axisValue[AXIS_VALUE_SIZE-AXIS_VALUE_DECIMALS-1] != '.'){
    const char temp = axisValue[0];
    for(unsigned int i=1; i<AXIS_VALUE_SIZE; i++) {
      axisValue[i-1] = axisValue[i];
    }
    axisValue[AXIS_VALUE_SIZE-1] = temp;
  }
}

float AxisValueParser::pos() {
  shiftAll();
  return atof(axisValue);
};