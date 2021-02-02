#include <Adafruit_GPS.h>
#define GPSSerial Serial2
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
#define soundPin 32
#define gpsEnable 21
#define ampPower 14
#include <Dusk2Dawn.h>
#include <Wire.h>
#include <RTClibExtended.h>
RTC_DS3231 RTC; 
int hoursAdd = 0;
int minutesAdd = 0;
int inivSunset = 0;
int timeOff = 0;
//double realLong = 0.0;
//double realLat = 0.0;
uint32_t timer = millis();
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR float realLong = 0.0;
RTC_DATA_ATTR float realLat = 0.0;
RTC_DATA_ATTR bool check = false;
float conv_coords(float in_coords)
 {
 //Initialize the location.
 float f = in_coords;
 // Get the first two digits by turning f into an integer, then doing an integer divide by 100;
 // firsttowdigits should be 77 at this point.
 int firsttwodigits = ((int)f)/100; //This assumes that f < 10000.
 float nexttwodigits = f - (float)(firsttwodigits*100);
 float theFinalAnswer = (float)(firsttwodigits + nexttwodigits/60.0);
 return theFinalAnswer;
 }
 void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case 1  : 
      Serial.println("Wakeup caused by external signal using RTC_IO");
      digitalWrite(ampPower, HIGH);
      delay(1000); 
      digitalWrite(soundPin, LOW);
      delay(20000);
      digitalWrite(soundPin, HIGH);
      digitalWrite(ampPower, LOW);
      break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Adafruit GPS library basic test!");
  Wire.begin();
  pinMode(gpsEnable, OUTPUT);
  if(!check)digitalWrite(gpsEnable, HIGH);
  else if(check)digitalWrite(gpsEnable,LOW);
  
  pinMode(ampPower, OUTPUT);
  digitalWrite(ampPower, LOW);
  pinMode(soundPin, OUTPUT);
  digitalWrite(soundPin, HIGH);
  RTC.begin();
  if(bootCount == 0 ){
    GPS.begin(9600);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS.sendCommand(PGCMD_ANTENNA);
    delay(1000);
    GPSSerial.println(PMTK_Q_RELEASE);
  }
  RTC.armAlarm(1, false);
  RTC.clearAlarm(1);
  RTC.alarmInterrupt(1, false);
  RTC.armAlarm(2, false);
  RTC.clearAlarm(2);
  RTC.alarmInterrupt(2, false);
  RTC.writeSqwPinMode(DS3231_OFF);
 
}  
 void loop() {
     if(!check)
  {
   char c = GPS.read();
     if (GPSECHO)
    if (c) Serial.print(c);
     if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    Serial.println(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }
  if(GPS.fix) {
   Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
  
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
   
    Serial.flush();
    delay(1000);
    realLong = (-1.0)* conv_coords(GPS.longitude);
    realLat = conv_coords(GPS.latitude);
   
    Serial.print(realLong);
    Serial.println("longitude corrected");
    Serial.print(realLat);
    Serial.println("latitutde corrected");
    
    Dusk2Dawn universal(realLat,realLong, -12);
    int univSunset = universal.sunset(GPS.year, GPS.month, GPS.day, false);
    timeOff = univSunset;
  Serial.println();
  
  Serial.print(timeOff);
  Serial.println("timeOff");
  
   hoursAdd = timeOff/60;
   minutesAdd = timeOff%60;
 RTC.adjust(DateTime(GPS.year,GPS.month,GPS.day-1,hoursAdd,minutesAdd ,GPS.seconds));
  Serial.println();
  Serial.print(hoursAdd);
  Serial.println("hoursAdd");
  Serial.print(minutesAdd);
  Serial.println("minutesAdd");
  check = true;
  }
  }
     if(check)
     {
  DateTime now = RTC.now();
    
    delay(1000);
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    //Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

  ++bootCount;
Serial.println("Boot number: " + String(bootCount));
Serial.flush();
delay(1000);
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
   soundOff();
    //Set alarm1 every day at 18:33
   //RTC.setAlarm(ALM1_MATCH_HOURS, 33, 18, 0);
   Dusk2Dawn universal(realLat,realLong, -12);
     int univSunset = universal.sunset(now.year(), now.month(), now.day(), false);
    timeOff = univSunset;
    
   hoursAdd =  timeOff/60;
   minutesAdd =  timeOff%60; 
   Serial.println();
  Serial.print(hoursAdd);
  Serial.println("hoursAdd");
  Serial.print(minutesAdd);
  Serial.println("minutesAdd");
  RTC.setAlarm(ALM1_MATCH_HOURS, minutesAdd,hoursAdd, 0);   //set your wake-up time here
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low
  RTC.alarmInterrupt(1, true);
  
  //Go to sleep now
  Serial.println("Going to sleep now");
  Serial.flush();
  delay(1000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
     }
 }
 void soundOff(){
  digitalWrite(14, HIGH);
      //delay(1000); 
      digitalWrite(32, LOW);
      delay(20000);
      //digitalWrite(32, HIGH);
     // digitalWrite(14, LOW);
 }



 
 
  
 
    
    
