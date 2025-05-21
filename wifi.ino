#define NUM_JSON_LINES_PER_DAY 9
#define JSON_BUF_SIZE 256
#define URL_LENGTH 70

// String URL = "https://isomer-user-content.by.gov.sg/muis_prayers_timetable_2025.json";
char URL_format[] = "https://isomer-user-content.by.gov.sg/muis_prayers_timetable_%d.json";
char URL[URL_LENGTH + 1] = {0};
unsigned int localPort = 2390;  // local port to listen for UDP packets
/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
IPAddress timeServerIP;
const char* ntpServerName = "sg.pool.ntp.org";
const int NTP_PACKET_SIZE = 48;      // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets
WiFiUDP udp;                         // A UDP instance to let us send and receive packets over UDP
time_t SGT;
struct tm tm_buf;
uint16_t year = 0;
char response[JSON_BUF_SIZE];
JSONVar json;
bool LED_status = false;

void init_wifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_SSID, PASSWORD);
    #ifdef DEBUG
    Serial.println("");
    Serial.print("Connecting");
    #endif
    while (WiFi.status() != WL_CONNECTED) {
        yield();
        delay(500);
        digitalWrite(CYD_LED_BLUE, LED_status = !LED_status);
    }
    digitalWrite(CYD_LED_BLUE, HIGH);
    #ifdef DEBUG
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    #endif
}

void init_udp() {
    udp.begin(localPort);
    #ifdef DEBUG
    Serial.print("Starting UDP on port ");
    Serial.println(localPort);
    #endif
}

bool request_prayer_times() {
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClientSecure client;
        client.setInsecure();
        HTTPClient https;
        #ifdef DEBUG
        Serial.print("Requesting ");
        Serial.println(URL);
        #endif
        int httpCode = 0;

        if (https.begin(client, URL)) {
            httpCode = https.GET();
            #ifdef DEBUG
            Serial.println("============== Response code: " + String(httpCode));
            #endif
            if (httpCode == HTTP_CODE_OK) {
                int day_year = tm_buf.tm_yday;
                int num_lines_skip = 1 + day_year * NUM_JSON_LINES_PER_DAY;
                WiFiClient *stream = https.getStreamPtr();
                //skip all the previous days, i.e. 1 Jan till yesterday
                for (int i = 0; i < num_lines_skip; i++)
                    stream->readBytesUntil(0x0A, response, JSON_BUF_SIZE);
                stream->readBytesUntil(':', response, JSON_BUF_SIZE); //skip date string
                size_t num_bytes = stream->readBytesUntil('}', response, JSON_BUF_SIZE - 2);
                response[num_bytes] = '}';
                response[num_bytes + 1] = 0;
                #ifdef DEBUG
                printf("%s[%d]: response:\n", __FILE__, __LINE__);
                Serial.print(response);
                #endif
            }
            https.end();
            if (httpCode == HTTP_CODE_OK) {
                json = JSON.parse(response);
                yield();
            }
        } else {
            #ifdef DEBUG
            Serial.printf("[HTTPS] Unable to connect\n");
            #endif
        }
        if (httpCode > 0) return true;
    }
    return false;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
    #ifdef DEBUG
    Serial.println("sending NTP packet...");
    #endif
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;  // LI, Version, Mode
    packetBuffer[1] = 0;           // Stratum, or type of clock
    packetBuffer[2] = 6;           // Polling Interval
    packetBuffer[3] = 0xEC;        // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123);  // NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

uint16_t request_time() {
  // get a random server from the pool
    WiFi.hostByName(ntpServerName, timeServerIP);
    sendNTPpacket(timeServerIP);  // send an NTP packet to a time server
    // wait to see if a reply is available
    yield();
    delay(1000);

    int cb = udp.parsePacket();
    if (!cb) {
        #ifdef DEBUG
        Serial.println("no packet yet");
        #endif
        return 2400;
    }
    #ifdef DEBUG
    Serial.print("packet received, length=");
    Serial.println(cb);
    #endif

    udp.read(packetBuffer, NTP_PACKET_SIZE);  // read the packet into the buffer
    // the timestamp starts at byte 40 of the received packet and is four bytes,
    //  or two words, long. First, extract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    const unsigned long SGT_OFFSET = 8 * 60 * 60UL;
    SGT = secsSince1900 - seventyYears + SGT_OFFSET;
    struct tm *ptm = gmtime_r(&SGT, &tm_buf);
    year = ptm->tm_year + 1900;
    snprintf(URL, URL_LENGTH + 1, URL_format, year);
    
    #ifdef DEBUG
    Serial.print("Unix time = ");
    Serial.println(SGT);
    // printf("local:     %s", asctime(localtime(&SGT)));
    #endif

    return ptm->tm_hour * 100 + ptm->tm_min;
}
