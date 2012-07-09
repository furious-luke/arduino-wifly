#include "SpiUartDevice.h"
#include "WiFlyDevice.h"

SpiUartDevice spi_serial;
WiFlyDevice wifly( &spi_serial );
