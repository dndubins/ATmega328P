The PID (Proportional - Integral - Derivative) Controller is perhaps overkill for the hobbyist, but a really interesting combined closed-loop feedback strategy to get a system to settle quickly. PID.ino is an example sketch that programs a transistor-actuated fan on Pin 11 to cool an LM35 sensor, read by pin A1. The response can be observed using the Serial Monitor. Remove all serial commands if you plan on using this routine in a finished circuit.

PID-TMP36.ino is the same sketch, using the TMP36 temperature sensor instead of the LM35.
