#include <Stepper.h>

#define MOTOR_STEPS 2048 //한바퀴
#define HALF_TURN_STEPS 1024 //반바퀴
#define PIR_PIN 7 //PIR 센서
#define TRIG_PIN 9 //초음파
#define ECHO_PIN 10 //초음파
#define ULTRA_LED_PIN 11
#define SPEAKER_PIN 13 // 스피커 핀
#define WIFI_PIN 12 //wifi보드에서 받아오는 PIN

//핀들 받아오는 변수들
int pirvalue = 0;
int wifivalue = 0;

Stepper motor1(MOTOR_STEPS, 2, 3, 4, 5); // 모터 핀

long duration, distance; //초음파함수 변수

int notdisp = 0;

void setup() {
  Serial.begin(9600); 
  pinMode(ULTRA_LED_PIN, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT); // 스피커 출력 핀 설정
  pinMode(PIR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(WIFI_PIN, INPUT_PULLUP);
  motor1.setSpeed(10);  // 모터 속도를 낮춰 설정
}

void loop() {
  wifivalue = digitalRead(WIFI_PIN); // WIFI보드에서 가져오는 값 저장
  pirvalue = digitalRead(PIR_PIN); // pir밸류값 저장
  Serial.println(pirvalue);
  getdistance(); //초음파센서 거리값 받아오는 함수
  
  if (distance > 0 && distance > 18 && pirvalue==1) {  // 초음파센서 부분 50cm 이내면서 근처에 사람이 있을경우
    digitalWrite(ULTRA_LED_PIN, HIGH);
    Serial.println("Ultrasonic: LED ON");
    lemi(); //사운드 레미 출력
    nodisp = 1; //모터 + 사운드 작동X
  } else {
    digitalWrite(ULTRA_LED_PIN, LOW);
    nodisp = 0; //출력 정상화
  }
  
  if (pirvalue==1 && wifivalue==1) {  // PIR 센서 감지되면서 wifi에서 받은 밸류가 1일경우 (notdisp)
    Serial.println("Motion Detected!");  
    motor1.step(MOTOR_STEPS); //모터 한바퀴
    lemisol(); //사운드 레미솔 출력
    }

  // 음악 재생이 끝났을 때 스피커를 끔
  noTone(SPEAKER_PIN);
  delay(500); // 재생 후 0.5초 대기
}

void getdistance(){ //초음파센서 거리값 받아오는 함수
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn (ECHO_PIN, HIGH); //물체에 반사되어돌아온 초음파의 시간을 변수에 저장합니다.
  distance = duration * 17 / 1000; 
  Serial.print(distance); //측정된 물체로부터 거리값(cm값)을 보여줍니다.
  Serial.println(" Cm");
}
void lemi(){ //레미
  tone(SPEAKER_PIN, 294, 100);
  delay(200);
  tone(SPEAKER_PIN, 330, 100);
  delay(1000);
}
void lemisol(){ //레미솔
  tone(SPEAKER_PIN, 294, 100);
  delay(200);
  tone(SPEAKER_PIN, 330, 100);
  delay(200);
  tone(SPEAKER_PIN,392, 100);
  // 재생 후 1초 대기
  delay(1000);

}