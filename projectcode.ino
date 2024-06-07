#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include<Wire.h>
#include<Stepper.h>

//지문인식 센서
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id;

///초음파 센서 설정
int trigPin = 4;
int echoPin = 5;
//lcd 센서
LiquidCrystal_I2C lcd(0x27, 16, 2);

//steppper모터 센서
const int stepsPer = 2048;
Stepper step(stepsPer, 8, 10, 9, 11);

//층별 정보 저장

struct Employee {
  int id;
  const char* name;
  const char* floor;
  int height;
};

Employee employees[] = {
  {1, "seonghwan", "1Floor", 2},
  {2, "yeonseo", "2Floor", 10},
  {3, "minjeong", "3Floor", 21},
  {4, "dongjin", "1Floor", 2},
  {5, "hyunwoo", "2Floor", 10},
  {6, "jihye", "3Floor", 21},
  {7, "sangmin", "1Floor", 2},
  {8, "eunji", "2Floor", 10},
  {9, "jaehyun", "3Floor", 21},
  {10, "sooyeon", "1Floor", 2}
};

void setup() {
  Serial.begin(9600); // 시리얼 통신 시작
  lcd.init(); // LCD 초기화
  lcd.backlight(); // LCD 백라이트 켜기
  step.setSpeed(14);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin,INPUT);

  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  //지문인식 센서 디바이스 확인
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  //Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  //Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  //Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  //Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  //Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  //Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  //Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  
  //등록된 지문 유무 판
  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    //Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    //Serial.println("Waiting for valid finger...");
      //Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

float dist(){
  float dist, duration;

  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  dist = ((float)(340 * duration) / 10000) / 2;

  return dist;
}

void moveToHeight(int targetHeight) {
  if (targetHeight == 10) { // 1층의 위치가 아닌 경우에만 작동
    while (true) {
      int distance = dist();
      Serial.print("Current distance: ");
      Serial.println(distance);
      
      // 목표 높이에 도달하지 않았으면 모터를 작동시킵니다.
      if (distance < targetHeight) {
        step.step(stepsPer); // 올바른 변수명으로 수정함
      } else {
        // 목표 높이에 도달하면 모터 작동을 멈추고 5초간 대기
        Serial.println("Reached target height (2nd floor), waiting for 5 seconds...");
        delay(5000);
        break;
      }
      delay(100); // 잠시 대기
    }
  } else if (targetHeight == 21) { // 3층의 경우
    while (true) {
      int distance = dist();
      Serial.print("Current distance: ");
      Serial.println(distance);
      
      // 목표 높이에 도달하지 않았으면 모터를 작동시킵니다.
      if (distance < targetHeight) {
        step.step(stepsPer); // 올바른 변수명으로 수정함
      } else {
        // 목표 높이에 도달하면 모터 작동을 멈추고 5초간 대기
        Serial.println("Reached target height (3rd floor), waiting for 5 seconds...");
        delay(5000);
        break;
      }
      delay(100); // 잠시 대기
    }
  }
  
  // 1층으로 복귀하는 로직
  // 여기서는 1층의 높이를 2로 가정합니다. 실제 높이에 맞게 조정해 주세요.
  Serial.println("Returning to 1st floor...");
  while (true) {
    int distance = dist();
    Serial.print("Current distance: ");
    Serial.println(distance);
    
    // 1층의 높이보다 높은 경우 아래로 이동
    if (distance > 2) {
      Serial.println("Moving down...");
      step.step(-stepsPer); // 역방향으로 이동
    } else {
      Serial.println("Reached 1st floor.");
      break; // 1층에 도달하면 반복문을 종료합니다.
    }
    delay(100); // 잠시 대기
  }
}

void displayFloorAndMove(int fingerprintID) {
  bool found = false;
  for (int i = 0; i < sizeof(employees) / sizeof(employees[0]); i++) {
    if (employees[i].id == fingerprintID) {
      lcd.clear();
      lcd.print("Floor: ");
      lcd.print(employees[i].floor);      
      found = true;
      delay(2000); // LCD에 정보를 2초간 표시
      
      moveToHeight(employees[i].height); // 해당 층의 높이까지 모터를 작동
      delay(5000); // 5초간 정지

      if (employees[i].height != 3) {
        moveToHeight(3); // 1층으로 복귀
      }
      break;
    }
  }
  if (!found) {
    lcd.clear();
    lcd.print("Employee not");
    lcd.setCursor(0, 1);  // 다음 줄로 이동
    lcd.print("found.");
    delay(2000); // 메시지를 2초간 표시
  }
}
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {

  Serial.println("---------------Menu--------------");
  Serial.println("1.enroll, 2.fingerpring, 3.delete");
  switch(readnumber()){
    case 1:
      Serial.print("Enrolling ID #");
      id = readnumber();
      if (id == 0) {// ID #0 not allowed, try again!
      return;
      }
      Serial.print("Enrolling ID #");
      Serial.println(id);
      while (! getFingerprintEnroll() ){

      }
      break;
    case 2:
      
      while(!getFingerprintID() );
      break;
    case 3:
      Serial.println("Please type in the ID # (from 1 to 127) you want to delete...");
      uint8_t id = readnumber();
      if (id == 0) {// ID #0 not allowed, try again!
        return;
        }
      Serial.print("Deleting ID #");
      Serial.println(id);
      deleteFingerprint(id);
      break;
  }

}

//지문 등록 코드 enroll
uint8_t getFingerprintEnroll() {
  
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}
//등록된 지문 정보 확인 Fingerprint

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("put your finger");
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }


  // found a match!
  displayFloorAndMove(finger.fingerID);
  Serial.print("Found ID #"); Serial.println(finger.fingerID);
  //Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

//지문 삭제 delete
uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
