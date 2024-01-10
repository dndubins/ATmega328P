Sending a PWM signal (e.g. to a servo or device) is so easy in the Arduino IDE. All that is required is an analogWrite() command.
However, what if you would like to decode a PWM signal? Perhaps you'd like to measure a PWM signal between two devices.
This sketch does exactly that. It's not great (for instance, it doesn't give accurate results on the bottom and top ends).
However, it works well enough.

In this example, a potentiometer is used as a voltage divider to set the PWM signal in the Uno.
I set the same Uno up to send a PWM signal, and to read it. Pin 5 is used to send, and Pin 3 is used to read.
