#include <ESP8266WiFi.h> 
#include <Wire.h>
#include <time.h>
#include <LiquidCrystal_I2C.h>
 LiquidCrystal_I2C lcd(0x27, 16, 2); 
 
const char* ssid = ""; //wifi name
const char* password = ""; //wifi p.w

//시간 변수들 설정
int timezone = 9; //미국시간 3, 한국시간 9
char timedate[80]; //lcd 출력 문자열

int hour = 0; //시간 받아올 변수
int minute = 0; // 분 받아올 변수

//시간 함수용
unsigned long previousMillis = 0;
const long interval = 1000; 

// API 설정
const char* stationName = "%EB%B4%89%EB%8B%B4%EC%9D%8D"; // 봉담읍 url 인코딩돌린거
const char* serviceKey = ""; //api 발급신청받은키

//에어코리아 서버
const char* host = "apis.data.go.kr"; //에어코리아
const int port = 80; //포트

//api 호출용 계산 (호스트 옵션)
String url = "/B552584/ArpltnInforInqireSvc/getMsrstnAcctoRltmMesureDnsty?stationName=" + String(stationName) \ 
+ "&dataTerm=DAILY" \
+ "&pageNo=1" \
+ "&numOfRows=1" \
+ "&returnType=xml" \
+ "&serviceKey=" + String(serviceKey)\
+ "&ver=1.3";

//미세먼지 농도 문자열
String Pm25 = "";
String Pm10 = "";
String Dtime = "";
String Dhour = "";

//미세먼지 농도값
int pm10data = 0; //초미세먼지
int pm25data = 0; //미세먼지
int Dhourdata = 0; //측정데이터의 day

WiFiClient client;

void setup() { 
   Serial.begin(115200); 
   Serial.setDebugOutput(true); 

   connectWiFi(); //와이파이 연결함수
   settime(); //시간 초기 설정 함수

   lcd.init(); //lcd 초기화
   pinMode(D5, OUTPUT); // 디지털 핀 D4의 값을 UNO에게 보낼겁니다.
 }

 void loop() {
  gettime();

  if(hour-Dhourdata>1 || Dhourdata == 0){ //현재시간이 Ddaydata와 1시간 이상 차이나면 or Ddaydata가 0일때
    getData();
      
    //getdata 출력부문
    Serial.println("미세먼지 : " + Pm25);
    Serial.println("초미세먼지 : " + Pm10);
    Serial.println("측정시간 : " + Dtime);
    Serial.print("몇시의 데이터인가요? : ");
    Serial.println(Dhourdata);

    //lcd에 미세먼지정보 업로드
    lcd.setCursor(0, 0);              // 0번째, 0라인
    lcd.print("Pm25:"+Pm25);              // lcd 출력
    lcd.setCursor(0, 1);              // 0번째, 0라인
    lcd.print("Pm10:"+Pm10);              // lcd 출력

    if(pm10data>80 || pm25data>35){ //미세먼지나 초미세먼지가 나쁨이상일경우
      lcd.setCursor(9, 1);              // 0번째, 0라인
      lcd.print("Mask!!");              // lcd 출력
      digitalWrite(D5, HIGH);             // D4에 TRUE 출력 
    }
    else if(pm10data==0 && pm25data>35==0){
      lcd.setCursor(9, 1);              // 0번째, 0라인
      lcd.print("*LOAD*");              // lcd 출력
      digitalWrite(D5, LOW);             // D4에 False 출력 
    }
    else{
      lcd.setCursor(9, 1);              // 0번째, 0라인
      lcd.print("Fresh!");              // lcd 출력
      digitalWrite(D5, LOW);             // D4에 False 출력 
    }
    delay(1000);
    gettime();
  }
 }

 void settime(){
   configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov"); //시간 서버에서 받아오는듯
   Serial.println("\nWaiting for time"); 

   while (!time(nullptr)) { 
     Serial.print("."); 
     delay(1000); 
   } //받아오는중일때 ...표시
   Serial.println(""); 

 }

 void gettime(){
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    time_t now = time(nullptr); //현재시간 받아오기
    struct tm* timeinfo = localtime(&now); //구조체에 현재시간 담기

    strftime(timedate, 80, "%H:%M", timeinfo); //문자열 양식대로 다듬기 년-월-일 시:분
    //Serial.println(timedate); //양식대로 시리얼 모니터 출력

    lcd.backlight();                  // 백라이트 켜기
    lcd.setCursor(10, 0);              // 0번째, 0라인
    lcd.print(timedate);              // lcd 출력

    hour = timeinfo->tm_hour; // 시 분 받아오기
    minute = timeinfo->tm_min;
    //Serial.print("초 :"); //초 출력
    //Serial.println(timeinfo->tm_sec); 
    //Serial.println(minute); //분 출력
  }
 }

 void connectWiFi() { //인터넷 연결
  Serial.println();
  Serial.println();
  Serial.println("Connecting to ");
  Serial.print(ssid);
  WiFi.hostname("Name");
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void getData(){ //xml에서 값 구해오기
if(hour-Dhourdata>1 || Dhourdata == 0){ //현재시간이 Ddaydata와 1시간 이상 차이나면 or Ddaydata가 0일때
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  } // 연결실패
  if(client.connect(host, port)){
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" + 
    "Connection: close\r\n\r\n");
    //Serial.println("request sent");
    delay(15000);
  } // api 호출
    
  String data = "";
  while(client.available()){
    String line = client.readStringUntil('\r');
    data += line;
  } // 호출한 api를 전부 data 변수에 입력
  //값구하기 findBetween은 아래 함수 참고면
  Pm10 = findBetween("<pm10Value>", "</pm10Value>", data);
  Pm25 = findBetween("<pm25Value>", "</pm25Value>", data);
  Dtime = findBetween("<dataTime>", "</dataTime>", data);
  Dhour = Dtime.substring(11,14); //Dtime 문자열 짤라서 구하기

  //값 다듬기
  Pm10.trim();
  Pm25.trim();
  Dtime.trim();
  Dhour.trim();

  //문자열 > Int 변환
  pm10data = Pm10.toInt();
  pm25data = Pm25.toInt();
  Dhourdata = Dhour.toInt();
  }
}
String findBetween(String a, String b, String data) { //두 텍스트 사이에 있는 문자열 추출
  return data.substring(data.indexOf(a) + a.length(), data.indexOf(b));
}
