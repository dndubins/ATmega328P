This script is a very simple read of the TCS3200 colour sensor.<p>

There are better sketches out there. This one just gives you the raw readings. If you would like to interpret colour, I was particularly proud of this neural network sketch:
https://github.com/dndubins/ATmega328P/blob/main/ModelFitting/NN/NNsensor.ino<p>

Another approach I tried was a linear model to fit colour sensor readings to wavelength, using SVD (singular value decomposition):
https://github.com/dndubins/ATmega328P/blob/main/ModelFitting/SVD/SVDsensor.ino<p>

This program didn't work as well as I had hoped, but it does crudely report a wavelength with given TCS3500 readings.<p>

An important realization I made about this sensor is that once you set the colour channel using S2 and S3, you can take as many pulseIn() readings as you'd like. So I came up with a function:<p>
void readColourN(int colourArr[3], int n) <p>

This function takes an array as an input argument (remember, arrays in this context are call-by-value), and takes n readings of each colour. This speeds up a reading considerably since you should wait at least ~100msec before taking a colour reading after setting S2 and S3. But then you can take multiple readings, and take the average over that colour.

Also remember that a LOW reading on the red channel means that the colour has lots of red. I think my rookie mistake was looking at an RGB reading like  "11, 43, 39" and thinking there was no red in it. Nope, that's red.
