/*
  axis_value_parser.h - Library for SPI message deserialization
  with zcor slave (https://github.com/padand/zcor)
  Created by Andrei Paduraru, January 11, 2021.
  Released into the public domain, MIT license.
*/
#ifndef AXIS_VALUE_PARSER
#define AXIS_VALUE_PARSER

#include<stdint.h>

#define AXIS_VALUE_SIZE 6
#define AXIS_VALUE_DECIMALS 2
#define AXIS_VALUE_VERIFY_CYCLES 2

class AxisValueParser
{
  public:
    AxisValueParser();
    // fills axisValue array with 0;
    // fills axisValueVerify array with 0;
    void init();
    // checks if axisValue elements have matched for
    // at least AXIS_VALUE_VERIFY_CYCLES cycles
    bool verify();
    // adds a value and shifts the index
    void add(char value);
    // returns the current position
    float pos();
  private:
    // current axis value (eg: 101.43)
    // negative values not allowed!
    // padded with zeroes
    char axisValue[AXIS_VALUE_SIZE];
    // checks if the axisValue was filled correctly
    // axisValue is correct when it passes a few
    // cycles (AXIS_VALUE_VERIFY_CYCLES) without changing
    unsigned int axisValueVerify[AXIS_VALUE_SIZE];
    // current index for the axis value
    volatile unsigned int axisValueIndex = 0;
    // increments the index for the axis value,
    // making shure to start over if the end was reached
    void shift();
    // shifts the values while the dot is in the correct position
    void shiftAll();
};

#endif