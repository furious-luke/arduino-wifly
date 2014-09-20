#include "globals.h"
#include "WiFlyDevice.h"
#include "configuration.h"
#include "log.h"
#include "assert.h"
#include "insist.h"

#define COMMAND_MODE_ENTER_RETRY_ATTEMPTS 5
#define COMMAND_MODE_GUARD_TIME 250 // in milliseconds
#define SOFTWARE_REBOOT_RETRY_ATTEMPTS 5

#if USE_HARDWARE_RESET
#define REBOOT _hardware_reboot
#else
#define REBOOT _software_reboot
#endif

#define IP_ADDRESS_BUFFER_SIZE 16 // "255.255.255.255\0"
#define TIME_SIZE 11 // 1311006129

///
///
///
/// Note: Supplied UART should/need not have been initialised first.
///
WiFlyDevice::WiFlyDevice( SpiUartDevice* uart )
   : _uart( uart ),
     _server_port( DEFAULT_SERVER_PORT )
{
}

// // TODO: Create a constructor that allows a SpiUartDevice (or better a "Stream") to be supplied
// //       and/or allow the select pin to be supplied.

///
///
///
void
WiFlyDevice::set_uart( SpiUartDevice* uart )
{
   _uart = uart;
}

// TODO: Create a `begin()` that allows IP etc to be supplied.

///
///
///
void
WiFlyDevice::begin()
{
   if( _uart == NULL )
   {
      _uart = new SpiUartDevice;
      _uart->begin();
   }

   _reboot(); // Reboot to get device into known state
   // //requireFlowControl();
   _set_configuration();
}

///
///
///
bool
WiFlyDevice::join( const char* ssid,
                   bool command_mode )
{
   LOG( "Joining: " );
   LOGVLN( ssid );

   // Make sure we are in command mode.
   if( !command_mode )
      _enter_command_mode();

   _send_command( F( "set wlan ssid "), true );
   _send_command( ssid );
   if( _send_command( F( "join" ), false, "Associated!") )
   {
      LOGLN( "Associated." );

      // We should wait for the WiFly shield to tell us it's
      // got an IP before we assume we've successfully connected.
      return _find_in_response( "IP=", 10000 );
   }
   return false;
}

///
///
///
bool
WiFlyDevice::join( const char* ssid,
                   const char* passphrase,
                   bool is_wpa )
{
   // Make sure we are in command mode.
   _enter_command_mode();

   // TODO: Handle escaping spaces/$ in passphrase and SSID

   _send_command( F( "set wlan "), true );
   if( is_wpa )
      _send_command( F( "passphrase "), true );
   else
      _send_command( F( "key "), true );
   _send_command( passphrase );
   return join( ssid, true );
}

// ///
// ///
// ///
// bool
// WiFlyDevice::configure( byte option,
//                         unsigned long value )
// {
//    // TODO: Allow options to be supplied earlier?

//    switch( option )
//    {
//       case WIFLY_BAUD:
//          // TODO: Use more of standard command sending method?
//          _enter_command_mode();
//          _uart->print( "set uart instant " );
//          _uart->println( value );
//          delay(10); // If we don't have this here when we specify the
//          // baud as a number rather than a string it seems to
//          // fail. TODO: Find out why.
//          _uart->begin( value );
//          // For some reason the following check fails if it occurs before
//          // the change of SPI UART serial rate above--even though the
//          // documentation says the AOK is returned at the old baud
//          // rate. TODO: Find out why
//          if( !_find_in_response( "AOK", 100 ) )
//             return false;
//          break;

//       default:
//          return false;
//    }
//    return true;
// }

///
///
///
void
WiFlyDevice::sleep( unsigned seconds )
{
   _enter_command_mode();
   _send_command( F( "set sys wake " ), true );
   _uart->print( seconds + 1 );
   _send_command( F( "" ) );
   _send_command( F( "set sys sleep 1" ) );
   _send_command( F( "exit" ), false, "EXIT" );
   delay( 2 );
}

///
///
///
const char*
WiFlyDevice::ip()
{
   /*

     The return value is intended to be dropped directly
     into calls to 'print' or 'println' style methods.

   */
   static char ip[IP_ADDRESS_BUFFER_SIZE] = "";

   // TODO: Ensure we're not in a connection?

   _enter_command_mode();

   // Version 2.19 of the WiFly firmware has a "get ip a" command but
   // we can't use it because we want to work with 2.18 too.
   _send_command( F( "get ip" ), false, "IP=" );

   char newChar;
   byte offset = 0;

   // Copy the IP address from the response into our buffer
   while( offset < IP_ADDRESS_BUFFER_SIZE )
   {
      newChar = _uart->read();

      if( newChar == ':' )
      {
         ip[offset] = '\x00';
         break;
      }
      else if( newChar != -1 )
      {
         ip[offset] = newChar;
         offset++;
      }
   }

   // This handles the case when we reach the end of the buffer
   // in the loop. (Which should never happen anyway.)
   // And hopefully this prevents us from failing completely if
   // there's a mistake above.
   ip[IP_ADDRESS_BUFFER_SIZE-1] = '\x00';

   // This should skip the remainder of the output.
   // TODO: Handle this better?
   _wait_for_response( "<" );
   while (_uart->read() != ' '); // Skip the prompt.

   // For some reason the "sendCommand" approach leaves the system
   // in a state where it misses the first/next connection so for
   // now we don't check the response.
   // TODO: Fix this
   _uart->println( "exit" );
   //_send_command("exit", false, "EXIT");

   return ip;
}

///
///
///
/// Notes: Resets the timeout on the stream.
///
bool
WiFlyDevice::_find_in_response( const char* to_match,
                                unsigned timeout )
{
   unsigned long old_to = _uart->timeout();
   _uart->setTimeout( timeout );
   bool result = _uart->find( (char*)to_match );
   _uart->setTimeout( old_to );
   return result;
}

///
///
///
bool
WiFlyDevice::_enter_command_mode()
{
   LOGLN( "Entering command mode." );
   _uart->print( F( "$$$" ) );
   int match = _uart->match_P( 2, F( "CMD" ), F( "$$$" ) );
   if( match == 1 )
   {
      _uart->println();
      _uart->flush();
   }
#ifndef NLOG
   if( match != 0 && match != 1 )
      LOGLN( "Failed." );
   else
      LOGLN( "Succeeded." );
#endif
   return match == 0 || match == 1;
}

///
///
///
void
WiFlyDevice::_skip_remainder_of_response()
{
   while( !(_uart->available() && _uart->read() == '\n') );
}

///
///
///
void
WiFlyDevice::_wait_for_response( const char* to_match )
{
   while( !_find_in_response( to_match ) );
}

///
///
///
bool
WiFlyDevice::_software_reboot( bool is_after_boot )
{
   for( unsigned ii = 0; ii < SOFTWARE_REBOOT_RETRY_ATTEMPTS; ++ii )
   {
      if( _enter_command_mode() )
      {
	 _uart->println( F( "reboot" ) );
	 bool ok = _find_in_response( "*READY*", 5000 );
	 _skip_remainder_of_response();
	 if( ok )
	    return ok;
      }
   }
   return false;
}

///
///
///
bool
WiFlyDevice::_hardware_reboot()
{
   LOGLN( "Performing hardware reset." );
   _uart->io_set_direction( 0b00000010 );
   _uart->io_set_state( 0b00000000 );
   delay( 10 );
   _uart->io_set_state( 0b00000010 );
   bool ok = _find_in_response( "*READY*", 5000 );
#ifndef NLOG
   if( ok )
      LOGLN( "Okay." );
   else
      LOGLN( "Failed." );
#endif
   return ok;
}

///
///
///
void
WiFlyDevice::_reboot()
{
   INSIST( REBOOT(), == true, "WiFly reboot failed." );
}

///
///
///
bool
WiFlyDevice::_send_command( const char* command,
                            bool multipart,
                            const char* response )
{
   _uart->print( command );
   delay( 20 );
   if( !multipart )
   {
      _uart->flush();
      _uart->println();

      // TODO: Handle other responses
      //       (e.g. autoconnect message before it's turned off,
      //        DHCP messages, and/or ERR etc)
      if( !_find_in_response( response ) )
         return false;
   }
   return true;
}

///
///
///
bool
WiFlyDevice::_send_command( const __FlashStringHelper* command,
                            bool multipart,
                            const char* response )
{
   _uart->print( command );
   delay( 20 );
   if( !multipart )
   {
      _uart->flush();
      _uart->println();

      // TODO: Handle other responses
      //       (e.g. autoconnect message before it's turned off,
      //        DHCP messages, and/or ERR etc)
      // if( !_find_in_response( response ) )
      //    return false;
   }
   return true;
}

///
///
///
void
WiFlyDevice::_set_configuration()
{
   INSIST( _enter_command_mode(), == true, "Failed to enter command mode." );

   // TODO: Handle configuration better
   // Turn off auto-connect
   _send_command( F( "set wlan join 0" ) );

   // TODO: Turn off server functionality until needed
   //       with "set ip protocol <something>"

   // Set server port
   _send_command( F( "set ip localport " ), true );
   // TODO: Handle numeric arguments correctly.
   _uart->print( _server_port );
   _send_command( "" );

   // Turn off remote connect message
   _send_command( F( "set comm remote 0" ) );

   _send_command( F( "set t z 23" ) );
   _send_command( F( "set time address 129.6.15.28" ) );
   _send_command( F( "set time port 123" ) );
   _send_command( F( "set t e 15" ) );

   // CDT: Enable the DHCP mode again, if the shield
   // was last used in AdHoc mode we won't do things correctly without
   // these changes.
   _send_command( F( "set wlan auth 4" ) );
   _send_command( F( "set ip dhcp 1" ) );

   // Turn off status messages
   // _send_command(F("set sys printlvl 0"));

   // TODO: Change baud rate and then re-connect?

   // Turn off RX data echo
   // TODO: Should really treat as bitmask
   // _send_command(F("set uart mode 0"));
}
