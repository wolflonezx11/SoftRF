/*
 * WebHelper.cpp
 * Copyright (C) 2016-2017 Linar Yusupov
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <nRF905.h>

#include "WebHelper.h"
#include "BatteryHelper.h"

ESP8266WebServer server ( 80 );
const char* serverIndex = "\
<html>\
  <head>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <title>Firmware update</title>\
  </head>\
<body>\
<body>\
 <h1 align=center>Firmware update</h1>\
 <hr>\
 <table width=100\%>\
  <tr>\
    <td align=left>\
    <form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>\
    </td>\
  </tr>\
 </table>\
</body>\
</html>";

//char hexdata[2 * PKT_SIZE + 1] ;
static uint32_t prev_rx_pkt_cnt = 0;

byte getVal(char c)
{
   if(c >= '0' && c <= '9')
     return (byte)(c - '0');
   else
     return (byte)(toupper(c)-'A'+10);
}

void Hex2Bin(String str, byte *buffer)
{
  char hexdata[2 * PKT_SIZE + 1];
  
  str.toCharArray(hexdata, sizeof(hexdata));
  for(int j = 0; j < PKT_SIZE * 2 ; j+=2)
  {
    buffer[j>>1] = getVal(hexdata[j+1]) + (getVal(hexdata[j]) << 4);
  }
}

String Bin2Hex(byte *buffer)
{
  String str = "";
  for (int i=0; i < PKT_SIZE; i++) {
    byte c = buffer[i];
    str += (c < 0x10 ? "0" : "") + String(c, HEX);
  }
  return str;
}

/*



*/

void handleSettings() {

  char temp[2600];

  snprintf ( temp, 2600,

"<html>\
<head>\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
<title>Settings</title>\
</head>\
<body>\
<h1 align=center>Settings</h1>\
<form action='/input' method='GET'>\
<table width=100\%>\
<tr>\
<th align=left>Mode</th>\
<td align=right>\
<select name='mode'>\
<option %s value='%d'>Normal</option>\
<option %s value='%d'>Tx Test</option>\
<option %s value='%d'>Rx Test</option>\
<option %s value='%d'>Bridge</option>\
<option %s value='%d'>UAV</option>\
</select>\
</td>\
</tr>\
<tr>\
<th align=left>Band</th>\
<td align=right>\
<select name='band'>\
<option %s value='%d'>EU (868.4 MHz)</option>\
<option %s value='%d'>RU1 (868.2 MHz)</option>\
<option %s value='%d'>RU2 (868.8 MHz)</option>\
<option %s value='%d'>CN (433 MHz)</option>\
<option %s value='%d'>US (915 MHz)</option>\
<option %s value='%d'>NZ (869.25 MHz)</option>\
<option %s value='%d'>AU (921 MHz)</option>\
</select>\
</td>\
</tr>\
<tr>\
<th align=left>Tx Power (mW)</th>\
<td align=right>\
<select name='txpower'>\
<option %s value='%d'>10</option>\
<option %s value='%d'>6</option>\
<option %s value='%d'>0.6</option>\
<option %s value='%d'>0.1</option>\
<option %s value='%d'>OFF</option>\
</select>\
</td>\
</tr>\
<tr>\
<th align=left>Volume</th>\
<td align=right>\
<select name='volume'>\
<option %s value='1'>Low</option>\
<option %s value='2'>Medium</option>\
<option %s value='3'>High</option>\
</select>\
</td>\
</tr>\
<tr>\
<th align=left>NMEA:</th>\
</tr>\
<tr>\
<th align=left>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;GNSS</th>\
<td align=right>\
<input type='radio' name='nmea_g' value='0' %s>Off\
<input type='radio' name='nmea_g' value='1' %s>On\
</td>\
</tr>\
<tr>\
<th align=left>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Private</th>\
<td align=right>\
<input type='radio' name='nmea_p' value='0' %s>Off\
<input type='radio' name='nmea_p' value='1' %s>On\
</td>\
</tr>\
<tr>\
<th align=left>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Legacy</th>\
<td align=right>\
<input type='radio' name='nmea_l' value='0' %s>Off\
<input type='radio' name='nmea_l' value='1' %s>On\
</td>\
</tr>\
<tr>\
<tr>\
<th align=left>LED ring:</th>\
</tr>\
<th align=left>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Direction</th>\
<td align=right>\
<select name='pointer'>\
<option %s value='%d'>CoG Up</option>\
<option %s value='%d'>North Up</option>\
</select>\
</td>\
</tr>\
</table>\
<p align=center><INPUT type='submit' value='Save and restart'><p>\
</form>\
</body>\
</html>",
  (settings->mode == SOFTRF_MODE_NORMAL ? "selected" : "") , SOFTRF_MODE_NORMAL,
  (settings->mode == SOFTRF_MODE_TX_TEST ? "selected" : ""), SOFTRF_MODE_TX_TEST,
  (settings->mode == SOFTRF_MODE_RX_TEST ? "selected" : ""), SOFTRF_MODE_RX_TEST,
  (settings->mode == SOFTRF_MODE_BRIDGE ? "selected" : ""), SOFTRF_MODE_BRIDGE,
  (settings->mode == SOFTRF_MODE_UAV_BEACON ? "selected" : ""), SOFTRF_MODE_UAV_BEACON,
  (settings->band == RF_BAND_EU ? "selected" : ""), RF_BAND_EU,
  (settings->band == RF_BAND_RU1 ? "selected" : ""), RF_BAND_RU1,
  (settings->band == RF_BAND_RU2 ? "selected" : ""), RF_BAND_RU2,
  (settings->band == RF_BAND_CN ? "selected" : ""), RF_BAND_CN,
  (settings->band == RF_BAND_US ? "selected" : ""),  RF_BAND_US,
  (settings->band == RF_BAND_NZ ? "selected" : ""), RF_BAND_NZ,
  (settings->band == RF_BAND_AU ? "selected" : ""),  RF_BAND_AU,
  (settings->txpower == NRF905_PWR_10 ? "selected" : ""),  NRF905_PWR_10,
  (settings->txpower == NRF905_PWR_6 ? "selected" : ""),  NRF905_PWR_6,
  (settings->txpower == NRF905_PWR_n2 ? "selected" : ""),  NRF905_PWR_n2,
  (settings->txpower == NRF905_PWR_n10 ? "selected" : ""),  NRF905_PWR_n10,
  (settings->txpower == NRF905_TX_PWR_OFF ? "selected" : ""),  NRF905_TX_PWR_OFF,
  (settings->volume == 1 ? "selected" : "") , (settings->volume == 2 ? "selected" : ""), (settings->volume == 3 ? "selected" : ""),
  (settings->nmea_g == 0 ? "checked" : "") , (settings->nmea_g == 1 ? "checked" : ""),
  (settings->nmea_p == 0 ? "checked" : "") , (settings->nmea_p == 1 ? "checked" : ""),
  (settings->nmea_l == 0 ? "checked" : "") , (settings->nmea_l == 1 ? "checked" : ""),
  (settings->pointer == DIRECTION_TRACK_UP ? "selected" : ""), DIRECTION_TRACK_UP,
  (settings->pointer == DIRECTION_NORTH_UP ? "selected" : ""), DIRECTION_NORTH_UP
  );
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 200, "text/html", temp );
}


void handleRoot() {
  char Root_temp[2048];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  float vdd = Battery_voltage() ;

  time_t timestamp = ThisAircraft.timestamp;
  unsigned int sats = gnss.satellites.value(); // Number of satellites in use (u32)
  char str_lat[16];
  char str_lon[16];
  char str_Vcc[8];
  int32_t alt = ThisAircraft.altitude;

  dtostrf(ThisAircraft.latitude, 8, 4, str_lat);
  dtostrf(ThisAircraft.longtitude, 8, 4, str_lon);
  dtostrf(vdd, 4, 2, str_Vcc);

  snprintf ( Root_temp, 2048,

"<html>\
  <head>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <title>SoftRF status</title>\
  </head>\
<body>\
 <h1 align=center>SoftRF status</h1>\
 <table width=100\%>\
  <tr><th align=left>Device Id</th><td align=right>%X</td></tr>\
  <tr><th align=left>Software Version</th><td align=right>%s</td></tr>\
  <tr><th align=left>Uptime</th><td align=right>%02d:%02d:%02d</td></tr>\
  <tr><th align=left>Battery voltage</th><td align=right>%s</td></tr>\
  <tr><th align=left>Packets:</th></tr>\
  <tr><th align=left>&nbsp;&nbsp;&nbsp;&nbsp;- transmitted</th><td align=right>%u</td></tr>\
  <tr><th align=left>&nbsp;&nbsp;&nbsp;&nbsp;- received</th><td align=right>%u</td></tr>\
 </table>\
 <h2 align=center>Most recent GNSS fix</h2>\
 <table width=100\%>\
  <tr><th align=left>Time</th><td align=right>%u</td></tr>\
  <tr><th align=left>Sattelites</th><td align=right>%d</td></tr>\
  <tr><th align=left>Latitude</th><td align=right>%s</td></tr>\
  <tr><th align=left>Longtitude</th><td align=right>%s</td></tr>\
  <tr><th align=left>Altitude</th><td align=right>%d</td></tr>\
 </table>\
 <hr>\
 <table width=100\%>\
  <tr>\
    <td align=left><input type=button onClick=\"location.href='/settings'\" value='Settings'></td>\
    <td align=right><input type=button onClick=\"location.href='/firmware'\" value='Firmware update'></td>\
  </tr>\
 </table>\
</body>\
</html>",
    ThisAircraft.addr, SOFTRF_FIRMWARE_VERSION,
    hr, min % 60, sec % 60, str_Vcc, tx_packets_counter, rx_packets_counter,
    timestamp, sats, str_lat, str_lon, alt
  );
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send ( 200, "text/html", Root_temp );

}

#define BOOL_STR(x) (x ? "true":"false")

void handleInput() {

  char temp[1024];

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    if (server.argName(i).equals("mode")) {
      settings->mode = server.arg(i).toInt();
    } else if (server.argName(i).equals("band")) {
      settings->band = server.arg(i).toInt();
    } else if (server.argName(i).equals("txpower")) {
      settings->txpower = server.arg(i).toInt();
    } else if (server.argName(i).equals("volume")) {
      settings->volume = server.arg(i).toInt();
    } else if (server.argName(i).equals("nmea_g")) {
      settings->nmea_g = server.arg(i).toInt();
    } else if (server.argName(i).equals("nmea_p")) {
      settings->nmea_p = server.arg(i).toInt();
    } else if (server.argName(i).equals("nmea_l")) {
      settings->nmea_l = server.arg(i).toInt();
    } else if (server.argName(i).equals("pointer")) {
      settings->pointer = server.arg(i).toInt();
    }
  }
  snprintf ( temp, 1024,
"<html>\
<head>\
<meta http-equiv='refresh' content='15; url=/'/>\
<meta name='viewport' content='width=device-width, initial-scale=1'>\
<title>SoftRF Settings</title>\
</head>\
<body>\
<h1 align=center>New settings:</h1>\
<table width=100\%>\
<tr><th align=left>Mode</th><td align=right>%d</td></tr>\
<tr><th align=left>Band</th><td align=right>%d</td></tr>\
<tr><th align=left>Tx Power</th><td align=right>%d</td></tr>\
<tr><th align=left>Volume</th><td align=right>%d</td></tr>\
<tr><th align=left>NMEA GNSS</th><td align=right>%s</td></tr>\
<tr><th align=left>NMEA Private</th><td align=right>%s</td></tr>\
<tr><th align=left>NMEA Legacy</th><td align=right>%s</td></tr>\
<tr><th align=left>LED pointer</th><td align=right>%d</td></tr>\
</table>\
<hr>\
  <p align=center><h1 align=center>Restart is in progress... Please, wait!</h1>\<p>\
</body>\
</html>",
  settings->mode, settings->band, settings->txpower, settings->volume,
  BOOL_STR(settings->nmea_g), BOOL_STR(settings->nmea_p), BOOL_STR(settings->nmea_l),
  settings->pointer
  );
  server.send ( 200, "text/html", temp );
  EEPROM_store();
  delay(3000);
  ESP.restart();
}

void handleNotFound() {

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void Web_setup()
{
  //server.on ( "/", handleRxRoot );

  server.on ( "/", handleRoot );
  server.on ( "/settings", handleSettings );
  
  server.on ( "/input", handleInput );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.on("/firmware", HTTP_GET, [](){
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/html", serverIndex);
  });
  server.onNotFound ( handleNotFound );

  server.on("/update", HTTP_POST, [](){
    server.sendHeader("Connection", "close");
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
    ESP.restart();
  },[](){
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if(!Update.begin(maxSketchSpace)){//start with max available size
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_WRITE){
      if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_END){
      if(Update.end(true)){ //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });

  server.begin();
  Serial.println ( "HTTP server started" );

//  TxDataTemplate.toCharArray(hexdata, sizeof(hexdata));

  delay(1000);
}

void Web_loop()
{
  server.handleClient();
}

