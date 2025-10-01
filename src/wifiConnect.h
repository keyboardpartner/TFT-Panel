#ifndef WIFI_CONNECT_H
#define WIFI_CONNECT_H

// Not a real header file, just some code to be included

// ##############################################################################
// Connect to WiFi if enabled by WIFI_ENABLED
// This function connects to the WiFi network
// using the settings stored in the settings struct


 bool wifi_connected() {
  bool is_ok = (WiFi.status() == WL_CONNECTED || (settings.wifiAPenabled && settings.wifiEnabled));
  timeOK &= is_ok;
  return is_ok;
 }

// Default WPS configuration
static esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

bool use_wps = false;
volatile bool wps_in_progress = false;

/*
String wpspin2string(uint8_t a[]) {
  char wps_pin[9];
  for (int i = 0; i < 8; i++) {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}
*/

void wifi_event(WiFiEvent_t event){
  int i = 0;
  DEBUG_PRINT("WIFI Event: ");
  switch(event) {
  case SYSTEM_EVENT_STA_START:
  case SYSTEM_EVENT_AP_START:
    DEBUG_PRINT("STA or AP started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    DEBUG_PRINT("STA connected to: " + String(WiFi.SSID()));
    DEBUG_PRINT(" with own IP: ");
    DEBUG_PRINT(WiFi.localIP());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    DEBUG_PRINT("STA Disconnected");
    if (use_wps) {
      DEBUG_PRINT(", try to reconnect");
      WiFi.reconnect();
    }
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    if (use_wps) {
      DEBUG_PRINT("WPS ok, stop WPS and connect to: " + String(WiFi.SSID()));
      esp_wifi_wps_disable();
      delay(10);
      WiFi.begin();
    } else {
      DEBUG_PRINT("Success, WPS not used");
    }
    wps_in_progress = false;
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    if (use_wps) {
      DEBUG_PRINT("WPS failed, retry");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
    }
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    if (use_wps) {
      DEBUG_PRINT("WPS timeout, retry");
      esp_wifi_wps_disable();
      esp_wifi_wps_enable(&config);
      esp_wifi_wps_start(0);
    }
    break;
  case SYSTEM_EVENT_STA_STOP:
  case SYSTEM_EVENT_AP_STOP:
    DEBUG_PRINT("STA orAP stopped");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    DEBUG_PRINT("STA connected");
    break;
/*
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    DEBUG_PRINTLN("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    break;
*/

  default:
    DEBUG_PRINT(event);
    break;
  }
  DEBUG_PRINTLN("");
}

void config_time() {
  // Zeitanzeige initialisieren
  dialogBox.draw(F("Get time from server"), MY_NTP_SERVER, DB_INFO);
  configTzTime(MY_TIMEZONE, MY_NTP_SERVER); // --> Here is the IMPORTANT ONE LINER needed in your sketch!
  // timeinfo should be valid here, assigned in setup()
  if(getLocalTime(timeinfo, 5000)) {
    timeOK = true;
  } else {
    dialogBox.message(F("Failed to obtain time from"), MY_NTP_SERVER, 2000, DB_ERROR);
  }
}


// Start WPS connection
void wps_connect() {
  timeOK = false;
  use_wps = true;
  char ssid[32];     // Router SSID
  spkrBeep(25);
  dialogBox.draw(F("WPS config started"), F("Please wait"), DB_INFO);
  DEBUG_PRINTLN("WPS config started");
  wps_in_progress = true;
  WiFi.onEvent(wifi_event);
  WiFi.begin(); // Start WiFi in STA mode
  WiFi.mode(WIFI_STA);
  esp_wifi_wps_enable(&config);
  esp_wifi_wps_start(0);
  while (wps_in_progress) {
    delay(50);
  }
  use_wps = false;
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.SSID().toCharArray(ssid, WiFi.SSID().length() + 1);
    spkrBeep(50);
    dialogBox.draw(F("Connected to"), ssid, DB_INFO);
    DEBUG_PRINTLN("WPS successful");
    delay(1000);
    settings.wifiWPSused = true;
    init_server();
    // Zeitanzeige initialisieren
    config_time();
  } else {
    dialogBox.modalDlg(F("WPS failed"), "", DB_ERROR);
    DEBUG_PRINTLN("WPS failed");
    WiFi.disconnect();
    settings.wifiWPSused = false;
  }
  esp_wifi_wps_disable();
}

// Connect to WiFi in Access Point mode
wl_status_t wifi_connect_ap() {
  timeOK = false;
  use_wps = false;
  WiFi.softAP("KBP PanelMeter", "");	// access point, kein PW
  delay(500);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.onEvent(wifi_event); // for debugging

  spkrBeep(25);
  dialogBox.draw(F("Access Point IP:"), WiFi.softAPIP().toString(), DB_INFO);
  delay(2000);
  DEBUG_PRINT(F("Created Access Point with IP "));
  DEBUG_PRINTLN(WiFi.softAPIP());
  init_server();
  return WL_CONNECTED; // Return true to indicate AP mode is active
}

// Connect to WiFi in Station mode, start server if connected
// Returns the WiFi status
wl_status_t wifi_connect_sta() {
  timeOK = false;
  use_wps = false; // Use WPS if enabled in settings
  WiFi.mode(WIFI_STA);
  WiFi.onEvent((WiFiEventCb)wifi_event); // defined in wps.h
  int i;
  wl_status_t status;
  char ssid[32];     // Router SSID
  String password = settings.password;
  i = 0;
  spkrBeep(25);
  if (settings.wifiWPSused) {
    WiFi.begin();
    dialogBox.draw(F("Connecting to"), F("known network"), DB_INFO);
    DEBUG_PRINT(F("Connected to known network"));
    delay(1000);
    while ((WiFi.status() != WL_CONNECTED) && (i < 25)) {
      digitalWrite(LED_PIN, i & 1);
      delay(250);
      i++;
    }
    WiFi.SSID().toCharArray(ssid, WiFi.SSID().length() + 1);
    dialogBox.draw(F("Connected to"), ssid, DB_INFO);
    delay(1000);
    DEBUG_PRINT(F("Connected to: "));
    DEBUG_PRINTLN(WiFi.SSID());
  } else {
    WiFi.begin(settings.ssid, settings.password);
    dialogBox.draw(F("Connecting to"), settings.ssid, DB_INFO);
    delay(1000);
    while ((WiFi.status() != WL_CONNECTED) && (i < 25)) {
      digitalWrite(LED_PIN, i & 1);
      delay(250);
      i++;
    }
    DEBUG_PRINT(F("Connect to network in settings: "));
    DEBUG_PRINTLN(settings.ssid);
  }
  status = WiFi.status();
  if (status == WL_CONNECTED) {
    spkrOKbeep();
    dialogBox.draw(F("IP for Browser Config:"), WiFi.localIP().toString(), DB_INFO);
    delay(2000);
    init_server();
    // Zeitanzeige initialisieren
    config_time();
  } else {
    dialogBox.modalDlg(F("WIFI not available"), F(""), DB_ERROR);
  }
  return status;
}

// WiFi.scanNetworks will return the number of networks found.
int wifi_scanNetworks(menuArr_t wifiArr) {
    int n = WiFi.scanNetworks();
    DEBUG_PRINT(F("Scan done, "));
    if (n == 0) {
      DEBUG_PRINTLN(F("no networks found"));
      strcpy(wifiArr[0], "No networks found");
      n = 1;
    } else {
      if (n > 15) {
        n = 15;
      }
      DEBUG_PRINTLN(F(" networks found"));
      DEBUG_PRINTLN(F("Nr | SSID                             | RSSI | CH | Encryption"));
      for (int i = 0; i < n; ++i) {
        // Print SSID and RSSI for each network found
        strcpy(wifiArr[i], WiFi.SSID(i).c_str());
        DEBUG_PRINTF("%2d",i + 1);
        DEBUG_PRINT(F(" | "));
        DEBUG_PRINTF("%-32.32s", WiFi.SSID(i).c_str());
        DEBUG_PRINT(F(" | "));
        DEBUG_PRINTF("%4d", WiFi.RSSI(i));
        DEBUG_PRINT(F(" | "));
        DEBUG_PRINTF("%2d", WiFi.channel(i));
        DEBUG_PRINT(F(" | "));
        switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN:
            DEBUG_PRINT(F("open"));
            break;
        case WIFI_AUTH_WEP:
            DEBUG_PRINT(F("WEP"));
            break;
        case WIFI_AUTH_WPA_PSK:
            DEBUG_PRINT(F("WPA"));
            break;
        case WIFI_AUTH_WPA2_PSK:
            DEBUG_PRINT(F("WPA2"));
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            DEBUG_PRINT(F("WPA+WPA2"));
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            DEBUG_PRINT(F("WPA2-EAP"));
            break;
        case WIFI_AUTH_WPA3_PSK:
            DEBUG_PRINT(F("WPA3"));
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            DEBUG_PRINT(F("WPA2+WPA3"));
            break;
        case WIFI_AUTH_WAPI_PSK:
            DEBUG_PRINT(F("WAPI"));
            break;
        default:
            DEBUG_PRINT(F("unknown"));
        }
        DEBUG_PRINTLN();
      }
    }
    DEBUG_PRINTLN("");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    return n; // Return the number of networks found
  }

// ##############################################################################

#endif // WIFI_CONNECT_H