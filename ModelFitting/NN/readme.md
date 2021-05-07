NN.ino is a Neural Network model fitting routine, adapted from Ralph Heymsfeld's program available with a fantastic description here:
http://robotics.hobbizine.com/arduinoann.html
The sketch trains a small neural network to recognize colour categories (yellow, red, etc.) based on experimentally collected values from an RGB TCS3200 colour sensor. The Input matrix holds the measured values from the colour sensor (with bucketed colours indicated in the comments).
The Target matrix holds the corresponding buckets (red, green, blue, etc.) that we want to train the neural network to recognize.

NNsensor.ino takes this routine to the next level, providing a serial monitor interface complete with menu to set up, train, and use the neural network. The solved network is then written to EPROM and loaded next time, so you don't have to train each time.
