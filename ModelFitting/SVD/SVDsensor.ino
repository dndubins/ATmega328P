/* SVDsensor.ino (for TCS3200 Colour Sensor)
 * Singular Value Decomposition coding based on:
 * William H. Press. Numerical recipes in C (2nd ed.): the art of scientific computing
 * Cambridge University Press New York, NY, USA ©1992 
 * 
 * TCS3200 color recognition sensor 
 * Sensor connection pins to Arduino are shown in comments
 * SVD is used to fit colour intensities into an overall "wavelength", and then the wavelength ranges are bucketed to a colour at the end.
 *
 * Author of this sketch: David Dubins
 * Date: 6-Feb-19
 * Last updated: 16-Dec-24
 * 
 * Connections:
 * TCS3200 - Arduino Uno
 * ---------------------
 *  VCC - 5V
 *  OUT - 8
 *  S2  - 9
 *  S3  - 10
 *  S0  - 11
 *  S1  - 12
 *  OE  - GND
 *  GND - GND
 */

#include <EEPROM.h>    // for saving SVD matrix
#define NUMREADS 1000  // number of readings per colour reading (for averaging)
#define DEBUG          //comment out to remove extra serial debugging messages
#define MENU           //comment out for just continous readings
//#define EPROMLOAD    //comment out if not loading/saving weights from EPROM (useful for first time running sketch)
const int MP = 14;  // total number of experimental observations in the system
const int NP = 4;   // number of parameters (# columns + intercept)

// Colour sensor module pins and setup
#define S0 11
#define S1 12
#define S2 9
#define S3 10
#define OUT 8
int reading[3] = { 0, 0, 0 };  // to store red, green, blue reading

struct SVDweights {
  float Input[MP][NP - 1];
  float Y[MP];
} SVD;

char choice = '\0';  // For serial menu. Initialize choice with NULL.

const char* TargetNames[7] = { "red", "orange", "yellow", "green", "cyan", "blue", "violet" };  // titles to match training set

// for defining SVD matrix manually: (not needed with EPROMLOAD)
#ifndef EPROMLOAD
float Input[MP][NP - 1] = {
  //training set (sensor readings)
  { 11, 43, 39 },    // red
  { 11, 47, 43 },    // red
  { 7, 24, 27 },     // orange
  { 6, 20, 24 },     // orange
  { 10, 18, 28 },    // yellow
  { 10, 16, 27 },    // yellow
  { 19, 17, 26 },    // green
  { 20, 18, 25 },    // green
  { 36, 35, 26 },    // cyan
  { 32, 29, 22 },    // cyan
  { 61, 61, 33 },    // blue
  { 113, 105, 58 },  // blue
  { 21, 31, 17 },    // violet
  { 22, 38, 23 }     // violet
};
#endif

//Read in Y matrix (outcomes to map to - this is "wavelength")
float Y[MP] = {
  700,  //red
  700,  //red
  620,  //orange
  620,  //orange
  590,  //yellow
  590,  //yellow
  520,  //green
  520,  //green
  490,  //cyan
  490,  //cyan
  460,  //blue
  460,  //blue
  400,  //violet
  400   //violet
};

//SVD variables:
float X[MP], A[NP];
float U[MP][NP], V[NP][NP], W[NP];
//Initialize variables:

void setup() {
  Serial.begin(9600);
#ifdef EPROMLOAD  // if loading previous values from EPROM
  Serial.println(F("Loading weights form EEPROM."));
  EEPROM.get(0, SVD);  // get weights from EEPROM
  Serial.println(F("Weights loaded."));
#else  // if using historical data, copy back the values into SVD
  Serial.println(F("Loading default historical data."));
  for (int i = 0; i < MP; i++) {
    SVD.Y[i] = Y[i];
    for (int j = 0; j < NP - 1; j++) {
      SVD.Input[i][j] = Input[i][j];
    }
  }
  Serial.println(F("Data loaded."));
#endif
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  solveSVD();
#ifdef MENU
  printMenu();  //reprint user menu
#endif
}

void loop() {
#ifdef MENU
  if (Serial.available()) {
    choice = Serial.read();
  } else if (choice == 'r') {
    Serial.println(F("\nReading sample:"));
    readSample();
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice == 'c') {
    readSample();
    delay(100);  //short delay
  } else if (choice == 't') {
    for (int i = 0; i < MP; i++) {  // train each colour twice
                                    // Use this for manual wavelength
      //To use stored wavelengths in Y matrix, uncomment the following 5 lines:
      Serial.print(F("Enter 'r' to read response for "));
      Serial.print(Y[i]);
      Serial.println(F(" nm>"));
      while (!Serial.available()) { ; }  // wait for input
      choice = Serial.read();            // throw away r (not needed)
      //Alternately, to enter wavelength in manually, uncomment the following 3 lines:
      //Serial.println(F("Enter wavelength> "));
      //while (!Serial.available()) { ; }  // wait for input
      //SVD.Y[i] = Serial.parseInt();
      Serial.print(F("Reading response for "));
      Serial.print(SVD.Y[i]);
      Serial.println(F(" nm."));
      readColourN(reading, NUMREADS);  // read sensor NUMREADS times
      Serial.print(F("Reading: "));
      Serial.print(reading[0]);
      Serial.print(F(","));
      Serial.print(reading[1]);
      Serial.print(F(","));
      Serial.println(reading[2]);
      SVD.Input[i][0] = reading[0];  //red
      SVD.Input[i][1] = reading[1];  //green
      SVD.Input[i][2] = reading[2];  //blue
    }
    solveSVD();
#ifdef EPROMLOAD
    Serial.println(F("Saving weights to EEPROM."));
    EEPROM.put(0, SVD);  // write weights to EEPROM
    Serial.println(F("Weights saved."));
#endif
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  } else if (choice != '\0') {
    Serial.println(F("\nInvalid option."));
    choice = '\0';  //erase choice
    printMenu();    //reprint user menu
  }
#else
  readSample();
  delay(100);  //short delay
#endif
}

void solveSVD() {
  //Initialize variables:
  init1D(X, MP);
  init1D(A, NP);
  init2D(U, MP, NP);
  init2D(V, NP, NP);
  init1D(W, NP);
  svdfit(SVD.Y, MP, A, NP, U, V, W);
#ifdef DEBUG
  //Print out the results
  Serial.println(F("Parameter Estimates:"));
  for (int i = 0; i < NP; i++) {
    Serial.println(A[i]);
  }
#endif
}

void svbksb(float u[MP][NP], float w[], float v[NP][NP], int m, int n, float b[], float x[]) {
  /*Solves A·X = B for a vector X, where A is specified by the arrays u[1..m][1..n], w[1..n],
v[1..n][1..n] as returned by svdcmp. m and n are the dimensions of a, and will be equal for
square matrices. b[1..m] is the input right-hand side. x[1..n] is the output solution vector.
No input quantities are destroyed, so the routine may be called sequentially with different b’s.
    U is mp x np (POSITION 1)
    W is 1 x np  (POSITION 2)
    v is np x np (POSITION 3)
    m is POSITION 4
    n is POSITION 5
    b is 1 x mp (POSITION 6)
    x is 1 x np (POSITION 7) (output solution vector)
*/
  int jj, j, i;
  float s;
  float tmp[n];  // should be # params
  for (j = 0; j < n; j++) {
    s = 0.0;
    if (w[j] != 0) {
      for (i = 0; i < m; i++) s += u[i][j] * b[i];
      s = s / w[j];
    }
    tmp[j] = s;
  }
  for (j = 0; j < n; j++) {
    s = 0.0;
    for (jj = 0; jj < n; jj++) s = s + v[j][jj] * tmp[jj];
    x[j] = s;
  }
}

void svdcmp(float a[MP][NP], int m, int n, float w[], float v[NP][NP]) {
  /*Given a matrix a[1..m][1..n] this routine computes its singular value decomposition, 
  A=U*W*VT. The matrix U replaces a on output. The diagonal matrix of singular values W is output
  as a vector w[1..n]. The matrix V (not the transpose VT) is output as v[1..n][1..n]. 
  m: number of experimental values or data points (rows)
  n: number of parameters (columns)
  */
  int flag, i, its, j, jj, k, L, nm;
  float anorm, c, f, g, h, s, scale1, x, y, z, rv1[n];
  init1D(rv1, n);
  g = 0.0;
  scale1 = 0.0;
  anorm = 0.0;
  for (i = 0; i < n; i++) {
    L = i + 1;
    rv1[i] = scale1 * g;
    g = 0.0;
    s = 0.0;
    scale1 = 0.0;
    if (i < m) {
      for (k = i; k < m; k++) scale1 = scale1 + abs(a[k][i]);
      if (scale1 != 0.0) {
        for (k = i; k < m; k++) {
          a[k][i] = a[k][i] / scale1;
          s = s + a[k][i] * a[k][i];
        }
        f = a[i][i];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][i] = f - g;
        for (j = L; j < n; j++) {
          s = 0.0;
          for (k = i; k < m; k++) s = s + a[k][i] * a[k][j];
          f = s / h;
          for (k = i; k < m; k++) a[k][j] = a[k][j] + f * a[k][i];
        }
        for (k = i; k < m; k++) a[k][i] = scale1 * a[k][i];
      }  // end if
    }    // end if
    w[i] = scale1 * g;
    g = 0.0;
    s = 0.0;
    scale1 = 0.0;
    if (i < m && i != (n - 1)) {
      for (k = L; k < n; k++) scale1 = scale1 + abs(a[i][k]);
      if (scale1 != 0) {
        for (k = L; k < n; k++) {
          a[i][k] = a[i][k] / scale1;
          s = s + a[i][k] * a[i][k];
        }
        f = a[i][L];
        g = -SIGN(sqrt(s), f);
        h = f * g - s;
        a[i][L] = f - g;
        for (k = L; k < n; k++) rv1[k] = a[i][k] / h;
        for (j = L; j < m; j++) {
          s = 0.0;
          for (k = L; k < n; k++) s = s + a[j][k] * a[i][k];
          for (k = L; k < n; k++) a[j][k] = a[j][k] + s * rv1[k];
        }
        for (k = L; k < n; k++) a[i][k] = a[i][k] * scale1;
      }
    }
    anorm = MAX(anorm, (abs(w[i] + abs(rv1[i]))));
  }

  for (i = (n - 1); i > -1; i--) {  // Accumulation of right-hand transformation
    if (i < (n - 1)) {
      if (g != 0) {
        for (j = L; j < n; j++) v[j][i] = (a[i][j] / a[i][L]) / g;  // Double division to avoid possible underflow
        for (j = L; j < n; j++) {
          s = 0.0;
          for (k = L; k < n; k++) s = s + a[i][k] * v[k][j];
          for (k = L; k < n; k++) v[k][j] = v[k][j] + s * v[k][i];
        }
      }
      for (j = L; j < n; j++) {
        v[i][j] = 0.0;
        v[j][i] = 0.0;
      }
    }
    v[i][i] = 1.0;
    g = rv1[i];
    L = i;
  }
  for (i = (MIN(m, n) - 1); i > -1; i--) {  // Accumulation of left-hand transformation
    L = i + 1;
    g = w[i];
    for (j = L; j < n; j++) a[i][j] = 0.0;
    if (g != 0) {
      g = 1.0 / g;
      for (j = L; j < n; j++) {
        s = 0.0;
        for (k = L; k < m; k++) s = s + a[k][i] * a[k][j];
        f = (s / a[i][i]) * g;
        for (k = i; k < m; k++) a[k][j] = a[k][j] + f * a[k][i];
      }
      for (j = i; j < m; j++) a[j][i] = a[j][i] * g;
    } else {
      for (j = i; j < m; j++) a[j][i] = 0.0;
    }
    a[i][i] = a[i][i] + 1.0;
  }
  for (k = (n - 1); k > -1; k--) {    // diagonalization of the bidiagonal form; loop over
    for (its = 0; its < 30; its++) {  //singular values, and over allowed iteration
      flag = 1;
      for (L = k; L > -1; L--) {  // k was set to n-1 so it's ok to set L=k here
        nm = L - 1;
        if (abs(rv1[L] + anorm) == anorm) {
          flag = 0;
          break;  //exit for
        }
        if (abs(w[nm] + anorm) == anorm) break;
      }
      if (flag != 0) {  // this doesn't seem to run
        c = 0.0;
        s = 1.0;
        for (i = L; i <= k; i++) {  //needs to go all the way to k. k has been adjusted.
          f = s * rv1[i];
          rv1[i] = c * rv1[i];
          if (abs(f) + anorm == anorm) break;
          g = w[i];
          h = pythag(f, g);
          w[i] = h;
          h = 1.0 / h;
          c = g * h;
          s = -f * h;
          for (j = 0; j < m; j++) {
            y = a[j][nm];
            z = a[j][i];
            a[j][nm] = (y * c) + (z * s);
            a[j][i] = (z * c) - (y * s);
          }
        }
      }
      z = w[k];
      if (L == k) {     //Convergence.
        if (z < 0.0) {  //Singular value is made nonnegative.
          w[k] = -z;
          for (j = 0; j < n; j++) v[j][k] = -v[j][k];
        }
        break;  //exit for condition
      }
      if (its == 29) {
#ifdef DEBUG
        Serial.println(F("No convergence in 30 SVDCMP iterations."));
#endif
      }
      x = w[L];
      nm = k - 1;
      y = w[nm];
      g = rv1[nm];
      h = rv1[k];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = pythag(f, 1.0);
      f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
      c = 1.0;
      s = 1.0;
      for (j = L; j <= nm; j++) {  // k has been adjusted so for loop needs to go all the way to nm
        i = j + 1;
        g = rv1[i];
        y = w[i];
        h = s * g;
        g = c * g;
        z = pythag(f, h);
        rv1[j] = z;
        c = f / z;
        s = h / z;
        f = (x * c) + (g * s);
        g = (g * c) - (x * s);
        h = y * s;
        y = y * c;
        for (jj = 0; jj < n; jj++) {
          x = v[jj][j];
          z = v[jj][i];
          v[jj][j] = (x * c) + (z * s);
          v[jj][i] = (z * c) - (x * s);
        }
        z = pythag(f, h);
        w[j] = z;  // Rotation can be arbitrary if z=0
        if (z != 0) {
          z = 1.0 / z;
          c = f * z;
          s = h * z;
        }
        f = (c * g) + (s * y);
        x = (c * y) - (s * g);
        for (jj = 0; jj < m; jj++) {
          y = a[jj][j];
          z = a[jj][i];
          a[jj][j] = (y * c) + (z * s);
          a[jj][i] = (z * c) - (y * s);
        }
      }
      rv1[L] = 0.0;
      rv1[k] = f;
      w[k] = x;
    }
  }
}

float SIGN(float a, float b) {
  if (b >= 0.0) {
    return abs(a);
  } else {
    return -abs(a);
  }
}

float MAX(float a, float b) {
  if (a >= b) {
    return a;
  } else {
    return b;
  }
}

float MIN(float a, float b) {
  if (a >= b) {
    return b;
  } else {
    return a;
  }
}

void svdfit(float y[], int ndata, float a[], int ma, float u[MP][NP], float v[NP][NP], float w[]) {
  /*Given a set of data points x[1..ndata],y[1..ndata], use χ2 minimization to determine the coefficients a[1..ma] of the fitting
function y = sum i ai × afunci(x). Here we solve the fitting equations using singular
value decomposition of the ndata by ma matrix, as in §2.6. Arrays u[1..ndata][1..ma],
v[1..ma][1..ma], and w[1..ma] provide workspace on input; on output they define the
singular value decomposition, and can be used to obtain the covariance matrix. The program
returns values for the ma fit parameters a. The user supplies a routine
funcs(x,afunc,ma) that returns the ma basis functions evaluated at x = x in the array
afunc[1..ma].*/
  const float TOL = 0.00001;
  int i, j;
  float wmax, tmp, thresh, sum;
  float afunc[ndata], b[ndata];  //b should have the dimensions of the # data points
  init1D(afunc, ndata);          // initialize afunc and b
  init1D(b, ndata);
  for (i = 0; i < ndata; i++) {  //Accumulate coefficients of the fitting matrix
    funcs(i, afunc, ma);         // fills up afunc with right index values (specified in funcs)
    for (j = 0; j < ma; j++) u[i][j] = afunc[j];
    b[i] = y[i];
  }
  svdcmp(u, ndata, ma, w, v);  //Singular value decomposition.
  wmax = 0.0;                  //Edit the singular values, given TOL
  for (j = 0; j < ma; j++) {
    if (w[j] > wmax) wmax = w[j];
  }
  thresh = TOL * wmax;
  for (j = 0; j < ma; j++) {
    if (w[j] < thresh) {
      w[j] = 0.0;
#ifdef DEBUG
      Serial.print(F("Model parameter "));
      Serial.print(j);
      Serial.println(F(" could be removed from the model."));
#endif
    }
  }
  svbksb(u, w, v, ndata, ma, b, a);
}

float pythag(float a, float b) {
  //Computes sqrt(a^2+b^2) without destructive overflow or underflow
  float absa, absb;
  absa = abs(a);
  absb = abs(b);
  if (absa > absb) {
    return absa * sqrt(1.0 + (absb / absa) * (absb / absa));
  } else if (absb == 0.0) {
    return 0.0;
  } else {
    return absb * sqrt(1.0 + (absa / absb) * (absa / absb));
  }
}

void init1D(float arr[], const int m) {
  for (int i = 0; i < m; i++) {
    arr[i] = 0.0;
  }
}

void init2D(float arr[][NP], const int m, const int n) {
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      arr[i][j] = 0.0;
    }
  }
}

void printMenu() {  // user menu
  Serial.println(F("r: Read one sample"));
  Serial.println(F("c: Continuous reads"));
  Serial.println(F("t: train program with new values"));
  Serial.println(F("Enter choice: "));
}

void funcs(int x, float p[], int np) {  // coefficients of matrix A
  int j = 0;
  p[0] = 1;
  for (j = 1; j < np; j++) {
    p[j] = SVD.Input[x][j - 1];
  }
}

float useSVD(int R, int G, int B) {  // use SVD model to solve for wavelength
  float measuredInput[3];
  measuredInput[0] = R;
  measuredInput[1] = G;
  measuredInput[2] = B;
  float sum = A[0];
  for (int j = 1; j < 4; j++) {
    sum = sum + A[j] * measuredInput[j - 1];
  }
  return sum;
}

// This function takes an array as an input agument, and calculates the average
// of n readings on each colour channel.
void readColourN(int colourArr[3], int n) {  // arrays are always passed by value
#define TIMEOUT 1000                           // for timeout (in microseconds) on reading a colour
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
    colourArr[i] = thisRead[i];  //write back to colourArr
  }
}

void readSample() {                // take one reading and apply model to calculate output
  readColourN(reading, NUMREADS);  // take measurement
  int wavelength = useSVD(reading[0], reading[1], reading[2]);
#ifdef DEBUG
  Serial.print(F("R:"));
  Serial.print(reading[0]);
  Serial.print(F(" G:"));
  Serial.print(reading[1]);
  Serial.print(F(" B:"));
  Serial.print(reading[2]);
  Serial.print(F("  wavelength:"));
  Serial.print(wavelength);
  String colourname = "";
  //Wavelengths from https://en.wikipedia.org/wiki/Color
  //https://academo.org/demos/wavelength-to-colour-relationship/
  if (wavelength < 700 && wavelength >= 635) colourname = "red";
  if (wavelength < 635 && wavelength >= 590) colourname = "orange";
  if (wavelength < 590 && wavelength >= 560) colourname = "yellow";
  if (wavelength < 560 && wavelength >= 520) colourname = "green";
  if (wavelength < 520 && wavelength >= 490) colourname = "cyan";
  if (wavelength < 490 && wavelength >= 450) colourname = "blue";
  if (wavelength < 450 && wavelength >= 400) colourname = "violet";
  Serial.println("   " + colourname);
#else
  Serial.println(wavelength);
#endif
}
