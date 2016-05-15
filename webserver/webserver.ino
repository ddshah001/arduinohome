/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

Circuit:
* Ethernet shield attached to pins 10, 11, 12, 13
Wire   Pin
Echo    7
Trig    8
LDR     A0
DHT     2



 */

#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

#define DHTPIN 2     // what digital pin we're connected to DHT
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
#define LEDPin 13 // Onboard LED
int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance
int tanklevel = 100;
bool tanksensor;

int ldrpin = A0;
int ldrvalue;
bool light_m;

float TempC,TempF,Humidity,hif,hic; //hif: Heat Index in Fahrenheit, hic: Heat Index in Celsius
bool dhtsensor;

int rel = 3;
String readString;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
   dht.begin();
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);
   pinMode(LEDPin, OUTPUT);
    pinMode(rel, OUTPUT);
    digitalWrite(rel, HIGH);
    tanksensor = false;
    dhtsensor = false;
    light_m = false;
}


void loop() {
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (readString.length() < 100) {
          //store characters to string
          readString += c;
          //Serial.print(c);
         }
        
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
       
        if (c == '\n') {
          //Call Sensor Functions 
         dist();
         ldr();
         dhtread();
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><title>Home View</title></head>");
          // output the value of each analog input pin
          
            //int sensorReading = 10;
            if(tanksensor==false)
            {
              client.print("Check Connection of Tank Sensor");
              client.println("<br />");
            }
            else
            {
            client.print("Tank Level(%):");
            client.print(tanklevel);
            client.print("  Dist To Water:");
            client.print(distance);
            client.println("<br />");
            }
         // client.print("LDR Reading :");
          //  client.print(ldrvalue);
            client.println("<br />");
            if(ldrvalue>950 && light_m==true)
            {
              client.print("Turn OFF any One light both are ON");
              client.println("<br />");
            }
            
            if(dhtsensor==false)
            {
               client.print("Check Connection of Temperature(DHT11) Sensor");
               client.println("<br />");
            }
            else
            {
            client.print("Humidity(%) :");
            client.print(Humidity);
            client.print("   Temperature(*C):");
            client.print(TempC);
            client.print("   Temperature(*F):");
            client.print(TempF);
            client.print("   Heat Index(*C):");
            client.print(hic);
            client.print("   Heat Index(*F):");
            client.print(hif);
            client.println("<br />");
            }
            if(light_m==false)
            {
            client.println("<a href=\"/?button1on\"\">Turn On LED</a>");
            }
            else
            {
            client.println("<a href=\"/?button1off\"\">Turn Off LED</a><br />");
            }   
            client.println("<br />");     
          
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
    if (readString.indexOf("?button1on") >0){
               digitalWrite(rel, LOW);
               light_m = true;
           }
           if (readString.indexOf("?button1off") >0){
               digitalWrite(rel, HIGH);
               light_m = false;
           }
           
          
            //clearing string for next read
            readString=""; 
  }
}


/*Function For Tank Level*/
void dist()
{
  digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 distance = duration/58.2;
 
 if (distance >= maximumRange || distance <= minimumRange){
 /* Send a negative number to computer and Turn LED ON 
 to indicate "out of range" */
 Serial.println("Sensor Data Not Valid (Check Sensor Connation)");
 tanksensor = false;
  
 }
 else {
 /* Send the distance to the computer using Serial protocol, and
 turn LED OFF to indicate successful reading. */
 tanksensor = true;
 Serial.println("Water Distance from Sensor:  ");
 Serial.println(distance);
 tanklevel = 120-distance;
 tanklevel = tanklevel*100;
 tanklevel = tanklevel/120;

 
 }
}

/*Function for DHT11 Sensor*/
void dhtread()
{
   // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  Humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  TempC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  TempF = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(Humidity) || isnan(TempC) || isnan(TempF)) {
    dhtsensor = false;
    Serial.println("Failed to read from DHT sensor!");
    
  }
  else
  {
    dhtsensor = true;
  }

  // Compute heat index in Fahrenheit (the default)
  hif = dht.computeHeatIndex(TempF, Humidity);
  // Compute heat index in Celsius (isFahreheit = false)
  hic = dht.computeHeatIndex(TempC, Humidity, false);
  Serial.print("Humidity: ");
  Serial.print(Humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(TempC);
  Serial.print(" *C ");
  Serial.print(TempF);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
  
}

/*Function for LDR*/
void ldr()
{
  ldrvalue = analogRead(ldrpin);
  Serial.println("LDR Reading: ");
  Serial.println(ldrvalue);

}


