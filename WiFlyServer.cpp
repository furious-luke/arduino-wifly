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
  // TODO: Ensure no active non-server client connection.

   // if( !WiFly.serverConnectionActive ) {
   //    activeClient._port = 0;
   // }

  // TODO: Ensure begin() has been called.

   // Return immediately if there is nothing present in the
   // buffer.
   if( !_dev->_uart->available() )
      return false;

  // Return active server connection if present
   // TODO: Handle this better
   if( _dev->_uart->find( TOKEN_MATCH_OPEN ) )
   {
      // The following values indicate that the connection was
      // created when acting as a server.

      // TODO: Work out why this alternate instantiation code doesn't work:
      //activeClient = WiFlyClient((uint8_t*) NULL, _port);

      client.connect( _port, NULL, NULL );
      return true;
   }
   else
      return false;
}
