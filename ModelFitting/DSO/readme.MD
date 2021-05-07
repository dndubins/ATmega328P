DSO.ino is a random search optimization routine by Rein Luus that can be used for model fitting. The routine is a bit slower, but with careful manipulation can converge even with complicated nonlinear models. The example sketch solves an arbitrary test system of linear equations (AX=Y) where Y is the measured (observed) dependent variable, and A is the series of coefficients (or readings) for the independent parameters X. The program fits parameter values X to satisfy AX=Y. For instance, for our RGB colour-to-wavelength example, one row of the data set to be fit could be:

  16•xp(0) + 45•xp(1) + 33•xp(2) = 700
  
where the measured readings coming from the colour sensor are stored in A={16, 45, 33),
X holds the fitted parameters for red, green, blue,
and Y = 700 nm (red wavelength).

The program will guess, refine, and provide parameter estimates for xp(0), xp(1), and xp(2), by minimizing (Yfit - Yobs)^2.
