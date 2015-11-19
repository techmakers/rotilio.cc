/*
    Rotilio.cc firmware
    carlo@techmakers.io

*/

#define FIRMWARE_CLASS      "ROTILIO SEED FIRMWARE"
#define FIRMWARE_VERSION    0.1

//STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

#include "OneWire/OneWire.h"
#include "Particle-BaroSensor/Particle-BaroSensor.h"

#define UICONFIGARRAYSIZE 12
#define UICONFIGVERSION 1

String uiConfig[UICONFIGARRAYSIZE] = {
    // page title
    "[{'t':'head','text':'General purpose application'}]",
    // sensors
    "[{'n':'temperature','l':'Temperature 1'},{'n':'exttemp','l':'Temperature 2'}]", // no 't' means, default:text, no 'l' means use 'n' as label
    "[{'n':'temperaturesetpoint','step':1, 'min':-10,'max':30,'l':'Set temperature','t':'slider'},{'n':'relais','t':'led','l':'Relais status'}]",
    "[{'n':'humidity','l':'Humidity'},{'n':'pressure','l':'Pressure'}]",    
    "[{'n':'photoresistor','l':'Light'}]",
    "[{'n':'button1','t':'led','l':'Button 1'},{'n':'button2','t':'led','l':'Button 2'},{'n':'switch','l':'Switch','t':'switch-readonly'}]", 
    "[{'n':'relaisIsInManualMode','l':'Relais is manual','t':'switch'}]",
    // actions
    "[{'t':'button','l':'Manual relais on','m':'setrelais:on'},{'t':'button','l':'Manual relais off','m':'setrelais:off'}]",  // Relais on or off, normally open, button label for open: Warm up, button label for close: Off
    "[{'t':'button','l':'Relais on for 1 second','m':'setrelais:1000'}]",    // Relais pulse on click, for 100 msec, button label: Open door
    "[{'t':'button','l':'2 Beeps','m':'setalarm:2'}]",
    "[{'t':'button','l':'3 Beeps','m':'setalarm:3'}]",
    "[{'t':'button','l':'Reset','m':'reset:now'}]"
};


#define DEBUG_INTERVAL_SECS 30

#define Si7020_ADDRESS    0x40
#define SAMPLE_NUMBER        1

#define BUTTON_1            D2
#define BUTTON_2            D6
#define SWITCH              D5
#define BUZZER              A0
#define RELAIS_SET          D4
#define RELAIS_RESET        D3
#define RELAIS_FDB          A3

#define TEMP_ADJUST_OFFSET  -0.7

#define LOOP_DELAY          1000.0


// variable Inizialization
double temperature = 99999 ; 
double extTemperature = 99999 ;
int humidity = 9999 ;
double pressure = -1 ;
double photoresistor = -1 ;
int trimmer = -1 ;
int button1 = false ;
int button2 = false ;
int switch0 = false ; 
int relais = false ;
int alarm = 0 ;

String status = "{}" ; 
String status_template = "{\"relaisIsInManualMode\":<relaisIsInManualMode>,\"temperature\":<temperature>,\"exttemp\":<exttemp>,\"humidity\":<humidity>,\"pressure\":<pressure>,\"photoresistor\":<photoresistor>,\"trimmer\":<trimmer>,\"button1\":<button1>,\"button2\":<button2>,\"switch\":<switch>,\"relais\":<relais>,\"alarm\":<alarm>,\"temperaturesetpoint\":<temperaturesetpoint>,\"humiditysetpoint\":<humiditysetpoint>,\"pressuresetpoint\":<pressuresetpoint>,\"trimmersetpoint\":<trimmersetpoint>,\"photoresistorsetpoint\":<photoresistorsetpoint>}" ;

// counter for debug function
int l=0;
int i=0;

void setup(){
    
    sendDebug(0) ;
    
    BaroSensor.begin();
      
    Particle.variable("status", status) ;
    
    Particle.function("message", message);      // see workMessage function     
    
    pinMode(BUZZER, OUTPUT);  
    pinMode(RELAIS_SET, OUTPUT);  
    pinMode(RELAIS_RESET, OUTPUT);     
    
    pinMode(RELAIS_FDB,INPUT_PULLUP);  
    pinMode(BUTTON_1, INPUT_PULLUP);     
    pinMode(BUTTON_2, INPUT_PU  LLUP);   
    pinMode(SWITCH, INPUT_PULLUP); 
    
    digitalWrite(BUZZER, LOW);
    digitalWrite(RELAIS_SET, LOW);
    digitalWrite(RELAIS_RESET, LOW);    

    Serial.begin(115200);
    Wire.begin();              // si connette col bus i2c
    
}

void loop(){
    
    if (alarm > 0){
        sound(1000,250);
        alarm-- ;
    }
    
    i++ ;
    if (i > DEBUG_INTERVAL_SECS){
        i = 0;
        l++ ;
        sendDebug(l) ;
    }
    
    temperature = temperatureRead() + TEMP_ADJUST_OFFSET;
    pressure = round((double)BaroSensor.getPressure(OSR_8192)*10)/10 ;
    extTemperature = BaroSensor.getTemperature() + TEMP_ADJUST_OFFSET;
    extTemperature = round(extTemperature*10)/10 ;
    humidity = humidityRead() ;
    photoresistor = analogRead(A1) ;
    trimmer = analogRead(A2) ;

    status = String(status_template) ; // copying ;

    status.replace("<temperature>",String(temperature));
    status.replace("<exttemp>",String(extTemperature)) ;
    status.replace("<humidity>",String(humidity));
    status.replace("<pressure>",String(pressure));
    status.replace("<photoresistor>",String(photoresistor));
    status.replace("<trimmer>",String(trimmer));
    status.replace("<switch>",String(switch0));
    status.replace("<relais>",String(relais));
    status.replace("<button1>",String(button1));
    status.replace("<button2>",String(button2));
    status.replace("<alarm>",String(alarm));
    status.replace("<temperaturesetpoint>",String(TempSetPoint));
    status.replace("<humiditysetpoint>",String(HumiSetPoint));
    status.replace("<pressuresetpoint>",String(PressureSetPoint));
    status.replace("<trimmersetpoint>",String(TrimmerSetPoint));
    status.replace("<photoresistorsetpoint>",String(LightSetPoint));
    status.replace("<relaisIsInManualMode>",String(relaisIsInManualMode));
    status.replace("<timerangeon>",String(timeRangeOn));
    
    delay(LOOP_DELAY); 
}

int message(String message){
    // ex: setrelais:on
    // ex: setrelais:off
    // ex: setalarm:10
    int colonPos = message.indexOf(":") ;
    if (colonPos < 0) return -1 ;                 // syntax error not found
    String command = message.substring(0,colonPos) ;
    String argument = message.substring(colonPos+1) ;
    
    if (command.equals("setrelais")){
        return setrelais(argument) ;
    } else if (command.equals("setalarm")){
        return setalarm(argument) ;
    } else if (command.equalsIgnoreCase("getuiconfig")){
        return sendUIConfig();
    } else if (command.equalsIgnoreCase("reset")){
        System.reset();
        return 0;
    } else {
        return -2 ;                         // command not found
    }
}

void sound(int cicles,int period){ // emette il suono .. 
    for(int n=0;n<cicles;n++) {
        digitalWrite(BUZZER, HIGH);
        delayMicroseconds(period);
        digitalWrite(BUZZER, LOW);
        delayMicroseconds(period);           
    }
}

double temperatureRead(void){
    int accumulo=0;
    for(byte n=0;n<SAMPLE_NUMBER;n++)
          {
            Wire.beginTransmission(Si7020_ADDRESS);   // "start" i2c
            Wire.write(0xF3);                           // comando lettura temperatura
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(Si7020_ADDRESS, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    double res = ((175.72*(accumulo/SAMPLE_NUMBER))/655  36)-46.85;   // calcola la media e converte la lettura grezza in temperatura (gradi centigradi)
    return round(res*10)/10 ;
}

double humidityRead(void){
    int accumulo=0;
    for(byte n=0;n<SAMPLE_NUMBER;n++)
          {
            Wire.beginTransmission(Si7020_ADDRESS);   // "start" i2c
            Wire.write(0xF5);                           // comando lettura umidita'
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(Si7020_ADDRESS, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    
    return ((125*(accumulo/SAMPLE_NUMBER))/655  36)-6;   // calcola la media e converte la lettura grezza in umidita' (% relativa)     
}

int setalarm(String command){
    alarm = command.toInt();
    return alarm ;
}

void setRelaisOn(bool force){
    digitalWrite(RELAIS_SET, HIGH);
}

void setRelaisOff(){
    digitalWrite(RELAIS_SET, LOW);
}

int setrelais(String command){
    int cmd = LOW ;
    if (command.equalsIgnoreCase("on"))  {
        cmd = HIGH ;
        setRelaisOn(true) ;
    } else if (command.equalsIgnoreCase("off")){
        setRelaisOff() ;
    } else {
        cmd = command.toInt() ;
        setRelaisOn(true) ;
        delay(cmd) ;
        setRelaisOff() ;
    }
    return cmd ;
}

int sendUIConfig(){
    for (int iii=0;iii<UICONFIGARRAYSIZE;iii++){
        String row = "{'id':" + String(iii) + ",'c':" + uiConfig[iii] + "}" ;
        Particle.publish("uiConfig",row,60,PRIVATE) ;
        delay(500) ; // needed to avoid message drops by Particle cloud
    }
    return UICONFIGVERSION ;
}

void sendDebug(int count){
        String debugMsg = FIRMWARE_CLASS + String(",V ") + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() ;
        Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
}