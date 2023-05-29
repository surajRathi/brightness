# Brightness

A simple commandline tool to set display brighness on Linux systems. It uses the `/sys/class/backlight` interface.

```bash
$ ./brightness --help
Usage: brightness [OPTIONS] [NEW_VALUE]
Used to get_brightness and set the brightness of backlight devices.

Mandatory arguments to long options are mandatory for short options too.
    -d, --device=DEV	specify the device name,
			              if none give, uses the first available device
    -h, --help		    display this help and exit
    -s, --show         Show percent for all devices
NEW_VALUE should be of the form:
    [+,-]NUMBER[%]
$ ./brightness
amdgpu_bl0:	46%
$ ./brightness -d amdgpu_bl0 +20%
amdgpu_bl0:	66%
$ ./brightness -d amdgpu_bl0 100%
amdgpu_bl0:	100%
$ ./brightness -d amdgpu_bl0 250
amdgpu_bl0:	97%
$ ./brightness --show
amdgpu_bl0:	97%
$
```
