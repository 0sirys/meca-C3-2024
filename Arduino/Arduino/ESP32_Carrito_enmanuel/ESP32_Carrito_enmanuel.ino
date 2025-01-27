#include <PS4Controller.h>
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 10
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18
#define INTERMITENTES 14
//radiofrecuencia
//retencion de pulso en cualquier
class Button {
private:
  int pinbutton;
public:
  uint8_t previousState = HIGH;  // ultimo estado del boton
  uint8_t currentState = LOW;    // estado actual del boton
  unsigned long lapse = 0;       // track control del rebote de boton
  Button(int pin) {              // constructor
    this->pinbutton = pin;
  };
  // Funcion para actualizar el estado del boton
  uint8_t setState(uint8_t mode) {
    if (millis() - lapse >= 15) {  // Intervalo de rebote

      if (mode > 0) {                              // condicion para solo activar en high
        uint8_t pivot = this->previousState;       // pivote para acarreo de estado.
        this->previousState = this->currentState;  //acarreo de estado actual a anterior.
        this->currentState = pivot;                // modifico el estado actual 1 o 0
        lapse = millis();                          // reinicio de conteo
        return this->currentState;                 // devuelvo el nuevo estado anclado.
      }
    }
    return this->currentState;  // devuelvo el estado anclado.
  };
  int pin() {
    return this->pinbutton;
  };
};

//propiedades y funciones del motor
class Motor {
private:
  uint8_t d01;
  uint8_t d02;

public:
  void setOutput(uint8_t val1, uint8_t val2) {
    this->d01 = val1;
    this->d02 = val2;
    pinMode(val1, OUTPUT);
    pinMode(val2, OUTPUT);
  };
  uint8_t marcha(uint8_t D1, uint8_t D2) {
    if (D1 <= 0 && D2 <= 0) {
      analogWrite(this->d01, 0);
      analogWrite(this->d02, 0);
      return 0;
    }
    if (!(D1 > D2)) {
      analogWrite(this->d01, D2);
      analogWrite(this->d02, 0);
      return D2;
    }
    if (!(D2 > D1)) {
      analogWrite(this->d02, D1);
      analogWrite(this->d01, 0);
      return D1;
    }
    return 0;
  }
};

typedef struct data_controller {
  byte R2_gas;
  byte L2_brake;
  byte axyz;
  byte botones;
} Controllerr;
//objeto controllerr
Controllerr buffer;



//Motores 1 y 2 independientes
Motor rightEdge;
Motor leftEdge;
// botones con retencion
Button buttonkeeper(INTERMITENTES);

void onDisConnect() {
  Serial.println("Disconnected!");
}

void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}



void onConnect() {
  Serial.println("Connected!");
}
//inicializacion de parametros
void mapper() {
  buffer.axyz = PS4.LStickX();      // joystick
  buffer.L2_brake = PS4.L2Value();  // reversa y freno
  buffer.R2_gas = PS4.R2Value();    // avance
  buffer.botones = PS4.Cross();     // intermitentes
};

// funciones del carrito
void Duty() {
  if (buffer.axyz == 127) {
    leftEdge.marcha(255, 0);
    rightEdge.marcha(0, 255);
  } else if (buffer.axyz == 129) {
    leftEdge.marcha(0, 255);
    rightEdge.marcha(255, 0);
  } else {
    leftEdge.marcha(buffer.R2_gas, buffer.L2_brake);
    rightEdge.marcha(buffer.R2_gas, buffer.L2_brake);
  }
  digitalWrite(buttonkeeper.pin(), buttonkeeper.setState(buffer.botones));
}


void setup() {
  rightEdge.setOutput(28, 27);
  leftEdge.setOutput(26, 25);
  pinMode(buttonkeeper.pin(), OUTPUT);
  Serial.begin(115200);

  Serial.println("MAC Address: " + macAddress);
  PS4.attachOnConnect(onConnect);
  PS4.attachOnDisconnect(onDisConnect);
  PS4.begin();
  if (!PS4.isConnected()) {
    while (!PS4.isConnected()) {}
  }
}
//obtiene los datos y los almacena en el buffer
void receptor() {
  mapper();
  char release[100];
  sprintf(release, "R2: %d, L2:%d, Botones:%d, axisX:%d\n", buffer.R2_gas, buffer.L2_brake, buttonkeeper.setState(buffer.botones), buffer.axyz);
  Serial.print(release);
  Duty();
}

void loop() {
  receptor();
}
