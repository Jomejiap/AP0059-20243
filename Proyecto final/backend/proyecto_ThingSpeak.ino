  #include "ThingSpeak.h"
  #include "WiFi.h"
  
  #include <DHT.h>
  
  const char* ssid = "jakson";                      
  const char* password = "123456789";
  
  unsigned long channelID = 2838382;                
  const char* WriteAPIKey = "A6GGSDNYTC5IFQVV";
  
  #define DHTPIN 4     
  #define DHTTYPE DHT11
  #define MQ135_PIN 26
  
  float temperatura;
  float humedad;
  float ppmCO2;
  float rain;
  float light;
  const int rainSensorPin = 27; 
  const int lightSensorPin = 32;
  
  const float RLOAD = 10.0;    // Resistencia de carga en kΩ
  const float RZERO = 76.63;   // Resistencia en aire limpio
  const float PARA = 116.6020682;
  const float PARB = 2.769034857;
  
  WiFiClient cliente;
  DHT dht(DHTPIN, DHTTYPE);
  
  void inicializarDHT11() {
    dht.begin();
    Serial.println("Sensor DHT11 inicializado!");
  }
  
  void inicializarMQ135() {
      pinMode(MQ135_PIN, INPUT);
      // Tiempo de calentamiento inicial
      Serial.println("Calentando sensor MQ135 (20 segundos)...");
      delay(20000);
      Serial.println("Sensor MQ135 listo!");
  }
  
  //Función para lectura de temperatura
  float leerTemperatura() {
    float temperatura = dht.readTemperature();
    if (isnan(temperatura)) {
      Serial.println("Error al leer temperatura!");
      return -999; // Valor de error
    }
    return temperatura;
  }
  
  //Función para lectura de humedad del medio ambiente
  float leerHumedad() {
    float humedad = dht.readHumidity();
    if (isnan(humedad)) {
      Serial.println("Error al leer humedad!");
      return -999; // Valor de error
    }
    return humedad;
  }
  
  //Función para lectura de CO2 en el medio ambiente en partes por millón
  float leerCO2() {
      // Leer valor analógico y convertir a voltaje
      float valorADC = analogRead(MQ135_PIN);
      float voltaje = (valorADC / 4095.0) * 3.3;
  
      // Calcular la resistencia del sensor
      float rs = ((3.3 * RLOAD) / voltaje) - RLOAD;
  
      // Calcular PPM de CO2
      float ratio = rs / RZERO;
      float ppm = PARA * pow((float)ratio, -PARB);
  
      return ppm;
  }
  
  // Función para leer el sensor de lluvia
  float readRainSensor() {
      float rainstate = analogRead(rainSensorPin);
      return rainstate;
  }
  
  // Función para leer el sensor de luz
  float readLightSensor() {
      int light = analogRead(lightSensorPin);
      return light;
  }
  
  
  void setup() {
    Serial.begin(115200);
    inicializarDHT11();
    inicializarMQ135();
    pinMode(rainSensorPin, INPUT); 
    pinMode(lightSensorPin, INPUT);
    
    WiFi.begin(ssid,password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("Wifi conectado!");
    ThingSpeak.begin(cliente);
  
  }
  
  void loop() {
    temperatura = leerTemperatura();
    humedad = leerHumedad();
    ppmCO2 = leerCO2();
    light= readLightSensor();
    rain = readRainSensor();
    
    enviarDatosTS(humedad,temperatura,ppmCO2,light,rain);
  }
  
  
  //Enviar datos al puerto Serial y a ThingSpeak
  void enviarDatosTS(float humedad,float temperatura, float ppmCO2, float light, float rain) {
    //Enviar datos a ThingSpeak
    ThingSpeak.setField(1,humedad);
    ThingSpeak.setField(2,temperatura);
    ThingSpeak.setField(3,ppmCO2);
    ThingSpeak.setField(4,light);
    ThingSpeak.setField(5,rain);
    
    ThingSpeak.writeFields(channelID,WriteAPIKey);
    Serial.println("Datos enviados a ThingSpeak!");
    delay(14000);

    //Imprimir los datos en el monitor serial
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print("°C, Humedad: ");
    Serial.print(humedad);
    Serial.println("%");
    Serial.print("CO2 (ppm): ");
    Serial.println(ppmCO2); 
    Serial.print("Valor del sensor de lluvia: ");
    Serial.println(rain);
    Serial.print("Valor del sensor de luz: ");
    Serial.println(light);
  }
