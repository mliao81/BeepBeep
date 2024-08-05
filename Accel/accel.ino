#include <stdio.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include <stdbool.h>

// object sensor
const int trig1 = 6;
const int echo1 = 7;
const int buzzer = 8;
int threshold = 50;
int buzzer_delay = 0;
float duration1;
float distance1;

// accelerometer
int current_amp = 0;
bool curr_amp_set = false;

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
bool fall = false;        // stores if a fall has occurred
boolean trigger1 = false; // stores if first trigger (lower threshold) has occurred
boolean trigger2 = false; // stores if second trigger (upper threshold) has occurred
boolean trigger3 = false; // stores if third trigger (orientation change) has occurred
byte trigger1count = 0;   // stores the counts past since trigger 1 was set true
byte trigger2count = 0;   // stores the counts past since trigger 2 was set true
byte trigger3count = 0;   // stores the counts past since trigger 3 was set true
int angleChange = 0;

void setup()
{
    pinMode(trig1, OUTPUT);
    pinMode(echo1, INPUT);
    pinMode(buzzer, OUTPUT);

    Serial.begin(115200);
    Wire.begin();
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B); // PWR_MGMT_1 register
    Wire.write(0);    // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    // SCL = A4, SDA = A5
}

void loop()
{
    // object detection
    digitalWrite(trig1, LOW);
    delayMicroseconds(2);
    digitalWrite(trig1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig1, LOW);

    duration1 = pulseIn(echo1, HIGH);
    // speed of sound in cm/microsecond = 0.0343
    distance1 = (duration1 * 0.0343) / 2;
    Serial.print("Distance1: ");
    Serial.println(distance1);
    delay(90);

    if (distance1 < threshold)
    {
        buzzer_delay = distance1 * 7;
        tone(buzzer, 500);
        delay(buzzer_delay);
        noTone(buzzer);
        delay(buzzer_delay);
    }

    // accelerometer
    mpu_read();
    ax = (AcX - 2050) / 16384.00;
    ay = (AcY - 77) / 16384.00;
    az = (AcZ - 1947) / 16384.00;
    gx = (GyX + 270) / 131.07;
    gy = (GyY - 351) / 131.07;
    gz = (GyZ + 136) / 131.07;
    // calculating Amplitute vactor for 3 axis
    float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
    int Amp = Raw_Amp * 10;
    Serial.println(Amp);
    // buzzer turns on if there is a fall
    if (fall == 1)
    {
        // Serial.print("curr_amp_set: "); Serial.println(curr_amp_set);
        if (curr_amp_set == false)
        {

            Serial.println("FALL DETECTED");
            current_amp = Amp;
            // Serial.print("current_amp: "); Serial.println(current_amp);
            tone(buzzer, 400);
            curr_amp_set = true;
            delay(25);
        }
        compare_amps(Amp, current_amp, curr_amp_set);
    }

    if (Amp <= 2 && trigger2 == false)
    { // if AM breaks lower threshold (0.4g)
        trigger1 = true;
        Serial.println("TRIGGER 1 ACTIVATED");
    }
    if (trigger1 == true)
    {
        trigger1count++;
        if (Amp >= 12)
        { // if AM breaks upper threshold (3g)
            trigger2 = true;
            Serial.println("TRIGGER 2 ACTIVATED");
            trigger1 = false;
            trigger1count = 0;
        }
    }
    if (trigger2 == true)
    {
        trigger2count++;
        angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
        Serial.println(angleChange);
        if (angleChange >= 30 && angleChange <= 400)
        { // if orientation changes by between 80-100 degrees
            trigger3 = true;
            trigger2 = false;
            trigger2count = 0;
            Serial.println(angleChange);
            Serial.println("TRIGGER 3 ACTIVATED");
        }
    }
    if (trigger3 == true)
    {
        trigger3count++;
        if (trigger3count >= 10)
        {
            angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
            Serial.println(angleChange);
            if ((angleChange >= 0) && (angleChange <= 10))
            { // if orientation changes remains between 0-10 degrees
                fall = true;
                trigger3 = false;
                trigger3count = 0;
                Serial.println(angleChange);
                Serial.print("FALL = ");
                Serial.println(fall);
            }
        }
    }

    if (trigger2count >= 6)
    { // allow 0.5s for orientation change
        trigger2 = false;
        trigger2count = 0;
        Serial.println("TRIGGER 2 DECACTIVATED");
    }
    if (trigger1count >= 6)
    { // allow 0.5s for AM to break upper threshold
        trigger1 = false;
        trigger1count = 0;
        Serial.println("TRIGGER 1 DECACTIVATED");
    }

    delay(50);
}

void mpu_read()
{
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void compare_amps(int amp_var, int current_amp, bool curr_amp_set_var)
{
    if (curr_amp_set_var)
    {
        if (amp_var != current_amp)
        {
            Serial.print("amp_var: ");
            Serial.println(amp_var);
            Serial.print("current amp in compare amp: ");
            Serial.println(current_amp);
            noTone(buzzer);
            Serial.println("PICKED UP");
            curr_amp_set = false;
            fall = false;
            current_amp = 0;
        }
    }
}