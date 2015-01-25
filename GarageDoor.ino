#include "config.h"

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
        

class ButtonRC
{
public:
    ButtonRC(uint8_t pin)
        : pin_(pin)
        , state_(false)
        , last_state_(true)
    {
        DEBOUNCE_COUNT_UP_ = 50;
        DEBOUNCE_COUNT_DOWN_ = 500;
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
                if(last_edge_time_ + DEBOUNCE_COUNT_UP_ <= millis())
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
                if(last_edge_time_ + DEBOUNCE_COUNT_DOWN_ <= millis())
                {
                    state_ = false;
                } 
            }
        }

        return state_;
    }

private:
    uint8_t pin_;
    bool last_state_;
    bool state_;
    unsigned long last_edge_time_;

    int DEBOUNCE_COUNT_UP_;   // number of millis to consider it as up
    int DEBOUNCE_COUNT_DOWN_; // number of millis to consider it as down
};


ButtonRC button_a(A2);
ButtonRC button_b(A0);
ButtonRC button_c(A3);
ButtonRC button_d(A1);

void setup()
{
    log_init();
    log_logln("Garage Door...Ready");

    pinMode(13, OUTPUT);          
    digitalWrite(13, LOW);       

    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(2, LOW);     
    digitalWrite(3, LOW);     
    digitalWrite(4, LOW);     
    digitalWrite(5, LOW);     
}

void loop()
{
    static bool last_a = false;
    static bool last_b = false;
    static bool last_c = false;
    static bool last_d = false;
    

    bool a =  button_a.update();
    bool b =  button_b.update();
    bool c =  button_c.update();
    bool d =  button_d.update();


    if (last_a != a)
    {
        if (a)
        {
            log_logmsln("A On");
        }
        else
        {
            log_logmsln("A Off");
        }
        last_a = a;
    }

    if (last_b != b)
    {
        if (b)
        {
            log_logmsln("B On");
        }
        else
        {
            log_logmsln("B Off");
        }
        last_b = b;
    }

    if (last_c != c)
    {
        if (c)
        {
            log_logmsln("C On");
        }
        else
        {
            log_logmsln("C Off");
        }
        last_c = c;
    }

    if (last_d != d)
    {
        if (d)
        {
            log_logmsln("D On");
        }
        else
        {
            log_logmsln("D Off");
        }
        last_d = d;
    }




    digitalWrite(2, !a);     
    digitalWrite(3, !b);     
    digitalWrite(4, !c);     
    digitalWrite(5, !d);

    digitalWrite(13, a);     
}
