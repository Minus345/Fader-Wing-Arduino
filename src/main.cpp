#include <Arduino.h>
#include <Servo.h>
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8
#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR3_A 5
#define MOTOR3_B 7
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR1_PWM 11
#define MOTOR2_PWM 3
#define MOTOR3_PWM 6
#define MOTOR4_PWM 5
#define SERVO1_PWM 10
#define SERVO2_PWM 9
#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4
Servo servo_1;
Servo servo_2;

int b1;
int v1 = 100;
int b2;
int v2 = 100;

int pos[4];
int posAlt[4];

int layer = 1;

int layer1[4] = {500, 500, 500, 500};
int layer2[4] = {500, 500, 500, 500};
int layer3[4] = {500, 500, 500, 500};
int layer4[4] = {500, 500, 500, 500};
int layer5[4] = {500, 500, 500, 500};

int layer1old[4] = {500, 500, 500, 500};
int layer2old[4] = {500, 500, 500, 500};
int layer3old[4] = {500, 500, 500, 500};
int layer4old[4] = {500, 500, 500, 500};
int layer5old[4] = {500, 500, 500, 500};

int speed = 200;

void setup()
{
  Serial.begin(9600);
  Serial.println("Simple Adafruit Motor Shield sketch");
  servo_1.attach(SERVO1_PWM);
  servo_2.attach(SERVO2_PWM);
  pinMode(A8, INPUT);
  pinMode(A9, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);
  // Buttons
  pinMode(53, INPUT);
  digitalWrite(53, HIGH);
  pinMode(51, INPUT);
  digitalWrite(51, HIGH);
  pinMode(49, INPUT);
  digitalWrite(49, HIGH);
  pinMode(47, INPUT);
  digitalWrite(47, HIGH);
  pinMode(45, INPUT);
  digitalWrite(45, HIGH);
}

void readMotor()
{
  // pos[0] = analogRead(A8);
  pos[0] = map(analogRead(A8), 9, 979, 0, 1023);
  // pos[1] = analogRead(A9);
  pos[1] = map(analogRead(A9), 5, 995, 0, 1023);
  // pos[2] = analogRead(A10);
  pos[2] = map(analogRead(A10), 21, 1011, 0, 1023);
  // pos[3] = analogRead(A11);
  pos[3] = map(analogRead(A11), 22, 996, 0, 1023);

  for (int i = 0; i < 4; i++)
  {
    if (pos[i] > 1024)
    {
      pos[i] = 1024;
    }
    if (pos[i] < 0)
    {
      pos[i] = 0;
    }
  }
}

void shiftWrite(int output, int high_low)
{
  static int latch_copy;
  static int shift_register_initialized = false;
  // Do the initialization on the fly,
  // at the first time it is used.
  if (!shift_register_initialized)
  {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);
    // Set pins for shift register to default value (low);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    // Enable the shift register, set Enable pin Low.
    digitalWrite(MOTORENABLE, LOW);
    // start with all outputs (of the shift register) low
    latch_copy = 0;
    shift_register_initialized = true;
  }
  // The defines HIGH and LOW are 1 and 0.
  // So this is valid.
  bitWrite(latch_copy, output, high_low);
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5); // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5); // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}
void motor_output(int output, int high_low, int speed)
{
  int motorPWM;
  switch (output)
  {
  case MOTOR1_A:
  case MOTOR1_B:
    motorPWM = MOTOR1_PWM;
    break;
  case MOTOR2_A:
  case MOTOR2_B:
    motorPWM = MOTOR2_PWM;
    break;
  case MOTOR3_A:
  case MOTOR3_B:
    motorPWM = MOTOR3_PWM;
    break;
  case MOTOR4_A:
  case MOTOR4_B:
    motorPWM = MOTOR4_PWM;
    break;
  default:
    speed = -3333;
    break;
  }
  if (speed != -3333)
  {
    shiftWrite(output, high_low); // set PWM only if it is valid
    if (speed >= 0 && speed <= 255)
    {
      analogWrite(motorPWM, speed);
    }
  }
}
void motor(int nMotor, int command, int speed)
{
  int motorA, motorB;
  if (nMotor >= 1 && nMotor <= 4)
  {
    switch (nMotor)
    {
    case 1:
      motorA = MOTOR1_A;
      motorB = MOTOR1_B;
      break;
    case 2:
      motorA = MOTOR2_A;
      motorB = MOTOR2_B;
      break;
    case 3:
      motorA = MOTOR3_A;
      motorB = MOTOR3_B;
      break;
    case 4:
      motorA = MOTOR4_A;
      motorB = MOTOR4_B;
      break;
    default:
      break;
    }
    switch (command)
    {
    case FORWARD:
      motor_output(motorA, HIGH, speed);
      motor_output(motorB, LOW, -1); // -1: no PWM set
      break;
    case BACKWARD:
      motor_output(motorA, LOW, speed);
      motor_output(motorB, HIGH, -1); // -1: no PWM set
      break;
    case BRAKE:
      motor_output(motorA, LOW, 255); // 255: fully on.
      motor_output(motorB, LOW, -1);  // -1: no PWM set
      break;
    case RELEASE:
      motor_output(motorA, LOW, 0);  // 0: output floating.
      motor_output(motorB, LOW, -1); // -1: no PWM set
      break;
    default:
      break;
    }
  }
}
void driveToPlace(int location, int motorNumber, int round)
{

  int speedDTP = 0;

  switch (round)
  {
  case 0:
    speedDTP = speed;
    break;
  case 1:
    speedDTP = speed / 2;
    break;
  case 2:
    speedDTP = speed / 3;
    break;
  case 3:
    speedDTP = speed / 4;
    break;
  case 4:
    speedDTP = speed / 5;
    break;
  }

  boolean drive;
  drive = true;
  int posNumber = motorNumber - 1;

  motor(motorNumber, RELEASE, speedDTP);

  if (pos[posNumber] > location)
  {

    while (drive)
    {

      motor(motorNumber, FORWARD, speedDTP);
      readMotor();
      // Serial.print(pos[posNumber]);
      // Serial.print(" V M:");
      // Serial.print(motorNumber);
      // Serial.print(" L: ");
      // Serial.println(location);
      if (pos[posNumber] <= location)
      {
        drive = false;
        // Serial.println("Stop");
      }
    }
    motor(motorNumber, RELEASE, speedDTP);
  }

  if (pos[posNumber] < location)
  {
    while (drive)
    {
      motor(motorNumber, BACKWARD, speedDTP);
      readMotor();
      // Serial.print(pos[posNumber]);
      // Serial.print(" ^ M: ");
      // Serial.print(motorNumber);
      // Serial.print(" L: ");
      // Serial.println(location);
      if (pos[posNumber] >= location)
      {
        drive = false;
        // Serial.println("Stop");
      }
    }
    motor(motorNumber, RELEASE, speedDTP);
  }
}

boolean array_cmp(int *a, int *b, int len_a, int len_b)
{
  int n;

  // if their lengths are different, return false
  if (len_a != len_b)
    return false;

  // test each element to be the same. if not, return false
  for (n = 0; n < len_a; n++)
  {
    if (a[n] != b[n])
      return false;
  }
  // ok, if we have not returned yet, they are equal :)
  return true;
}

void loop()
{
  boolean button1 = digitalRead(45);
  boolean button2 = digitalRead(47);
  boolean button3 = digitalRead(49);
  boolean button4 = digitalRead(51);
  boolean button5 = digitalRead(53);

  int rounds = 4;

  if (button1 == 0)
  {
    layer = 1;
    for (int i = 0; i < rounds; i++)
    {
      driveToPlace(layer1[0], 1, i);
      driveToPlace(layer1[1], 2, i);
      driveToPlace(layer1[2], 3, i);
      driveToPlace(layer1[3], 4, i);
      delay(10);
    }
    for (int i = 1; i < 4; i++)
    {
      motor(i, RELEASE, speed);
    }
  }
  if (button2 == 0)
  {
    layer = 2;
    for (int i = 0; i < rounds; i++)
    {
      driveToPlace(layer2[0], 1, i);
      driveToPlace(layer2[1], 2, i);
      driveToPlace(layer2[2], 3, i);
      driveToPlace(layer2[3], 4, i);
      delay(10);
    }
    for (int i = 1; i < 4; i++)
    {
      motor(i, RELEASE, speed);
    }
  }
  if (button3 == 0)
  {
    layer = 3;
    for (int i = 0; i < rounds; i++)
    {
      driveToPlace(layer3[0], 1, i);
      driveToPlace(layer3[1], 2, i);
      driveToPlace(layer3[2], 3, i);
      driveToPlace(layer3[3], 4, i);
      delay(10);
    }
    for (int i = 1; i < 4; i++)
    {
      motor(i, RELEASE, speed);
    }
  }
  if (button4 == 0)
  {
    layer = 4;
    for (int i = 0; i < rounds; i++)
    {
      driveToPlace(layer4[0], 1, i);
      driveToPlace(layer4[1], 2, i);
      driveToPlace(layer4[2], 3, i);
      driveToPlace(layer4[3], 4, i);
      delay(10);
    }
    for (int i = 1; i < 4; i++)
    {
      motor(i, RELEASE, speed);
    }
  }
  if (button5 == 0)
  {
    layer = 5;
    for (int i = 0; i < rounds; i++)
    {
      driveToPlace(layer5[0], 1, i);
      driveToPlace(layer5[1], 2, i);
      driveToPlace(layer5[2], 3, i);
      driveToPlace(layer5[3], 4, i);
      delay(10);
    }
    for (int i = 1; i < 4; i++)
    {
      motor(i, RELEASE, speed);
    }
  }

  readMotor();

  switch (layer)
  {
  case 1:
    for (int i = 0; i < 4; i++)
    {
      layer1[i] = pos[i];
    }
    break;

  case 2:
    for (int i = 0; i < 4; i++)
    {
      layer2[i] = pos[i];
    }
    break;

  case 3:
    for (int i = 0; i < 4; i++)
    {
      layer3[i] = pos[i];
    }
    break;

  case 4:
    for (int i = 0; i < 4; i++)
    {
      layer4[i] = pos[i];
    }
    break;

  case 5:
    for (int i = 0; i < 4; i++)
    {
      layer5[i] = pos[i];
    }
    break;

  default:
    break;
  }

  if (array_cmp(posAlt, pos, 4, 4) == false)
  {
    Serial.print(map(layer1[0], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer1[1], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer1[2], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer1[3], 0, 1023, 0, 255));
    Serial.print("|");

    Serial.print(map(layer2[0], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer2[1], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer2[2], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer2[3], 0, 1023, 0, 255));
    Serial.print("|");

    Serial.print(map(layer3[0], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer3[1], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer3[2], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer3[3], 0, 1023, 0, 255));
    Serial.print("|");

    Serial.print(map(layer4[0], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer4[1], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer4[2], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer4[3], 0, 1023, 0, 255));
    Serial.print("|");

    Serial.print(map(layer5[0], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer5[1], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer5[2], 0, 1023, 0, 255));
    Serial.print("|");
    Serial.print(map(layer5[3], 0, 1023, 0, 255));
    Serial.print("|");

    Serial.println();

    for (int i = 0; i < 4; i++)
    {
      posAlt[i] = pos[i];
    }
  }
}