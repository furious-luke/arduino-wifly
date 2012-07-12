#ifndef WiFly_WiFlyClient_h
#define WiFly_WiFlyClient_h

#include <Stream.h>
#include "WiFlyDevice.h"

class WiFlyClient
   : public Stream
{
public:

   WiFlyClient( WiFlyDevice& dev,
                uint8_t* ip = NULL,
                uint16_t port = 0 );

   WiFlyClient( WiFlyDevice& dev,
                const char* domain,
                uint16_t port );

   bool
   connect( uint16_t port = 0,
            uint8_t* ip = NULL,
            const char* domain = NULL );

   size_t
   write( byte value );

   size_t
   write( const char* str );

   size_t
   write( const uint8_t* buffer,
          size_t size );

   int
   available();

   int
   read();

   String
   readString();

   bool
   find( const char* str,
         unsigned timeout = 1000 );

   void
   flush();

   int
   peek();

   bool
   connected();

   void
   close();

private:

   WiFlyDevice* _dev;
   uint8_t* _ip;
   uint16_t _port;
   const char *_domain;
   bool _open;
};

#endif
