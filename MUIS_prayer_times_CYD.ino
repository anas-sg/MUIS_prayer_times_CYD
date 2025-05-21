#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "wifi_credentials.h"

#define CYD_LED_BLUE 17
#define CYD_LED_RED 4
#define CYD_LED_GREEN 16
#define PRAYER_FETCH_INTERVAL_MS (12 * 60 * 60 * 1000)
#define NTP_FETCH_INTERVAL_MS (5 * 1000)
// #define DEBUG

uint16_t current_time;
unsigned long prayer_fetch_time;

void wait_for_time() {
    bool got_time = false;
    do {
        current_time = request_time();
        got_time = update_time();
        yield();
        digitalWrite(CYD_LED_RED, LOW);
    } while (!got_time);
    digitalWrite(CYD_LED_RED, HIGH);
}

void wait_for_prayer_times() {
    bool got_prayer_times = false;
    do {
        got_prayer_times = request_prayer_times();
        yield();
        delay(500);
        digitalWrite(CYD_LED_GREEN, LOW);
    } while (!got_prayer_times);
    digitalWrite(CYD_LED_GREEN, HIGH);
    prayer_fetch_time = millis();
    update_prayer_times();
    update_time();
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    pinMode(CYD_LED_RED, OUTPUT);
    pinMode(CYD_LED_GREEN, OUTPUT);
    pinMode(CYD_LED_BLUE, OUTPUT);
    // Turn LED Off
    digitalWrite(CYD_LED_RED, HIGH); // The LEDs are "active low", meaning HIGH == off, LOW == on
    digitalWrite(CYD_LED_GREEN, HIGH);
    digitalWrite(CYD_LED_BLUE, HIGH);
    #ifdef DEBUG
    Serial.println("Finished init LED");
    #endif
    init_ST7789();
    #ifdef DEBUG
    Serial.println("Finished init_ST7789");
    #endif
    init_wifi();
    #ifdef DEBUG
    Serial.println("Finished init_wifi");
    #endif
    init_udp();
    #ifdef DEBUG
    Serial.println("Finished init_udp");
    #endif
    
    wait_for_time();
    wait_for_prayer_times();
    update_display();
}

void loop() {
    // put your main code here, to run repeatedly:
    wait_for_time();
    display_datetime();
    if ((current_time == 0) || ((millis() - prayer_fetch_time) >= PRAYER_FETCH_INTERVAL_MS)) {
        wait_for_prayer_times();
    }
    display_prayer_times();
    delay(NTP_FETCH_INTERVAL_MS);
}
