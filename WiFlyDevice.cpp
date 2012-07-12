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
   // Make sure we are in command mode.
   if( !command_mode )
      _enter_command_mode();

   _send_command( F( "join " ), true );
   if( _send_command( ssid, false, "Associated!") )
   {
      delay( 2000 );
      return _find_in_response( "IP=" );
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

// ///
// ///
// ///
// bool
// WiFlyDevice::_response_matched( const char* to_match )
// {
//    LOG( 1, "Enter 'WiFlyDevice::_response_matched'." );
// #ifndef NLOG
//    bool result = _find_in_response( to_match );
//    LOG( 1, "Exit 'WiFlyDevice::_response_matched'." );
//    return result;
// #else
//    return _find_in_response( to_match );
// #endif
// }

///
///
///
bool
WiFlyDevice::_enter_command_mode()
{
   _uart->print( F( "$$$" ) );
   int match = _uart->match_P( 2, F( "CMD" ), F( "$$$" ) );
   if( match == 1 )
   {
      _uart->println();
      _uart->flush();
   }
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
      if( !_enter_command_mode() )
         return false;

      _uart->println( F( "reboot" ) );
      if( _find_in_response( "*READY*", 2000 ) )
         return true;
   }

   return false;
}

///
///
///
bool
WiFlyDevice::_hardware_reboot()
{
   _uart->io_set_direction( 0b00000010 );
   _uart->io_set_state( 0b00000000 );
   delay( 1 );
   _uart->io_set_state( 0b00000010 );
   return _find_in_response( "*READY*", 2000 );
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
      if( !_find_in_response( response ) )
         return false;
   }
   return true;
}

// ///
// ///
// ///
// void
// WiFlyDevice::_require_flow_control()
// {
//    /*


//      Note: If flow control has been set but not saved then this
//      function won't handle it correctly.

//      Note: Any other configuration changes made since the last
//      reboot will also be saved by this function so this
//      function should ideally be called immediately after a
//      reboot.

//    */

//    LOG( 1, "Enter 'WiFlyDevice::_require_flow_control()'." );

//    _enter_command_mode();

//    // TODO: Reboot here to ensure we get an accurate response and
//    //       don't unintentionally save a configuration we don't intend?

//    _send_command( F( "get uart" ), false, "Flow=0x" );

//    while( !_uart->available() ); // Wait to ensure we have the full response

//    char flowControlState = _uart->read();

//    _uart->flush();

//    if( flowControlState == '1' )
//       return;

//    // Enable flow control
//    _send_command( F( "set uart flow 1" ) );

//    _send_command( F( "save" ), false, "Storing in config" );

//    // Without this (or some delay--but this seemed more useful/reliable)
//    // the reboot will fail because we seem to lose the response from the
//    // WiFly and end up with something like:
//    //     "*ReboWiFly Ver 2.18"
//    // instead of the correct:
//    //     "*Reboot*WiFly Ver 2.18"
//    // TODO: Solve the underlying problem
//    _send_command( F( "get uart" ), false, "Flow=0x1" );

//    _reboot();
// }

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

// ///
// /// Returns the time based on the NTP settings and time zone.
// ///
// long
// WiFlyDevice::time()
// {
//    LOG( 1, "Enter 'WiFlyDevice::time()'." );

//    char newChar;
//    byte offset = 0;
//    char buffer[TIME_SIZE + 1];

//    _enter_command_mode();

//    //_send_command("time"); // force update if it's not already updated with NTP server
//    _send_command( F( "show t t" ), false, "RTC=" );

//    // copy the time from the response into our buffer
//    while( offset < TIME_SIZE )
//    {
//       char ch = _uart->read();
//       if( ch != -1 )
//          buffer[offset++] = ch;
//    }
//    buffer[offset] = 0;
//    // This should skip the remainder of the output.
//    // TODO: Handle this better?
//    _wait_for_response( "<" );
//    _find_in_response( " " );

//    // For some reason the "_send_command" approach leaves the system
//    // in a state where it misses the first/next connection so for
//    // now we don't check the response.
//    // TODO: Fix this
//    _uart->println( F( "exit" ) );
//    //_send_command(F("exit"), false, "EXIT");


//    LOG( 1, "Exit 'WiFlyDevice::time()'." );
//    return strtol( buffer, NULL, 0 );
// }
