/*
 * Singular Value Decomposition coding based on:
 * William H. Press. Numerical recipes in C (2nd ed.): the art of scientific computing
 * Cambridge University Press New York, NY, USA ©1992
 * Author of this sketch: David Dubins
 * Date: 6-Feb-19
 * Last updated: 12-Dec-24
*/
#define DEBUG  //comment out for no serial debugging commands

const int MP = 10;  // total number of experimental observations in the system
const int NP = 4;   // number of parameters (R,G,B terms + intercept)

float READINGS[MP][NP - 1] = {
  // Sensor measurements (from RGB colour sensor)
  { 12, 54, 43 },  // red
  { 12, 52, 42 },  // red
  { 11, 15, 28 },  // yellow
  { 11, 16, 28 },  // yellow
  { 26, 18, 27 },  // green
  { 25, 17, 26 },  // green
  { 74, 37, 16 },  // blue
  { 67, 32, 14 },  // blue
  { 25, 41, 19 },  // violet
  { 25, 41, 19 }   // violet
};

//Y matrix (wavelengths we want sensor to map to)
float Y[MP] = {
  700,
  700,
  580,
  580,
  530,
  530,
  470,
  470,
  420,
  420
};

void setup() {
  Serial.begin(9600);
  float X[MP], A[NP];
  float CVM[NP][NP], RC[NP][NP];
  float U[MP][NP], V[NP][NP], W[NP];
  float ERR[MP];
  //Initialize variables:
  float CHISQ = 0.0;
  float MAXCOR = 0.0;
  float SS = 0.0;
  float SRK = 0.0;
  init1D(X, MP);
  init1D(A, NP);
  init2D(CVM, NP, NP);
  init2D(RC, NP, NP);
  init2D(U, MP, NP);
  init2D(V, NP, NP);
  init1D(W, NP);
  init1D(ERR, MP);

  svdfit(Y, MP, A, NP, U, V, W, CHISQ, SRK);
  svdvar(V, NP, W, CVM);
  for (int i = 0; i < NP; i++) {
    ERR[i] = sqrt(CVM[i][i]);
  }
  MAXCOR = 0.0;
  for (int i = 0; i < NP; i++) {
    for (int j = 0; j < NP; j++) {
      RC[i][j] = CVM[i][j] / (sqrt(CVM[i][i]) * sqrt(CVM[j][j]));
      if (i != j) {
        if (abs(RC[i][j]) > MAXCOR) MAXCOR = abs(RC[i][j]);
      }
    }
  }
#ifdef DEBUG
  //Print out the results
  Serial.println(F("Parameter Estimates:"));
  for (int i = 0; i < NP; i++) {
    Serial.println(A[i]);
  }
  Serial.println(F("Parameter Errors:"));
  for (int i = 0; i < NP; i++) {
    Serial.println(ERR[i], 8);
  }
  Serial.print(F("Model CHISQ: "));
  Serial.println(String(CHISQ, 8));
  Serial.print(F("Max correlation value: "));
  Serial.println(MAXCOR, 8);
  Serial.print(F("Correlation Matrix:"));
  for (int j = 0; j < NP; j++) {
    for (int i = 0; i < NP; i++) {
      Serial.print(RC[i][j]);
      Serial.print(F("  "));
    }
    Serial.println();
  }
  Serial.print(F("Rank: "));
  Serial.println(SRK);
#endif
}

void loop() {
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
  as a vector w[1..n]. The matrix V (not the transpose VT) is output as v[1..n][1..n]. */
  int flag, i, its, j, jj, k, L, nm;
  float anorm, c, f, g, h, s, scale1, x, y, z, rv1[NP];
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

void svdfit(float y[], int m, float a[], int n, float u[MP][NP], float v[NP][NP], float w[], float &chisq, float &srk) {
  /*Given a set of data points x[1..m],y[1..m], use χ2 minimization to determine the coefficients a[1..n] of the fitting
function y = sum i ai × afunci(x). Here we solve the fitting equations using singular
value decomposition of the m by n matrix, as in §2.6. Arrays u[1..m][1..n],
v[1..n][1..n], and w[1..n] provide workspace on input; on output they define the
singular value decomposition, and can be used to obtain the covariance matrix. The program
returns values for the n fit parameters a, and χ2, chisq. The user supplies a routine
funcs(x,afunc,n) that returns the n basis functions evaluated at x = x in the array
afunc[1..n].*/
  const float TOL = 0.00001;
  int i, j;
  float wmax, tmp, thresh, sum, ss;
  float afunc[m], b[m];  //b should have the dimensions of the # data points
  init1D(afunc, m);      // initialize afunc and b
  init1D(b, m);
  for (i = 0; i < m; i++) {  //Accumulate coefficients of the fitting matrix
    funcs(i, afunc, n);      // fills up afunc with right index values (specified in funcs)
    for (j = 0; j < n; j++) u[i][j] = afunc[j];
    b[i] = y[i];
  }
  svdcmp(u, m, n, w, v);  //Singular value decomposition.
  wmax = 0.0;             //Edit the singular values, given TOL
  for (j = 0; j < n; j++) {
    if (w[j] > wmax) wmax = w[j];
  }
  thresh = TOL * wmax;
  for (j = 0; j < n; j++) {
    if (w[j] < thresh) {
      w[j] = 0.0;
#ifdef DEBUG
      Serial.print(F("Model parameter "));
      Serial.print(j);
      Serial.println(F(" could be removed from the model."));
#endif
    }
  }
  srk = 0;
  for (j = 0; j < n; j++) {
    if (w[j] != 0) {
      srk++;
    }
  }
  svbksb(u, w, v, m, n, b, a);
  chisq = 0.0;  //Evaluate chi-square.
  ss = 0.0;
  for (i = 0; i < m; i++) {
    funcs(i, afunc, n);
    sum = 0.0;
    for (j = 0; j < n; j++) {
      sum = sum + a[j] * afunc[j];
    }
    ss = (y[i] - sum);
    chisq = chisq + (ss * ss);
  }
}

void svdvar(float v[NP][NP], int n, float w[], float cvm[NP][NP]) {
  /*To evaluate the covariance matrix cvm[1..n][1..n] of the fit for n parameters obtained
by svdfit, call this routine with matrices v[1..n][1..n], w[1..n] as returned from
svdfit.*/
  int i, j, k;
  float sum, wti[n];  //n used as index value here
  init1D(wti, n);     // initialize wti
  for (i = 0; i < n; i++) {
    wti[i] = 0.0;
    if (w[i] != 0.0) wti[i] = 1.0 / (w[i] * w[i]);
  }
  for (i = 0; i < n; i++) {  //Sum contributions to covariance matrix (15.4.20).
    for (j = 0; j <= i; j++) {
      sum = 0.0;
      for (k = 0; k < n; k++) sum += v[i][k] * v[j][k] * wti[k];
      cvm[i][j] = sum;
      cvm[j][i] = sum;
    }
  }
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

//This is the function used to fit the model AX=B. The following function is linear. It will perform the fit:
//y = p[0] + p[1]x[0] + p[2]x[1] + p[3]x[2]...
void funcs(int x, float p[], int np) {  // coefficients of matrix A
  int j = 0;
  p[0] = 1;
  for (j = 1; j < np; j++) {
    p[j] = READINGS[x][j - 1];  // use READINGS as p[] coefficients for the model
  }
}

//Here is an example of a fitting routine for a polynomial of degree np-1, that can be used in place of funcs:
//with coefficients in the array p(1..np).
//y[i] = x[0] + x[1]a[i] + x[2]a[i]^2 + x[3]a[i]^3...
//where p[0..n] are the coefficients [1, a[i], a[i]^2 ...]
/*void funcs_poly(int i, float p[], int np){
  int j=0;
  p[0]=1;
  for(j=1;j<np;j++){
    p[j]=p[j-1]*READINGS[i];
  }
}*/
