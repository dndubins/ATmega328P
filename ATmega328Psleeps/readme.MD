These two sketches show simple implementations of interrupt and watchdog sleep modes. There are two wake-on-interrupt pins for the ATmega328P: 0 (Pin 2), and 1 (Pin 3).

There isn't a whole lot of power savings putting the Arduino Uno R3 MCU into sleep mode, since the other on-board components still light up. Consider a "bareduino", or an off-board MCU ATmega328P-PU chip in order to benefit from sleep mode. You can also scratch off the on-board LEDs if you are trying to get power consumption down.

The sketch speepInterrupt.ino is a "wake on interrupt" routine called sleep(), where you specify the pin number inside the routine. When this pin state falls (from HIGH to LOW), the microprocessor will wake up again.

The sketch sleepWatchdog.ino is a "wake on timeout" strategy, where you specify how long you want the MCU to sleep for, and then it will wake up after that time period has elapsed.

Remember that during sleep, millis() will stop counting, and delay() won't work. Before you put your MCU to sleep, set all pins to INPUT mode for ultimate power savings. When you wake the chip up, give it at least a ~20 msec delay before you start taking ADC readings again, because the chip will be a little groggy.
