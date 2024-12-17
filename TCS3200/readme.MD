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
#define TIMEOUT 1000                           // for timeout (in microseconds) on reading a colour
  unsigned long thisRead[3] = { 0, 0, 0 };     // for data averaging
  int maxRead = 0;                             // maximum reading (for normalizing the colour signal to the highest intensity)
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

Also remember that a LOW reading on a channel means that there's a lot of that colour detected. I think my rookie mistake was looking at an RGB reading like  "11, 43, 39" and thinking there was no red in it. Nope, that's red. Think physics - you are filtering OUT that channel with a red filter. So if there's red light, it won't get to the sensor as well. <p>

You can crudely define ranges that "colours" can be identified. TCS3200_RGBLED.ino is my first use of enums. I normalized the RGB signal by taking the maximum reading value of the three channels (max of R,G,B), then calculating normalized signal = (max - signal)/max. This gives a value for each channel between 0 and 1. The number 0.25 serves as a useful cutoff of a "meaningful" signal. Orange and red are a bit more tricky difficult to resolve. Once the sketch decodes the colour, it lights up a little RGB module with the corresponding colour.
