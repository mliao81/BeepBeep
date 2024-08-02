# BeepBeep

BeepBeep is an IoT device that utilizes four different sensors to
turn a white cane smart. This was made as apart of our final project at COSMOS.

Our first function uses a TSL2591 Light Sensor to read IR levels, during the night this will signal for LEDs on the cane to turn on so others can visibly see the user with the cane.

The second function uses an ultrasonic sensor and piezo buzzer as object detection. As the cane gets closer to an object, the buzzer will beep to signal this.

The third function uses an accelerometer to track the positioning of the cane. If the cane is dropped or comes across any collision, it will beep until picked up.

The last feature uses a GPS module and ESP32 that acts as a tracker, with the location of the cane shown on a map on the Adafruit IO. The data from the GPS is sent over the cloud to the IO feed.
