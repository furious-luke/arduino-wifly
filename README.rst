============
WiFly Library
============

Description
===========

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
=========

I found a few issues with the standard implementation of the
Arduino stream functionality. In addition, I added a routine
to simplify some stream based matching used in this WiFly
library. Included in the repository is two extra files under the
"system" directory, Stream.h
and Stream.cpp. These need to replace the existing files
typically stored in ".../hardware/arduino/cores/arduino/". It's
annoying, I know.
