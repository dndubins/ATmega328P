# Exponential Notation Function: `float2exp()`

C++ allows you to **assign float values using exponential notation**, like this:

```cpp
float y = -0.532e12;
float z = 3234e-4;
```

However, when it comes to **printing those values in exponential format**, the Arduino `Serial` monitor doesn't help much — it just prints the decimal form.

###  Solution

I wrote this sketch (with some **AMAZING debugging help from ChatGPT**) to create a function called `float2exp()` that converts a `float` value into a string in **scientific (exponential) notation**, with a specified number of **significant digits**.

### Code
```
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
```

### Example

```cpp
Serial.println(float2exp(y, 3)); 
Serial.println(float2exp(z, 4));
```

### Output from the Serial Monitor:

```
-5.32e+11
3.234e-1
```

### Notes

- The `float2exp()` function is Arduino-friendly — it avoids `sprintf()` so it works even on boards with limited standard library support (like the Uno).
- You specify the **number of significant digits**, not just decimal places.
- It handles large and small numbers and formats them cleanly for display or logging.

---

> Special thanks to ChatGPT (OpenAI) for assistance with rounding logic and scientific notation formatting.

---

Feel free to copy, adapt, and share! If you improve the function or find a cool use for it, I'd love to hear about it.
