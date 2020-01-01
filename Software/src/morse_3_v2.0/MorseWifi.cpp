#include <Arduino.h>

#include <ESPmDNS.h>       // DNS functionality
#include <Update.h>        // update "over the air" (OTA) functionality
//#include <WiFiClient.h>    //WiFi clinet library

#include "MorseWifi.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"


using namespace MorseWifi;
///////////////////////////////////////////////////////////////////////////////////////////////////
/// stuff using WiFi - ask for access point credentials, upload player file, do OTA software update
///////////////////////////////////////////////////////////////////////////////////////////////////


namespace internal {
    void startMDNS();
    boolean errorConnect(String msg);
    boolean wifiConnect();
    String getContentType(String filename);
}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void MorseWifi::startAP() {
  //IPaddress a;
  WiFi.mode(WIFI_AP);
  WiFi.setHostname(ssid);
  WiFi.softAP(ssid);
  //a = WiFi.softAPIP();
  MorseDisplay::clear();
  MorseDisplay::printOnStatusLine(true, 0,    "Enter Wifi Info @");
  MorseDisplay::printOnScroll(0, REGULAR, 0,  "AP: morserino");
  MorseDisplay::printOnScroll(1, REGULAR, 0,  "URL: m32.local");
  MorseDisplay::printOnScroll(2, REGULAR, 0,  "RED to abort");

  //printOnScroll(2, REGULAR, 0, WiFi.softAPIP().toString());
  //Serial.println(WiFi.softAPIP());

  internal::startMDNS();

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", myForm);
  });

  server.on("/set", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", "Wifi Info updated - now restarting Morserino-32...");
    String ssid = server.arg("ssid");
    String passwd = server.arg("pw");
    MorsePreferences::writeWifiInfo(ssid, passwd);
    ESP.restart();
  });

  server.onNotFound(handleNotFound);

  server.begin();
  while (true) {
      server.handleClient();
      delay(20);
      volButton.Update();
      if (volButton.clicks) {
        MorseDisplay::clear();
        MorseDisplay::printOnStatusLine(true, 0, "Resetting now...");
        delay(2000);
        ESP.restart();
      }
  }
}


void updateFirmware()   {                   /// start wifi client, web server and upload new binary from a local computer
  if (! wifiConnect())
    return;

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateLoginIndex);
  });

  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      //Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  //Serial.println("Starting web server");
  server.begin();
  MorseDisplay::clear();
  MorseDisplay::printOnStatusLine(true, 0, "Waiting f. Update ");
  MorseDisplay::printOnScroll(0, REGULAR, 0,  "URL: m32.local");
  MorseDisplay::printOnScroll(1, REGULAR, 0,  "IP:");
  MorseDisplay::printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());
  while(true) {
    server.handleClient();
    delay(10);
  }
}


boolean internal::wifiConnect() {                   // connect to local WLAN
  // Connect to WiFi network
  if (MorsePreferences::prefs.wlanSSID == "")
      return internal::errorConnect(String("WiFi Not Conf"));

  WiFi.begin(MorsePreferences::prefs.wlanSSID.c_str(), MorsePreferences::prefs.wlanPassword.c_str());

  // Wait for connection
  long unsigned int wait = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if ((millis() - wait) > 20000)
      return internal::errorConnect(String("No WiFi:"));
  }
  //Serial.print("Connected to ");
  //Serial.println(MorsePreferences::prefs.wlanSSID);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  internal::startMDNS();
  return true;
}

boolean internal::errorConnect(String msg) {
  MorseDisplay::clear();
  MorseDisplay::printOnStatusLine(true, 0, "Not connected");
  MorseDisplay::printOnScroll(0, INVERSE_BOLD, 0, msg);
  MorseDisplay::printOnScroll(1, REGULAR, 0, MorsePreferences::prefs.wlanSSID);
  delay(3500);
  return false;
}

void startMDNS() {
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://m32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
      if (MDNS.begin(host))
        break;
    }
  }
  //Serial.println("mDNS responder started");
}

void uploadFile() {
  if (! internal::wifiConnect())
    return;
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", uploadLoginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

server.on("/update", HTTP_POST,                       // if the client posts to the upload page
    [](){ server.sendHeader("Connection", "close");
    server.send(200, "text/plain", "OK");
    ESP.restart();},                                  // Send status 200 (OK) to tell the client we are ready to receive; when done, restart the ESP32
    handleFileUpload                                    // Receive and save the file
  );

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();                           // Actually start the server
  //Serial.println("HTTP server started");
  MorseDisplay::clear();
  MorseDisplay::printOnStatusLine(true, 0, "Waiting f. Upload ");
  MorseDisplay::printOnScroll(0, REGULAR, 0,  "URL: m32.local");
  MorseDisplay::printOnScroll(1, REGULAR, 0,  "IP:");
  MorseDisplay::printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());
  while(true) {
    server.handleClient();
    //delay(5);
  }
}


String internal::getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  //Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = internal::getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {     // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                            // If there's a compressed version available
      path += ".gz";                                          // Use the compressed verion
    File file = SPIFFS.open(path, "r");                       // Open the file
    server.streamFile(file, contentType);                     // Send it to the client
    file.close();                                             // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  //Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    //Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open("/player.txt", "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      MorsePreferences::prefs.fileWordPointer = 0;                              // reset word counter for file player
      MorsePreferences::writeWordPointer();

      //Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
      //server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

