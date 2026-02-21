# ATmega328P Fast Pin Functions

The **ATmega328P** is the microprocessor used for the Arduino Uno.  

With a bit of help from **ChatGPT**, I wrote a few barebones `#define` functions that help keep memory usage minimal:  

```cpp
// Fast pin modes, writes, and reads for the ATmega328P (digital pins 1-13)
// Modes: 0 = INPUT, 1 = OUTPUT, 2 = INPUT_PULLUP
#define pinModeFast(p, m) \
  do { \
    if ((p) >= 2 && (p) <= 7) { \
      if ((m)&1) DDRD |= (1 << (p)); \
      else DDRD &= ~(1 << (p)); \
      if (!((m)&1)) ((m)& 2 ? PORTD |= (1 << (p)) : PORTD &= ~(1 << (p))); \
    } else if ((p) >= 8 && (p) <= 13) { \
      if ((m)&1) DDRB |= (1 << ((p) - 8)); \
      else DDRB &= ~(1 << ((p) - 8)); \
      if (!((m)&1)) ((m)& 2 ? PORTB |= (1 << ((p)-8)) : PORTB &= ~(1 << ((p)-8))); \
    } \
  } while (0)

#define digitalWriteFast(p, v) \
  do { \
    if ((p) >= 2 && (p) <= 7) { \
      (v) ? PORTD |= (1 << (p)) : PORTD &= ~(1 << (p)); \
    } else if ((p) >= 8 && (p) <= 13) { \
      (v) ? PORTB |= (1 << ((p)-8)) : PORTB &= ~(1 << ((p)-8)); \
    } \
  } while (0)

#define digitalReadFast(p) \
  (((p) >= 2 && (p) <= 7) ? \
    ((PIND & (1 << (p))) ? 1 : 0) : \
   ((p) >= 8 && (p) <= 13) ? \
    ((PINB & (1 << ((p) - 8))) ? 1 : 0) : 0)

// Here are the classic bit functions:
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define bit_is_set(sfr, bit) (_SFR_BYTE(sfr) & _BV(bit))
#define bit_is_clear(sfr, bit) (!(_SFR_BYTE(sfr) & _BV(bit)))
#define loop_until_bit_is_set(sfr, bit) do { } while (bit_is_clear(sfr, bit))
#define loop_until_bit_is_clear(sfr, bit) do { } while (bit_is_set(sfr, bit))
```


