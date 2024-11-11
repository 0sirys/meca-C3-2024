#include <Servo.h>
#include <RF24.h>
#define CE_PIN 8
#define CSN_PIN 9
#define IRQ_PIN 10
#define MOSI_PIN 11
#define MISO_PIN 12
#define SCK_PIN 13
int Luces_estado = 0;
int ITON_estado = 0;
struct data_controller {
  byte R2_gas;
  byte L2_brake;
  byte axys;
  byte botones;
};
typedef struct data_controller Controllerr;
Controllerr buffer;
Servo direccion;
//objeto nrf24L01
const uint64_t address = 0xE8E8F0F0E1LL;
RF24 radio(CE_PIN, CSN_PIN);

void setparameters() {
  buffer.axys = 127;
  buffer.L2_brake = 0;
  buffer.R2_gas = 0;
  buffer.botones = 0;
}

void pines() {
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}
void samples() {
  if (buffer.L2_brake < 255) {
    int samp = map(buffer.L2_brake, 0, 255, 255, 0);
    analogWrite(7, samp);
  } else if (buffer.R2_gas < 255) {
    int samp = map(buffer.R2_gas, 0, 255, 255, 0);
    analogWrite(7, samp);
  } else {
    int samp = 0;
    analogWrite(7, samp);
  }
}

void setup() {
  pines();
  direccion.attach(4);
  direccion.writeMicroseconds(600);
  setparameters();
  Serial.begin(115200);

  if (!radio.begin()) {
    Serial.println("Conexion Nrf24 fallida");
    while (1) {};

  } else {
    Serial.print("!!Conexion Nrf24 exitosa!!\n");
    radio.setChannel(100);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.openReadingPipe(1, address);
    radio.startListening();
  }
}
void Duty() {
  direccion.write(buffer.axys);


  analogWrite(6, buffer.L2_brake);
  analogWrite(2, buffer.R2_gas);



  samples();
  if (buffer.botones != 0) {
    if (buffer.botones == 1) {
      if (Luces_estado == 1) {
        digitalWrite(3, LOW);
        Luces_estado = 0;
      } else {
        digitalWrite(3, HIGH);
        Luces_estado = 1;
      }
    } else if (buffer.botones == 2) {
      if (ITON_estado == 1) {
        digitalWrite(5, LOW);
        ITON_estado = 0;
      } else {
        digitalWrite(5, HIGH);
        ITON_estado = 1;
      }
    }
  }
}
void receptor() {
  while (radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    char release[100];
    sprintf(release, "R2: %d, L2:%d, Botones:%d, axisX:%d\n", buffer.R2_gas, buffer.L2_brake, buffer.botones, buffer.axys);
    Serial.print(release);
    Duty();
  }
}

void loop() {
  receptor();
}
