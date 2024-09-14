#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your mobile hotspot credentials
const char* ssid = "Redmi 9 Power";
const char* password = "VinayGupta";

AsyncWebServer server(80);

// GPIO pins for relays and onboard LED
const int fan1Pin = 13;
const int fan2Pin = 12;
const int light1Pin = 14;
const int light2Pin = 27;
// const int fan1Pin = 16;
// const int fan2Pin = 5;
// const int light1Pin = 4;
// const int light2Pin = 2;
const int onboardLedPin = 2;  // Onboard LED pin (typically GPIO2)

void setup() {
  Serial.begin(115200);

  // Configure GPIO pins
  pinMode(fan1Pin, OUTPUT);
  pinMode(fan2Pin, OUTPUT);
  pinMode(light1Pin, OUTPUT);
  pinMode(light2Pin, OUTPUT);
  pinMode(onboardLedPin, OUTPUT);

  // Initialize all appliances as OFF and onboard LED as OFF
  digitalWrite(fan1Pin, LOW);
  digitalWrite(fan2Pin, LOW);
  digitalWrite(light1Pin, LOW);
  digitalWrite(light2Pin, LOW);
  digitalWrite(onboardLedPin, HIGH);

  // Connect to mobile hotspot Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting to connect...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Turn on the onboard LED to indicate successful connection
  digitalWrite(onboardLedPin, LOW);

  // Serve the HTML control page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getHTMLPage());
  });

  // Handle HTTP requests to control appliances
  server.on("/control", HTTP_POST, [](AsyncWebServerRequest *request){
    String appliance;
    String state;

    if (request->hasParam("appliance", true) && request->hasParam("state", true)) {
      appliance = request->getParam("appliance", true)->value();
      state = request->getParam("state", true)->value();
      Serial.println(appliance + " -> " + state);
      controlAppliance(appliance, state);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  server.begin();  // Start the web server
}

void loop() {
  // Auto-reconnect to WiFi if connection is lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost connection. Reconnecting...");
    digitalWrite(onboardLedPin, HIGH);  // Turn off the LED to indicate offline status
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Reconnecting...");
    }
    Serial.println("Reconnected.");
    digitalWrite(onboardLedPin, LOW);  // Turn on the LED to indicate online status
  }
}

// Function to control appliances based on HTTP request
void controlAppliance(String appliance, String state) {
  int pin;
  if (appliance == "FAN_1") {
    pin = fan1Pin;
  } else if (appliance == "FAN_2") {
    pin = fan2Pin;
  } else if (appliance == "LIGHT_1") {
    pin = light1Pin;
  } else if (appliance == "LIGHT_2") {
    pin = light2Pin;
  }

  if (state == "on") {
    digitalWrite(pin, HIGH);  // Turn on appliance
  } else {
    digitalWrite(pin, LOW);   // Turn off appliance
  }
}

// HTML page with buttons for controlling appliances
String getHTMLPage() {
  return String(
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "  <meta charset=\"UTF-8\">"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "  <title>KALPANA HOME AUTOMATION SYSTEM</title>"
    "  <style>"
    "    body {"
    "      font-family: Arial, sans-serif;"
    "      background-color: #add8e6; /* Light blue outer background */"
    "      margin: 0;"
    "      padding: 0;"
    "      display: flex;"
    "      justify-content: center;"
    "      align-items: center;"
    "      height: 100vh;"
    "      box-sizing: border-box;"
    "    }"
    "    .container {"
    "      background: #e0f7fa; /* Light blue inside the box */"
    "      border-radius: 8px;"
    "      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);"
    "      border: 5px solid black; /* Black thick border */"
    "      width: 90%;"
    "      max-width: 600px;"
    "      padding: 20px;"
    "      text-align: center;"
    "    }"
    "    h1 {"
    "      color: #333;"
    "    }"
    "    .section {"
    "      margin: 20px 0;"
    "    }"
    "    button {"
    "      background-color: #f44336; /* Red (off by default) */"
    "      border: none;"
    "      color: white;"
    "      padding: 15px 32px;"
    "      text-align: center;"
    "      display: inline-block;"
    "      font-size: 16px;"
    "      font-weight: bold; /* Bold text */"
    "      margin: 5px;"
    "      cursor: pointer;"
    "      border-radius: 4px;"
    "      box-shadow: 0 4px #999; /* Shadow for 3D effect */"
    "      position: relative;"
    "      transition: background-color 0.3s, box-shadow 0.2s;"
    "    }"
    "    button:active {"
    "      box-shadow: 0 2px #666; /* Lower shadow on press */"
    "      transform: translateY(2px); /* Move button down slightly when clicked */"
    "    }"
    "    button.on {"
    "      background-color: #4CAF50; /* Green (on state) */"
    "      box-shadow: 0 2px #666; /* Shadow when pressed */"
    "      transform: translateY(2px); /* Move button down slightly when pressed */"
    "    }"
    "    button.off {"
    "      background-color: #f44336; /* Red (off state) */"
    "    }"
    "    @media (max-width: 600px) {"
    "      button {"
    "        width: 100%;"
    "        box-sizing: border-box;"
    "      }"
    "    }"
    "  </style>"
    "</head>"
    "<body>"
    "  <div class=\"container\">"
    "    <h1>KALPANA HOME AUTOMATION SYSTEM</h1>"
    "    <div class=\"section\">"
    "      <h2>FAN-CONTROL</h2>"
    "      <button id=\"FAN_1\" class=\"off\" onclick=\"toggleAppliance('FAN_1')\">FAN 1: OFF</button><br>"
    "      <button id=\"FAN_2\" class=\"off\" onclick=\"toggleAppliance('FAN_2')\">FAN 2: OFF</button>"
    "    </div>"
    "    <div class=\"section\">"
    "      <h2>LIGHT-CONTROL</h2>"
    "      <button id=\"LIGHT_1\" class=\"off\" onclick=\"toggleAppliance('LIGHT_1')\">LIGHT 1: OFF</button><br>"
    "      <button id=\"LIGHT_2\" class=\"off\" onclick=\"toggleAppliance('LIGHT_2')\">LIGHT 2: OFF</button>"
    "    </div>"
    "  </div>"
  "  <script>"
    "    function toggleAppliance(appliance) {"
    "      var button = document.getElementById(appliance);"
    "      var state = button.textContent.includes('OFF') ? 'on' : 'off';"
    "      var xhr = new XMLHttpRequest();"
    "      xhr.open('POST', '/control', true);"
    "      xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');"
    "      xhr.send('appliance=' + appliance + '&state=' + state);"
    "      button.textContent = appliance.replace('_', ' ').toUpperCase() + ': ' + (state === 'on' ? 'ON' : 'OFF');"
    "      button.className = state === 'on' ? 'on' : 'off';" // Change color based on state
    "    }"
    "  </script>"
    "</body>"
    "</html>"
  );
}

