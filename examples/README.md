#Examples

 * NMEA Multiplexer

##NMEA Multiplexer

Example sketch for using an Arduino Mega (1280 or 2560) as an NMEA 0183 
multiplexer.  Serial ports 1, 2, and 3  are connected to the outputs of 
MAX13085E RS485/422 to TTL converters --other converters will work just 
as well--. Their output is then forwarded to the default serial port 
that can be listened to on the TTL pins (0 and 1) or the USB output.

