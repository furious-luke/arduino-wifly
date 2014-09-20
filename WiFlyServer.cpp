#include "globals.h"
#include "WiFlyDevice.h"
#include "WiFlyServer.h"
#include "WiFlyClient.h"
#include "log.h"

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
   LOGLN( "Checking for incoming connections." );

   // Return immediately if there is nothing present in the
   // buffer.
   if( !_dev->_uart->available() )
      return false;
   LOGLN( "Data available at server." );

   // Return active server connection if present
   if( _dev->_uart->find( TOKEN_MATCH_OPEN ) )
   {
      LOGLN( "Client available." );
      client.connect( _port, NULL, NULL );
      return true;
   }
   else
      return false;
}
