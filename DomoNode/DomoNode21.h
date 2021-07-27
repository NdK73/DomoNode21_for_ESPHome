#include "esphome.h"
#include "PCA9555.h"

PCA9555 ext;

// On GPIOs: HIGH = ON, LOW = OFF
enum {
    RL_1=15,
    RL_2=14,
    RL_3=13,
    RL_4=12
};

// On ext
enum {
    RL_6=0,
    RL_5,
    IN_6,
    IN_5,
    IN_4,
    IN_3,
    IN_2,
    IN_1,
    LED_CONN,
    LED_USER,
    LED_1,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
};

#define LED_ON HIGH
#define LED_OFF LOW
#define RL_ON HIGH
#define RL_OFF LOW

// Updated by DomoNode21.update()
// Avoid reading 6 times the same register via I2C
static uint8_t lastin=0;

void relay(uint8_t n, bool state) {
    int r=0;
    if(n<=4) { // Direct GPIO
        switch(n) {
            case 1:
                r=RL_1;
                break;
            case 2:
                r=RL_2;
                break;
            case 3:
                r=RL_3;
                break;
            case 4:
                r=RL_4;
                break;
            default:
                return;
        }
        digitalWrite(r, state);
    } else {
        switch(n) {
            case 5:
                r=RL_5;
                break;
            case 6:
                r=RL_6;
                break;
            default:
                return;
        }
        ext.digitalWrite(r, state);
    }
}

// 6 inputs and 2 relays on port A
#define IOPORT_A_DIR 0b11111100

void led(int l, bool s) {
    ext.digitalWrite(l, s);
}

// Handle inputs
class DN21In: public Component, public BinarySensor {
    public:
        DN21In(byte in):
            _input(in), // Redundant: I'd only need mask, but this helps debugging
            _mask(0b10000000>>in)
        {
//            ESP_LOGD("custom", "Init In%d (%02x)", _input, _mask);
        }
        void loop() override {
            bool cur=lastin&this->_mask;
            if(last!=cur) {
                last=cur;
                publish_state(cur);
//                ESP_LOGD("custom", "In%d = %d", _input, state);
            }
        }
    private:
        bool last;
        byte _input, _mask;
};

// Handle relays updates
class DN21Relay: public Component, public BinaryOutput {
    public:
        DN21Relay(uint8_t rl):
            _relay(rl)
        {
        }
        void write_state(bool state) override {
//            ESP_LOGD("custom", "RL%d=%d", this->_relay, state);
            relay(this->_relay, state);
        }
    private:
        uint8_t _relay;
};

// Handle LEDs updates
class DN21Led: public Component, public BinaryOutput {
    public:
        DN21Led(uint8_t l):
            _led(LED_CONN+l)
        {
        }
        void write_state(bool state) override {
            led(this->_led, state);
        }
    private:
        uint8_t _led;
};

// Device initialization and input caching
class DomoNode21 : public Component {
 public:
    float get_setup_priority() const override {
        return esphome::setup_priority::BUS;
    }

    void setup() override {
        Wire.begin();

        pinMode(RL_1, OUTPUT);
        pinMode(RL_2, OUTPUT);
        pinMode(RL_3, OUTPUT);
        pinMode(RL_4, OUTPUT);
        PCA9555Status s=ext.begin();

        // TODO: better error handling
        if(PCA9555Status::OK != s) {
            ESP_LOGD("custom", "Could not initialize PCA9555");
        }
        ext.portMode(PCA9555Port::A, IOPORT_A_DIR); // Port A mixed
        ext.portMode(PCA9555Port::B, 0b00000000); // Port B as ouput (LEDs)
        ext.inputInv(0b0000000011111100); // Invert inputs, to read '1' when 220V is applied

        // Turn off everything
        for(uint8_t rl=1; rl<=6; ++rl) {
            relay(rl, RL_OFF);
        }
        for(uint8_t l=0; l<8; ++l) {
            led(l+LED_CONN, LED_OFF);
        }
    }
    void loop() override {
        // Poll inputs ASAP from here, to avoid overloading the bus
        lastin=ext.readPort(PCA9555Port::A) & IOPORT_A_DIR;
    }
};
