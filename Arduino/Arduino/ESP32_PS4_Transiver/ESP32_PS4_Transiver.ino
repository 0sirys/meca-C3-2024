#include <Bluepad32.h>
#include <stdlib_noniso.h>
#include <RF24.h>
#define CSN_PIN 4

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

struct data_controller {
  byte R2_gas;
  byte L2_brake;
  byte axys;
  byte botones;
};
typedef struct data_controller Controllerr;
Controllerr tosend;


//objeto nrf24L01
const uint64_t address = 0xE8E8F0F0E1LL;
RF24 radio(SS, CSN_PIN);




// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                    properties.product_id);
      myControllers[i] = ctl;
      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}


void processGamepad(ControllerPtr ctl) {
  // There are different ways to query whether a button is pressed.
  // By query each button individually:
  //  a(), b(), x(), y(), l1(), etc...
  if (ctl->a()) {
    static int colorIdx = 0;
    // Some gamepads like DS4 and DualSense support changing the color LED.
    // It is possible to change it by calling:
    switch (colorIdx % 3) {
      case 0:
        // Red
        ctl->setColorLED(255, 0, 0);
        break;
      case 1:
        // Green
        ctl->setColorLED(0, 255, 0);
        break;
      case 2:
        // Blue
        ctl->setColorLED(0, 0, 255);
        break;
    }
    colorIdx++;
  }

  if (ctl->b()) {
    // Turn on the 4 LED. Each bit represents one LED.
    static int led = 0;
    led++;
    // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
    // support changing the "Player LEDs": those 4 LEDs that usually indicate
    // the "gamepad seat".
    // It is possible to change them by calling:
    ctl->setPlayerLEDs(led & 0x0f);
  }

  if (ctl->x()) {
    // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
    // It is possible to set it by calling:
    // Some controllers have two motors: "strong motor", "weak motor".
    // It is possible to control them independently.
    ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                        0x40 /* strongMagnitude */);
  }

  // Another way to query controller data is by getting the buttons() function.
  // See how the different "dump*" functions dump the Controller info.
  mapper(ctl, &tosend);
  prtmap(&tosend);
}



void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      } else {
        Serial.println("Unsupported controller");
      }
    }
  }
}

void mapper(ControllerPtr ct, Controllerr* obj) {
  obj->R2_gas = map(ct->throttle(), 1023, 0, 0, 255);
  obj->L2_brake = map(ct->brake(), 1023, 0, 0, 255);
  obj->botones = ct->buttons();
  obj->axys = map(ct->axisX(), -511, 511, 0, 180);
}
void prtmap(Controllerr* obj) {
  Serial.printf(" R2: %4d, L2 : %4d, Boton: %4d, Joytick: %4d\n", obj->R2_gas, obj->L2_brake, obj->botones, obj->axys);
}

// Arduino setup function. Runs in CPU 1
void setup() {
  Serial.begin(115200);
  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  BP32.enableVirtualDevice(false);
  if (!radio.begin()) {
    Serial.print(F("No se detecta modulo nrf24\n"));
  } else {
    Serial.print(F(" modulo nrf24 conectado\n"));
    // NRF24 configuracion
    radio.openWritingPipe(address);
    radio.setChannel(100);
    radio.setAutoAck(false);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MAX);
    radio.printPrettyDetails();
  }
}

// Arduino loop function. Runs in CPU 1.
void loop() {
  // This call fetches all the controllers' dac:\Users\Eliza\OneDrive\Documents\Arduino\Receptor_RF_Nano\Receptor_RF_Nano.inota.
  bool dataUpdated = BP32.update();
  if (dataUpdated) {
    processControllers();
    bool ok = radio.write(&tosend, sizeof(tosend));
    if (ok) {
      Serial.println("Envio exitoso");
    } else {
      Serial.println("Envio fallido");
    }
  }
  
}
