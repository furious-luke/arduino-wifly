#include "SpiUartDevice.h"
#include "log.h"
#include "assert.h"
#include "insist.h"

// See section 8.10 of the datasheet for definitions
// of bits in the Enhanced Features Register (EFR)
#define EFR_ENABLE_CTS 1 << 7
#define EFR_ENABLE_RTS 1 << 6
#define EFR_ENABLE_ENHANCED_FUNCTIONS 1 << 4

// See section 8.4 of the datasheet for definitions
// of bits in the Line Control Register (LCR)
#define LCR_ENABLE_DIVISOR_LATCH 1 << 7

#define XTAL_FREQUENCY 14745600UL // On-board crystal (New mid-2010 Version)

// See datasheet section 7.8 for configuring the
// "Programmable baud rate generator"
#define PRESCALER 1 // Default prescaler after reset
#define BAUD_RATE_DIVISOR( baud ) ((XTAL_FREQUENCY/PRESCALER)/(baud*16UL))

// TODO: Handle configuration better
// SC16IS750 register values
struct spi_uart_cfg_type
{
   char data_format;
   char flow;
};

struct spi_uart_cfg_type spi_uart_cfg = {
   0x03,
   // We need to enable flow control or we overflow buffers and
   // lose data when used with the WiFly. Note that flow control 
   // needs to be enabled on the WiFly for this to work but it's
   // possible to do that with flow control enabled here but not there.
   // TODO: Make this able to be configured externally?
   EFR_ENABLE_CTS | EFR_ENABLE_RTS | EFR_ENABLE_ENHANCED_FUNCTIONS
};

///
/// Initialize SPI and UART communications.
///
void
SpiUartDevice::begin( unsigned long baud )
{
   SPI.begin();
   _init_uart( baud );
}

///
/// Get the number of bytes (characters) available for reading.
///
int
SpiUartDevice::available()
{
   /*
    * 
    *
    * This is data that's already arrived and stored in the receive
    * buffer (which holds 64 bytes).
    */

   // This alternative just checks if there's data but doesn't
   // return how many characters are in the buffer:
   //    readRegister(LSR) & 0x01
   return _read_register( RXLVL );
}

///
/// Read byte from UART.
///
int
SpiUartDevice::read()
{
   if( !available() )
      return -1;
   else
      return _read_register( RHR );
}

///
/// Write byte to UART.
///
size_t
SpiUartDevice::write( byte value )
{
   // Wait for space in TX buffer
   while( _read_register( TXLVL ) == 0 );
   _write_register( THR, value );
   return 1;
}

///
/// Write string to UART.
///
size_t
SpiUartDevice::write( const uint8_t* buffer,
                      size_t size )
{
   while( size-- )
   {
      write( *buffer++ );
      while( _read_register( TXLVL ) < 64 ); // Wait for empty TX buffer (slow)
   }
}

///
/// Flush characters from SC16IS750 receive buffer.
///
void
SpiUartDevice::flush()
{
   // Note: This may not be the most appropriate flush approach.
   //       It might be better to just flush the UART's buffer
   //       rather than the buffer of the connected device
   //       which is essentially what this does.
   while( available() > 0 )
      read();
}

///
///
///
int
SpiUartDevice::peek()
{
   return 0;
}

///
///
///
void
SpiUartDevice::io_set_direction( byte bits )
{
   _write_register( IODIR, bits );
}

///
///
///
void
SpiUartDevice::io_set_state( byte bits )
{
   _write_register( IOSTATE, bits );
}

///
/// Selects the SPI device.
///
void
SpiUartDevice::_select()
{
   digitalWrite( SS, LOW );
}

///
/// Deslects the SPI device.
///
void
SpiUartDevice::_deselect()
{
   digitalWrite( SS, HIGH );
}

///
/// Write <data> byte to the SC16IS750 register <registerAddress>
///
void
SpiUartDevice::_write_register( byte addr,
                                byte data )
{
   _select();
   SPI.transfer( addr );
   SPI.transfer( data );
   _deselect();
}

///
/// Read byte from SC16IS750 register at <registerAddress>.
///
byte
SpiUartDevice::_read_register( byte addr )
{
   // Used in SPI read operations to flush slave's shift register
   const byte dummy_byte = 0xFF; 

   char result;
   _select();
   SPI.transfer( SPI_READ_MODE_FLAG | addr );
   result = SPI.transfer( dummy_byte );
   _deselect();

   return result;
}

///
/// Initialise the UART.
///
void
SpiUartDevice::_init_uart( unsigned long baud )
{
   // Initialise and test SC16IS750
   _configure_uart( baud );
   INSIST( _uart_connected(), == true, "UART is not connected." );
}

///
///
///
void
SpiUartDevice::_configure_uart( unsigned long baud )
{
   // TODO: Improve with use of constants and calculations.
   _set_baud_rate( baud );

   _write_register( LCR, 0xBF ); // access EFR register
   _write_register( EFR, spi_uart_cfg.flow ); // enable enhanced registers
   _write_register( LCR, spi_uart_cfg.data_format); // 8 data bit, 1 stop bit, no parity
   _write_register( FCR, 0x06 ); // reset TXFIFO, reset RXFIFO, non FIFO mode
   _write_register( FCR, 0x01 ); // enable FIFO mode
}

///
///
///
void
SpiUartDevice::_set_baud_rate( unsigned long baud )
{
   unsigned long divisor = BAUD_RATE_DIVISOR( baud );
   _write_register( LCR, LCR_ENABLE_DIVISOR_LATCH ); // "Program baudrate"
   _write_register( DLL, lowByte( divisor ) );
   _write_register( DLM, highByte( divisor ) );
}

///
///
///
bool
SpiUartDevice::_uart_connected()
{
   const char test_char = 'H';
   _write_register( SPR, test_char );
   return _read_register(SPR) == test_char;
}
