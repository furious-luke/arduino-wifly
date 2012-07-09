#ifndef insist_h
#define insist_h

#include "assert.h"

#ifndef NDEBUG

#define INSIST( stmnt, cmp, msg )               \
   ASSERT( (stmnt) cmp, msg )

#else

#define INSIST( stmnt, cmp, msg ) stmnt;

#endif

#endif
