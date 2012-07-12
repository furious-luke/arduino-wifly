#include "WiFlyDevice.h"
#include "WiFlyServer.h"
#include "WiFlyClient.h"

#define TOKEN_MATCH_OPEN "*OPEN*"

///
///
///
WiFlyServer::WiFlyServer( WiFlyDevice& dev,
                          uint16_t port )
   : _dev( &dev ),
     _port( port )
{
}

///
///
///
void
WiFlyServer::begin()
{
}

///
///
///
bool
WiFlyServer::available( WiFlyClient& client )
{
   // Return immediately if there is nothing present in the
   // buffer.
   if( !_dev->_uart->available() )
      return false;

   // Return active server connection if present
   if( _dev->_uart->find( TOKEN_MATCH_OPEN ) )
   {
      client.connect( _port, NULL, NULL );
      return true;
   }
   else
      return false;
}
