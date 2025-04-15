//float2exp.ino: Serial print in scientific notation
//Author: D. Dubins
//Date: 14-Apr-25
//Description: Converts float to scientific notation string with desired significant digits.
//Special thanks to ChatGPT (OpenAI) for assistance with rounding logic and exponent handling.

float x = 3.141e10;  // Inputting like this works in C++

void setup() {
  Serial.begin(9600); //start the serial montior
}

void loop() {
  Serial.print(x,3); // print x with 3 decimals
  Serial.print(":  ");
  //print x in exp format to 3 dec places:
  Serial.println(float2exp(x, 3)); 
  delay(1000); //wait a bit
  x /= 10; //change number
}

String float2exp(float num, byte sigDigits) {
  if (num == 0) return "0.00e+0";
  if (isnan(num)) return "NaN";
  int exponent = floor(log10(abs(num))); // find out the order
  float scaled = num / pow(10, exponent); // scale the number
  // Round scaled to (sigDigits - 1) decimal places
  float rounded = round(scaled * pow(10, sigDigits - 1)) / pow(10, sigDigits - 1); // round it
  // Handle rounding edge case where 9.999... rounds to 10.0
  if (rounded >= 10.0) {
    rounded /= 10.0;
    exponent++;
  }
  // Build string
  String expStr = String(rounded, sigDigits - 1);  // this controls how many decimal places show
  expStr += "e" + String((exponent >= 0 ? "+" : "")) + String(exponent);
  return expStr;
}
