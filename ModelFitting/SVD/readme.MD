Singlular Value Decomposition (SVD) is a really cool algebraic model-fitting routine that gives you an approximation if the number of equations is less than the number of unknowns, a solution if the number of equations is equal to the number of unknowns, and a fit if the number of equations is greater than the number of unknowns. This routine was adapted from  William Press, Numerical Recipes in C++: The Art of Scientific Computing (2nd edition). Cambridge University Press New York, NY, USA ©1992. 

In SVD.ino, I use SVD to solve the red, green, and blue data I have collected from a colour sensor. The 2x2 "READINGS" matrix holds the measured readings from an RGB TCS3200 colour sensor. The matrix "Y" holds wavelengths I would like to map those readings to.

I took this program on as a challenge to try and get the colour sensor to report a wavelength instead of a bucketed colour. It worked pretty well, considering!

SVDsensor.ino is the same program, with a Serial Monitor interface added to solve a new SVD model and save the solved parameters to EPROM. These solved parameters can then be loaded and used next time.
