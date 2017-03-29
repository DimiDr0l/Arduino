volatile float time = 0;
volatile float time_last = 0;
volatile int rpm_array[5] = {0, 0, 0, 0, 0};
int rpm = 0;
int speed;
#define pinStart 9 //ШИМ

void setup()
{
  //Digital Pin 2 Set As An Interrupt
  pinMode(2, INPUT_PULLUP);
  pinMode (pinStart, OUTPUT);
  pinMode(13, OUTPUT);
  attachInterrupt(0, fan_interrupt, FALLING);
  Serial.begin(115200);
}

void loop(){
  delay(400);
  Serial.print ("RPM = ");
  Serial.println (rpm);
  
    char data[3]="";
    if(Serial.available()){
    byte i = 0;
    while(Serial.available()){
      data[i] = Serial.read(); //получаем данные из сериал.
      i++;
      delay(2);
    }
    data[i] = 0;
    speed=atol(data);
    Serial.println(speed);
    if (speed>=0 && speed<=255) analogWrite (pinStart, speed);
  }
}

void fan_interrupt()
{
  rpm = (1000000.0 / (micros() - time_last)) * 60;
  time_last = micros();
}
