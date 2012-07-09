#ifndef WiFly_h
#define WiFly_h

#include "SpiUartDevice.h"
#include "WiFlyDevice.h"
#include "WiFlyClient.h"
#include "WiFlyServer.h"

// TODO: Don't make this extern
// TODO: How/whether to allow sending of arbitrary data/commands over serial?
// NOTE: Only the WiFly object is intended for external use
extern SpiUartDevice spi_serial;
extern WiFlyDevice wifly;

#endif
