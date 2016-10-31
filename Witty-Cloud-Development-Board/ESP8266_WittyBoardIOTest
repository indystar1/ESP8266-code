const int LDR = A0;
const int BUTTON = 4;
const int RED = 15;
const int GREEN = 12;
const int BLUE = 13;
 
void setup() 
{
    Serial.begin(115200);
 
    pinMode(LDR, INPUT);
    pinMode(BUTTON, INPUT);
    pinMode(RED, OUTPUT);
    pinMode(GREEN, OUTPUT);
    pinMode(BLUE, OUTPUT);
}
 
void loop()
{
      byte count = 0;
      Serial.println("To change mode, press Button for more than 0.5 sec!");
      Serial.println("All LED mixed testing...");
      while (1) {
        analogWrite(RED, random(0,511));
        analogWrite(GREEN, random(0,511));
        analogWrite(BLUE, random(0,511));
        //analogWrite(RED, random(0,1023));	// too bright
        //analogWrite(GREEN, random(0,1023));
        //analogWrite(BLUE, random(0,1023));
        Serial.print("LDR: ");
        Serial.print(analogRead(LDR));
        Serial.print("\tBUTTON: ");
        Serial.println(digitalRead(BUTTON));
        if (digitalRead(BUTTON) == LOW) break;
        delay(500);
      }

      delay(500);
      while (digitalRead(BUTTON) == LOW) delay(100);
      Serial.println("RED LED testing...");
      analogWrite(GREEN,0);
      analogWrite(BLUE,0);
      while (1) {
         analogWrite(RED, random(0,511));
         count++;
         if (count > 5) {
              count = 0;
              Serial.print("LDR: ");
              Serial.print(analogRead(LDR));
              Serial.print("\tBUTTON: ");
              Serial.println(digitalRead(BUTTON));
              if (digitalRead(BUTTON) == LOW) break;
         }
         delay(100);
      }

      delay(500);
      while (digitalRead(BUTTON) == LOW) delay(100);
      Serial.println("GREEN LED testing...");
      analogWrite(RED,0);
      analogWrite(BLUE,0);
      while (1) {
          analogWrite(GREEN, random(0,511));
          count++;
          if (count > 5) {
              count = 0;
              Serial.print("LDR: ");
              Serial.print(analogRead(LDR));
              Serial.print("\tBUTTON: ");
              Serial.println(digitalRead(BUTTON));
              if (digitalRead(BUTTON) == LOW) break;
          }
          delay(100);
       }

      delay(500);
      while (digitalRead(BUTTON) == LOW) delay(100);
      Serial.println("BLUE LED testing...");
      analogWrite(GREEN,0);
      analogWrite(RED,0);
      while (1) {
          analogWrite(BLUE, random(0,511));
          count++;
          if (count > 5) {
              count = 0;
              Serial.print("LDR: ");
              Serial.print(analogRead(LDR));
              Serial.print("\tBUTTON: ");
              Serial.println(digitalRead(BUTTON));
              if (digitalRead(BUTTON) == LOW) break;
          }
          delay(100);
      }
      while (digitalRead(BUTTON) == LOW) delay(100);
      Serial.println();
}
