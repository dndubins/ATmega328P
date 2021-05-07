If this is your first stab at controlling a stepper motor, you are in for a treat. Stepper motors are pretty amazing! You might hear some jittering, humming, purring, and see some frustrating erratic motions, but once you get your motor tweaked it is in my opinion one of the most satisfying moments of a build.

4WStepper is just a simple generic program using the built-in Aruduino IDE library Stepper.h to control a stepper motor.

I have nothing against using a good library, but I do like writing my own sketches to try and understand how things work on a deeper level. This minimalist sketch will control a 4-wire stepper motor backwards and forwards at a given rpm in full or half steps, without calling any external libraries.
