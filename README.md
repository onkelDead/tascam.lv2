# Tascam US-16x08 LV2 plugin

LV2 plugin to control Tascam US-16x08 interface via cutsom alsa driver.
This plugin has no effect on any audio stream.

Precondition is a linux kernel version 4.12 or higher, or a kernel patch 
which could be found at https://github.com/onkelDead/tascam-driver-path.

It consists of four separate plugins, which are:

* EQ - Tascam US-16x08
* EQ Stereo - Tascam US-16x08
* Compressor - Tascam US-16x08
* Compressor Stereo - Tascam US-16x08

It has been tested on various Ardour versions.

When adding one of those plugin to a DAW, it has to be configured for the hardware channel it should act on.

![screenshot.png](/screenshot.png?raw=true)



