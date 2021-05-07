At first, I found the A4988 motor driver frustrating, and finicky. I must have toasted at least 10 of them before I got the hang of setting them up properly.

My first piece of advice for you is: don't hot swap the wires! This little chip is so sensitive to voltage spikes, that if you realize you have plugged a wire in the wrong pin, it's best to disconnect the power first. There are plenty of guides on how to hook up the A4988, and lots of libraries as well. I wrote this standalone library-free sketch for my electronics class to use. It provides full control of the A4988, complete with direction and microstepping control.

My second piece of advice for you: don't order just one of these units. They are inexpensive, and its best to have a backup. Once you have the unit going, they are ROCK solid, making this my favourite motor little driver.
