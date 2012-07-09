#ifndef log_h
#define log_h

#include <Arduino.h>

#define LOG_LEVEL 2

#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

#ifndef NLOG

#define LOG( level, message )                   \
   if( LOG_LEVEL >= level )                     \
   {                                            \
      Serial.println( message );                \
   }

#define LOGW( level, message )                  \
   if( LOG_LEVEL >= level )                     \
   {                                            \
      Serial.print( message );                  \
   }

#else

#define LOG( level, message )
#define LOGW( level, message )

#endif

#endif
