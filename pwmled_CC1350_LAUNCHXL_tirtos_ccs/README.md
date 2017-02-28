## Example Summary

Sample application to control on-board LEDs with the PWM driver.

## Peripherals Exercised

* `Board_PWM0` - PWM instance used to control `Board_GPIO_LED1` brightness
* `Board_PWM1` - PWM instance used to control `Board_GPIO_LED2` brightness

## Resources & Jumper Settings

> If you're using an IDE (such as CCS or IAR), please refer to Board.html in your project
directory for resources used and board-specific jumper settings. Otherwise, you can find
Board.html in the directory \<SDK_INSTALL_DIR\>/source/ti/boards/\<BOARD\>.


## Example Usage

* Run the example.

* The onboard LEDs will slowly vary in intensity.

* If `Board_PWM0` and `Board_PWM1` are different (connected to two LEDs),
both LEDs will fade-in and fade-out when running the application.  Otherwise,
only one LED will fade-in and fade-out.

## Application Design Details

This application uses one thread, `mainThread` , which performs the following actions:

1. Opens and initializes PWM driver objects.

2. Uses the PWM driver to change the intensity of the LEDs.

3. The thread sleeps for 50 milliseconds before changing LED intensity again.

TI-RTOS:

* When building in Code Composer Studio, the configuration project will be imported
along with the example. The configuration project is referenced by the example, so it
will be built first.  These projects can be found under
\<SDK_INSTALL_DIR>\/kernel/tirtos/builds/\<BOARD\>/(release|debug)/(ccs|gcc).

FreeRTOS:

* Please view the `FreeRTOSConfig.h` header file for example configuration
information.
