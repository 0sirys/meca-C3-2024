#include <RF24.h>
#define CE_PIN 4
#define CSN_PIN 5
#define IRQ_PIN 10
#define MOSI_PIN 23
#define MISO_PIN 19
#define SCK_PIN 18
#define INTERMITENTES 10
//retencion de pulso en cualquier
class StateKeeper {
private:
  unsigned long lapse;
  uint8_t pin;
  uint8_t currentState;
  uint8_t previousState = 1;
public:
  StateKeeper(uint8_t _pin, uint8_t default_state /* 1 or 0*/) {
    this->lapse = millis();
    this->pin = _pin;
    this->currentState = default_state;
    this->previousState;
  };
  uint8_t setSatate(uint8_t mode /*1 or 0*/) {
    if ((millis() - lapse) == 24) {
      this->lapse = millis();
      if (mode > this->previousState) {
        this->previousState = this->currentState;
        this->currentState = mode;
        return this->currentState;
      }
    }
    return this->previousState;
  };
  uint8_t getCurrentState() {
    return this->currentState;
  };
  uint8_t getPreviousState() {
    this->previousState;
  };
  uint8_t getpin() {
    return this->pin;
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
StateKeeper buttonkeeper(INTERMITENTES, LOW);

//inicializacion de parametros
void setparameters() {
  buffer.axys = 127;
  buffer.L2_brake = 0;
  buffer.R2_gas = 0;
  buffer.botones = 0;
};
// estatus de antena de radio frecuencia
void radioStatus() {
  if (!radio.begin()) {
    Serial.println("Conexion Nrf24 fallida");
    while (1) {};
  }
  Serial.print("!!Conexion Nrf24 exitosa!!\n");
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(1, address);
  radio.startListening();
}
// funciones del carrito
void Duty() {
  rightEdge.marcha(buffer.R2_gas, buffer.L2_brake);
  leftEdge.marcha(buffer.R2_gas, buffer.L2_brake);
  digitalWrite(buttonkeeper.getpin(), buttonkeeper.setSatate(buffer.botones));
}


void setup() {
  rightEdge.setOutput(15, 13);
  leftEdge.setOutput(0, 2);
  setparameters();
  Serial.begin(115200);
  radioStatus();
}
//obtiene los datos y los almacena en el buffer
void receptor() {
  while (radio.available()) {
    radio.read(&buffer, sizeof(buffer));
    char release[100];
    sprintf(release, "R2: %d, L2:%d, Botones:%d, axisX:%d\n", buffer.R2_gas, buffer.L2_brake, buttonkeeper.setSatate(buffer.botones), buffer.axys);
    Serial.print(release);
    Duty();
  }
}

void loop() {
  receptor();
}
