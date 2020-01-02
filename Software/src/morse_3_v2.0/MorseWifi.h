#ifndef MORSEWIFI_H_
#define MORSEWIFI_H_

#include <Arduino.h>
//#include <Wire.h>          // Only needed for Arduino 1.6.5 and earlier
//#include "ClickButton.h"   // button control library
//#include <SPI.h>           // library for SPI interface
//#include <LoRa.h>          // library for LoRa transceiver
#include <WiFi.h>          // basic WiFi functionality
#include <WebServer.h>     // simple web sever
//#include "FS.h"
#include "SPIFFS.h"


namespace MorseWifi {
    ////////////////// Variables for file handling and WiFi functions

//    File file;

    extern WebServer server;    // Create a webserver object that listens for HTTP request on port 80

    extern File fsUploadFile;              // a File object to temporarily store the received file

    extern const char* host;               // hostname of the webserver


    /// WiFi constants
    extern const char* ssid;
    extern const char* password;


    // HTML for the AP server - ued to get SSID and Password for local WiFi network - needed for file upload and OTA SW updates
    extern const char* myForm;



    /*
     * HTML for Upload Login page
     */

    extern const char* uploadLoginIndex;
    extern const char* updateLoginIndex;
    extern const char* serverIndex;


    void startAP();
    boolean wifiConnect();
    void uploadFile();
    void updateFirmware();
    String getContentType(String filename); // convert the file extension to the MIME type
    bool handleFileRead(String path);       // send the right file to the client (if it exists)
    void handleFileUpload();                // upload a new file to the SPIFFS
}




#endif /* MORSEWIFI_H_ */
