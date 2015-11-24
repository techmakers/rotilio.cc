#include "Particle_BaroSensor/Particle_BaroSensor.h"

void setup(){

	Serial.begin(115200);
	Serial.println("Termo & Pressure sample application v 1.0, press any key to proceed.");
	while (Serial.available()==0){
	    Particle.process() ;
	}
	Serial.println("run...") ;
	BaroSensor.begin();
	
}

void loop(){
    
	float pressure = BaroSensor.getPressure(OSR_8192) ;
	float temperature = BaroSensor.getTemperature(CELSIUS) ;
	Serial.print("Pressure:");
	Serial.println(pressure) ;
	Serial.print("Temperature:");
	Serial.println(temperature) ;
	
	BaroSensor.dumpDebugOutput() ;
	delay(5000) ;
}
