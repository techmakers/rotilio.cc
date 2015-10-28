
/*

 test per scheda scuola Carlo


 su seriale USB manda gli stati delle varie cose.
 
 poi:
 
 SE SWITCH IN POSIZIONE "ON": ATTIVA AL CICALINO
 
 SE PULSANTE "P2" PREMUTO: ATTIVA RELAIS

*/

#define INDIRIZZO_Si7020    0x40
#define NUMERO_LETTURE      5

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

double temperature = 99999 ; 
double humidity = 9999 ;
int photoresistor = -1 ;
int trimmer = -1 ;
int button1 = false ;
int button2 = false ;
int switch0 = false ; 
int relais = false ;
int alarm = 0 ;


// accessToken = 397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b
// deviceId = 30001c000647343232363230

// curl "https://api.particle.io/v1/devices/30001c000647343232363230/temperature?access_token=397c2c0c884ee6ed7b0c33acc25ff890fa59cf6b"


void setup()
{
    
    Particle.variable("temperature", temperature);
    Particle.variable("humidity", humidity);
    Particle.variable("photoresistor", photoresistor);
    Particle.variable("trimmer", trimmer);
    Particle.variable("switch", switch0) ;
    Particle.variable("relais", relais) ;
    Particle.variable("button1", button1) ;
    Particle.variable("button2", button2) ;
    Particle.variable("alarm", alarm) ;
    
    Particle.function("setrelais", setrelais); // accepts "on" to switch on the relais 
    Particle.function("setalarm", setalarm); // accept a nummber of beeps to emit (one per second)
    Particle.function("relaispulse", relaispulse); // accept a number of milliseconds for releais on
    
    
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

void loop()
{

    if (alarm > 0){
        sound(1000,250);
        alarm-- ;
    }
    
    double lastTemp = temperature ;
    temperature = temperatureRead() ;
    if (abs(lastTemp - temperature) > tempDeltaEvent) Particle.publish("temperature", String(temperature), 60, PRIVATE);
    
    double lastHumi = humidity ;
    humidity = humidityRead() ;
    if (abs(lastHumi - humidity) > humiDeltaEvent) Particle.publish("humidity", String(humidity), 60, PRIVATE);
    
    int lastPhotoRes = photoresistor ;
    photoresistor = analogRead(A1) ;
    if (abs(lastPhotoRes - photoresistor) > photoDeltaEvent) Particle.publish("photoresistor", String(photoresistor), 60, PRIVATE);
    
    int lastTrimmer = trimmer ;
    trimmer = analogRead(A2) ;
    if (abs(lastTrimmer - trimmer) > trimmerDeltaEvent) Particle.publish("trimmer", String(trimmer), 60, PRIVATE);
    
    int lastSwitch = switch0 ;
    switch0 = digitalRead(SWITCH) == HIGH ;
    if (lastSwitch != switch0) Particle.publish("switch", switch0 ? "true" : "false", 60, PRIVATE);
    
    int lastRelais = relais ;
    relais = digitalRead(RELAIS_FDB) == HIGH ;
    if (lastRelais != relais) Particle.publish("relais", relais ? "true" : "false", 60, PRIVATE);
    

    if(digitalRead(SWITCH)==LOW) sound(500,340) ;
    
    if (digitalRead(PULSANTE_1)==LOW){
        sound(1000,170) ;
        button1 = !button1 ;
        Particle.publish("button1", button1 ? "true" : "false", 60, PRIVATE);
    }
    
    if (digitalRead(PULSANTE_2)==LOW){
        sound(1000,170) ;
        button2 = !button2 ;
        Particle.publish("button2", button2 ? "true" : "false", 60, PRIVATE);
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


    delay(1000); 
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
    if (alarm > 0) {
        sound(1000,250);
        alarm-- ;
    }
    return command.toInt() ;
}

int setrelais(String command){
    int cmd = LOW ;
    if (command == "on")  {
        cmd = HIGH ;
    } 
    digitalWrite(RELAIS_SET, cmd);
    return cmd ;
}

int relaispulse(String command){
    int duration = command.toInt() ;
    digitalWrite(RELAIS_SET, HIGH);
    delay(duration) ;
    digitalWrite(RELAIS_SET, LOW);
    return duration ;
}

