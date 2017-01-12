#RS485 Relay board

a USB 8 channel controller not modbus.

###Description :

This board allow to connect via **RS485** bus 8 relays, the 8 relays that it have on board are capable of switching 12VDC/10A or 240VAC/7A.
Every relays have a led to indicate if it work or not.

This RS485 8 relays board has an option to change the **board number** (from 01 to 15) manually through a DIP Switch placed near RS485 connector.

With this system you can **connect up to 15x8 channel**  Relay controllers up to a **distance of 1200m**.

![RelayBoard RS485](../images/rs485_8_relay_box_DIP.jpg)

###Required power:
12VDC / 500 mA.

###Specifications :
Relays have Normally Open (NO) and Normally Closed (NC) Contacts each capable of switching max:
- 12VDC/15A
- 24VDC/15A
- 125VAC/15A
- 250VAC/10A

###Communication Parameters:

8 Data, 1 Stop, No Parity
Baud rate : 9600

###Commands:

Relay commands:
OFF command : FF xx 00 (HEX) or 255 xx 0 (DEC)
ON command : FF xx 01 (HEX) or 255 xx 1 (DEC)

###Software:

a firmware for control the relay board  for [Rotilio Pro](http://techmakers.io/index.html#pro) and [Rotilio Maker](http://techmakers.io/index.html#rotilio) is available on our [Github repository](https://github.com/techmakers/rotilio.cc/tree/master/firmware).

###Demo: 

a project is available on [Hackster.io](https://www.hackster.io/Techmakers/how-to-command-a-rs485-relays-9674a9)


