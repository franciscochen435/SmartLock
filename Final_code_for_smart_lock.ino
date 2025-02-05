#include <EEPROM.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

Servo myservo;

const int passwordButton1Pin = 6; // passcodebutton1
const int passwordButton2Pin = 5; // passcodebutton2
const int passwordButton3Pin = 4; // passcodebutton3
const int settingButtonPin = 3; // set-up button
const int buzzerPin = 7;
const int RST_PIN = 9; 
const int SS_PIN = 10;

MFRC522 mfrc522(SS_PIN, RST_PIN);

const int passwordLength = 6;
int inputPassword[passwordLength];
int storedPassword[passwordLength];
int inputIndex = 0;

bool servoPosition = false; // false represesnts 0 degree，true represesnts 90 degree

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myservo.attach(2);
  pinMode(settingButtonPin, INPUT_PULLUP);
  pinMode(passwordButton1Pin, INPUT_PULLUP);
  pinMode(passwordButton2Pin, INPUT_PULLUP);
  pinMode(passwordButton3Pin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  myservo.write(90); // 初始舵机位置
  
  Serial.println("Please set your password using the buttons. Press each button to add a digit (1, 2, or 3).");
  setPassword();
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print("Card UID:");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // check if the tag is corresponding to the reader
    if (mfrc522.uid.uidByte[0] == 0xCC && mfrc522.uid.uidByte[1] == 0x73 &&
        mfrc522.uid.uidByte[2] == 0xD2 && mfrc522.uid.uidByte[3] == 0x17) {
      // if it is
      myservo.write(90); // spin the servo
      beep(500); // make a noise
      servoPosition = true;
    } else {
      Serial.println("Card UID does not match.");
      servoPosition = false;
    }
    mfrc522.PICC_HaltA(); // stop to read card
  }

  if (digitalRead(settingButtonPin) == LOW) {
    delay(500); // make a delay
    if (!servoPosition ) {
      
      Serial.println("Re-setting the password...");
      handlePasswordInput();
    } else {
      // if servo has spined, return the origial position
      myservo.write(0);
      beep(500);
      servoPosition = false;
    }
  } else {
    // deal with the input passcode
    handlePasswordInput();
  }
}

void setPassword() {
  myservo.write(90); // lock as the servo spins to 90 degree
  delay(1000); // delay
  Serial.println("Please set your password using the buttons. Press each button to add a digit (1, 2, or 3).");


  inputIndex = 0;
  while (inputIndex < passwordLength) {
    if (digitalRead(passwordButton1Pin) == LOW) {
      delay(200);
      beep(100);
      addDigitToPassword(1);
    } else if (digitalRead(passwordButton2Pin) == LOW) {
      delay(200);
      beep(200);
      addDigitToPassword(2);
    } else if (digitalRead(passwordButton3Pin) == LOW) {
      delay(200);
      beep(300);
      addDigitToPassword(3);
    }
  }

  // store the passcode
  for (int i = 0; i < passwordLength; i++) {
    EEPROM.write(i, inputPassword[i]);
    storedPassword[i] = inputPassword[i]; // update the passcode
  }
  
  beep(500); // make a noise showing setting up the passcode
  Serial.println("\nPassword set successfully.");
  myservo.write(0); // lock, return the servo

}

void handlePasswordInput() {
  static int tempPassword[passwordLength];
  static int index = 0;

  if (digitalRead(passwordButton1Pin) == LOW) {
    delay(500); 
    beep(100);
    if (index < passwordLength) tempPassword[index++] = 1;
  } else if (digitalRead(passwordButton2Pin) == LOW) {
    delay(500);
    beep(200);
    if (index < passwordLength) tempPassword[index++] = 2;
  } else if (digitalRead(passwordButton3Pin) == LOW) {
    delay(500); 
    beep(300);
    if (index < passwordLength) tempPassword[index++] = 3;
  }

  if (index == passwordLength) {
    if (checkPassword(tempPassword)) {
      myservo.write(servoPosition ? 0 : 90);
      beep(500);
      servoPosition = !servoPosition;
    }
    index = 0; // reenter the passcode
  }

  
}

bool checkPassword(int* inputPwd) {
  for (int i = 0; i < passwordLength; i++) {
    if (EEPROM.read(i) != inputPwd[i]) {
      return false; // if any of the element in the passcode is wrong, return false
    }
  }
  return true; // return true
}

void addDigitToPassword(int digit) {
  if (inputIndex < passwordLength) {
    inputPassword[inputIndex] = digit;
    inputIndex++;
    Serial.print(digit); 
  }
}

void beep(int duration) {
  tone(buzzerPin, 1000, duration);
  delay(duration); 
}