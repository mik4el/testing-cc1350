## Example Summary

The WSN Dual Mode Concentrator example illustrates how to implement a sub-1GHz Wireless Sensor Network Concentrator device which listens for packets from other nodes. This example is meant to be used together with the WSN Dual Mode Node example to form a one-to-many network where the nodes send messages to the concentrator. Both the Nodes and Concentrator can be configured to also send BLE beacons.

The Dual Mode Concentrator receives sensor data from the Dual Mode Nodes and displays the sensor reading on UART and LCD. The concentrator can also be configured by a button press to also send out BLE advertisements (beacons) packets, the beacon then contain sensor data from one of the nodes. The concentrator effectivly acts as a relay for the sub-1GHz sensor data to the BLE beacon packet.

This examples showcases the use of several Tasks, Semaphores and Events to
receive packets, send acknowledgements and display the received data on the
LCD. For the radio layer, this example uses the EasyLink API which provides
an easy-to-use API for the most frequently used radio operations.

## Peripherals Exercised

* `LCD` - When the concentrator receives data from a new node, it is given a new
row on the LCD/UART and the reveived value is shown. If more than 7 nodes are detected, the device list rolls over, overriding the first. The supported LCD is the MSP430-SHARP96 boosterpack.

* `UART` - The information displayed on the LCD is also replicated on the UART
incase the LCD is not fitted.

* `Board_PIN_LED1` - Toggled when Sub1-GHz data is received over the RF interface.

* `Board_PIN_LED2` - Toggled when a BLE beacon is sent over the RF interface.

* `Board_PIN_BUTTON0` - Selects the device which BLE data is forwarded for.
Selected device is indecated by a '*'next to the device on te LCD and UART

* `Board_PIN_BUTTON1` - Selects the BLE beacon type used;

 - Manufacturer Specific (MS) + Eddystone URL. This is the default.
 - MS
 - Eddystone URL + TLM
 - Eddystone UID + TLM
 - None


> Whenever an updated value is received from a node, it is updated on
the LCD display.

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory \<SDK_INSTALL_DIR\>/source/ti/boards/\<BOARD\>.
For convenience, a short summary is also shown below.

| Development board | Notes                                                  |
| ----------------- | ------                                                 |
| CC1350 Sensortag  | DEVPACK-DEBUG needed for UART. DEVPACK-WATCH for LCD   |
| CC1350LAUNCHXL    | Boosterpack 430BOOST-SHARP96 needed for LCD            |

> Fields left blank have no specific settings for this example.

## Example Usage

* Run the example. On another board (or several boards) run the
WSN Dual Mode Node example. The LCD will show the discovered node(s). To create a sub-1GHz only device, without the capability to send BLE beacon, one can use the Wireless Sensor Network examples also.

*Use the buttons to select a node and beacon type and use any smarthone
application that decodes the Eddystone beacons to view the beacon data. To view MS beacon, use the TI SensorTag smartphone app.

## Application Design Details

* This examples consists of two tasks, one application task and one radio
protocol task.

* The Dual Mode ConcentratorRadioTask handles the radio protocol. This sets
up the EasyLink API and uses it to always wait for packets on a set
frequency. When it receives a valid packet, it checks to see if a BLE
beacon should be sent and sends an ACK and then forwards it to the
ConcentratorTask. By defuat the Dual Mode ConcentratorRadioTask does not send
BLE beacons. Board_PIN_BUTTON0 and Board_PIN_BUTTON1 should be used to configure the
beacons.

* The ConentratorTask receives packets from the ConcentratorRadioTask and
displays the data on the LCD.

* *RadioProtocol.h* can also be used to change the
PHY settings to be either the default IEEE 802.15.4g 50kbit,
Long Range Mode or custom settings. In the case of custom settings,
the *smartrf_settings.c* file is used. This can be changed either
by exporting from Smart RF Studio or directly in the file.

> For SensorTags there is a pin conflict, so either the DEVPACK-DEBUG or the DEVPACK-WATCH must be used
and `BOARD_DISPLAY_EXCLUDE_UART` must be added to the global precompiler defines in order to use LCD.

> For IAR users using any SensorTag(STK) Board, the XDS110 debugger must be
selected with the 4-wire JTAG connection within your projects' debugger
configuration.

## References
* For more information on the [EasyLink API](http://processors.wiki.ti.com/index.php/SimpleLink-EasyLink) and usage.