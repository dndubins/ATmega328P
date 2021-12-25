This project involves using a SoftwareSerial connection between pins 2 and 3 of two different Unos to send data from one MCU to the other. Each MCU is set up with a different 4x4 matrix keypad to illustrate how the data can be handled.<p>
The "MASTER" MCU is the MCU that's receiving the data from the SLAVE, and sending it along to the regular Serial Monitor, as well as its own data.<p>
The MASTER MCU is powered through USB. The SLAVE MCU is powered parasitically from the master (Vin - Vin, GND - GND). That way there only needs to be one connection to the computer.<p>
However, the slave.ino sketch will need to be uploaded to the SLAVE MCU through USB first.
