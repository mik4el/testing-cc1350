## Example Summary

The WSN Node example illustrates how to create a Dual Mode Wireless Sensor
Network Node device which sends sensor data to a Sub-1GHz concentrator and
also send out BLE beacons. The BLE beacons can be picked up by any BLE capable
device. This example is meant to be used together with the WSN Dual Mode
Concentrator example.

This examples showcases the use of several Tasks, Semaphores and Events to
get sensor updates and send packets with acknowledgement from the concentrator.
For the radio layer, this example uses the EasyLink API which provides an
easy-to-use API for the most frequently used radio operations.

## Peripherals Exercised

* `Board_PIN_LED1` - Toggled when Sub1-GHz data is sent over the RF interface.

* `Board_PIN_LED2` - Toggled when a BLE beacon is sent over the RF interface.

* `Board_PIN_BUTTON0` - Selects fast report or slow report mode. In slow report
mode the sensor data is sent at least every 5s or as fast as every 1s if there
is a significant change in the ADC reading. In fast reporting mode the sensor
data is sent every 1s regardles of the change in ADC value. The default is slow
reporting mode.

* `Board_PIN_BUTTON1` - Selects the BLE beacon type used;

 - Manufacturer Specific (MS) + Eddystone URL. This is the default.
 - MS
 - Eddystone URL + TLM
 - Eddystone UID + TLM
 - None

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

Run the example. On another board run the WSN Dual Mode Concentrator example.
This node should show up on the LCD/UART of the Concentrator. To create a sub-1GHz only device, without the capability to send BLE beacon, one can use the Wireless Sensor Network examples also.

## Application Design Details

* This examples consists of two tasks, one application task and one radio
protocol task. It also consists of an Sensor Controller Engine (SCE) task which
samples the ADC once per second.

* On initialisation the CM3 application sets the minimum report interval and
the minimum change value which is used by the SCE task to wake up the CM3. The
ADC task on the SCE checks the ADC value once per second. If the ADC value has
changed by the minimum change amount since the last time it notified the CM3,
it wakes it up again. If the change is less than the masked value, then it
does not wake up the CM3 unless the minimum report interval time has expired.

* The NodeTask waits to be woken up by the SCE. When it wakes up it toggles
`Board_PIN_LED1` and sends the new ADC value to the NodeRadioTask. Then it sends out
sensor data on the sub-1GHz RF interface and if configured to do so it will also send a BLE Beacon. The sensor data is; ADC value, battery level, time since last
reboot, number of packets sent and the button state.

* The NodeRadioTask handles the radio protocol. This sets up the EasyLink
API and uses it to send new ADC values to the concentrator. After each sent
packet it waits for an ACK packet back. If it does not get one, then it retries
three times. If it did not receive an ACK by then, then it gives up.

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

* For more information on the EasyLink API and usage refer to [SimpleLink-EasyLink](http://processors.wiki.ti.com/index.php/SimpleLink-EasyLink).