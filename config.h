#ifndef _CONFIG_h
#define _CONFIG_h

// Activate log by default
// Each file can override this setting 
// Eg: to disable log
//      #define DEBUG_LOG   0
#ifndef DEBUG_LOG
    #define DEBUG_LOG   1
#endif


#if( DEBUG_LOG  == 1 )
    #define log_init()    Serial.begin(9600)
    #define log_logln(a)  Serial.println(a)
    #define log_log(a)    Serial.print(a)

    #define log_logms(a)      Serial.print(millis());Serial.print(":");Serial.print(a)
    #define log_logmsln(a)    Serial.print(millis());Serial.print(":");Serial.println(a)
#else
    #define log_init()
    #define log_logln(a)
    #define log_log(a) 
    #define log_logms(a)
    #define log_logmsln(a)
#endif


    
#endif