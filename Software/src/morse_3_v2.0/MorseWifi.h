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

    const char* host = "m32";               // hostname of the webserver


    /// WiFi constants
    const char* ssid = "morserino";
    const char* password = "";


                              // HTML for the AP server - ued to get SSID and Password for local WiFi network - needed for file upload and OTA SW updates
    const char* myForm = "<html><head><meta charset='utf-8'><title>Get AP Info</title><style> form {width: 420px;}div { margin-bottom: 20px;}"
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

    const char* uploadLoginIndex =
     "<form name='loginForm'>"
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


    const char* updateLoginIndex =
     "<form name='loginForm'>"
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


    const char* serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
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


    void startAP();
    boolean wifiConnect();
    void uploadFile();
    void updateFirmware();
    String getContentType(String filename); // convert the file extension to the MIME type
    bool handleFileRead(String path);       // send the right file to the client (if it exists)
    void handleFileUpload();                // upload a new file to the SPIFFS
}




#endif /* MORSEWIFI_H_ */
