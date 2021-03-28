const int picMic = A0;

unsigned long prevMicros = 0;
unsigned long interval = 25;

void setup(void)
{
  Serial.begin(1000000);
}
  
void loop(void)
{
  unsigned long currentMicros = micros();

  if(currentMicros - prevMicros >= interval)
  {
    prevMicros = currentMicros;

    Serial.println(analogRead(A0));
  }
}
