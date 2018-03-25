#include "DHT.h"
#define DHTPIN gv_PIN_DHT
#define DHTTYPE DHT22

const int LIGHTSENSORPIN = gv_PIN_LUXSENSOR;
const int MOVINGSENSORPIN = gv_PIN_PIRSENSOR;

//  DHT Sensor pinout, virado para ele, esq - VCC, seguir data, depois GND e GND
    // nao funciona no PIN D0
    // funciona D1, D2, D3, D4, D5, D6, D7, D8
// PIR Sensor, com o cap e o Q para nos, esq - vcc, meio signal, dir (lado cap) GND
    // funciona D0 funciona bem...
// LUX Sensor, tem na placa..
    // so no A0

int pirValue = 0;
int pirState = LOW;             // we start, assuming no motion detected

// const int

DHT dht(DHTPIN, DHTTYPE);


void sensorsSetup(){
  dht.begin();
  pinMode(MOVINGSENSORPIN, INPUT);
  pinMode(LIGHTSENSORPIN, INPUT);
}


void luxLoop(){
  float reading = analogRead(LIGHTSENSORPIN); //Read light level
  float square_ratio = reading / 1023.0;      //Get percent of maximum value (1023)
  square_ratio = pow(square_ratio, 2.0);      //Square to make response more obvious
  gv_lux = analogRead(LIGHTSENSORPIN) * 0.9765625;  // 1000/1024
  // analogWrite(LEDPIN, 255.0 * square_ratio);  //Adjust LED brightness relatively
  // Serial.println(reading);
  Serial.print("Readling Lux: ");
  Serial.println(gv_lux);

  String payload = "{";
  payload += "\"luminosity\":";
  payload += gv_lux;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish(gv_sensorsTopicTeleLUX, attributes);
  gv_luxdata = payload;
}

void dhtLoop(){
  // Serial.println("Collecting temperature data.");
  // Reading temperature or humidity takes about 250 milliseconds!
  // Measured by AP it was about 275 milliseconds!!

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.println(" *C ");

  String temperature = String(t);
  String humidity = String(h);
  String heatIndex = String(hic);

  // Just debug messages
  // Serial.print( "Sending temperature and humidity : [" );
  // Serial.print( temperature ); Serial.print( "," );
  // Serial.print( humidity );
  // Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  // payload += "\"humidity\":"; payload += humidity;
  payload += "\"humidity\":"; payload += humidity; payload += ",";
  payload += "\"heatIndex\":"; payload += heatIndex;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( gv_sensorsTopicTeleDHT, attributes );
  gv_dhtdata = payload;
  // gv_dhtdata =
  // Serial.print("Attribute: ");
  // Serial.println( attributes );


}

void pirLoop(){
  pirValue = digitalRead(MOVINGSENSORPIN);  // read input value
  if (pirValue == HIGH) {            // check if the input is HIGH
    // digitalWrite(ledPin3, HIGH);  // turn LED ON
    if (pirState == LOW) {
      // we have just turned on
      Serial.println("Motion detected!");
      // We only want to print on the output change, not state
      pirState = HIGH;
      client.publish( gv_sensorsTopicTeleMOV, "on" );
      gv_pirdata = "Moving";

    }
  } else {
    // digitalWrite(ledPin3, LOW); // turn LED OFF
    if (pirState == HIGH){
      // we have just turned of
      Serial.println("Motion ended!");
      // We only want to print on the output change, not state
      pirState = LOW;
      gv_pirdata = "clear";
      client.publish( gv_sensorsTopicTeleMOV, "off" );

    }
  }

}


void sensorsLoop(){
  long now = millis();

  if ( now - lastSendDHT > valTimerDHT ) { // Update and send only after 5 seconds
    dhtLoop();
    lastSendDHT = now;
    now = millis(); // Because dhtloop takes about 275ms is better to register the new value for the next timers
  }
  if ( now - lastSendLUX > valTimerLUX ) { // Update and send only after 300 seconds
    luxLoop();
    lastSendLUX = now;
  }

  if ( now - lastSendPIR > valTimerPIR ) { // Update and send only after 1 seconds
    pirLoop();
    lastSendPIR = now;
  }

}
