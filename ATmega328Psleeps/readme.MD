This file is the ATtiny84 version of the sleep sketch I wrote for the ATmega328P-PU, for timed (watchdog timer) and interrupt sleep modes. There are two wake-on-interrupt pins for the ATmega328P: 0 (Pin 2), and 1 (Pin 3).

There isn't a lot of power savings putting the Uno MCU into sleep mode, since the other on-board components still light up. Consider a "bareduino", or DIP version of the ATmega328P-PU in order to benefit from sleep mode. You can also scratch off the on-board LEDs if you are trying to get power consumption down.

This sketch shows two routines. The first is a "wake on interrupt" routine called sleep_interrupt(pin#), where you specify the pin number. When this pin changes (from LOW to HIGH or from HIGH to LOW), the microprocessor will wake up again.

The second routine is a "wake on timeout" strategy, where you specify how long you want the MCU to sleep for, and then it will wake up after that time period has elapsed. If it's important in your sketch just to capture the "falling" edge (from HIGH to LOW as you push the switch down), you could read the state of the pin inside ISR(PCINT0_vect)() to a volatile bool declared in global space.

Remember that during sleep, millis() will stop counting, and delay() won't work. Before you put your MCU to sleep, set all pins to INPUT mode for ultimate power savings. When you wake the chip up, give it at least a ~20 msec delay before you start taking ADC readings again, because the chip will be a little groggy.
