//SLAVE CODE


#define ENA 9
#define IN1 8
#define IN2 7
#define PUMP_PIN 10

#define TRIG 2
#define ECHO 3

#define MIN_DISTANCE 5
#define MAX_DISTANCE 45

long duration;
int distance;

bool cleaningNow = false;
bool movingForward = true;
bool pumpPhase = false;
unsigned long pumpStartTime = 0;

void setup()
{
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  stopMotor();
  stopPump();
}

void loop()
{
  readSerialCommand();

  if (cleaningNow)
  {
    // ----- Pump Phase (5 seconds) -----
    if (pumpPhase)
    {
      if (millis() - pumpStartTime >= 5000)
      {
        stopPump();
        pumpPhase = false;
      }
      return;
    }

    distance = readDistance();

    if (distance == -1)
    {
      stopMotor();
      return;
    }

    Serial.print("DIST:");
    Serial.println(distance);

    if (movingForward)
    {
      moveForward();

      if (distance >= MAX_DISTANCE)
      {
        stopMotor();
        delay(300);
        movingForward = false;
      }
    }
    else
    {
      moveBackward();

      if (distance <= MIN_DISTANCE)
      {
        stopMotor();
        cleaningNow = false;
        Serial.println("DONE");
      }
    }

    delay(150);
  }
}

void readSerialCommand()
{
  if (Serial.available())
  {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "START")
    {
      cleaningNow = true;
      movingForward = true;

      pumpPhase = true;
      pumpStartTime = millis();
      startPump();
    }
    else if (cmd == "STOP")
    {
      cleaningNow = false;
      stopMotor();
      stopPump();
    }
  }
}

int readDistance()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  duration = pulseIn(ECHO, HIGH, 30000);

  if (duration <= 0)
    return -1;

  int dist = duration * 0.034 / 2;

  if (dist < 2 || dist > 400)
    return -1;

  return dist;
}

void moveForward()
{
  analogWrite(ENA, 200);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void moveBackward()
{
  analogWrite(ENA, 200);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void stopMotor()
{
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// ---- ACTIVE LOW RELAY ----
void startPump()
{
  digitalWrite(PUMP_PIN, HIGH);
}

void stopPump()
{
  digitalWrite(PUMP_PIN, LOW);
}
