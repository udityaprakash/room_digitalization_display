#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// OLED display size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Wi-Fi and server settings
const char* ssid = "FTTH";
const char* password = "13141314";
ESP8266WebServer server(80);

// Time setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 19800, 60000);  // Offset for Delhi (GMT+5:30)

// Variables
String receivedText = "";
bool textReceived = false;
int textSize = 1;
bool isScrolling = false;
bool showClockOnly = true;  // New variable to handle clock mode

void setup() {
  // Initialize serial and OLED
  Serial.begin(115200);
  
  // Set up I2C pins for OLED display
  Wire.begin(D2, D1); // D2 -> SDA (GPIO4), D1 -> SCL (GPIO5)
  
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for most OLED displays
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Infinite loop if OLED initialization fails
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");

  // Start NTP client
  timeClient.begin();

  // Set up web server
  server.on("/", handleRoot);  // Route to handle text
  server.begin();
  Serial.println("Server started");

  // Initial OLED screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(2, 5);
  display.println("LOADING");
  display.display();
}

void loop() {
  server.handleClient();
  timeClient.update();

  if (showClockOnly) {
    // Display full-screen clock
    displayFullScreenClock();
    textReceived = false;
  } else if (textReceived) {

    if (isScrolling && receivedText.length() > (128 / (6 * textSize))) {
      int textWidth = receivedText.length() * 6 * textSize;  // Calculate the width of the text in pixels

      // Scroll the text if it's longer than the display width
      for (int scrollPosition = 0; scrollPosition < textWidth + 128; scrollPosition++) {        
        display.clearDisplay(); // Redraw text and update small clock below
        display.setTextSize(textSize);
        // Set cursor to scroll the text from right to left
        display.setCursor(128 - scrollPosition, 0);
        
        // Print the received text
        display.print(receivedText);
        
        // Keep the small clock displayed at the bottom
        displaySmallClock();

        // Update the display with the new text position
        display.display();
        
        delay(100);  // Adjust delay for scrolling speed
      }
    } else {
      // Display the received text
      display.clearDisplay();
      display.setTextSize(textSize);
      display.setCursor(0, 0);
      display.print(receivedText);
      displaySmallClock();
      display.display();
    }
    // Display time at the bottom without flickering
  } else {
    textReceived = false;
    // If no text is received, default to the full-screen clock
    displayFullScreenClock();
  }
}

// Function to handle HTTP requests from Flutter
void handleRoot() {
  if (server.hasArg("text")) {
    receivedText = server.arg("text");
    textSize = server.arg("size").toInt();
    isScrolling = server.arg("scroll") == "true";
    textReceived = true;
    showClockOnly = false;
  }

  if (server.hasArg("clock")) {
    showClockOnly = server.arg("clock") == "true";
  }

  server.send(200, "text/plain", "Request processed");
}

// Function to display the small clock at the bottom
void displaySmallClock() {
  display.setTextSize(1);
  display.setCursor(0, 56);

  // Get the current time in 24-hour format
  int hours = timeClient.getHours();
  String minutes = timeClient.getMinutes() < 10 ? "0" + String(timeClient.getMinutes()) : String(timeClient.getMinutes());
  String seconds = timeClient.getSeconds() < 10 ? "0" + String(timeClient.getSeconds()) : String(timeClient.getSeconds());

  // Convert hours to 12-hour format
  String ampm = (hours >= 12) ? " PM" : " AM";
  hours = (hours % 12 == 0) ? 12 : hours % 12;

  // Format the time string in 12-hour format
  String timeString = String(hours) + ":" + minutes + ":" + seconds;
  
  // Display the time with AM/PM
  display.print(timeString + ampm);
}

// void displaySmallClock() {
//   display.setTextSize(1);
//   display.setCursor(0, 56);

//   String timeString = timeClient.getFormattedTime();
//   String ampm = (timeClient.getHours() >= 12) ? " PM" : " AM";
  
//   display.print(timeString + ampm);
// }

// Function to display the full-screen clock
void displayFullScreenClock() {
  display.clearDisplay();
  
  // Set text size and cursor position
  display.setTextSize(2);
  display.setCursor(0, 24);

  // Get the current time in 24-hour format
  int hours = timeClient.getHours();
  String minutes = timeClient.getMinutes() < 10 ? "0" + String(timeClient.getMinutes()) : String(timeClient.getMinutes());
  String seconds = timeClient.getSeconds() < 10 ? "0" + String(timeClient.getSeconds()) : String(timeClient.getSeconds());
  // Convert hours to 12-hour format
  String ampm = (hours >= 12) ? "PM" : "AM";
  hours = (hours % 12 == 0) ? 12 : hours % 12;

  // Format the time string in 12-hour format
  String timeString = String(hours) + ":" + minutes + ":" + seconds;

  // Display the time
  display.print(timeString);

  // Display AM/PM on the next line
  display.setTextSize(2);  // Adjust text size for AM/PM
  display.setCursor(0, 48);  // Position for AM/PM on the next line
  display.print(ampm);
  
  display.display();
}
// void displayFullScreenClock() {
//   display.clearDisplay();
//   display.setTextSize(2);
//   display.setCursor(0, 24);

//   String timeString = timeClient.getFormattedTime();
  
//   display.print(timeString);

//   display.setTextSize(2);  // Smaller text size for AM/PM
//   display.setCursor(0, 48);  // Adjust Y position for AM/PM
//   String ampm = (timeClient.getHours() >= 12) ? "PM" : "AM";
  
//   display.print(ampm);
//   display.display();
// }