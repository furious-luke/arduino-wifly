#ifndef WiFly_WiFlyServer_h
#define WiFly_WiFlyServer_h

#include <stdint.h>

class WiFlyDevice;
class WiFlyClient;

extern WiFlyDevice wifly;
 
class WiFlyServer  // TODO: Should subclass Print to be consistent
{
public:

   WiFlyServer( WiFlyDevice& dev = wifly,
                uint16_t port = 80 );

   void
   begin();

   bool
   available( WiFlyClient& client );

 private:

   WiFlyDevice* _dev;
   uint16_t _port;
};

#endif
