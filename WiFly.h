#ifndef wifly_h
#define wifly_h

#include "assert.h"
#include "insist.h"
#include "log.h"
#include "SpiUartDevice.h"
#include "WiFlyDevice.h"
#include "WiFlyClient.h"
#include "WiFlyServer.h"

// TODO: Don't make this extern
// TODO: How/whether to allow sending of arbitrary data/commands over serial?
// NOTE: Only the WiFly object is intended for external use
extern WiFlyDevice wifly;

#endif
