# HID esp32
HIDesp32 is a firmware for the espressive esp32 (with BLE support).
It simulates a bluetooth keyboard and mouse. The simulates bluetooh keyboard an mouse can be connected to a device, such a notebook or mobilephone.
HIDesp32 takes commands from serial console, udp or tcp.
HIDesp32 can build up an access point for udp and tcp connections.

# HIDesp32 with autowork
Autowork is a GUI written in golang with the fyne framework. Autowork can connect to HIDesp32.
You can record keyboard and mouse input and replay it.
With autowork you can remote control a other device.

# What is the purpose of HIDesp32 and autowork
I write these tools for my daily work.
I have a lot of recurring work. Our working devices are protected and i have no posibilities to install programms on it.
This is where HIDesp32 and Autowork come into game.

# TODO
- documentation
- unittests
- clean up code