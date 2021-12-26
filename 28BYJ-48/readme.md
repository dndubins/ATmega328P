Stepper motors are precise, powerful, easy to implement, and so incredibly useful, but that doesn't mean they need to be expensive. Mass production has made the 28BYJ-48 very accessible (at a few dollars on eBay, including the ULN2003, a dedicated driver board!). This stepper motor is capable of a stepping angle of 0.08789° (or 4096 steps per revolution). What's more, the 5V version of the motor can run with the ULN2003 off the Arduino Uno without needing an external power supply.<p>

28BYJ-48.ino is a library-free standalone test sketch to get your motor spinning without taking up heaps of memory. It is an example of how simple a program can be to control a unipolar stepper motor.<p>

I turned this sketch into a library, available here: https://github.com/dndubins/28BYJ-48
