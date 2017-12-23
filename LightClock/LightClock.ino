#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t

#define PIN_LIGHT 3
#define PIN_STOP 4

struct Alarm {
  int32_t start, end;
  uint8_t wdays;
};

const int countAlarms=4;
struct Alarm alarms[countAlarms];

int light=0;
time_t disableTo=0;

void setup() {
  for(unsigned int n=0; n<countAlarms; n++) alarms[n].wdays=0;
  
  pinMode(PIN_LIGHT, OUTPUT);
  pinMode(PIN_STOP, INPUT);
  digitalWrite(PIN_STOP, HIGH);
  
  Serial.begin(9600);
  delay(200);

  tmElements_t tm;
  RTC.read(tm);
  if(RTC.chipPresent()) {
    Serial.println("RTC present");
  }else {
    Serial.println("RTC not present");
  }
}

void loop() {
  setLight();
  
  char bufSerial[64];
  int readed=Serial.readBytesUntil(10, bufSerial, sizeof(bufSerial));
  if(readed>0) {
    bufSerial[readed]=0;
    Serial.println(bufSerial);
    executeCommand(bufSerial);
  }
  
  showTime();
  delay(1000);
}

void setLight() {
  light=0;
  
  tmElements_t tm;
  if(!RTC.read(tm)) {
    analogWrite(PIN_LIGHT, 1);
    Serial.println("Clock not set");
    return;
  }
  
  time_t now=RTC.get();
  if(now<disableTo) {
    Serial.println("Disabled");
    analogWrite(PIN_LIGHT, light);
    return;
  }

  int wday=tm.Wday;
  int32_t time=(int32_t)tm.Hour*3600+(int32_t)tm.Minute*60+(int32_t)tm.Second;
  Serial.println(wday, DEC);
  Serial.println(time, DEC);
  wday--;
  for(unsigned int n=0; n<countAlarms; n++) {
    struct Alarm* alarm=&alarms[n];
    if(((alarm->wdays>>wday)&1)==0) continue;
    Serial.print("Alarm ");
    Serial.print(n, DEC);
    Serial.println(" is active at current week day");
    
    if(time<alarm->start) {
      Serial.println("Too early");
      continue;
    }
    if(time>alarm->end) {
      Serial.println("Too late");
      continue;
    }
    if(digitalRead(PIN_STOP)==LOW) {
      disableTo=now+alarm->end-time;
      if(disableTo<0) disableTo=0;
      disableTo+=10;
      Serial.println("Canceled");
      break;
    }
    
    Serial.println("Alarm");
    int phase=(time-alarm->start)*255/(alarm->end-alarm->start);
    if(phase>light) light=phase;
  }
  
  analogWrite(PIN_LIGHT, light);
}

void executeCommand(char* cmd) {
  if(strncmp(cmd, "Time: ", 6)==0) {
    uint32_t timeNew;
    if(sscanf(&cmd[6], "%ld", &timeNew)==1) {
      RTC.set(timeNew);
      blink(1);
    }
  }else if(strncmp(cmd, "Light: ", 7)==0) {
    int indexAlarm;
    long int timeStart, timeEnd;
    int dayOfWeek;
    if(sscanf(&cmd[6], "%d %ld %ld %d", &indexAlarm, &timeStart, &timeEnd, &dayOfWeek)==4) {
      setAlarm(indexAlarm, timeStart, timeEnd, dayOfWeek);
      blink(2);
    }
  }else {
    Serial.print("Unknown command: ");
    Serial.println(cmd);
  }
}

void setAlarm(int indexAlarm, uint32_t timeStart, uint32_t timeEnd, uint8_t wdays) {
  if(indexAlarm<0 || indexAlarm>=countAlarms) return;
  alarms[indexAlarm].start=timeStart;
  alarms[indexAlarm].end=timeEnd;
  alarms[indexAlarm].wdays=wdays;
}

void blink(int count) {
  analogWrite(PIN_LIGHT, 0);
  delay(2000);
  for(int n=0; n<count; n++) {
    analogWrite(PIN_LIGHT, 255);
    delay(500);
    analogWrite(PIN_LIGHT, 0);
    delay(500);
  }
  delay(2000);
  analogWrite(PIN_LIGHT, light);
}

void showTime() {
  time_t t=RTC.get();
  if(!t) return;
  Serial.print(t, DEC);

  tmElements_t tm;
  if(!RTC.read(tm)) {
    Serial.println("");
    return;
  }
  Serial.print(": ");
  Serial.print(tm.Year+1970, DEC);
  Serial.print(".");
  Serial.print(tm.Month, DEC);
  Serial.print(".");
  Serial.print(tm.Day, DEC);
  Serial.print(" ");
  Serial.print(tm.Hour, DEC);
  Serial.print(":");
  Serial.print(tm.Minute, DEC);
  Serial.print(":");
  Serial.print(tm.Second, DEC);
  Serial.print(",");
  Serial.println(tm.Wday, DEC);
}

