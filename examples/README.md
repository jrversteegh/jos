==NMEA Multiplexer==

Example sketch for using an Arduino Mega (1280 or 2560) as an NMEA 0183 multiplexer. 
Serial ports 2 to 4 are connected to the outputs of MAX13085E RS485/422 to TTL converters 
--other converters will work just as well--. Their output is then forwarded to serial port 1 
that can be listened to on the TTL pins or the USB output.
