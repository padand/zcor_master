/*
  zcor_protocol.h - Definitions for SPI message protocol
  with zcor slave (https://github.com/padand/zcor)
  Created by Andrei Paduraru, January 12, 2021.
  Released into the public domain, MIT license.
*/

#ifndef ZCOR_PROTOCOL
#define ZCOR_PROTOCOL

#define REQUEST_RESET 0
#define RESPONSE_RESET 0
#define REQUEST_POSITION_READ(POS) POS
#define REQUEST_POSITION_STATUS 10
#define RESPONSE_POSITION_STATUS_OK(POS) ((REQUEST_POSITION_STATUS) + (POS))
#define REQUEST_POSITION_DIGIT 100
#define RESPONSE_IS_POSITION_DIGIT(RES) (RES > 100)
#define RESPONSE_POSITION_DIGIT(RES) (RES % 100)

#endif