#define TIME_Y_POS 0
#define DATE_Y_POS 55
#define FAJR_Y_POS 115
#define HIJRI_Y_POS 90
#define PRAYER_Y_SPACING 40
#define MAX_DATE_LENGTH 21
#define TIME_LENGTH 4

#define FSS9 &FreeSans9pt7b
#define FSS12 &FreeSans12pt7b
#define FSS18 &FreeSans18pt7b
#define FSS24 &FreeSans24pt7b

#define FSSB9 &FreeSansBold9pt7b
#define FSSB12 &FreeSansBold12pt7b
#define FSSB18 &FreeSansBold18pt7b
#define FSSB24 &FreeSansBold24pt7b

#define FM9 &FreeMono9pt7b
#define FM12 &FreeMono12pt7b
#define FM18 &FreeMono18pt7b
#define FM24 &FreeMono24pt7b

#define FMB9 &FreeMonoBold9pt7b
#define FMB12 &FreeMonoBold12pt7b
#define FMB18 &FreeMonoBold18pt7b
#define FMB24 &FreeMonoBold24pt7b

// use 12 bit precission for LEDC timer
#define LEDC_TIMER_12_BIT  12
// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite sprite = TFT_eSprite(&tft);

enum MUIS_Items {
    FAJR,
    DHUHR,
    ASR,
    MAGHRIB,
    ISHA,
    NONE
};
char prayers[5][6];
uint16_t prayer_time_ints[5];
bool is_prayer_now[5] = {false, false, false, false, false};
uint8_t current_prayer = NONE;
uint16_t prayer_end;

char DATE[MAX_DATE_LENGTH] = {0};
char HIJRI[MAX_DATE_LENGTH] = {0};
char TIME[TIME_LENGTH + 1] = {0};
extern JSONVar json;
extern struct tm tm_buf;
extern uint16_t current_time;

void init_ST7789() {
    tft.init();
    ledcAttach(TFT_BL, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcWrite(TFT_BL, 4096 / 32);
    tft.setRotation(2);
    // Add leading space for all prayers
    for (uint8_t i = 0; i < 5; i++) {
        prayers[i][0] = ' ';
    }
    sprite.setColorDepth(8);
    sprite.createSprite(tft.width(), tft.height());
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextDatum(TC_DATUM);
    sprite.pushSprite(0, 0);
}

void display_datetime(void) {
    //time
    sprite.setTextColor(TFT_GREEN, TFT_BLACK, true);
    sprite.setTextFont(7); // Set the text font to font number 2
    sprite.drawString(TIME, tft.width() / 2, TIME_Y_POS);
    //date
    sprite.setTextColor(TFT_WHITE, TFT_BLACK, true);
    sprite.setFreeFont(FSSB18);
    sprite.drawString(DATE, tft.width() / 2, DATE_Y_POS);
    //hijri date
    sprite.setFreeFont(FSS12);
    sprite.drawString(HIJRI, tft.width() / 2, HIJRI_Y_POS);
    sprite.pushSprite(0, 0);
}

void display_prayer_times(void) {
    #ifdef DEBUG
    Serial.println("Updating prayer times");
    #endif
    sprite.setFreeFont(FSS24);
    for (uint8_t i = 0; i < 5; i++) {
        if (is_prayer_now[i])
            sprite.setTextColor(TFT_YELLOW, TFT_BLACK, true);
        else
            sprite.setTextColor(TFT_WHITE, TFT_BLACK, true);
        sprite.drawString(prayers[i], tft.width() / 2, PRAYER_Y_SPACING * i + FAJR_Y_POS);
    }
    sprite.pushSprite(0, 0);
}

void update_display(void) {
    display_datetime();
    display_prayer_times();
}

bool update_time(void) {
    #ifdef DEBUG
    Serial.println(current_time);
    #endif
    if (current_time < 2400) {
        strftime(DATE, MAX_DATE_LENGTH - 1,"%a %d %b", &tm_buf);
        sprintf(TIME, "%04d", current_time);
        memset(is_prayer_now, false, 5);
        if (current_time >= prayer_time_ints[FAJR] && current_time < prayer_end) {
            is_prayer_now[FAJR] = true;
            #ifdef DEBUG
            Serial.println("It's fajr now");
            #endif
        } else if (current_time >= prayer_time_ints[DHUHR] && current_time < prayer_time_ints[ASR]) {
            is_prayer_now[DHUHR] = true;
            #ifdef DEBUG
            Serial.println("It's dhuhr now");
            #endif
        } else if (current_time >= prayer_time_ints[ASR] && current_time < prayer_time_ints[MAGHRIB]) {
            is_prayer_now[ASR] = true;
            #ifdef DEBUG
            Serial.println("It's asr now");
            #endif
        } else if (current_time >= prayer_time_ints[MAGHRIB] && current_time < prayer_time_ints[ISHA]) {
            is_prayer_now[MAGHRIB] = true;
            #ifdef DEBUG
            Serial.println("It's maghrib now");
            #endif
        } else if (current_time >= prayer_time_ints[ISHA] || current_time < prayer_time_ints[FAJR]) {
            is_prayer_now[ISHA] = true;
            #ifdef DEBUG
            Serial.println("It's isha now");
            #endif
        }
        return true;
    }
    return false;
}

void update_prayer_times(void) {
    String hijriString = json["hijri_date"];
    const char *hijri = hijriString.c_str();
    snprintf(HIJRI, MAX_DATE_LENGTH - 1, "%s", hijri);
    bool found_space = false;
    //Ignore hijri year; stop after month
    for (uint8_t i = 0; i < MAX_DATE_LENGTH; i++) {
        if (HIJRI[i] == ' ') {
            if (!found_space) found_space = true;
            else {
                HIJRI[i] = 0;
                break;
            }
        }
    }
    
    String fajrString = json["subuh"];
    const char *fajr = fajrString.c_str();
    strncpy(prayers[FAJR] + 1, fajr, 4);
    prayer_time_ints[FAJR] = (fajr[0] - '0') * 100 + (fajr[2] - '0') * 10 + fajr[3] - '0';
    prayer_end = prayer_time_ints[FAJR] + 100;
    
    String dhuhrString = json["zohor"];
    const char *dhuhr = dhuhrString.c_str();
    //dhuhr is either 12:xy or 1:xy
    if (dhuhr[1] == '2') {
      strncpy(prayers[DHUHR], dhuhr, 5);
      prayer_time_ints[DHUHR] = 12 * 100 + (dhuhr[3] - '0') * 10 + dhuhr[4] - '0';
    } else {
      strncpy(prayers[DHUHR] + 1, dhuhr, 4);
      prayer_time_ints[DHUHR] = 13 * 100 + (dhuhr[2] - '0') * 10 + dhuhr[3] - '0';
    }

    String asrString = json["asar"];
    const char *asr = asrString.c_str();
    strncpy(prayers[ASR] + 1, asr, 4);
    prayer_time_ints[ASR] = (asr[0] - '0' + 12) * 100 + (asr[2] - '0') * 10 + asr[3] - '0';
    
    String maghribString = json["maghrib"];
    const char *maghrib = maghribString.c_str();
    strncpy(prayers[MAGHRIB] + 1, maghrib, 4);
    prayer_time_ints[MAGHRIB] = (maghrib[0] - '0' + 12) * 100 + (maghrib[2] - '0') * 10 + maghrib[3] - '0';
    
    String ishaString = json["isyak"];
    const char *isha = ishaString.c_str();
    strncpy(prayers[ISHA] + 1, isha, 4);
    prayer_time_ints[ISHA] = (isha[0] - '0' + 12) * 100 + (isha[2] - '0') * 10 + isha[3] - '0';

    #ifdef DEBUG
    printf("\n%s[%d]:\n", __FILE__, __LINE__);
    for (uint8_t i = 0; i < 5; i++) {
        Serial.print(prayers[i]);
        Serial.print("\t");
        Serial.print(prayer_time_ints[i]);
        Serial.print("\t");
        Serial.println(is_prayer_now[i]);
    }
    #endif
}
