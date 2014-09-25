#include <SPI.h>
#include <WiFi.h>

// the IP address for the shield:
static IPAddress ip(10, 3, 13, 60);

char ssid[]  = "EECS";      // your network SSID (name) 
char pass[]  = "";          // your network password
int keyIndex = 0;           // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

// global temp vars
float currTemp = 0;
float avg1sTemp = 0;
float avg10sTemp = 0;
int counter = 0;
float runningAvg = 0;

const int NUM_READINGS = 1000;

int avgCounter = 0;
float averagesArray[10];

void setup() {
  pinMode(A0, INPUT);

  for( int i=0; i < 10; i++ ) {
    averagesArray[i] = 0;
  }

  //Initialize serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  Serial.print(F("Firmware version: "));
  Serial.println(WiFi.firmwareVersion());

  WiFi.config(ip);

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid);

    // wait 10 seconds for connection:
    delay(10000);
  } 
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}


void loop() {

  if( counter >= NUM_READINGS ) {
    counter = 0;
    getAverages();
  }

  // check if we have a client waiting
  handleClient();
  
  getTemp();
  counter++;
  delay(8);  
}


void getTemp() {
  // Grab analog reading from thermistor
  currTemp = convertToTemp( analogRead( A0 ) ); 
  
  // Set these averages before we have enough readings
  if( !avg1sTemp ) {
    avg1sTemp = currTemp;
  }
  if( !avg10sTemp ) {
    avg10sTemp = currTemp;
  }

  runningAvg += currTemp / NUM_READINGS; 
}

// Convert 0 to 1023 to a degrees Celcius
float convertToTemp( int reading ) {
   return reading/10 - 8; 
}

// Calculate our average temperature
void getAverages() {
  // grab our running average, then reset it
  avg1sTemp = runningAvg;
  runningAvg = 0;
  averagesArray[avgCounter] = avg1sTemp;

  // getting 10s average
  avg10sTemp = 0;
  int i;
  for( i=0; i < 10; i++ ) {
    avg10sTemp += averagesArray[i];
    if( averagesArray[i] == 0 ) break; // break if we have a zero value
  }
  avg10sTemp /= i;

  // increment or reset the counter
  if( avgCounter >= 9 ) {
    avgCounter = 0;
  } 
  else {
    avgCounter++;
  }
}

// handle a connection to our IP from iPad
void handleClient() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json;charset=utf-8");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.print("{");
          client.print("\"currentTemp\":\"");
          client.print(currTemp);
          client.print("\",");
          client.print("\"avg10Temp\":\"");
          client.print(avg10sTemp);
          client.print("\",");
          client.print("\"avg1Temp\":\"");
          client.print(avg1sTemp);
          client.print("\"");
          client.print("}");
          client.println();
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
