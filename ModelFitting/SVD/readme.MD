Singlular Value Decomposition (SVD) is a really cool algebraic model-fitting routine that gives you an approximation if the number of equations is less than the number of unknowns, a solution if the number of equations is equal to the number of unknowns, and a fit if the number of equations is greater than the number of unknowns.

In this example, I use SVD to solve the red, green, and blue data I have collected from a colour sensor. The 2x2 "READINGS" matrix holds the measured readings from the colour sensor. The matrix "Y" holds wavelengths I would like to map those readings to.

I took this program on as a challenge to try and get the colour sensor to report a wavelength instead of a bucketed colour. It worked pretty well, considering!
