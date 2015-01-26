#include "config.h"
#include <Arduino.h>

// Connections:
//
//    Remote Rc
//    XY-DJM-5v <--> Arduino
//       GND           GND
//       5V            5V
//       D0            A0
//       D1            A1
//       D2            A2
//       D3            A3
//
//    Relays 
//    HL-54S V1.2 <--> Arduino
//       GND           GND
//       IN1           D5
//       IN2           D4
//       IN3           D3
//       IN4           D2
//       5V            5V



#define R1 5
#define R2 4
#define R3 3
#define R4 2

// digitalWrite(R1, LOW);


class ButtonRC
{
public:
    ButtonRC(uint8_t pin)
        : pin_(pin)
        , state_(false)
        , last_state_(true)
    {

    }

    bool update()
    {
        //bool raw_state = analogRead(pin_) > 512 ? LOW : HIGH;
        bool raw_state = analogRead(pin_) > 512;

        if(raw_state)
        {
            // Detect rising edges
            if(!last_state_)
            {
                last_state_ = true;
                last_edge_time_ = millis();
            }

            if(!state_)
            {
                // Debouncing complete ?
                if(last_edge_time_ + DEBOUNCE_COUNT_UP__ <= millis())
                {
                    state_ = true;
                } 
            }
        }
        else
        {
            // Detect failling edges
            if(last_state_)
            {
                last_state_ = false;
                last_edge_time_ = millis();
            }

            if(state_)
            {
                // Debouncing complete ?
                if(last_edge_time_ + DEBOUNCE_COUNT_DOWN__ <= millis())
                {
                    state_ = false;
                } 
            }
        }

        return state_;
    }

private:
    enum Config
    {
        DEBOUNCE_COUNT_UP__ = 50,     // number of millis to consider it as up
        DEBOUNCE_COUNT_DOWN__ = 500,  // number of millis to consider it as down
    };

    uint8_t pin_;
    bool last_state_;
    bool state_;
    unsigned long last_edge_time_;

};

class Door
{
private:
    enum Config
    {
        TIME_TO_OPEN__  = 5000,        // In ms
        //TIME_TO_CLOSE__ = 60000,        // In ms
        RELAY_ACTIVATION_TIME__ = 50,   // Relay activation in ms  normaly it's around ~75ms
    };

    enum eState
    {
        eOPENED = 0,
        eCLOSED,
        eCLOSING,
        eOPENING,
        eOPEN_REQUESTED,
        eCLOSE_REQUESTED,
        eSTOP_REQUESTED,
        eSTOPED, 
    };
    static const char* enum_state_name__[];

public:
    Door(uint8_t pin_out_open, uint8_t pin_out_close)
        : pin_out_open_(pin_out_open)
        , pin_out_close_(pin_out_close)
        , state_(eCLOSED)
        //, last_action_(eCLOSE)
        , door_steps_(0)
    {
        // TODO read last state on eeprom
    }

    void update()
    {
        unsigned long ms = millis();

        if (eCLOSE_REQUESTED == state_)
        {
            if (ms >  end_target_timestamp_)
            {
                setState(eCLOSING);
            }
        }
        else if (eOPEN_REQUESTED == state_)
        {
            if (ms >  end_target_timestamp_)
            {
                setState(eOPENING);
            }
        }
        else if (eSTOP_REQUESTED == state_)
        {
            if (ms >  end_target_timestamp_)
            {
                setState(eSTOPED);
            }
        }
        else if (eCLOSING == state_)
        {
            if (ms >  end_target_timestamp_)
            {
                setState(eCLOSED);
            }
        }
        else if (eOPENING == state_)
        {
            if (ms >  end_target_timestamp_)
            {
                setState(eOPENED);
            }
        }
    }

    void open()
    {
        if (state_ != eOPENING && state_ != eOPENED)
        {
            setState(eOPEN_REQUESTED);
        }
    }

    void close()
    {
        if (state_ != eCLOSING && state_ != eCLOSED)
        {
            setState(eCLOSE_REQUESTED);
        }
    }

    void stop()
    {
        if (state_ != eCLOSED && state_ != eOPENED)
        {
            setState(eSTOPED);
        }
    }

private:

    void setState(eState new_state)
    {
        if (state_ == new_state) return;

        //
        // Manage on exit state
        //
        if (state_ == eOPENING)
        {
            digitalWrite(pin_out_open_, HIGH);
            if (new_state != eOPENED && end_target_timestamp_ > millis() )
            {
                // Open was not reach
                door_steps_ +=  millis() - state_timestamp_;
            }
        }
        else if (state_ == eCLOSING)
        {
            digitalWrite(pin_out_close_, HIGH);
            if (new_state != eCLOSED  && end_target_timestamp_ > millis())
            {
                // Close was not reach
                door_steps_ -=  millis() - state_timestamp_;
            }
        }

        //
        //Assign state
        //
        eState last_state = state_;
        state_ = new_state;
        state_timestamp_ = millis();
        log_log("Enter state: ");log_logln(enum_state_name__[state_]);
    

        //
        // Manage on enter state
        //
        if (state_ == eOPEN_REQUESTED)
        {
            digitalWrite(pin_out_open_, LOW);
            end_target_timestamp_ = state_timestamp_ + RELAY_ACTIVATION_TIME__;
        }
        else if (state_ == eCLOSE_REQUESTED)
        {
            digitalWrite(pin_out_close_, LOW);
            end_target_timestamp_ = state_timestamp_ + RELAY_ACTIVATION_TIME__;
        }
        else if (state_ == eOPENING)
        {
            end_target_timestamp_ = state_timestamp_;
            end_target_timestamp_ += TIME_TO_OPEN__  - door_steps_;
        }
        else if (state_ == eCLOSING)
        {
            end_target_timestamp_ = state_timestamp_;
            end_target_timestamp_ +=  door_steps_;
        }
        else if (state_ == eOPENED)
        {
            digitalWrite(pin_out_open_, HIGH);
            door_steps_ = TIME_TO_OPEN__;
        }
        else if (state_ == eCLOSED)
        {
            digitalWrite(pin_out_close_, HIGH);
            door_steps_ = 0;
        }
        else if (state_ == eSTOP_REQUESTED)
        {
            setState(eSTOPED);
        }
    }


    eState state_;
    unsigned long   state_timestamp_;       // Timestamp of state
    unsigned long   end_target_timestamp_;

    //enum eLastAction
    //{
    //    eOPEN,
    //    eCLOSE,
    //};
    //eLastAction last_action_;

    long door_steps_;  // [0 - TIME_TO_OPEN__]  0 is close
    uint8_t pin_out_open_;
    uint8_t pin_out_close_;
};
const char* Door::enum_state_name__[] = 
{
    "eOPENED",
    "eCLOSED",
    "eCLOSING",
    "eOPENING",
    "eOPEN_REQUESTED",
    "eCLOSE_REQUESTED",
    "eSTOP_REQUESTED",
    "eSTOPED",
};


ButtonRC button_a(A2);
ButtonRC button_b(A0);
ButtonRC button_c(A3);
ButtonRC button_d(A1);
Door door(R1, R2);


void onButtonA()
{
    log_logmsln("onButtonA");
    door.open();
}

void onButtonB()
{
    log_logmsln("onButtonB");
}

void onButtonC()
{
    log_logmsln("onButtonC");
    door.close();
}

void onButtonD()
{
    log_logmsln("onButtonD");
    door.stop();
}

void updateButton()
{
    static bool last_a = false;
    static bool last_b = false;
    static bool last_c = false;
    static bool last_d = false;
    
    // Read new state of rc button
    bool a =  button_a.update();
    bool b =  button_b.update();
    bool c =  button_c.update();
    bool d =  button_d.update();

    if (last_a != a)
    {
        if (a)
        {
            onButtonA();
        }
        last_a = a;
    }

    if (last_b != b)
    {
        if (b)
        {
            onButtonB();
        }
        last_b = b;
    }

    if (last_c != c)
    {
        if (c)
        {
            onButtonC();
        }
        last_c = c;
    }

    if (last_d != d)
    {
        if (d)
        {
            onButtonD();
        }
        last_d = d;
    }

}


void setup()
{
    log_init();
    log_logln("Garage Door...Ready");

    pinMode(13, OUTPUT);          
    digitalWrite(13, LOW);       

    pinMode(R1, OUTPUT);
    pinMode(R2, OUTPUT);
    pinMode(R3, OUTPUT);
    pinMode(R4, OUTPUT);

    digitalWrite(R1, HIGH);     
    digitalWrite(R2, HIGH);     
    digitalWrite(R3, HIGH);     
    digitalWrite(R4, HIGH);     
}

void loop()
{
    updateButton();

    door.update();     
}
