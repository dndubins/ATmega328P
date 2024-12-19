TCS3200.ino is a very simple read of the TCS3200 colour sensor.<p>

There are better sketches out there. This one just gives you the raw readings. If you would like to interpret colour, I was particularly proud of this neural network sketch:
https://github.com/dndubins/ATmega328P/blob/main/ModelFitting/NN/NNsensor.ino<p>

Another approach I tried was a linear model to fit colour sensor readings to wavelength, using SVD (singular value decomposition):
https://github.com/dndubins/ATmega328P/blob/main/ModelFitting/SVD/SVDsensor.ino<p>

This program didn't work as well as I had hoped, but it does crudely report a wavelength with given TCS3500 readings.<p>

An important realization I made about this sensor is that once you set the colour channel using S2 and S3, you can take as many pulseIn() readings as you'd like. So I came up with a function:<p>
```
// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN(float colourArr[3], int n) {  // arrays are always passed by value
#define TIMEOUT 200                            // for timeout (in microseconds) on reading a colour
  unsigned long thisRead[3] = { 0, 0, 0 };     // for data averaging
  bool pinStates[3][2] = {
    { LOW, LOW },    // S2,S3 are LOW for RED
    { HIGH, HIGH },  // S2,S3 are HIGH for GREEN
    { LOW, HIGH }    // S2=LOW,S3=HIGH for BLUE
  };
  for (int i = 0; i < 3; i++) {                   // i=0: red, i=1: green, i=2: blue
    digitalWrite(S2, pinStates[i][0]);            // set S2 to correct pin state
    digitalWrite(S3, pinStates[i][1]);            // set S3 to correct pin state
    thisRead[i] = 0;                              // initialize colour
    delay(100);                                   // wait for reading to stabilize
    for (int j = 0; j < n; j++) {                 // collect n readings on channel i
      thisRead[i] += pulseIn(OUT, LOW, TIMEOUT);  // read colour
    }
    thisRead[i] /= n;                   // report the average
    colourArr[i] = (float)thisRead[i];  //write back to colourArr
  }
}
```
This function takes an array as an input argument (remember, arrays in this context are call-by-value), and takes n readings *of each colour*. This speeds up a reading considerably since you should wait at least ~100msec before taking a colour reading after setting S2 and S3. But then you can take multiple readings, and take the average over that colour, rather than start all over with three separate readings.<p>

Also remember that a LOW reading on a channel means that there's a lot of that colour detected. I think my rookie mistake was looking at an RGB reading like  "11, 43, 39" and thinking there was no red in it. Nope, that's red. What happens on the TCS3200 is essentially a light-intensity-to-frequency converter. Each channel reports a square wave, with the frequency of proportional to the intensity of light. The higher the intensity, the higher the frequency.<p>

The function pulseIn() is used to measure the period of the square wave on each channel (R, G, B, CLEAR). A higher frequency means a shorter period. This means that if there is more RED in the signal, pulseIn() will return a lower number.

After a lot of experimenting with normalizing the channels, I realized that there is a "CLEAR" channel that is very useful as a baseline to normalize against. I found that the following normalization algorithm worked very well in terms of normalizing the relative signals to intensity, and then coming up with a %intensity for each colour:<p>

1) Convert each channel from microseconds to Hz:
RED=1000000/RED
GREEN=1000000/GREEN
BLUE=1000000/BLUE
CLEAR=100000/CLEAR
2) Divide each channel by the CLEAR signal in Hz:
RED=RED/CLEAR
GREEN=GREEN/CLEAR
BLUE=BLUE/CLEAR<p>

This gives a signal that varies between 0 and 1 that is normalized by CLEAR signal intensity. A high number in the normalized channel now means more of that colour. In C++. the code looks like this:
```
  float norm[4] = { 0.0, 0.0, 0.0, 0.0 };  // to store normalized RED, GREEN, BLUE, CLEAR reading
  for (int i = 0; i < 4; i++) {
    norm[i] = 1000000.0 / (float)colourArr[i];  // calculate each signal in Hz (R, G, B, CLEAR)
  }
  for (int i = 0; i < 3; i++) {   // subtract each colour from CLEAR signal
    norm[i] = norm[i] / norm[3];  // calculate %signal of R, G, B (relative to CLEAR signal)
  }
```

This gave rise to the following sketches:
* TCS3200_4CH.ino - takes readings on all 4 channels (R, G, B, CLEAR). It has a function to take 1 reading "readColour()", and a function to take N readings "readColourN()".
* TCS3200_4CH_colourID.ino - this takes N readings and decodes them into colour buckets RED, ORANGE, YELLOW, GREEN, BLUE, and PURPLE (or NOT_DETECTED if the colour doesn't fit nicely into these buckets).
* TCS3200_RGBLED.ino - it's the colourID sketch above. Once the colour is detected, the sketch lights up an RGB LED accordingly with the identified colour.
