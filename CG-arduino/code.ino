#include <ESP8266.h>
#include <ESP8266Client.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int T1H = 0;
int RN1 = 1;
int REH = 2;
int WSD = 3;

#define SSID "wifi_id"
#define PASS "wifi_pw"

#define WEATHER_URL "" // EC2 서버 주소 - 80번으로 tcp보내니까 리다이렉트 설정해두어야함

SoftwareSerial esp8266Serial(52, 53);
ESP8266 wifi(esp8266Serial);

String bit_str = "";

struct weather
{
  float T1H;
  float RN1;
  float REH;
  float WSD;
};

struct col_weather { //T1H,RN1,REH,WSD
  float AVG;
  float MAX;
  float MIN;
};


struct col_weather col_weather_data[4] = {0};
struct weather weatherData[30];

String R = "011";
String Y = "001";
String G = "101";

int pin_map[12][3]; //dataPin(DS Pin) latchPin(ST_CP Pin) clockPin(SH_CP Pin)

int tmp_val[12];

String col_select(int type, int PIN) {
  float M_A = col_weather_data[type].MAX - col_weather_data[type].AVG; //최대 - 평균
  float A_m = col_weather_data[type].AVG - col_weather_data[type].MIN; //평균 - 최대
  switch (type) {
    case 0:
      if (weatherData[PIN].T1H > col_weather_data[type].AVG + M_A / 3) {
        return R;
      }
      else if (weatherData[PIN].T1H < col_weather_data[type].AVG - A_m / 3) {
        return Y;
      }
      else {
        return G;
      }
      break;
    case 1:
      if (weatherData[PIN].RN1 > col_weather_data[type].AVG + M_A / 3) {
        return R;
      }
      else if (weatherData[PIN].RN1 < col_weather_data[type].AVG - A_m / 3) {
        return Y;
      }
      else {
        return G;
      }
      break;
    case 2:
      if (weatherData[PIN].REH > col_weather_data[type].AVG + M_A / 3) {
        return R;
      }
      else if (weatherData[PIN].REH < col_weather_data[type].AVG - A_m / 3) {
        return Y;
      }
      else {
        return G;
      }
      break;
    default :
      if (weatherData[PIN].WSD > col_weather_data[type].AVG + M_A / 3) {
        return R;
      }
      else if (weatherData[PIN].WSD < col_weather_data[type].AVG - A_m / 3) {
        return Y;
      }
      else {
        return G;
      }
  }
}

int type = -1; //초기값

void setup() {
  digitalWrite(8,HIGH);
  
  lcd.begin();
  lcd.clear();

  esp8266Serial.begin(9600);
  wifi.begin();
  
  lcd.setCursor(0, 0);
  lcd.print("Wifi Start: ");

  lcd.setCursor(0, 1);
  lcd.print(getStatus(wifi.restart()));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wifi_joinAP: ");

  lcd.setCursor(0, 1);
  lcd.print(getStatus(wifi.joinAP(SSID, PASS)));
  
  delay(5000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wifi_connect: ");

  lcd.setCursor(0, 1);
  lcd.print(getStatus(wifi.connect(ESP8266_PROTOCOL_TCP, WEATHER_URL, 80)));
  
  delay(5000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wifi send: ");

  String url = "GET / HTTP/1.1\r\nHost: ";
  url.concat(WEATHER_URL);
  url.concat("\r\nConnection: close\r\n\r\n");

  lcd.setCursor(0, 1);
  lcd.print(getStatus(wifi.send(url)));
  
  delay(100);

  while (!wifi.available()) {
    delay(100);
  }

  String tmp ="";
  
  while (1) {
    tmp = wifi.readStringUntil('\n');
    if (tmp.length() >=80){
      break;
    }
  }

  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("Pin Setting ");
  lcd.setCursor(0, 1);
  lcd.print("In Progress");
  pinMode(8,OUTPUT);
  for (int i = 2, j = 0; i < 8; i++, j++) {
    pin_map[j / 3][j % 3] = i;
    pinMode(i, OUTPUT);
  }

  for (int i = 22, j = 6; i < 52; i++, j++) {
    pin_map[j / 3][j % 3] = i;
    pinMode(i, OUTPUT);
  }

  for (int i = 0; i < 5; i++) {
    pinMode(i + 9, INPUT);
  }

  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Done");
  delay(100);

  DynamicJsonDocument doc(3000);

  DeserializationError error = deserializeJson(doc, tmp);

  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("parsing: ");
  lcd.setCursor(0, 1);
  lcd.print("In Progress");
  // Fill weatherData array

  JsonArray weatherArray = doc["weather_data"];
  for (int i = 0; i < 30; i++)
  {
    JsonArray row = weatherArray[i];
    weatherData[i].REH = row[0];
    weatherData[i].RN1 = row[1];
    weatherData[i].T1H = row[2];
    weatherData[i].WSD = row[3];
  }

  delay(2000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Done");
  delay(100);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Data Calculationg: ");
  lcd.setCursor(0, 1);
  lcd.print("In Progress");
  //수치 계산
  col_weather_data[0].MAX = weatherData[0].T1H;
  col_weather_data[0].MIN = weatherData[0].T1H;
  col_weather_data[1].MAX = weatherData[0].RN1;
  col_weather_data[1].MIN = weatherData[0].RN1;
  col_weather_data[2].MAX = weatherData[0].REH;
  col_weather_data[2].MIN = weatherData[0].REH;
  col_weather_data[3].MAX = weatherData[0].WSD;
  col_weather_data[3].MIN = weatherData[0].WSD;

  for (int i = 0; i < 30; i++) {
    col_weather_data[0].AVG += weatherData[i].T1H;
    col_weather_data[1].AVG += weatherData[i].RN1;
    col_weather_data[2].AVG += weatherData[i].REH;
    col_weather_data[3].AVG += weatherData[i].WSD;

    if (col_weather_data[0].MAX < weatherData[i].T1H) {
      col_weather_data[0].MAX = weatherData[i].T1H;
    }

    if (col_weather_data[0].MIN > weatherData[i].T1H) {
      col_weather_data[0].MIN = weatherData[i].T1H;
    }

    if (col_weather_data[1].MAX < weatherData[i].RN1) {
      col_weather_data[1].MAX = weatherData[i].RN1;
    }

    if (col_weather_data[1].MIN > weatherData[i].RN1) {
      col_weather_data[1].MIN = weatherData[i].RN1;
    }

    if (col_weather_data[2].MAX < weatherData[i].REH) {
      col_weather_data[2].MAX = weatherData[i].REH;
    }

    if (col_weather_data[2].MIN > weatherData[i].REH) {
      col_weather_data[2].MIN = weatherData[i].REH;
    }

    if (col_weather_data[3].MAX < weatherData[i].WSD) {
      col_weather_data[3].MAX = weatherData[i].WSD;
    }

    if (col_weather_data[3].MIN > weatherData[i].WSD) {
      col_weather_data[3].MIN = weatherData[i].WSD;
    }
  }
  col_weather_data[0].AVG /= 30;
  col_weather_data[1].AVG /= 30;
  col_weather_data[2].AVG /= 30;
  col_weather_data[3].AVG /= 30;

  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Done");


  for (int i = 0; i < 30; i++) {
    bit_str += "010";
  }

  bit_str += "000000";

  for (int i = 0; i < 12; i++) {
    String binaryChunk = bit_str.substring(i * 8, (i + 1) * 8);
    int intValue = strtol(binaryChunk.c_str(), NULL, 2);
    
    digitalWrite(pin_map[i][1], LOW);
    shiftOut(pin_map[i][0], pin_map[i][2], LSBFIRST, intValue);
    digitalWrite(pin_map[i][1], HIGH);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Setting Done");
  lcd.setCursor(0, 1);
  lcd.print("plz press Button");
}

void loop() {

  if (digitalRead(9) == LOW) {
    if ( type != 0 ) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Function");
      lcd.setCursor(0, 1);
      lcd.print("Temperture");
      type = 0;
      bit_str = "";

      for (int i = 0; i < 30; i++) {
        bit_str += col_select(type, i);
      }

      bit_str += "111111";

      for (int i = 0; i < 12; i++) {
        String binaryChunk = bit_str.substring(i * 8, (i + 1) * 8);
        int intValue = strtol(binaryChunk.c_str(), NULL, 2);

        digitalWrite(pin_map[i][1], LOW);
        shiftOut(pin_map[i][0], pin_map[i][2], LSBFIRST, intValue);
        digitalWrite(pin_map[i][1], HIGH);

      }
      delay(2000);
    }
  }
  if (digitalRead(10) == LOW) {
    if ( type != 1 ) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Function");
      lcd.setCursor(0, 1);
      lcd.print("Humidity");
      type = 1;
      bit_str = "";

      for (int i = 0; i < 30; i++) {
        bit_str += col_select(type, i);

      }

      bit_str += "111111";

      for (int i = 0; i < 12; i++) {
        String binaryChunk = bit_str.substring(i * 8, (i + 1) * 8);
        int intValue = strtol(binaryChunk.c_str(), NULL, 2);
        
        Serial.println(binaryChunk);
        digitalWrite(pin_map[i][1], LOW);
        shiftOut(pin_map[i][0], pin_map[i][2], LSBFIRST, intValue);
        digitalWrite(pin_map[i][1], HIGH);

      }
      delay(2000);
    }
  }
  if (digitalRead(11) == LOW) {
    if ( type != 2 ) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Function");
      lcd.setCursor(0, 1);
      lcd.print("Precipitation");
      type = 2;
      bit_str = "";

      for (int i = 0; i < 30; i++) {
        bit_str += col_select(type, i);
      }

      bit_str += "111111";

      for (int i = 0; i < 12; i++) {
        String binaryChunk = bit_str.substring(i * 8, (i + 1) * 8);
        int intValue = strtol(binaryChunk.c_str(), NULL, 2);

        digitalWrite(pin_map[i][1], LOW);
        shiftOut(pin_map[i][0], pin_map[i][2], LSBFIRST, intValue);
        digitalWrite(pin_map[i][1], HIGH);
      }
      delay(2000);
    }
  }
  if (digitalRead(12) == LOW) {
    if ( type != 3 ) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Function");
      lcd.setCursor(0, 1);
      lcd.print("Wind Speed");
      type = 3;
      bit_str = "";

      for (int i = 0; i < 30; i++) {
        bit_str += col_select(type, i);
      }

      bit_str += "111111";
      
      for (int i = 0; i < 12; i++) {
        String binaryChunk = bit_str.substring(i * 8, (i + 1) * 8);
        int intValue = strtol(binaryChunk.c_str(), NULL, 2);
        digitalWrite(pin_map[i][1], LOW);
        shiftOut(pin_map[i][0], pin_map[i][2], LSBFIRST, intValue);
        digitalWrite(pin_map[i][1], HIGH);
      }
      delay(2000);
    }
  }
  
  if (digitalRead(13) == LOW) {
    if ( type != 4 ) {
      type = 4;
      digitalWrite(8,LOW);
    }
  }


  delay(200);
}

String getStatus(bool status)
  {
  return status ? "OK" : "NO";
  }

  String getStatus(ESP8266CommandStatus status)
  {
  switch (status)
  {
  case ESP8266_COMMAND_INVALID:
    return "INVALID";
  case ESP8266_COMMAND_TIMEOUT:
    return "TIMEOUT";
  case ESP8266_COMMAND_OK:
    return "OK";
  case ESP8266_COMMAND_NO_CHANGE:
    return "NO CHANGE";
  case ESP8266_COMMAND_ERROR:
    return "ERROR";
  case ESP8266_COMMAND_NO_LINK:
    return "NO LINK";
  case ESP8266_COMMAND_TOO_LONG:
    return "TOO LONG";
  case ESP8266_COMMAND_FAIL:
    return "FAIL";
  default:
    return "UNKNOWN COMMAND STATUS";
  }
  }
