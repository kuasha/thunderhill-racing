### Joystick commander and Agile trainer with Polysync Main Node

The work done in this section is based heavily on work done from John Chen on the behavioral cloning project from the Self-Driving Car Nano degree: [John's Agile Trainer](https://github.com/diyjac/AgileTrainer)

## Communicating with Polysync and the Kia Soul:

To be able to get data to and from polysync and the Kia Soul we had a simple architecture:

-------------------
|  data_buffer    |
|                 |
-------------------
        |
-------------------
|  client_node    |
|                 |
-------------------

The first portion consists of a data buffer which is essentially a queue to hold our data up to a limit we have specified in the data_buffer, when the queue hits that limit it will start sending data or poping whatever it contains to polysync.


In the Client Node, there are three distictive parallel threads:


-----------------------
|  make_prediction    | --> Gets data from the car as well to predict and send values using the third thread to make the car drive 
|                     |
-----------------------
        
-------------------
|  train_model    | --> Takes in values from the car and feeds into a preloaded trained model to keep training 
|                 |
-------------------

-------------------
|  send_values    | --> Communicates with the Main Node in Polysync to send values like the throttle and brake
|                 |  
-------------------

## Python Trainer:


