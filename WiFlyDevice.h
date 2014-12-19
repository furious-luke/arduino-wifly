#ifndef WiFlyDevice_h
#define WiFlyDevice_h

#include <Arduino.h>
#include "SpiUartDevice.h"
#include "insist.h"

#define DEFAULT_SERVER_PORT 80

class WiFlyServer;
class WiFlyClient;

class WiFlyDevice
{
public:

   WiFlyDevice( SpiUartDevice* uart = NULL );

   void
   set_uart( SpiUartDevice* uart );

   void
   begin();

   bool
   join( const char* ssid,
         bool command_mode = false );

   bool
   join( const char* ssid,
         const char* phrase,
         bool wpa = true );

   void
   sleep( unsigned seconds );

   // bool
   // configure( byte option,
   //            unsigned long value );

   // long
   // time();

   const char*
   ip();
    
private:
    
//    // TODO: Should these be part of a different class?
//    // TODO: Should all methods that need to be in command mode ensure
//    //       they are first?
//    void
//    _attempt_switch_to_command_mode();

//    void
//    _switch_to_command_mode();

   void
   _reboot();

   bool
   _software_reboot( bool is_after_boot = true );

   bool
   _hardware_reboot();

//    void
//    _require_flow_control();

   void
   _set_configuration();

   bool
   _send_command( const char* command,
                  bool multipart = false,
                  const char* response = "AOK",
		  unsigned time_out = 1000 );
   bool
   _send_command( const __FlashStringHelper* command,
                  bool multipart = false,
                  const char* response = "AOK",
		  unsigned time_out = 1000 );

   void
   _wait_for_response( const char* to_match );

   void
   _skip_remainder_of_response();

//    bool
//    _response_matched( const char* to_match );

   bool
   _find_in_response( const char* to_match,
                      unsigned time_out = 1000 );

   bool
   _enter_command_mode();

// private:

   SpiUartDevice* _uart;
   uint16_t _server_port;

   friend class WiFlyClient;
   friend class WiFlyServer;
};

#endif
