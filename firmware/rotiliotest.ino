/*
    Rotilio.cc firmware
    carlo@techmakers.io

*/

#define FIRMWARE_VERSION    0.23

#define LOOP_DELAY          1000

#define ARRAYSIZE           8

#define DEBUG_INTERVAL_SECS 30

#define INDIRIZZO_Si7020    0x40
#define NUMERO_LETTURE      1

#define PULSANTE_1          D2
#define PULSANTE_2          D6
#define SWITCH              D5
#define BUZZER              A0
#define RELAIS_SET          D4
#define RELAIS_RESET        D3
#define RELAIS_FDB          A3

#define PREMUTO             LOW
#define NON_PREMUTO         HIGH


#define tempDeltaEvent      0.1
#define humiDeltaEvent      1
#define photoDeltaEvent     200
#define trimmerDeltaEvent   100

#define RELAIS_OVERRIDE_NO  -1
#define RELAIS_OVERRIDE_ON  1

// relais override command
// 0 : no override
// 1 : override to value by setRalais

int relaisOverride = RELAIS_OVERRIDE_NO ;

// variable Inizialisation
double temperature = 99999 ; 
double humidity = 9999 ;
double pressure = -1 ;
int photoresistor = -1 ;
int trimmer = -1 ;
int button1 = false ;
int button2 = false ;
int switch0 = false ; 
int relais = false ;
int alarm = 0 ;
int actualTimeRangeIndex = -1 ;
int actualExpireDateIndex = -1 ;

String status = "{}" ;
String status_template = "{\"timerange\":<timerange>,\"expiredate\":<expiredate>,\"temperature\":<temperature>,\"humidity\":<humidity>,\"pressure\":<pressure>,\"photoresistor\":<photoresistor>,\"trimmer\":<trimmer>,\"button1\":<button1>,\"button2\":<button2>,\"switch\":<switch>,\"relais\":<relais>,\"alarm\":<alarm>}" ;

// storage for last published values 
double lastTemp = -999999 ;  
double lastHumi = -999999 ;
double lastPressure = -99999 ;
int lastPhotoRes = -999999 ;
int lastTrimmer = -999999 ;
int lastTimeRangeIndex = -1 ;
int lastExpireDateIndex = -1 ;

// counter for debug function
int l=0;
int i=0;

int timeZone = 1 ;

/*

SET POINTS

T = TEMP            es: T=22.1  = setpoint 22.1 °C 
H = HUMI            es: H=80    = setpoint 80% 
P = PRESSIONE       es: P=1800  = setpoint 1800 mbar
R = TRIMMER         es: R=3400  = setpoint 3400 units (adc, from 0 to 4095)
L = LIGHT           es: L=1200  = setpoint 1200 units (adc, from 0 to 4095)
S = SWITCH          es: S=1     = setpoint switch on (1=on, 0=off)
A = P1              es: A=1     = setpoint button1 == true
B = P2              es: B=0     = setpoint button2 == false


*/

String timeRanges[ARRAYSIZE] = { 
    //"00:00-23:59|MTWTFSS|JFMAMJJASOND", // alwais on, alwais off is : 00:00-00:00|mtwtfss|jfmamjjasond
    "0:06:00-07:30|sMTWTFs|JFmamjjasOND|T=21", // working days morning
    "1:07:31-12:59|sMTWTFs|JFmamjjasOND|T=19",
    "2:13:00-14:00|SMTWTFS|JFmamjjasOND|T=21", // lunch time every day
    "3:14:01-18:59|SMTWTFS|JFmamjjasOND|T=19",
    "4:19:00-22:00|SMTWTFS|JFmamjjasOND|T=21", // dinner time every day
    "5:22:00-05:59|sMTWTFs|JFmamjjasOND|T=19", // sleeping time working days
    "6:23:30-08:59|SmtwtfS|JFmamjjasOND|T=19", // sleeping time weekend
    "7:09:00-10:00|SmtwtfS|JFmamjjasOND|T=21", // weekend morning
};

String expireDates[ARRAYSIZE] = {
    "0:2015-06-30|08:00:00|2015-07-30|16:00:00|T=22", // set temperature setpoint to 22°C from 30 June 2015 8 o'clock untile 30 July 16 o'clock
    "",
    "",
    "",
    "",
    "",
    "",
    ""
};

int daysInMonthOfYear(int month, int year){
    int months[12] = {
        31,
        28,
        31,
        30,
        31,
        30,
        31,
        31,
        30,
        31,
        30,
        31
    };
    bool isLeapYear = (year/4) != round(year/4) ; 
    if (month == 2 && isLeapYear) return 29 ;
    return months[month] ;
}

long unsigned millisAtDate(String atDate){

    // 2015-06-30|08:00:00
    // 0123456789012345678
    //           1       

    int millisInSecond = 1000 ;
    int millisInMinute = 60 * millisInSecond ;
    int millisInHour = 60 * millisInMinute ;
    int millisInDay = 24 * millisInHour ;

    int year = atDate.substring(0,4).toInt();
    int month = atDate.substring(5,7).toInt();
    int day = atDate.substring(8,10).toInt();
    int hour = atDate.substring(11,13).toInt();
    int minute = atDate.substring(14,16).toInt();
    int second = atDate.substring(17,19).toInt();

    bool isLeapYear = (year/4) != round(year/4) ; 

    long unsigned int totDays = ((year - 1) * 365 + isLeapYear ? 1 : 0) + daysInMonthOfYear(month-1,year) + day - 1 ;
    long unsigned int millisAt = totDays * millisInDay + hour * millisInHour + minute * millisInMinute + second * millisInSecond ;

    return millisAt ;
}

bool isInExpireDate(unsigned long now, String eDate){
    /*
        eDate example: "0:2015-06-30|08:00:00|2015-07-30|16:00:00|T=22"
                        01234567890123456789012345678901234567890123456
                                  1         2         3         4
    */
    // calculating milliseconds from
    long unsigned millisFrom = millisAtDate(eDate.substring(2,21)) ;
    long unsigned millisTo = millisAtDate(eDate.substring(22,41)) ;

    bool yesItIs = (millisFrom <= now) && (now <= millisTo) ;

    return yesItIs ;

}

bool isInTimeRange(unsigned long now, String tRange){
    
    bool yesItIs = false ;
   
    /*
    example "4:01:16-01:44|MTWTFSS|JFMAMJJASOND|T=22.1"
             01234567890123456789012345678901234567890
                       1         2         3         4
                       
    means: from 6.50 to 8.30 of Monday, twensday,Wendsday,Thursday,Friday on Jan,Feb,Oct,Nov,Dec
    */
    
    // check for weekday
    
    int dayOfWeek = Time.weekday(now) ;

    yesItIs =  tRange.charAt(13+dayOfWeek) < 97 ; // Uppercase char in this position means the day of the week is active in the time range
    
    if (!yesItIs) return false ;
    
    
    // check fom month
    int month = Time.month(now) ;
    
    yesItIs = tRange.charAt(21+month) < 97 ;
    
    if (!yesItIs) return false ;
    
    
    // check for hour and minute
    int hourFrom = tRange.substring(2,4).toInt();
    int minuteFrom =  tRange.substring(5,7).toInt();
    int hourTo = tRange.substring(8,10).toInt() ;
    int minuteTo = tRange.substring(11,13).toInt() ;
    
    int hourMinuteFrom = hourFrom * 60 + minuteFrom ;
    int hourMinuteTo = hourTo * 60 + minuteTo ;
    
    int hourNow = Time.hour(now) + timeZone ;
    int minuteNow = Time.minute(now) ;
    
    int hourMinuteNow = minuteNow + hourNow * 60 ;
    
    if (hourMinuteFrom > hourMinuteTo){ 
        //23:00-08:59 @ 23:34
        
        int midNight = 23*60+60 ;
        int delta = midNight - hourMinuteFrom ;
        hourMinuteFrom = 0;
        hourMinuteTo = hourMinuteTo+delta;
        hourMinuteNow = hourMinuteNow + delta ;
        if (hourMinuteNow > midNight) {
            hourMinuteNow = hourMinuteNow - midNight ;
        }
    }
    
    yesItIs = (hourMinuteFrom <= hourMinuteNow) && (hourMinuteNow <= hourMinuteTo) ;
    
    return yesItIs ;
}

int getTimeRangeForNow(){
    
    if (digitalRead(SWITCH)==LOW) return -3 ; // no local time range but command only from the cloud
    
    unsigned long now = Time.now() ;
    int res = -1 ;
    for (int ii=0;ii<ARRAYSIZE;ii++){
        if (!timeRanges[ii].equals("")){
            if (isInTimeRange(now,timeRanges[ii])){
                res = ii ;
            }
        }
    }
    return res ;
}

int getExpireDateForNow(){
    
    if (digitalRead(SWITCH)==LOW) return -3 ; // no local expire date but command only from the cloud
    
    unsigned long now = Time.now() ;
    int res = -1 ;
    for (int ii=0;ii<ARRAYSIZE;ii++){
        if (!expireDates[ii].equals("")){
            if (isInExpireDate(now,expireDates[ii])){
                res = ii ;
            }
        }
    }
    return res ;
}

int setTimeRange(String timeRange){
    int index = timeRange.substring(0,1).toInt() ;
    if (index >= ARRAYSIZE){
        Particle.publish("error: timerange overflow",String(index),60,PRIVATE) ;
        return -1;
    }
    timeRanges[index] = timeRange ;
    return index ;
}

int setExpireDate(String expireDate){
    int index = expireDate.substring(0,1).toInt() ;
    if (index >= ARRAYSIZE){
        Particle.publish("error: expire date overflow",String(index),60,PRIVATE) ;
        return -1;
    }
    expireDates[index] = expireDate ;
    return index ;
}

void workTimeRange(){
    if (actualTimeRangeIndex < 0 ) return ; // no time range active
    String actualTimeRange = timeRanges[actualTimeRangeIndex] ;
    char c = actualTimeRange.charAt(35) ;
    switch (c)
    {
        
        case 'T': { // temp
            float TempSetPoint = actualTimeRange.substring(37).toFloat() ;
            //Particle.publish("Tempsetpoint",String(TempSetPoint),60,PRIVATE) ;
            if (temperature < TempSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'H': { // humidity
            int HumiSetPoint = actualTimeRange.substring(37).toInt() ;
            if (humidity < HumiSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'P': { // pressure
            int PressuerSetPoint = actualTimeRange.substring(37).toInt() ;
            if (pressure < PressuerSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'R': { // trimmer
            int TrimmerSetPoint = actualTimeRange.substring(37).toInt() ;
            if (trimmer < TrimmerSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'L': { // PhotoResistor
            int LightSetPoint = actualTimeRange.substring(37).toInt() ;
            if (photoresistor < LightSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'S': { // Switch
            int SwitchSetPoint = actualTimeRange.substring(37).toInt() ;
            if (SwitchSetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'A': { // button1
            int Button1SetPoint = actualTimeRange.substring(37).toInt() ;
            if (Button1SetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'B': { // button2
            int Button2SetPoint = actualTimeRange.substring(37).toInt() ;
            if (Button2SetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        default: {
            Particle.publish("error:unknown setpoint in timeRange",actualTimeRange,60,PRIVATE) ;
        }
    }
}

void workExpireDate(){
    if (actualExpireDateIndex < 0 ) return ; // no expire date active
    String actualExpireDate = expireDates[actualExpireDateIndex] ;
    char c = actualExpireDate.charAt(42) ;
    switch (c)
    {
        
        case 'T': { // temp
            float TempSetPoint = actualExpireDate.substring(44).toFloat() ;
            //Particle.publish("Tempsetpoint",String(TempSetPoint),60,PRIVATE) ;
            if (temperature < TempSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'H': { // humidity
            int HumiSetPoint = actualExpireDate.substring(44).toInt() ;
            if (humidity < HumiSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'P': { // pressure
            int PressuerSetPoint = actualExpireDate.substring(44).toInt() ;
            if (pressure < PressuerSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'R': { // 
            int TrimmerSetPoint = actualExpireDate.substring(44).toInt() ;
            if (trimmer < TrimmerSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'L': { // PhotoResistor
            int LightSetPoint = actualExpireDate.substring(44).toInt() ;
            if (photoresistor < LightSetPoint) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        
        case 'S': { // Switch
            int SwitchSetPoint = actualExpireDate.substring(44).toInt() ;
            if (SwitchSetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'A': { // button1
            int Button1SetPoint = actualExpireDate.substring(44).toInt() ;
            if (Button1SetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }

        case 'B': { // button2
            int Button2SetPoint = actualExpireDate.substring(44).toInt() ;
            if (Button2SetPoint > 0) {
                setrelais("on") ;
            } else {
                setrelais("off") ;
            }
            break;
        }
        default: {
            Particle.publish("error:unknown setpoint in expireDate",actualExpireDate,60,PRIVATE) ;
        }
    }
}

// accessToken = 397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b
// deviceId = 30001c000647343232363230

// curl "https://api.particle.io/v1/devices/30001c000647343232363230/temperature?access_token=397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b"

void sendDebug(int count){
        String debugMsg = "V " + String(FIRMWARE_VERSION) + "," + String(count) + "," +WiFi.SSID() + "," + WiFi.RSSI() ;
        Particle.publish("debugmsg", debugMsg, 60, PRIVATE);
}

String api_help = "\nsetrelais:on|off|auto|<msec on>,\nsetalarm:<beeps>" ;

int workMessage(String message){
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
    } else {
        return -2 ;                         // command not found
    }
}



void setup()
{
    sendDebug(0) ;
    /*
    Particle.variable("expiredate", actualExpireDateIndex) ;
    Particle.variable("timerange", actualTimeRangeIndex) ;
    Particle.variable("temperature", temperature);
    Particle.variable("humidity", humidity);
    Particle.variable("photoresist", photoresistor);
    Particle.variable("trimmer", trimmer);
    Particle.variable("switch", switch0) ;
    Particle.variable("relais", relais) ;
    Particle.variable("button1", button1) ;
    Particle.variable("button2", button2) ;
    Particle.variable("alarm", alarm) ;
    */
    
    
    
    Particle.variable("status", status) ;
    Particle.variable("message_help", api_help) ;
    
    Particle.function("message", workMessage); // accept a nummber of beeps to emit (one per second)
    Particle.function("setTimeRange",setTimeRange); // accept a string to set the desiderd time range
    Particle.function("setExpDate", setExpireDate); // accept a number of milliseconds for releais on
    
    
    pinMode(BUZZER, OUTPUT);  
    pinMode(RELAIS_SET, OUTPUT);  
    pinMode(RELAIS_RESET, OUTPUT);     
    
    pinMode(RELAIS_FDB,INPUT_PULLUP);  
    pinMode(PULSANTE_1, INPUT_PULLUP);     
    pinMode(PULSANTE_2, INPUT_PULLUP);   
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
    
    actualTimeRangeIndex = getTimeRangeForNow();
    if (actualTimeRangeIndex != lastTimeRangeIndex){
        if (actualTimeRangeIndex > -1){
            String actualTimeRange = timeRanges[actualTimeRangeIndex] ;
            Particle.publish("timeRangeEntered",actualTimeRange,60,PRIVATE) ;
        }
        lastTimeRangeIndex = actualTimeRangeIndex ;
    }

    actualExpireDateIndex = getExpireDateForNow();
    if (actualExpireDateIndex != lastTimeRangeIndex){
        if (actualExpireDateIndex > -1){
            String actualExpireDate = expireDates[actualExpireDateIndex] ;
            Particle.publish("expireDateEntered",actualExpireDate,60,PRIVATE) ;
        }
        lastExpireDateIndex = actualExpireDateIndex ;
    }
    
    i++ ;
    if (i > DEBUG_INTERVAL_SECS){
        i = 0;
        l++ ;
        sendDebug(l) ;
    }
    
    double delta = 0;
    
    temperature = temperatureRead() ;
    delta = lastTemp - temperature ;
    if (delta < 0) delta = delta * -1 ;
    if (delta >= tempDeltaEvent) {
        lastTemp = temperature ;
        Particle.publish("temperatureChanged", String(temperature), 60, PRIVATE);
    }
    

    humidity = humidityRead() ;
    delta = lastHumi - humidity ;
    if (abs(delta) > humiDeltaEvent) {
        lastHumi = humidity ;
        Particle.publish("humidityChanged", String(humidity), 60, PRIVATE);
    }
    

    photoresistor = analogRead(A1) ;
    if (abs(lastPhotoRes - photoresistor) >= photoDeltaEvent) {
        lastPhotoRes = photoresistor;
        Particle.publish("photoresistChanged", String(photoresistor), 60, PRIVATE);
    }
    
    
    trimmer = analogRead(A2) ;
    if (abs(lastTrimmer - trimmer) >= trimmerDeltaEvent) {
        lastTrimmer = trimmer;
        Particle.publish("trimmerChanged", String(trimmer), 60, PRIVATE);
    }
    
    int lastSwitch = switch0 ;
    switch0 = digitalRead(SWITCH) == HIGH ;
    if (lastSwitch != switch0) Particle.publish("switchChanged", switch0 ? "true" : "false", 60, PRIVATE);
    
    int lastRelais = relais ;
    relais = digitalRead(RELAIS_FDB) == HIGH ;
    if (lastRelais != relais) Particle.publish("relaisChanged", relais ? "true" : "false", 60, PRIVATE);
    

    //if(digitalRead(SWITCH)==LOW) sound(500,340) ;
    
    if (digitalRead(PULSANTE_1)==LOW){
        sound(1000,170) ;
        button1 = !button1 ;
        Particle.publish("button1Changed", button1 ? "true" : "false", 60, PRIVATE);
    }
    
    if (digitalRead(PULSANTE_2)==LOW){
        sound(1000,170) ;
        button2 = !button2 ;
        Particle.publish("button2Changed", button2 ? "true" : "false", 60, PRIVATE);
    }

/*
    if(digitalRead(PULSANTE_2)==PREMUTO) digitalWrite(RELAIS_SET, HIGH); else digitalWrite(RELAIS_SET, LOW);   
    Serial.print("TEMPERATURA: "); Serial.print(temperature);   Serial.println(" C");     
    Serial.print("UMIDITA'   : "); Serial.print(umidity);   Serial.println(" %");         
    Serial.print("FOTORESISTENZA:-> ");  Serial.println(photores);
    Serial.print("POTENZIOMETRO :-> ");  Serial.println(trimmer);   
    Serial.print("PULSANTE 'P1' "); if(digitalRead(PULSANTE_1)==HIGH) Serial.print("*NON*"); Serial.println(" PREMUTO");   
    Serial.print("PULSANTE 'P2' "); if(digitalRead(PULSANTE_2)==HIGH) Serial.print("*NON*"); Serial.println(" PREMUTO");       
    Serial.print("SWITCH "); if(digitalRead(SWITCH)==HIGH) Serial.println(" *OFF*"); else Serial.println(" *ON* - BUZZER ATTIVO");     
    Serial.print("IL RELAIS RISULTA "); if(digitalRead(RELAIS_FDB)==LOW) Serial.print(" *NON* "); Serial.println(" ECCITATO");     
    Serial.println(""); Serial.println(""); 
*/
    
    if (relaisOverride == RELAIS_OVERRIDE_NO){ // if the switch is off, we dont work time range and expire date
        if (actualExpireDateIndex > -1){
            // priority to ExpireDate on time range
            workExpireDate() ;
        } else {
            workTimeRange() ;
        }        
    }
    
    status = String(status_template) ; // copying ;

    status.replace("<expiredate>",String(actualExpireDateIndex));
    status.replace("<timerange>",String(actualTimeRangeIndex));
    status.replace("<temperature>",String(temperature));
    status.replace("<humidity>",String(humidity));
    status.replace("<pressure>",String(pressure));
    status.replace("<photoresistor>",String(photoresistor));
    status.replace("<trimmer>",String(trimmer));
    status.replace("<switch>",String(switch0));
    status.replace("<relais>",String(relais));
    status.replace("<button1>",String(button1));
    status.replace("<button2>",String(button2));
    status.replace("<alarm>",String(alarm));
    
    delay(LOOP_DELAY); 
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
    for(byte n=0;n<NUMERO_LETTURE;n++)
        {
            Wire.beginTransmission(INDIRIZZO_Si7020);   // "start" i2c
            Wire.write(0xF3);                           // comando lettura temperatura
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(INDIRIZZO_Si7020, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    double res = ((175.72*(accumulo/NUMERO_LETTURE))/65536)-46.85;   // calcola la media e converte la lettura grezza in temperatura (gradi centigradi)
    return round(res*10)/10 ;
}

double humidityRead(void){
    int accumulo=0;
    for(byte n=0;n<NUMERO_LETTURE;n++)
        {
            Wire.beginTransmission(INDIRIZZO_Si7020);   // "start" i2c
            Wire.write(0xF5);                           // comando lettura umidita'
            Wire.endTransmission();                     // "stop" i2c
            delay(20);                                  // 20msec richiesti dal dispositivo per acquisire il valore
            Wire.requestFrom(INDIRIZZO_Si7020, 3);      // chiede l'invio di 3 bytes
            unsigned char msb = Wire.read();
            unsigned char lsb = Wire.read();
            unsigned char chk = Wire.read();            // non utilizzato
        
            int sot=(((int) msb<<8) + (int) lsb);       // crea variabile "raw data"
            accumulo=accumulo+sot;                      // la accumula per poi farne la media
        }
    
    return ((125*(accumulo/NUMERO_LETTURE))/65536)-6;   // calcola la media e converte la lettura grezza in umidita' (% relativa)     
}

int setalarm(String command){
    alarm = command.toInt();
    return alarm ;
}

/*

    Accept this command:
    on : relais on, regardless timerange or expire date
    off: relais off, regardless timerange or expire date
    
    auto: rules from timerange or expire date

*/
int setrelais(String command){
    
    int cmd = LOW ;
    if (command.equalsIgnoreCase("on"))  {
        relaisOverride = RELAIS_OVERRIDE_ON ;
        cmd = HIGH ;
        digitalWrite(RELAIS_SET, HIGH);
    } else if (command.equalsIgnoreCase("off")){
        relaisOverride = RELAIS_OVERRIDE_ON ;
        digitalWrite(RELAIS_SET, LOW);
    } else if (command.equalsIgnoreCase("auto")){
        relaisOverride = RELAIS_OVERRIDE_NO ;
        cmd = -1 ;
    } else {
        cmd = command.toInt() ;
        relaisOverride = RELAIS_OVERRIDE_ON ;
        digitalWrite(RELAIS_SET, HIGH);
        delay(cmd) ;
        digitalWrite(RELAIS_SET, LOW);
        relaisOverride = RELAIS_OVERRIDE_NO ;
    }
    return cmd ;
}