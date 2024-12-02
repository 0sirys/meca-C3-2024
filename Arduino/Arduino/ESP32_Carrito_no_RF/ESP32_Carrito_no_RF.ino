#include <RF24.h>
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 10
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18
#define INTERMITENTES 10
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
    if (millis() - lapse >= 24) {  // Intervalo de rebote
      lapse = millis();            // reinicio de conteo

      if (mode > 0) {                              // condicion para solo activar en high
        uint8_t pivot = this->previousState;       // pivote para acarreo de estado.
        this->previousState = this->currentState;  //acarreo de estado actual a anterior.
        this->currentState = pivot;                // modifico el estado actual 1 o 0
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
    return 0;
  }
};

typedef struct data_controller {
  byte R2_gas;
  byte L2_brake;
  byte axys;
  byte botones;
} Controllerr;
//objeto controllerr
Controllerr buffer;

//objeto nrf24L01
RF24 radio(CE_PIN, CSN_PIN);
//direccion de antena
const uint64_t address = 0xE8E8F0F0E1LL;

//Motores 1 y 2 independientes
Motor rightEdge;
Motor leftEdge;
// botones con retencion
Button buttonkeeper(INTERMITENTES);

//inicializacion de parametros
void setparameters() {
  buffer.axys = 127;    // centro del joystick
  buffer.L2_brake = 0;  // reversa y freno
  buffer.R2_gas = 0;    // avance
  buffer.botones = 0;   // intermitentes
};

// funciones del carrito
void Duty() {
  rightEdge.marcha(buffer.R2_gas, buffer.L2_brake);
  leftEdge.marcha(buffer.R2_gas, buffer.L2_brake);
  digitalWrite(buttonkeeper.pin(), buttonkeeper.setState(buffer.botones));
}


void setup() {
  Serial.begin(115200);
  rightEdge.setOutput(15, 13);
  leftEdge.setOutput(0, 2);
  setparameters();
}
//obtiene los datos y los almacena en el buffer
void receptor() {
  while (radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    char release[100];
    sprintf(release, "R2: %d, L2:%d, Botones:%d, axisX:%d\n", buffer.R2_gas, buffer.L2_brake, buttonkeeper.setState(buffer.botones), buffer.axys);
    Serial.print(release);
    Duty();
  }
}

void loop() {
  receptor();
}
