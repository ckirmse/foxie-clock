#pragma once
#include <memory>
#include <vector>

#include <Wire.h>

#include "digit.hpp"
#include "settings.hpp"

class DigitManager
{
  private:
    enum LEDNumbers_e
    {
        // each number below refers to the top left LED of each column of
        // 20 offset LEDs, starting from the left of the PCB (digit 1)
        // to the right side (digit 6)
        //
        // Note: these numbers are 1 greater than the actual D### LED number
        // markings on the actual PCB, i.e. "D1" on the PCB = LED 0
        DIGIT_1_LED = 0,
        DIGIT_2_LED = 20,
        DIGIT_3_LED = 40,
        DIGIT_4_LED = 60,
        DIGIT_5_LED = 80,
        DIGIT_6_LED = 100,

        NUM_DIGITS = 6,
    };

    Adafruit_NeoPixel &m_leds;
    // shared pointers used so that destructing automatically deletes the digit
    std::vector<std::shared_ptr<Digit>> m_digits;

  public:
    DigitManager(Adafruit_NeoPixel &leds) : m_leds(leds)
    {
        CreateDigitDisplay();
    }

    void CreateDigitDisplay()
    {
        m_digits.clear();

        m_digits.push_back(CreateDigit(DIGIT_1_LED));
        m_digits.push_back(CreateDigit(DIGIT_2_LED));
        m_digits.push_back(CreateDigit(DIGIT_3_LED));
        m_digits.push_back(CreateDigit(DIGIT_4_LED));
        m_digits.push_back(CreateDigit(DIGIT_5_LED));
        m_digits.push_back(CreateDigit(DIGIT_6_LED));
    }

    void SetDigitValueColor(const size_t digitNum, const uint8_t value, const uint32_t color)
    {
        char s[100];
        sprintf(s, "digit %i setting value %i to color %u\n", digitNum, value, color);
        Serial.print(s);
        m_digits[digitNum]->SetValue(value, color);
    }

    void SetExclusiveDigitValueColor(const size_t digitNum, const uint8_t value, const uint32_t color)
    {
        char s[100];
        sprintf(s, "exclusive digit value color %i %i %u\n", digitNum, value, color);
        Serial.print(s);

        for (uint8_t i = 0; i < 10; i++)
        {
            if (i == value)
            {
                this->SetDigitValueColor(digitNum, i, color);
            }
            else
            {
                this->SetDigitValueColor(digitNum, i, 0);
            }
        }
    }

    void Draw()
    {
        char s[100];
        sprintf(s, "drawing\n");
        Serial.print(s);

        for (uint8_t i = 0; i < 6; i++)
        {
            m_digits[i]->Draw();
        }
    }

  private:
    std::shared_ptr<Digit> CreateDigit(const LEDNumbers_e firstLED)
    {
        if (1 || Settings::Get(SETTING_DIGIT_TYPE) == 1)
        {
            return std::make_shared<EdgeLitDigit>(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
        else
        {
            return std::make_shared<DisplayDigit>(m_leds, firstLED, Settings::Get(SETTING_COLOR));
        }
    }
};
