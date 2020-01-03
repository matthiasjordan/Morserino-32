#include <Arduino.h>

#include <ESPmDNS.h>       // DNS functionality
#include <Update.h>        // update "over the air" (OTA) functionality
//#include <WiFiClient.h>    //WiFi clinet library

#include "MorseWifi.h"
#include "MorseDisplay.h"
#include "MorsePreferences.h"
#include "MorseUI.h"

//using namespace MorseWifi;
///////////////////////////////////////////////////////////////////////////////////////////////////
/// stuff using WiFi - ask for access point credentials, upload player file, do OTA software update
///////////////////////////////////////////////////////////////////////////////////////////////////

WebServer MorseWifi::server(80);    // Create a webserver object that listens for HTTP request on port 80

File MorseWifi::fsUploadFile;              // a File object to temporarily store the received file

const char* MorseWifi::host = "m32";               // hostname of the webserver

/// WiFi constants
const char* MorseWifi::ssid = "morserino";
const char* MorseWifi::password = "";

// HTML for the AP server - ued to get SSID and Password for local WiFi network - needed for file upload and OTA SW updates
const char* MorseWifi::myForm =
        "<html><head><meta charset='utf-8'><title>Get AP Info</title><style> form {width: 420px;}div { margin-bottom: 20px;}"
                "label {display: inline-block; width: 240px; text-align: right; padding-right: 10px;} button, input {float: right;}</style>"
                "</head><body>"
                "<form action='/set' method='get'><div>"
                "<label for='ssid'>SSID of WiFi network?</label>"
                "<input name='ssid' id='ssid' ></div> <div>"
                "<label for='pw'>WiFi Password?</label> <input name='pw' id='pw'>"
                "</div><div><button>Submit</button></div></form></body></html>";

/*
 * HTML for Upload Login page
 */

const char* MorseWifi::uploadLoginIndex = "<form name='loginForm'>"
        "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
        "<td colspan=2>"
        "<center><font size=4><b>M32 File Upload - Login Page</b></font></center>"
        "<br>"
        "</td>"
        "<br>"
        "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
        "<td>Password:</td>"
        "<td><input type='Password' size=25 name='pwd'><br></td>"
        "<br>"
        "<br>"
        "</tr>"
        "<tr>"
        "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
        "</table>"
        "</form>"
        "<script>"
        "function check(form)"
        "{"
        "if(form.userid.value=='m32' && form.pwd.value=='upload')"
        "{"
        "window.open('/serverIndex')"
        "}"
        "else"
        "{"
        " alert('Error Password or Username')/*displays error message*/"
        "}"
        "}"
        "</script>";

const char* MorseWifi::updateLoginIndex = "<form name='loginForm'>"
        "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
        "<td colspan=2>"
        "<center><font size=4><b>M32 Firmware Update Login Page</b></font></center>"
        "<br>"
        "</td>"
        "<br>"
        "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
        "<td>Password:</td>"
        "<td><input type='Password' size=25 name='pwd'><br></td>"
        "<br>"
        "<br>"
        "</tr>"
        "<tr>"
        "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
        "</table>"
        "</form>"
        "<script>"
        "function check(form)"
        "{"
        "if(form.userid.value=='m32' && form.pwd.value=='update')"
        "{"
        "window.open('/serverIndex')"
        "}"
        "else"
        "{"
        " alert('Error Password or Username')/*displays error message*/"
        "}"
        "}"
        "</script>";

const char* MorseWifi::serverIndex = "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "<input type='file' name='update'>"
        "<input type='submit' value='Begin'>"
        "</form>"
        "<div id='prg'>Progress: 0%</div>"
        "<script>"
        "$('form').submit(function(e){"
        "e.preventDefault();"
        "var form = $('#upload_form')[0];"
        "var data = new FormData(form);"
        " $.ajax({"
        "url: '/update',"
        "type: 'POST',"
        "data: data,"
        "contentType: false,"
        "processData:false,"
        "xhr: function() {"
        "var xhr = new window.XMLHttpRequest();"
        "xhr.upload.addEventListener('progress', function(evt) {"
        "if (evt.lengthComputable) {"
        "var per = evt.loaded / evt.total;"
        "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
        "}"
        "}, false);"
        "return xhr;"
        "},"
        "success:function(d, s) {"
        "console.log('success!')"
        "},"
        "error: function (a, b, c) {"
        "}"
        "});"
        "});"
        "</script>";

namespace internal
{
    void startMDNS();
    boolean errorConnect(String msg);
    boolean wifiConnect();
    String getContentType(String filename);
    void handleNotFound();
    bool handleFileRead(String path);
    void handleFileUpload();
}

void internal::handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += MorseWifi::server.uri();
    message += "\nMethod: ";
    message += (MorseWifi::server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += MorseWifi::server.args();
    message += "\n";
    for (uint8_t i = 0; i < MorseWifi::server.args(); i++)
    {
        message += " " + MorseWifi::server.argName(i) + ": " + MorseWifi::server.arg(i) + "\n";
    }
    MorseWifi::server.send(404, "text/plain", message);
}

void MorseWifi::startAP()
{
    //IPaddress a;
    WiFi.mode(WIFI_AP);
    WiFi.setHostname(ssid);
    WiFi.softAP(ssid);
    //a = WiFi.softAPIP();
    MorseDisplay::clear();
    MorseDisplay::printOnStatusLine(true, 0, "Enter Wifi Info @");
    MorseDisplay::printOnScroll(0, REGULAR, 0, "AP: morserino");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "URL: m32.local");
    MorseDisplay::printOnScroll(2, REGULAR, 0, "RED to abort");

    //printOnScroll(2, REGULAR, 0, WiFi.softAPIP().toString());
    //Serial.println(WiFi.softAPIP());

    internal::startMDNS();

    server.on("/", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", myForm);
    });

    server.on("/set", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", "Wifi Info updated - now restarting Morserino-32...");
        String ssid = server.arg("ssid");
        String passwd = server.arg("pw");
        MorsePreferences::writeWifiInfo(ssid, passwd);
        ESP.restart();
    });

    server.onNotFound(internal::handleNotFound);

    server.begin();
    while (true)
    {
        server.handleClient();
        delay(20);
        MorseUI::volButton.Update();
        if (MorseUI::volButton.clicks)
        {
            MorseDisplay::clear();
            MorseDisplay::printOnStatusLine(true, 0, "Resetting now...");
            delay(2000);
            ESP.restart();
        }
    }
}

void MorseWifi::updateFirmware()
{                   /// start wifi client, web server and upload new binary from a local computer
    if (!wifiConnect())
        return;

    server.on("/", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", updateLoginIndex);
    });

    server.on("/serverIndex", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
    });

    /*handling uploading firmware file */
    server.on("/update", HTTP_POST, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []()
    {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
            //Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            { //start with max available size
                Update.printError(Serial);
            }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
                Update.printError(Serial);
            }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
            if (Update.end(true))
            { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            }
            else
            {
                Update.printError(Serial);
            }
        }
    });
    //Serial.println("Starting web server");
    server.begin();
    MorseDisplay::clear();
    MorseDisplay::printOnStatusLine(true, 0, "Waiting f. Update ");
    MorseDisplay::printOnScroll(0, REGULAR, 0, "URL: m32.local");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "IP:");
    MorseDisplay::printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());
    while (true)
    {
        server.handleClient();
        delay(10);
    }
}

boolean MorseWifi::wifiConnect()
{                   // connect to local WLAN
    // Connect to WiFi network
    if (MorsePreferences::prefs.wlanSSID == "")
        return internal::errorConnect(String("WiFi Not Conf"));

    WiFi.begin(MorsePreferences::prefs.wlanSSID.c_str(), MorsePreferences::prefs.wlanPassword.c_str());

    // Wait for connection
    long unsigned int wait = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
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

boolean internal::errorConnect(String msg)
{
    MorseDisplay::clear();
    MorseDisplay::printOnStatusLine(true, 0, "Not connected");
    MorseDisplay::printOnScroll(0, INVERSE_BOLD, 0, msg);
    MorseDisplay::printOnScroll(1, REGULAR, 0, MorsePreferences::prefs.wlanSSID);
    delay(3500);
    return false;
}

void internal::startMDNS()
{
    /*use mdns for host name resolution*/
    if (!MDNS.begin(MorseWifi::host))
    { //http://m32.local
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
            if (MDNS.begin(MorseWifi::host))
                break;
        }
    }
    //Serial.println("mDNS responder started");
}

void MorseWifi::uploadFile()
{
    if (!MorseWifi::wifiConnect())
        return;
    server.on("/", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", uploadLoginIndex);
    });
    server.on("/serverIndex", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.send(200, "text/html", serverIndex);
    });

    server.on("/update", HTTP_POST,                       // if the client posts to the upload page
            []()
            {   server.sendHeader("Connection", "close");
                server.send(200, "text/plain", "OK");
                ESP.restart();},            // Send status 200 (OK) to tell the client we are ready to receive; when done, restart the ESP32
            internal::handleFileUpload                                    // Receive and save the file
            );

    server.onNotFound([]()
    {                              // If the client requests any URI
                if (!internal::handleFileRead(server.uri()))// send it if it exists
                server.send(404, "text/plain", "404: Not Found");// otherwise, respond with a 404 (Not Found) error
            });

    server.begin();                           // Actually start the server
    //Serial.println("HTTP server started");
    MorseDisplay::clear();
    MorseDisplay::printOnStatusLine(true, 0, "Waiting f. Upload ");
    MorseDisplay::printOnScroll(0, REGULAR, 0, "URL: m32.local");
    MorseDisplay::printOnScroll(1, REGULAR, 0, "IP:");
    MorseDisplay::printOnScroll(2, REGULAR, 0, WiFi.localIP().toString());
    while (true)
    {
        server.handleClient();
        //delay(5);
    }
}

String internal::getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

bool internal::handleFileRead(String path)
{ // send the right file to the client (if it exists)
    //Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";          // If a folder is requested, send the index file
    String contentType = internal::getContentType(path);             // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {     // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                            // If there's a compressed version available
            path += ".gz";                                          // Use the compressed verion
        File file = SPIFFS.open(path, "r");                       // Open the file
        MorseWifi::server.streamFile(file, contentType);                     // Send it to the client
        file.close();                                             // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    //Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
    return false;
}

void internal::handleFileUpload()
{ // upload a new file to the SPIFFS
    HTTPUpload& upload = MorseWifi::server.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
        String filename = upload.filename;
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        //Serial.print("handleFileUpload Name: "); Serial.println(filename);
        MorseWifi::fsUploadFile = SPIFFS.open("/player.txt", "w");       // Open the file for writing in SPIFFS (create if it doesn't exist)
        filename = String();
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (MorseWifi::fsUploadFile)
            MorseWifi::fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (MorseWifi::fsUploadFile)
        {                                    // If the file was successfully created
            MorseWifi::fsUploadFile.close();                               // Close the file again
            MorsePreferences::prefs.fileWordPointer = 0;                              // reset word counter for file player
            MorsePreferences::writeWordPointer();

            //Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
            //server.sendHeader("Location","/success.html");      // Redirect the client to the success page
            //server.send(303);
        }
        else
        {
            MorseWifi::server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}

