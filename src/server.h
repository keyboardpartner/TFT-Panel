#ifndef OASERVER_h
#define OASERVER_h

// ##############################################################################
//
//   ######  ######## ########  ##     ## ######## ########
//  ##    ## ##       ##     ## ##     ## ##       ##     ##
//  ##       ##       ##     ## ##     ## ##       ##     ##
//   ######  ######   ########  ##     ## ######   ########
//        ## ##       ##   ##    ##   ##  ##       ##   ##
//  ##    ## ##       ##    ##    ## ##   ##       ##    ##
//   ######  ######## ##     ##    ###    ######## ##     ##
//
// ##############################################################################

#include <Arduino.h>
#include <SPIFFS.h>

#include <WiFi.h>
#include <WiFiClient.h>

// Server
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h> // https://docs.elegantota.pro/

#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "global_vars.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
WiFiClient client;

// ##############################################################################
// ElegantOTA Callbacks
// ##############################################################################

unsigned long ota_progress_millis = 0;

// Callback function called when OTA update starts
void onOTAStart() {
  DEBUG_PRINTLN("OTA update started!");

}

// Callback function called when OTA update progress is reported
void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  #ifdef DEBUG
    if (millis() - ota_progress_millis > 1000) {
      ota_progress_millis = millis();
      Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
    }
  #endif
}

// Callback function called when OTA update ends
void onOTAEnd(bool success) {
  #ifdef DEBUG
    // Log when OTA has finished
    if (success) {
      Serial.println("OTA update finished successfully!");
    } else {
      Serial.println("There was an error during OTA update!");
    }
  #endif
}

// handles uploads
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  DEBUG_PRINTLN(logmessage);
  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    DEBUG_PRINTLN(logmessage);
  }
  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    DEBUG_PRINTLN(logmessage);
  }
  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    DEBUG_PRINTLN(logmessage);
    request->redirect("/");
  }
}

// ##############################################################################
//
//  ##     ## ######## ##     ## ##
//  ##     ##    ##    ###   ### ##
//  ##     ##    ##    #### #### ##
//  #########    ##    ## ### ## ##
//  ##     ##    ##    ##     ## ##
//  ##     ##    ##    ##     ## ##
//  ##     ##    ##    ##     ## ########
// ##############################################################################

// Server HTML Processing
// Send values for root page, requested by placeholder in string var
String html_process_root(const String& var) {
  String param;
  if (var == "STA_SSID") {
    return String(settings.ssid);
  } else if (var == "STA_PASSWORD") {
    return String(settings.password);
  } else if (var == "SPKR_TICK") {
    if (settings.spkrTick)
      return "checked"; // oder gar nicht, "value" geht nicht!
    else
      return  String();
  } else if (var == "SPKR_BEEP") {
    if (settings.spkrBeep)
      return "checked"; // oder gar nicht, "value" geht nicht!
    else
      return  String();

  // Restliche Anzeige, keine Optionen. Tabellenzeilen <tr></tr> schicken
  } else if (var == "DIRECTORY") {
    String dirList = "";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    String filename;
    while (file) {
      filename = file.name();
      DEBUG_PRINTLN(file.name());
      //dirList = dirList + "<a href=\"" + file.name() + "\">" + file.name() + "</a> (" + file.size() + " Bytes)<br>";
      dirList = dirList + "<tr><td class=\"link\" style=\"width: 250px\"><a href=\"" + filename + "\">" + filename + "</a></td>";
      dirList = dirList + "<td style=\"width: 100px\"><small>" + file.size() + " Bytes</small></td>";
      dirList = dirList + "<td style=\"width: 200px\"><a href=\"" + filename + "\" download=\"" + filename + "\">Download</a>";
      if ((!filename.endsWith("html")) && (!filename.endsWith("css")))
        dirList = dirList + " or <a href=\"/?delete=/" + filename + "\">Delete</a>";
      dirList = dirList + "</td></tr>\r\n";
      file = root.openNextFile();
    }
    return dirList;

  } else {
    return String();
  }
}

// Send values for webpages, requested by placeholder in string var
String html_process_page(const String& var) {
  String param;
  if(var == "adcOffsAmps") {
    return String(settings.adcRawOffsetAmps);
  }
  if(var == "adcOffsVolts") {
    return String(settings.adcRawOffsetVolts);
  }
  for (int i = 0; i<10; i++) {
    param = "ACTIVE_" + String(i);
    if(((settings.ampRangeIdx == i) || (settings.voltRangeIdx == i)) && (var == param)) {
      return String("ACTIVE");
    }
    param = "adcScaling_" + String(i);
    if(var == param) {
      return String(settings.adcScalings[i],3);
    }
  }
  return String();
}

// ------------------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

// ##############################################################################
//
//   ######   ######## ########    ########   #######   ######  ########
//  ##    ##  ##          ##       ##     ## ##     ## ##    ##    ##
//  ##        ##          ##       ##     ## ##     ## ##          ##
//  ##   #### ######      ##       ########  ##     ##  ######     ##
//  ##    ##  ##          ##       ##        ##     ##       ##    ##
//  ##    ##  ##          ##       ##        ##     ## ##    ##    ##
//   ######   ########    ##       ##         #######   ######     ##
//
// ##############################################################################


// Server GET/POST Processing

// This function handles GET requests to the server
// It processes/dispatches the request parameters
// and sends a response back to the client
void myGEThandler(AsyncWebServerRequest *request) {
  DEBUG_PRINTLN("Server GET request");
  String value, param, redirect;
  bool do_save = false;
  redirect = "/";
  int param_count = request->params();
  const AsyncWebParameter* p;
  // Parameter anzeigen
  for (int i = 0; i < param_count; i++) {
    p = request->getParam(i);
    if (p->isFile()) { //p->isPost() is also true
      Serial.printf("FILE '%s': %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      // POST Parameter, also z.B. von HTML Formular
      Serial.printf("POST '%s': %s\n", p->name().c_str(), p->value().c_str());
    } else {
      // Normaler Parameter, also GET
      Serial.printf("GET '%s': %s\n", p->name().c_str(), p->value().c_str());

      // Hier die Parameter abarbeiten, die von der Webseite kommen
      if (p->name() == "adcOffsVolts") {
        redirect = "/scalings.html";
        settings.adcRawOffsetVolts = p->value().toInt();
        do_save = true; // ADC Skalierung wurde geändert, also speichern
      } else if(p->name() == "adcOffsAmps") {
        redirect = "/scalings.html";
        settings.adcRawOffsetAmps = p->value().toInt();
        do_save = true; // ADC Skalierung wurde geändert, also speichern
      }

      // Indizierte Parameter-Listen, 10 Einträge für Skalierung Messbereiche
      for (int j = 0; j<10; j++) {
        // adcScaling_0 bis adcScaling_9
        if (p->name() == "adcScaling_" + String(j)) {
          settings.adcScalings[j] = p->value().toFloat();
          do_save = true; // ADC Skalierung wurde geändert, also speichern
        }
      }

      if (p->name() == "spkrTick") {
        if (p->value() == "on") {
          settings.spkrTick = 1;
        } else {
          settings.spkrTick = 0;
        }
        do_save = true; // Lautsprecher Tick wurde geändert, also speichern
      } else if (p->name() == "spkrBeep") {
        if (p->value() == "on") {
          settings.spkrBeep = 1;
        } else {
          settings.spkrBeep = 0;
        }
        do_save = true; // Lautsprecher Beep wurde geändert, also speichern
      } else if (p->name() == "ssid_sta") {
        value = p->value();
        value.toCharArray(settings.ssid, value.length() + 1); // +1 for null terminator
        #ifdef DEBUG
          Serial.printf("SSID: %s\n", settings.ssid);
        #endif
        do_save = true; // SSID wurde geändert, also speichern
      } else if (p->name() == "pass_sta") {
        value = p->value();
        value.toCharArray(settings.password, value.length() + 1); // +1 for null terminator
        #ifdef DEBUG
          Serial.printf("PASS: %s\n", settings.password);
        #endif
        do_save = true; // Passwort wurde geändert, also speichern
      } else if (p->name() == "delete") {
        // Datei löschen, wenn Parameter "delete" gesetzt ist
        SPIFFS.remove(p->value());
      }
    }
  }
  // Wenn Änderungen vorgenommen wurden, dann speichern und Seite neu laden
  // redirect ist die Seite, die nach dem Speichern neu geladen wird
  if (do_save) {
    #ifdef DEBUG
      Serial.println("Settings changed by web page, saving to EEPROM");
    #endif
    saveCredentials();
  }
  // Seite nochmal aktualisiert senden
  request->redirect(redirect);
}

// ##############################################################################
//
//  #### ##    ## #### ########
//   ##  ###   ##  ##     ##
//   ##  ####  ##  ##     ##
//   ##  ## ## ##  ##     ##
//   ##  ##  ####  ##     ##
//   ##  ##   ###  ##     ##
//  #### ##    ## ####    ##
//
// ##############################################################################

// Initialize server routes and start ElegantOTA
void init_server() {
  DEBUG_PRINTLN("Installing Handlers for Server...");

  // Route for all /get GET requests
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request) { myGEThandler(request); });

  // Route for POST requests
  // run handleUpload function when any file is uploaded
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
      DEBUG_PRINTLN("Server UPLOAD request");
      request->send(200);
  }, handleUpload);

  // Route for root / web page GET requests
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Server INDEX ROOT request");
    if(request->hasParam("delete")) {
      Serial.println("Server DELETE request");
      String value = request->getParam("delete")->value();
      if ((!value.endsWith("html")) && (!value.endsWith("css")))
        SPIFFS.remove(value);
    }
    request->send(SPIFFS , "/index.html", String(), false, html_process_root);
  });

  // Route for scalings page
  server.on("/scalings.html", HTTP_GET, [](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Server SCALINGS PAGE request");
    request->send(SPIFFS , "/scalings.html", String(), false, html_process_page);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    DEBUG_PRINTLN("Server CSS request");
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.onNotFound(notFound);

  // andere Dateien, Grafiken
  server.serveStatic("/", SPIFFS, "/");

  ElegantOTA.begin(&server);    // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);  // Start server
  server.begin();
}

// Stops the server and cleans up resources
void stop_server() {
  DEBUG_PRINTLN("AsyncWebServer stopped");
  server.end();
}

#endif