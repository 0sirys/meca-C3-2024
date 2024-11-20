class StateKeeper {
private:
  unsigned long lapse;
  uint8_t pin;
  uint8_t currentState;
  uint8_t previousState;
public:
  StateKeeper(uint8_t _pin,uint8_t default_state){
    this->lapse=millis();
    this->pin=_pin;
    this->currentState=default_state;
    this->previousState=default_state;
  }
  uint8_t setSatate(uint8_t mode) {
    if((millis()-lapse)==24){
      this->lapse=millis();
      this->previousState = this->currentState;
    this->currentState = mode;

    return (this->currentState != this->previousState)? 1:-1;
    }
    return 0;
  };
  uint8_t getCurrentState(){
    return this->currentState;
  }
  uint8_t getPreviousState(){
    this->previousState;
  }
};