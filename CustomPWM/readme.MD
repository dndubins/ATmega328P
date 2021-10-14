# Fast PWM on the ATmega328p

Unfortunately, since the Arduino Playground shut their doors as a Wiki and became a website, I have nowhere particularly great to post this code. So, I'm posting it here!

One of the things that plague me is how to generate my own PWM frequencies, because you need to do this sometimes. A while ago, I posted a guide to FastPWM for the ATmega328p here:

https://playground.arduino.cc/Code/FastPWM/

In case this site disappears, I am reposting this information here.

This story began because I needed to do something that I thought would be simple, but ended up being quite challenging. I wanted a 370 kHz wave on Pin 3 of my Arduino Uno, because I needed to move a clock out from another pin to avoid a hardware conflict with an external chip (the 23LC1024 - here is a cheap plug for the library I wrote for it: https://github.com/dndubins/SRAMsimple). Little did I know that this was not a quick or easy problem. What the fclk! There are lots of explanations and code snippets out there to create custom frequency clock signals, but I quickly learned that I lacked the basic understanding of registers to do this.

There are three timers in the ATmega328P that time various important things. Timer 0 controls pins 5 and 6, along with the arduino time-related functions (e.g. delay() and millis()). Timer 1 controls pins 9 and 10. Finally, Timer 2 controls pins 3 and 11. So let's start with Timer 0. I will organize this page by what you might looking to do, so that hopefully you can find "the answer" quickly.

#Timer 0 (controls Pins 5 and 6)
-------------------------------
### "I want to output a specific frequency on Pin 5 only, using Timer 0."

Before you do this, know that messing with Timer 0 will screw with your delay() and millis() functions, which you can account for, but it's another headache. Timer 0 is a 8-bit clock, so the counters are limited to 255.

The code for a signal on Pin 5 only, using Timer 0:
```
  // Just Pin 5:
  pinMode(5, OUTPUT); // output pin for OCR0B, this is Arduino pin number
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR0B = _BV(WGM02)| _BV(CS00); // no prescaling
  //TCCR0B = _BV(WGM02) | _BV(CS01); // prescaler=8
  //TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00); // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02); // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00); // prescaler=1024
  OCR0A = 40; // counter limit: 255
  OCR0B = 20; //duty cycle, can't be greater than OCR0A. (OCR0B=0.5*OCR0A for 50% duty cycle)
  //duty cycle =OCR0B/OCR0A
```

There are two counters involved in this mode: OCR0A and OCR0B. Setting OCR0A will change the frequency of the output signal. The prescaler value is a multiplier which puts you in a different frequency range. In this mode (inverting, fast PWM), the formula for the output signal is:

output frequency=fclk/((OCR0A+1)*N), where fclk=16000000, and N is the prescaler you selected by un-commenting the appropriate line in the above code (which can just go in your setup loop if you like).

The duty cycle for this signal will be defined by OCR0B, and calculated by the formula: duty cycle=OCR0B/OCR0A. The example code above creates a signal on Pin 5 with a frequency of 390 kHz and a duty cycle of 50%.

Here is a chart of the frequencies you will get (in Hz), spanning the OCR0A range and prescalers you can use:

| OCR0A | N=1 | N=8 | N=64 | N=256 | N=1024 |
| --- | --- | --- | --- | --- | --- |
| 1 | 8000000 | 1000000 | 125000 | 31250 | 7813 |
| 5 | 2666667 | 333333 | 41667 | 10417 | 2604 |
| 10 | 1454545 | 181818 | 22727 | 5682 | 1420 |
| 20 | 761905 | 95238 | 11905 | 2976 | 744 |
| 40 | 390244 | 48780 | 6098 | 1524 | 381 |
| 60 | 262295 | 32787 | 4098 | 1025 | 256 |
| 80 | 197531 | 24691 | 3086 | 772 | 193 |
| 100 | 158416 | 19802 | 2475 | 619 | 155 |
| 120 | 132231 | 16529 | 2066 | 517 | 129 |
| 140 | 113475 | 14184 | 1773 | 443 | 111 |
| 160 | 99379 | 12422 | 1553 | 388 | 97 |
| 180 | 88398 | 11050 | 1381 | 345 | 86 |
| 200 | 79602 | 9950 | 1244 | 311 | 78 |
| 255 | 62500 | 7813 | 977 | 244 | 61 |

You can try generating this chart in Excel with the above formula. You can use any integer between 0-255 for the counter OCROA, I just wanted to make my tables a bit smaller by skipping rows.

### "I want to output a specific frequency on Pin 6 only, using Timer 0."

The code for a signal on Pin 6 only:

```
  //Just Pin 6:
  //Formula: frequency=fclk/((OCR0A+1)*2*N)
  pinMode(6,OUTPUT); // duty cycle fixed at 50% in this mode.
  TCCR0A =  _BV(COM0A0) | _BV(WGM01) | _BV(WGM00); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR0B = _BV(WGM02)| _BV(CS00); // no prescaling
  //TCCR0B = _BV(WGM02) | _BV(CS01); // prescaler=8
  //TCCR0B = _BV(WGM02) | _BV(CS01) | _BV(CS00); // prescaler=64
  //TCCR0B = _BV(WGM02) | _BV(CS02); // prescaler=256
  //TCCR0B = _BV(WGM02) | _BV(CS02) | _BV(CS00); // prescaler=1024
  OCR0A = 10; // counter limit: 255
```
OCR0B is left out of the picture here, and you are limited to a 50% duty cycle in this mode. The formula for frequency is slightly different (by a factor of 2):

output frequency=fclk/((OCR0A+1)*2*N)

The example code above creates a signal on Pin 6 with a frequency of 727 kHz and a duty cycle of 50%.

Here is a nice chart spanning the range of OCR0A and prescaler values:

| OCR0A | N=1 | N=8 | N=64 | N=256 | N=1024 |
| --- | --- | --- | --- | --- | --- |
| 1 | 4000000 | 500000 | 62500 | 15625 | 3906 |
| 5 | 1333333 | 166667 | 20833 | 5208 | 1302 |
| 10 | 727273 | 90909 | 11364 | 2841 | 710 |
| 20 | 380952 | 47619 | 5952 | 1488 | 372 |
| 40 | 195122 | 24390 | 3049 | 762 | 191 |
| 60 | 131148 | 16393 | 2049 | 512 | 128 |
| 80 | 98765 | 12346 | 1543 | 386 | 96 |
| 100 | 79208 | 9901 | 1238 | 309 | 77 |
| 120 | 66116 | 8264 | 1033 | 258 | 65 |
| 140 | 56738 | 7092 | 887 | 222 | 55 |
| 160 | 49689 | 6211 | 776 | 194 | 49 |
| 180 | 44199 | 5525 | 691 | 173 | 43 |
| 200 | 39801 | 4975 | 622 | 155 | 39 |
| 255 | 31250 | 3906 | 488 | 122 | 31 |

So far, so good? How about if you'd like to output a signal on both Pins 5 and Pin 6 at the same time?

### "I want a custom PWM signal on both Pins 5 and 6 at the same time, using Timer 0."

So you can also create a signal on both pins; however, in the modes I've tried, you can't make them have distinct (different) frequencies. They will have the same frequency, which is (less) tweakable. You can however assign them different duty cycles, which is nice.

The Code for a Signal on Pins 5 and 6:
```
 //Both Pins 5 and 6: In this mode, timer 0 always counts to 255.
 //Formula: wave frequency=fclk/(256*N) - this formula is more limited with respect to frequency
  pinMode(5, OUTPUT);
  pinMode(6,OUTPUT);
  TCCR0A = _BV(COM0A1) | _BV(COM0A0) | _BV(COM0B1) |_BV(COM0B0) | _BV(WGM01) | _BV(WGM00); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR0B = _BV(CS00); // no prescaling (62.5 kHz)
  //TCCR0B = _BV(CS01); // prescaler=8 (7.8 kHz)
  //TCCR0B = _BV(CS01) | _BV(CS00); // prescaler=64 (976 Hz)
  //TCCR0B = _BV(CS02); // prescaler=256 (244 Hz)
  //TCCR0B = _BV(CS02) | _BV(CS00); // prescaler=1024 (61 Hz)
  OCR0A = 50; // duty cycle for pin 6 is (255-OCR0A)/255. 128 is a 50% duty cycle.
  OCR0B = 128; //duty cycle for pin 5 is (255-OCR0B)/255. 128 is a 50% duty cycle.
```
The formula for the output frequency is now only dependent on the prescaler you select, since the counter will run out to the end of its cycle. The frequencies you have at your disposal then are governed by the formula fclk/(256*N), giving rise to the following options:

| Prescaler: | 	1 | 	8 | 	64 | 	256 | 	1024 | 
| --- | --- | --- | --- | --- | --- |
| Frequency (Hz): | 	62500 | 	7813 | 977 |	244 |	61 | 

The duty cycle for pin 6 is adjusted by changing OCR0A to a number between 0-255. For a duty cycle of 50%, select 128. The formula for the duty cycle is (255-OCR2A)/255, so a duty cycle of 25% would need an OCR2A value of 191. Pin 5's duty cycle can be adjusted independently using OCR0B, using the same formula as A. How great is that?!

The example code above creates a signal on Pin 5 with a frequency of 62.5 kHz and a duty cycle of 80%, and a signal on Pin 6 with the same frequency (62.5 kHz) and a duty cycle of 50%.

#Timer 1 (Controls Pins 9 and 10)
------------------------------------------
### "I want to output a specific frequency on Pin 10 only, using Timer 1."

Timer 1 is a 16 bit clock, so there is a bit more flexibility here (largest counter value is now 65535). The code for Pin 10 bears resemblance to that of Pin 5 only above, so I'm putting these code snippets in the same order.

The Code for a Signal on Pin 10 only:
```
  // Just Pin 10:
  //Formula: wave frequency=fclk/((OCR1A+1)*N)
  pinMode(10, OUTPUT); // output pin for OCR1B, this is Arduino pin number
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10); // fast PWM, 10bit resolution
  //uncomment for your desired prescaler:
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // no prescaling
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // prescaler=8
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10); // prescaler=64
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12); // prescaler=256
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); // prescaler=1024
  OCR1A = 40; // counter limit: 65535
  OCR1B = 10; //duty cycle, can't be greater than OCR1A. (OCR1B=0.5*OCR1A for 50% duty cycle)
  //duty cycle =OCR1B/OCR1A
```
Like before, setting OCR1A will change the frequency of the output signal. The formula for the output signal is: output frequency=fclk/((OCR1A+1)*N), where fclk=16000000, and N is the prescaler you selected by uncommenting the appropriate line above. The duty cycle is calculated using the formula: duty cycle = OCR1B/OCR1A (use half of 0CR1A for a 50% duty cycle)

The example code above creates a signal on Pin 10 with a frequency of 390.2 kHz and a duty cycle of 25%.

Here is a chart of the frequencies you will get, spanning the OCR1A range and prescalers you can use:

| OCR1A | N=1 | N=8 | N=64 | N=256 | N=1024 |
| --- | --- | --- | --- | --- | --- |
| 1 | 8000000.0 | 1000000.0 | 125000.0 | 31250.0 | 7812.5 |
| 10 | 1454545.5 | 181818.2 | 22727.3 | 5681.8 | 1420.5 |
| 50 | 313725.5 | 39215.7 | 4902.0 | 1225.5 | 306.4 |
| 100 | 158415.8 | 19802.0 | 2475.2 | 618.8 | 154.7 |
| 500 | 31936.1 | 3992.0 | 499.0 | 124.8 | 31.2 |
| 1000 | 15984.0 | 1998.0 | 249.8 | 62.4 | 15.6 |
| 5000 | 3199.4 | 399.9 | 50.0 | 12.5 | 3.1 |
| 10000 | 1599.8 | 200.0 | 25.0 | 6.2 | 1.6 |
| 50000 | 320.0 | 40.0 | 5.0 | 1.2 | 0.3 |
| 65535 | 244.1 | 30.5 | 3.8 | 1.0 | 0.2 |

### "I want to output a specific frequency on Pin 9 only, using Timer 1."

The Code for a Signal on Pin 9 Only:
```
  //Just Pin 9:
  //Formula: wave frequency=fclk/((OCR1A+1)*2*N)
  pinMode(9, OUTPUT); // duty cycle fixed at 50% in this mode.
  TCCR1A = _BV(COM1A0) | _BV(WGM11) | _BV(WGM10); // fast PWM, 10bit resolution
  //uncomment for your desired prescaler:
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10); // no prescaling
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11); // prescaler=8
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS11) | _BV(CS10); // prescaler=64
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12); // prescaler=256
  //TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10); // prescaler=1024
  OCR1A = 40; // counter limit: 65535
  //duty cycle=50%
```
Like with Pin 6, OCR1B is left out of the picture here, and you are limited to a 50% duty cycle in this mode. As with Pin 6, the formula is:

output frequency=fclk/((OCR1A+1)*2*N)

The example code above creates a signal on Pin 9 with a frequency of 195.1 kHz and a duty cycle of 50%.

Here is a chart spanning the range of OCR1A and prescaler values:

| OCR1A | N=1 | N=8 | N=64 | N=256 | N=1024 |
| --- | --- | --- | --- | --- | --- |
| 1 | 4000000.0 | 500000.0 | 62500.0 | 15625.0 | 3906.3 |
| 10 | 727272.7 | 90909.1 | 11363.6 | 2840.9 | 710.2 |
| 50 | 156862.7 | 19607.8 | 2451.0 | 612.7 | 153.2 |
| 100 | 79207.9 | 9901.0 | 1237.6 | 309.4 | 77.4 |
| 500 | 15968.1 | 1996.0 | 249.5 | 62.4 | 15.6 |
| 1000 | 7992.0 | 999.0 | 124.9 | 31.2 | 7.8 |
| 5000 | 1599.7 | 200.0 | 25.0 | 6.2 | 1.6 |
| 10000 | 799.9 | 100.0 | 12.5 | 3.1 | 0.8 |
| 50000 | 160.0 | 20.0 | 2.5 | 0.6 | 0.2 |
| 65535 | 122.1 | 15.3 | 1.9 | 0.5 | 0.1 |

### "I want to ouput a signal on both Pins 9 and 10, using Timer 1."

Again, the frequencies are limited with the mode I played with (modulated only by changing the prescaler value). However, you can set the duty cycles of 9 and 10 individually.

The code for signals on both Pins 9 and 10:
```
  // Both Pin 9 and Pin 10:
  //Formula: wave frequency=fclk/(N*1024)
  pinMode(9, OUTPUT); // output pin for OCR1A, this is Arduino pin number
  pinMode(10, OUTPUT); // output pin for OCR1B, this is Arduino pin number
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10); // fast PWM, 10bit resolution
  //uncomment for your desired prescaler:
  TCCR1B = _BV(WGM12) | _BV(CS10); // no prescaling (freq=15625 Hz)
  //TCCR1B = _BV(WGM12) | _BV(CS11); // prescaler=8 (freq=1953 Hz)
  //TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10); // prescaler=64 (freq=244 Hz)
  //TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS11) | _BV(CS10); // prescaler=256 (freq=61 Hz)
  //TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); // prescaler=1024 (freq=15 Hz)
  OCR1A = 512; // counter limit: 1024 (duty cycle pin9 =OCR1A/1024, 50% duty cycle=512)
  OCR1B = 102; // counter limit: 1024 (duty cycle pin10 =OCR1B/1024, 50% duty cycle=512)
```
The formula for the resulting frequency on both Pins 9 and 10 is: output frequency = fclk/(1024*N). Frequency depends only on the prescaler value, giving rise to the following options:

| Prescaler: | 	1 | 	8 | 	64 | 	256 | 	1024 | 
| --- | --- | --- | --- | --- | --- |
| Frequency (Hz): | 	15625 | 1953 | 244 |	61 |	15 | 

The duty cycle for pin 9 is adjusted by changing OCR1A to a number between 0-1024. For a duty cycle of 50%, select 512. The formula for the duty cycle is (OCR1A)/1024, so a duty cycle of 25% would need an OCR1A value of 256. Pin 10's duty cycle can be adjusted independently using OCR1B, using the same formula as A.

The example code above creates a signal on Pin 9 with a frequency of 15.6 kHz and a duty cycle of 50%, and a signal on Pin 10 with the same frequency (15.6 kHz) and a duty cycle of about 10%.

#Timer 2 (controls Pins 3 and 11)

### "I want to output a specific frequency on Pin 3 only, using Timer 2."

Timer 2 is a lot like Timer 0 (it's also an 8 bit clock and the registers are structured very similarly). One important distinction is that the options for the prescaler values are a little different. Also, changing Timer 2 doesn't mess up the arduino time functions.

The Code for a Signal on Pin 3 Only:
```
  //Just Pin 3:
  pinMode(3, OUTPUT); // output pin for OCR2B, this is Arduino pin number
  TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR2B = _BV(WGM22) | _BV(CS20); // no prescaling
  //TCCR2B = _BV(WGM22) | _BV(CS21); // prescaler=8
  //TCCR2B = _BV(WGM22) | _BV(CS21)| _BV(CS20); // prescaler=32
  //TCCR2B = _BV(WGM22) | _BV(CS22); // prescaler=64
  OCR2A = 40; // counter limit: 255
  OCR2B = 20; //duty cycle, can't be greater than OCR2A. (OCR2B=0.5*OCR2A for 50% duty cycle)
  //duty cycle =OCR2B/OCR2A
```
You can see that this is almost identical to the code for Pin 5 only, but with the Timer2 register names, and the prescaler options are different. The formula for the signal frequency is: output frequency=fclk/((OCR2A+1)*N). The duty cycle is set by the ratio of OCR2B/OCR2A.

The example code above creates a signal on Pin 3 with a frequency of 390.2 kHz and a duty cycle of 50%.

Here is a table spanning practical values of OCR2A and prescalers:

| OCR2A | N=1 | N=8 | N=32 | N=64 |
| --- | --- | --- | --- | --- |
| 1 | 8000000 | 1000000 | 250000 | 125000 |
| 5 | 2666667 | 333333 | 83333 | 41667 |
| 10 | 1454545 | 181818 | 45455 | 22727 |
| 20 | 761905 | 95238 | 23810 | 11905 |
| 40 | 390244 | 48780 | 12195 | 6098 |
| 60 | 262295 | 32787 | 8197 | 4098 |
| 80 | 197531 | 24691 | 6173 | 3086 |
| 100 | 158416 | 19802 | 4950 | 2475 |
| 120 | 132231 | 16529 | 4132 | 2066 |
| 140 | 113475 | 14184 | 3546 | 1773 |
| 160 | 99379 | 12422 | 3106 | 1553 |
| 180 | 88398 | 11050 | 2762 | 1381 |
| 200 | 79602 | 9950 | 2488 | 1244 |
| 255 | 62500 | 7813 | 1953 | 977 |


### "I want to output a specific frequency on Pin 11 only, using Timer 2."

The Code for a Signal on Pin 11 Only:

```
  //Just Pin 11:
  //Formula: frequency=fclk/((OCR2A+1)*2*N)
  pinMode(11,OUTPUT); // duty cycle fixed at 50% in this mode.
  TCCR2A =  _BV(COM2A0) | _BV(WGM21) | _BV(WGM20); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR2B = _BV(WGM22) | _BV(CS20); // no prescaling
  //TCCR2B = _BV(WGM22) | _BV(CS21); // prescaler=8
  //TCCR2B = _BV(WGM22) | _BV(CS21)| _BV(CS20); // prescaler=32
  //TCCR2B = _BV(WGM22) | _BV(CS22); // prescaler=64
  OCR2A = 10; // counter limit: 255
```
Again, this mode is limited to a 50% duty cycle, and the output frequency is governed by the formula: fclk/((OCR2A+1)*2*N).

The example code above creates a signal on Pin 11 with a frequency of 727.2 kHz and a duty cycle of 50%.

Here is a table of output frequencies spanning the ranges of OCR2A and prescaler values:

 | OCR2A | N=1 | N=8 | N=32 | N=64 |
 | --- | --- | --- | --- | --- |
 | 1 | 4000000 | 500000 | 125000 | 62500 |
 | 5 | 1333333 | 166667 | 41667 | 20833 |
 | 10 | 727273 | 90909 | 22727 | 11364 |
 | 20 | 380952 | 47619 | 11905 | 5952 |
 | 40 | 195122 | 24390 | 6098 | 3049 |
 | 60 | 131148 | 16393 | 4098 | 2049 |
 | 80 | 98765 | 12346 | 3086 | 1543 |
 | 100 | 79208 | 9901 | 2475 | 1238 |
 | 120 | 66116 | 8264 | 2066 | 1033 |
 | 140 | 56738 | 7092 | 1773 | 887 |
 | 160 | 49689 | 6211 | 1553 | 776 |
 | 180 | 44199 | 5525 | 1381 | 691 |
 | 200 | 39801 | 4975 | 1244 | 622 |
 | 255 | 31250 | 3906 | 977 | 488 |

### "I want to ouput signals on both Pins 3 and 11, using Timer 2."

Once again (assuming you have skipped down to this part and not read the above cases), the frequencies in this mode can only be changed by selecting the prescaler value, and once set, Pins 3 and 11 will have the exact same frequencies. However, you can set the duty cycles of pins 3 and 11 individually in this mode.

The Code for Signals on Both Pins 3 and 11:
```
  //Both Pin 3 and Pin 11:
  //Formula: wave frequency=fclk/(256*N) - this formula is more limited with respect to frequency
  pinMode(3, OUTPUT);
  pinMode(11,OUTPUT);
  TCCR2A = _BV(COM2A1) | _BV(COM2A0) | _BV(COM2B1) |_BV(COM2B0) | _BV(WGM21) | _BV(WGM20); // inverting mode, fast PWM
  //uncomment for your desired prescaler:
  TCCR2B = _BV(CS20); // no prescaling (62.5 kHz)
  //TCCR2B = _BV(CS21); // prescaler=8 (7.8 kHz)
  //TCCR2B = _BV(CS21)| _BV(CS20); // prescaler=32 (1.953 kHz)
  //TCCR2B = _BV(CS22); // prescaler=64 (976.6 Hz)
  OCR2A = 26; // duty cycle for pin 11 is (255-OCR2A)/255. 128 is a 50% duty cycle.
  OCR2B = 128; //duty cycle for pin 3 is (255-OCR2B)/255. 128 is a 50% duty cycle.
```
The formula for the frequency of the signal output by both Pins 3 and 11 is: output frequency=fclk/(256*N). This gives rise to the last table (yay!):

| N=1	| N=8	| N=32	| N=64 |
| --- | --- | --- | --- |
| 62500 |	7813 |	1953 | 977	Hz |

OCR2A and OCR2B are separately used to define the duty cycles of Pins 11 and 3, respectively, using the formula: duty cycle=(255-OCR2A)/255 for Pin 11, and duty cycle=(255-OCR2B)/255 for pin 3.

The example code above creates a signal on Pin 11 with a frequency of 62.5 kHz and a duty cycle of 90%, and a signal on Pin 3 with the same frequency (62.5 kHz) and a duty cycle of 50%.

So there you have it! You need to make a square wave with a specific frequency? I hope this page saves you some time.

References that Helped Me:
https://withinspecifications.30ohm.com/2014/02/20/Fast-PWM-on-AtMega328/ https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM https://playground.arduino.cc/Main/TimerPWMCheatsheet https://www.eprojectszone.com/how-to-modify-the-pwm-frequency-on-the-arduino-part1/

The ATmega328p datasheet from Atmel: https://www.atmel.com/Images/Atmel-42735-8-bit-AVR-Microcontroller-ATmega328-328P_datasheet.pdf

