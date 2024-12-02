#include <PS4Controller.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"

#define INTERMITENTES 10
//retencion de pulso en cualquier
class Button {
private:
  uint8_t state_input = 0;
  int pinbutton;
  uint8_t previousState = 0;  // ultimo estado del boton
  uint8_t currentState = 1;   // estado actual del boton
  unsigned long lapse = 0;
public:
  // track control del rebote de boton
  Button(int pin) {  // constructor
    this->pinbutton = pin;
    pinMode(pin, OUTPUT);
  };
  // Funcion para actualizar el estado del boton
  uint8_t Toggle(uint8_t mode) {
    this->currentState = mode;

    if (this->currentState == 0 && this->previousState == 1) {
      this->state_input = 1 - this->state_input;

      if (this->state_input == 1) {
        digitalWrite(this->pinbutton, HIGH);
      } else {
        digitalWrite(this->pinbutton, LOW);
      }
      this->previousState = this->currentState;
      return this->currentState;
    }

    return this->currentState;  // devuelvo el estado anclado.
  };

  int pin() {
    return this->pinbutton;
  };
};


class Motor {  //propiedades y funciones del motor
private:
  uint8_t d01;
  uint8_t d02;

public:
  void setOutput(uint8_t val1, uint8_t val2) {
    this->d01 = val1;
    this->d02 = val2;
  };
  uint8_t marcha(uint8_t D1, uint8_t D2) {
    if (!(D1 > D2)) {
      analogWrite(this->d02, D2);
      return D2;
    }
    if (!(D2 > D1)) {
      analogWrite(this->d01, D1);
      return D1;
    }
    if (D2 == D1) {
      analogWrite(this->d01, 0);
      analogWrite(this->d02, 0);
      return -1;
    }
    return 0;
  }
};


Motor rightEdge;  //Motor 1
Motor leftEdge;   // Motor 2


Button buttonkeeper(INTERMITENTES);  // botone con retencion

void removePairedController() {  // desvincula los mandos conectados.
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

void Duty() {  // funciones del carrito
  char release[100];
  sprintf(release, "R2: %d, L2:%d, cross:%d,\n", PS4.R2Value(), PS4.L2Value(), buttonkeeper.Toggle(PS4.Cross()));
  Serial.print(release);
  rightEdge.marcha(PS4.R2Value(), PS4.L2Value());
  leftEdge.marcha(PS4.R2Value(), PS4.L2Value());
}



void setup() {  // Inicializacion de esp32
  rightEdge.setOutput(15, 13);
  leftEdge.setOutput(0, 2);
  Serial.begin(115200);
  PS4.begin();
  removePairedController();
  while (!PS4.isConnected()) {
    Serial.print("Wait Controller");
  }
}

void loop() {
  Duty();
}
