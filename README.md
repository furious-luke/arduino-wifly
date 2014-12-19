WiFly Library
============

Description
-----------

This is a modified version of the Sparkfun WiFly Shield library
written by Chris Taylor and Philip J. Lindsay. I found the
Sparkfun library to be a little unreliable, mostly due to
a couple of bugs in the substring searching code. I've utilised
the parts I needed and rewritten sections to be more reliable,
cleaner and use later Arduino core libraries. In doing so I've
stripped out a lot of things I didn't need, such as AdHoc
network support. If there is any interest in having these features
returned I'm sure I could manage it.

Important
---------

I found a few issues with the standard implementation of the
Arduino stream functionality. In addition, I added a routine
to simplify some stream based matching used in this WiFly
library. Included in the repository is two extra files under the
"system" directory, Stream.h
and Stream.cpp. These need to replace the existing files
typically stored in "/usr/share/arduino/hardware/arduino/cores/arduino/". It's
annoying, I know.

Usage
-----

### Updating system libraries

As mentioned above, the standard Arduino stream implemntation was/is
a bit buggy. I've patched a couple of things and added a timed stream
matching method. The files stored under ``system`` need to be placed
in ``/usr/share/arduino/hardware/arduino/cores/arduino/``, replacing the existing
files.

### Dependencies

Uses another simple project of mine: ``arduino-debug``. You will need
to drop that into your libraries folder before using WiFly.

### Initialising the device

The class ``WiFlyDevice`` defines access to fundamental hardware functions.
Initialising the WiFly hardware requires defining a ``WiFlyDevice`` and 
calling ``begin``:

```c++
WiFlyDevice wifly;

void setup() {
  wifly.begin();
  wifly.join( ssid, passphrase )
}
```

The call to ``join`` connects to an SSID, returning true if successful.

### Starting a server

To run a server on your WiFly hardware, use the ``WiFlyServer`` class:

```c++
WiFlyDevice wifly;
WiFlyServer server( wifly );

void setup() {
  wifly.begin();
  wifly.join( ssid, passphrase )
  server.begin();
}
```

### Handling client connections

Individual incoming client connections are handled using the ``WiFlyClient``
class:

```c++
WiFlyDevice wifly;
WiFlyServer server( wifly );

void setup() {
  wifly.begin();
  wifly.join( ssid, passphrase )
  server.begin();
}

void loop() {
  WiFlyClient client( wifly );
  if( server.available( client ) )
  {
    // "client" is now ready for communication.
  }
}
```

### Reading and writing data

The ``WiFlyClient`` class has three primary methods for reading
and writing data, ``match_P``, ``readString`` and ``println``:

```c++
WiFlyDevice wifly;
WiFlyServer server( wifly );

void setup() {
  wifly.begin();
  wifly.join( ssid, passphrase )
  server.begin();
}

void loop() {
  WiFlyClient client( wifly );
  if( server.available( client ) )
  {
    // "client" is now ready for communication.

    // To perform timed stream matching on incoming data,
    // without needing to worry about delayed connections
    // (i.e. the matching will wait for new data to arrive):
    int match = client.match_P( 2, "GET / ", "POST / " );
    if( match == 0 )
    {
      // Have a GET.
      String data = client.readString();
      // Do something with the data...
      client.println( "<p> Results... </p>" );
    }
    else if( match == 1 )
    {
      // Have a POST.
    }
  }
}
```
