#include "globals.h"
#include "WiFly.h"
#include "WiFlyClient.h"

///
///
///
WiFlyClient::WiFlyClient( WiFlyDevice& dev,
                          uint8_t* ip,
                          uint16_t port)
   : _ip( ip ),
     _port( port ),
     _domain( NULL ),
     _dev( &dev ),
     _open( false )
{
}

///
///
///
WiFlyClient::WiFlyClient( WiFlyDevice& dev,
                          const char* domain,
                          uint16_t port )
   : _ip( NULL ),
     _port( port ),
     _domain( domain ),
     _dev( &dev ),
     _open( false )
{
}

///
///
///
size_t
WiFlyClient::write( byte value )
{
   return _dev->_uart->write( value );
}

///
///
///
size_t
WiFlyClient::write( const uint8_t *buffer,
                    size_t size )
{
   return _dev->_uart->write( buffer, size );
}

///
///
///
bool
WiFlyClient::connect( uint16_t port,
                      uint8_t* ip,
                      const char* domain )
{
   if( port != 0 )
      _port = port;
   if( ip != NULL || domain != NULL )
   {
      _ip = NULL;
      _domain = NULL;
   }
   _ip = ip;
   _domain = domain;

   if( _ip == NULL && _domain == NULL )
   {
   } else {
      ASSERT( 0, "Not implemented." );
      // _dev->_enter_command_mode();
      // _dev->_send_command( "open ", true, "" /* TODO: Remove this dummy value */ );
      // if (_ip != NULL) {
      //    for (int index = 0; /* break inside loop*/ ; index++) {
      //       _WiFly.uart->print(_ip[index], DEC);
      //       if (index == 3) {
      //          break;
      //       }
      //       _WiFly.uart->print('.');
      //    }
      // } else if (_domain != NULL) {
      //    _WiFly.uart->print(_domain);
      // } else {
      //    while (1) {
      //       // This should never happen
      //    }
      // }
    
      // _WiFly.uart->print(" ");
    
      // _WiFly.uart->print(_port, DEC);
    
      // _WiFly.sendCommand("", false, "*OPEN*");
    
      // // TODO: Handle connect failure
   }
  
   _open = true;
   return true;
}

///
///
///
int
WiFlyClient::available()
{
   if( !_open )
      return 0;
   return _dev->_uart->available();
}

///
///
///
int
WiFlyClient::read()
{
   if( !_open )
      return -1;
   return _dev->_uart->read();
}

///
///
///
String
WiFlyClient::readString()
{
   if( !_open )
      return String();
   return _dev->_uart->readString();
}

///
///
///
bool
WiFlyClient::find( const char* str,
                   unsigned timeout )
{
   if( !_open )
      return false;
   return _dev->_find_in_response( (char*)str, timeout );
}



///
///
///
int
WiFlyClient::peek()
{
   if( !_open )
      return -1;
   return _dev->_uart->peek();
}

///
///
///
void
WiFlyClient::flush()
{
   if( !_open )
      return;
   while( _dev->_uart->available() )
      _dev->_uart->read();
}

///
///
///
bool
WiFlyClient::connected()
{
   return _open;
}

///
///
///
void
WiFlyClient::close()
{
   _dev->_enter_command_mode();
   _dev->_uart->println( "close" );
   if( _dev->_uart->match_P( 2, F( "*CLOS*" ), F( "Err" ) ) == 1 )
      _dev->_uart->flush();
   _open = false; 
}
