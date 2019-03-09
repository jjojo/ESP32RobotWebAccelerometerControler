/***  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>

// You can customize the SSID name and change the password
const char* ssid     = "ESP32-Robot";
const char* password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Motor 1
int motor1Pin1 = 27;
int motor1Pin2 = 15;
int enable1Pin = 14; 

// Motor 2
int motor2Pin1 = 33;
int motor2Pin2 = 12;
int enable2Pin = 32;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 0;

// Decode HTTP GET value
String valueString = "0";
int pos1 = 0;
int pos2 = 0;

void setup() {
  Serial.begin(115200);
  
  // Set the Motor pins as outputs
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(motor2Pin1, OUTPUT);
  pinMode(motor2Pin2, OUTPUT);

  // Configure PWM channel functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // Attach the PWM channel 0 to the enable pins which are the GPIOs to be controlled
  ledcAttachPin(enable1Pin, pwmChannel);
  ledcAttachPin(enable2Pin, pwmChannel); 

  // Produce a PWM signal to both enable pins with a duty cycle 0
  ledcWrite(pwmChannel, dutyCycle);
  
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)...");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /forward") >= 0) {
              Serial.println("Forward");
              digitalWrite(motor1Pin1, LOW);
              digitalWrite(motor1Pin2, HIGH); 
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, HIGH);
            }  else if (header.indexOf("GET /left") >= 0) {
              Serial.println("Left");
              digitalWrite(motor1Pin1, LOW); 
              digitalWrite(motor1Pin2, LOW); 
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, HIGH);
            }  else if (header.indexOf("GET /stop") >= 0) {
              Serial.println("Stop");
              digitalWrite(motor1Pin1, LOW); 
              digitalWrite(motor1Pin2, LOW); 
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, LOW);             
            } else if (header.indexOf("GET /right") >= 0) {
              Serial.println("Right");
              digitalWrite(motor1Pin1, LOW); 
              digitalWrite(motor1Pin2, HIGH); 
              digitalWrite(motor2Pin1, LOW);
              digitalWrite(motor2Pin2, LOW);    
            } else if (header.indexOf("GET /reverse") >= 0) {
              Serial.println("Reverse");
              digitalWrite(motor1Pin1, HIGH);
              digitalWrite(motor1Pin2, LOW); 
              digitalWrite(motor2Pin1, HIGH);
              digitalWrite(motor2Pin2, LOW);          
            }

            // client.println("<!DOCTYPE HTML><html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <style>html{font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button{-webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50; border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer;}.button2{background-color: #555555;}</style> </head> <body> <div> <p><a href=\"/forward\"><button class=\"button\" onclick=\"moveForward()\">FORWARD</button></a></p><div style=\"clear: both;\"> <p> <a href=\"/left\"><button class=\"button\" onclick=\"moveLeft()\">LEFT </button></a> <a href=\"/stop\"><button class=\"button button2\" onclick=\"stopRobot()\">STOP</button></a> <a href=\"/right\"><button class=\"button\">RIGHT</button></a> </p></div><p><a href=\"/reverse\"><button class=\"button\">REVERSE</button></a></p><input type=\"range\" min=\"0\" max=\"100\" step=\"25\" id=\"motorSlider\" onchange=\"motorSpeed(this.value)\" value=\"" + valueString + "\"/> </div><button onclick=\"activateMotionControl()\">Activate motion control</button> <button onclick=\"deActivateMotionControl()\">Deactivate motion control</button> <pre class=\"output\"></pre> </body> <script>var output=document.querySelector('.output'); var lastMove=''; function activateMotionControl(){window.addEventListener('deviceorientation', handleMotion);}function deActivateMotionControl(){window.removeEventListener('deviceorientation', handleMotion);}function handleMotion(event){let x=event.beta; // In degree in the range [-180,180] let y=event.gamma; // In degree in the range [-90,90] // Because we don't want to have the device upside down // We constrain the x value to the range [-90,90] if (x > 90){x=90}; if (x < -90){x=-90}; // To make computation easier we shift the range of // x and y to [0,180] x +=90; y +=90; output.innerHTML=\"x : \" + x; output.innerHTML +=\"y: \" + y; if(y <=70) goInDirection('left'); if(y > 110) goInDirection('right'); if(x <=70) goInDirection('forward'); if(x > 110) goInDirection('reverse'); if(110 > x && x> 70 && 110 > y && y > 70) goInDirection('stop');}function goInDirection(direction){if(lastMove !==direction){lastMove=direction; var xhr=new XMLHttpRequest(); xhr.open('GET', \"/\" + direction, true); xhr.send();}}function motorSpeed(pos){var xhr=new XMLHttpRequest(); xhr.open('GET', \"/?value=\" + pos + \"&\", true); xhr.send();}</script><html>");
            
            // Display the HTML web page
            client.println("<!DOCTYPE HTML><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; background-color: #4CAF50;");
            client.println("border: none; color: white; padding: 12px 28px; text-decoration: none; font-size: 26px; margin: 1px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page        
            client.println("<body><div>");
            client.println("<p><a href=\"/forward\"><button class=\"button\">FORWARD</button></a></p>");
            client.println("<div style=\"clear: both;\"><p><a href=\"/left\"><button class=\"button\">LEFT </button></a>");
            client.println("<a href=\"/stop\"><button class=\"button button2\">STOP</button></a>");
            client.println("<a href=\"/right\"><button class=\"button\">RIGHT</button></a></p></div>");
            client.println("<p><a href=\"/reverse\"><button class=\"button\">REVERSE</button></a></p>");        
            client.println("<input type=\"range\" min=\"0\" max=\"100\" step=\"25\" id=\"motorSlider\" onchange=\"motorSpeed(this.value)\" value=\"" + valueString + "\"/>");
            client.println("<button onclick=\"activateMotionControl()\">Activate motion control</button>");
            client.println("<button onclick=\"deActivateMotionControl()\">Deactivate motion control</button>");
            client.println("<pre class=\"output\"></pre>");
            client.println("</body></div>");


            // JavaScript functions
            client.println("<script>");
            client.println("var motionControl = false;");
            client.println("var output = document.querySelector('.output');");
            client.println("var lastMove = '';");

            client.println("function activateMotionControl() {");
            client.println("window.addEventListener('deviceorientation', handleMotion);");
            client.println("motionControl = true;");
            client.println("}");

            client.println("function deActivateMotionControl() {");
            client.println("window.removeEventListener('deviceorientation', handleMotion);");
            client.println("motionControl = false;");
            client.println("}");

            client.println("function handleMotion(event) {");
            client.println("var x = event.beta;");
            client.println("var y = event.gamma;");
            client.println("if (x >  90) { x =  90};");
            client.println("if (x < -90) { x = -90};");
            client.println("x += 90;");
            client.println("y += 90;");
            client.println("output.innerHTML  = \"x : \" + x + \"\";");
            client.println("output.innerHTML += \"y: \" + y + \"\";");
            client.println("if(y <= 70) goInDirection('left');");
            client.println("if(y > 110) goInDirection('right');");
            client.println("if(x <= 70) goInDirection('forward');");
            client.println("if(x > 110) goInDirection('reverse');");
            client.println("if(110 > x && x> 70 && 110 > y && y > 70) goInDirection('stop');");
            client.println("}\n ");

            client.println("function goInDirection(direction) {");
            client.println("if(lastMove !== direction){");
            client.println("lastMove = direction;");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open('GET', \"/\" + direction, true); ");
            client.println("xhr.send();");
            client.println("}");
            client.println("}");

            client.println("function motorSpeed(pos) { ");
            client.println("var xhr = new XMLHttpRequest();"); 
            client.println("xhr.open('GET', \"/?value=\" + pos + \"&\", true);"); 
            client.println("xhr.send(); } </script>");
           
            client.println("</html>");
            

            
            /**
            client.println("<script>\n");
            client.println("var motionControl = false;");
            client.println("var output = document.querySelector('.output');");
            client.println("var lastMove = '';");
            
            client.println("function activateMotionControl() {");
            client.println("window.addEventListener('deviceorientation', handleMotion);");
            client.println("motionControl = true;");
            client.println("}");

            client.println("function handleMotion(event) {");
            client.println("var x = 2");
            client.println("var y = 2");
            //client.println("if (x >  90) { x =  90};");
            //client.println("if (x < -90) { x = -90};");
            //client.println("x += 90;");
            //client.println("y += 90;");
            client.println("output.innerHTML =\"x:\";");
            client.println("output.innerHTML +=\"y:\";");
            //client.println("if(y <= 70) goInDirection(\"left\");");
            //client.println("if(y > 110) goInDirection(\"right\");");
            //client.println("if(x <= 70) goInDirection(\"forward\");");
            //client.println("if(x > 110) goInDirection(\"reverse\");");
            //client.println("if(110 > x > 70 && 110 > y > 70) goInDirection(\"stop\");");
            client.println("} ");
            ***/
            
            /**
            client.println("function motorSpeed(pos) { ");
            client.println("var xhr = new XMLHttpRequest();"); 
            client.println("xhr.open('GET', \"/?value=\" + pos + \"&\", true);"); 
            client.println("xhr.send(); } </script>");
           
            client.println("</html>");

            **/

            
            //Request example: GET /?value=100& HTTP/1.1 - sets PWM duty cycle to 100
            if(header.indexOf("GET /?value=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              valueString = header.substring(pos1+1, pos2);
              //Set motor speed value
              if (valueString == "0") {
                ledcWrite(pwmChannel, 0);
                digitalWrite(motor1Pin1, LOW); 
                digitalWrite(motor1Pin2, LOW); 
                digitalWrite(motor2Pin1, LOW);
                digitalWrite(motor2Pin2, LOW);   
              }
              else { 
                dutyCycle = map(valueString.toInt(), 25, 100, 200, 255);
                ledcWrite(pwmChannel, dutyCycle);
                Serial.println(valueString);
              } 
            }         
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
