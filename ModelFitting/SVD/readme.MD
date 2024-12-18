Singlular Value Decomposition (SVD) is a really cool algebraic model-fitting routine that gives you an approximation if the number of equations is less than the number of unknowns, a solution if the number of equations is equal to the number of unknowns, and a fit if the number of equations is greater than the number of unknowns. This routine was adapted from  William Press, Numerical Recipes in C++: The Art of Scientific Computing (2nd edition). Cambridge University Press New York, NY, USA ©1992. 

In SVD.ino, I use SVD to solve the red, green, and blue data I have collected from a colour sensor. The "READINGS" matrix holds the measured readings from an RGB TCS3200 colour sensor. The matrix "Y" holds wavelengths I would like to map those readings to.

I took this program on as a challenge to try and get the colour sensor to report a wavelength instead of a bucketed colour. The SVD model didn't work out well at all for this problem. It does solve a "wavelength", but not close enough to the actual colours to be useful. The Neural Network program works much better for identifying colours:
https://github.com/dndubins/ATmega328P/blob/main/ModelFitting/NN/NNsensor.ino<p>

SVDsensor.ino is the same program, with a Serial Monitor interface added to solve a new SVD model and save the solved parameters to EPROM. These solved parameters can then be loaded and used next time. The structure of this program is very good. I'm hanging onto it for multi-dimensional sensor readings. 
