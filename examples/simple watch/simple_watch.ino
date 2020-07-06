/*
 * it's only the watch
 * you can switch it on and off by button
 * and it shows the time and the battery level
 */
 
#define LILYGO_TWATCH_2020_V1        // If you are using T-Watch-2020 version, please open this macro definition

#include <TTGO.h>
// #include "esp_wifi.h"
#include <WiFi.h>
#include <soc/rtc.h>

TTGOClass *ttgo;
char buf[128];
bool irq = false;
bool ison = true;

void setup()
{
    Serial.begin(115200);
    ttgo = TTGOClass::getWatch();
    ttgo->begin();
    ttgo->openBL();

    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(AXP202_INT, [] {
        irq = true;
    }, FALLING);

    ttgo->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);
    ttgo->power->clearIRQ();

    // low power
    // ttgo->bma->enableStepCountInterrupt(false);
    // WiFi.mode(WIFI_OFF);
    rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
}

void loop()
{
    if (irq) {
        ttgo->power->readIRQ();
        if (ttgo->power->isPEKShortPressIRQ()) {
            if (ison) {
              ttgo->closeBL();
              //ttgo->stopLvglTick();
              //ttgo->displaySleep();
              rtc_clk_cpu_freq_set(RTC_CPU_FREQ_160M);
            } else {
              rtc_clk_cpu_freq_set(RTC_CPU_FREQ_240M);
              ttgo->openBL();
            }
            ison = not ison;
        }
        irq = false;
        ttgo->power->clearIRQ();
    }

    if (ison) {
        ttgo->eTFT->fillScreen(TFT_BLACK);
        ttgo->eTFT->setTextColor(TFT_YELLOW, TFT_BLACK);
        snprintf(buf, sizeof(buf), "%s", ttgo->rtc->formatDateTime());
        ttgo->eTFT->drawString(buf, 5, 90, 7);
    
        ttgo->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
        int per = ttgo->power->getBattPercentage();
    
        ttgo->eTFT->setTextFont(4);
        ttgo->eTFT->setTextColor(TFT_WHITE, TFT_BLACK);
        ttgo->eTFT->setCursor(20, 210);
        ttgo->eTFT->print("Bat: "); ttgo->eTFT->print(per); ttgo->eTFT->println(" %");
        delay(1000);
    } else {
        delay(200);
    }
}
