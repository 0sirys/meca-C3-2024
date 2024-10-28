#include <Servo.h>
Servo test;
void setup() {
  test.attach(7);
  Serial.begin(115200);

}

void loop() {
  test.write(180);
  Serial.write("Hola\n");
 delay(1000);

}
