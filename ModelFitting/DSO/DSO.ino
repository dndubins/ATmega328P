/* Direct Search Optimization Fitting Routine
 * Source: Luus R., Jaakola THI. Optimization by Direct Search and Systematic Reduction in the Size of Search Region
 * AIChE Journal 19(4):760 - 766 · July 1973
 * This search routine can be used for nonlinear fits.
 * It is used here on a test matrix, where the solution vector should yeild:
 * xp[0] = 0.83
 * xp[1] = 0.37
 * xp[2] = 0.18
 * xp[3] = 0.93
 * 
 * The READINGS matrix could be sensor readings (e.g. RGB values from a colour sensor).
 * The Y matrix are the observed results.
 * The routine fits X values in the equation AX = Y.
 */

const int MP=10;  // total number of experimental observations in the system
const int NP=4;   // number of parameters
#define NPTS 1000 // number of points in dso region
#define NIT 500   // number of iterations for dso routine
#define TOLERANCE 0.0001  // tolerance for minimizing chisq
#define RED 0.95  //reduction factor for dso routine (0.9 > RED > 1.0)
                  // a higher reduction factor spans the space better but takes longer.
#define DEBUG     //comment out to reduce serial commands
                  // scroll down to set starting values and regions for XP

float READINGS[MP][NP] = {  // Parameter coefficients of the A matrix
  { 0.47, 0.89, 0.03 },
  { 0.59, 0.04, 0.06 },
  { 0.09, 0.88, 0.14 },
  { 0.48, 0.13, 0.78 },
  { 0.19, 0.82, 0.60 },
  { 0.55, 0.56, 0.18 },
  { 0.59, 0.71, 0.72 },
  { 0.12, 0.21, 0.39 },
  { 0.49, 0.74, 0.63 },
  { 0.9, 0.74, 0.85 }
}; 

//Y matrix (experimental results)
float Y[MP] = {
  1.1920,
  1.1113,
  1.1519,  
  1.7564,
  1.6059,
  1.3017,
  1.8457,
  1.2749,
  1.7304, 
  2.0867  
};
   
void setup(){
  Serial.begin(9600);
  float A[NP];
  float CHISQ=0.0;
  init1D(A,NP); 
  dso(A, Y, MP, NP, RED, NPTS, NIT, CHISQ);
  //Print out the results
  Serial.println(F("Parameter Estimates:"));
  for(int i=0;i<NP;i++){
    Serial.println(A[i],5);
  }
  Serial.println("Model CHISQ: "+String(CHISQ,8));
}  

void loop (){
}

void dso(float a[NP], float y[MP], int m, int n, float red, int npts, int nit, float &chisq){
  float xp[n]; // refined guess at right X values
  float xs[n]; // first guess at right X values
  float x[n]; // test X value matrix
  float v[m]; // V matrix is like Yfit.
  float vrand[m];
  float reg[n]; //region size to be examined spanning for each xp
  float afunc[m];
  int no=0;     //number of improvements made
  float test=0.0; //test value. Is iterating getting better?
  float perf=0.0; // performance index (in this example, sum of Vrand - V should be close to zero).
  float fm=0.0;   // to record a better P value
  int lw=0; // counter
  bool flag=false; // all-purpose flag
  int lim=20; //print out every lim iterations
  // First guesses for parameters: (make sure these lie within realistic ranges)
  xp[0]=0.5;
  xp[1]=0.5;
  xp[2]=0.5;
  xp[3]=0.5;
  // Bounds for region go here: (+/- range for the first XP value)
  reg[0]=1.0;
  reg[1]=1.0;
  reg[2]=1.0;
  reg[3]=1.0;
  //initialize matrices:
  init1D(x,n);
  init1D(v,n);
  init1D(vrand,n);
  perf=0.0;  
  for(int i=0;i<m;i++){ // DSO routine starts here.
    // The equation is a*x=v
    for (i=0;i<m;i++){  //// solve v[i] for all data values (#rows = m)
      funcs(i,afunc,n); // fills up afunc with the right row of a (or the correct function values depending on funcs)
      // This is for a 4-parameter linear model v[i] = xp[0] + x[0]*xp[1] + x[1]*xp[2] + x[2]*xp[3] (adjust as necessary)
      vrand[i]=afunc[0]*xp[0]; // same as vrand[i]=xp[0]
      for(int j=1;j<n;j++){ 
        vrand[i]=vrand[i]+xp[j]*afunc[j];  
      }
    }
  }
  for(int i=0;i<m;i++){
    v[i]=y[i]; // if there was some normalizing of y to do, you would do it here (e.g. baseline normalization).
  }
  test=0.0;
  for(int i=0;i<m;i++){
    test=test+((vrand[i]-v[i])*(vrand[i]-v[i])); // calculate the first chisq 
  }
  for(int i=0;i<n;i++){
    xs[i]=xp[i]; // first value of xs is xp
  }
  fm=test; // prevents exiting the loop if no better value is found on the first round.
  #ifdef DEBUG
    Serial.println("k, no, fm, xs1, reg1, xs2, reg2, xs3, reg3..."); // print headings
  #else
    Serial.println("Solving DSO routine. Please wait.");
  #endif
  for(int k=0;k<nit;k++){ // DSO proper starts here.
    for(int j=0;j<npts;j++){
      flag=false;
      for(int i=0;i<n;i++){ // fill up x with random guesses
        randomSeed(analogRead(A0));
        x[i]=xp[i]+reg[i]*(((float)random(1001)/1000.0)-0.5); // guess a number for each param within region limits
        //if(x[i]<0.0)flag=true; // This is how you would exclude impossible parameter guesses
      }
      if(!flag){
        for(int h=0;h<m;h++){  // note in this loop how x is used in place of xp.
          // The equation is a*x=v
          funcs(h,afunc,n); // fills up afunc with the right row of a (or the correct function values depending on funcs)
          // This is for a 4-parameter linear model v[i] = xp[0] + x[0]*xp[1] + x[1]*xp[2] + x[2]*xp[3] (adjust as necessary)
          vrand[h]=afunc[0]*x[0];
          for(int j=1;j<n;j++){ 
            vrand[h]=vrand[h]+x[j]*afunc[j];  
          }
        }
        for(int h=0;h<m;h++){
          v[h]=y[h]; // opportunity to normalize v here (otherwise this code is useless)
        }
        perf=0.0;
        for(int h=0;h<m;h++){
          perf=perf+((vrand[h]-v[h])*(vrand[h]-v[h])); // calculate the first chisq 
        }
        if(abs(perf)<abs(test)){ // we've found an improvement!
          no++;
          test=perf;
          fm=perf;
          for(int h=0;h<n;h++){
            xs[h]=x[h]; // store better values to xs
          }
        }
      }
    }
    #ifdef DEBUG
      if(lw<(lim-1)){
        lw++;
      }else{
        Serial.print(k);Serial.print(F(", "));
        Serial.print(no);Serial.print(F(", "));
        Serial.print(fm,5);Serial.print(F(", "));
        for(int h=0;h<n;h++){
          Serial.print(xs[h]);Serial.print(F(", "));
          Serial.print(reg[h]);Serial.print(F(", "));
        }
        Serial.println();
        lw=0; //reset LW
      }
    #else
      Serial.println(F("."));
    #endif
    for(int i=0;i<n;i++){
      reg[i]=reg[i]*red; // shrink region size by reduction factor
      xp[i]=xs[i]; // set xp to best result in random trial (xs)
    }
    if(abs(fm)<TOLERANCE){ // fm is the chisq
      Serial.println(F("Tolerance attained."));
      chisq=fm;
      for(int i=0;i<n;i++){
        a[i]=xs[i]; //record final parameter values
      }
      break;
    }
  }
}

void init1D(float arr[], const int m){
  for(int i=0;i<m;i++){
    arr[i]=0.0;
  }
}

//This is the function used to fit the model AX=B. The following function is linear. It will perform the fit:
//y = p[0] + p[1]x[0] + p[2]x[1] + p[3]x[2]...
void funcs(int i, float p[], int np){  // coefficients of matrix A
  int j=0;
  p[0]=1;
  for(j=1;j<np;j++){
    p[j]=READINGS[i][j-1];
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
