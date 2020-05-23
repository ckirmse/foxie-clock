#pragma once
#include <memory>
#include <vector>

#include "Adafruit_NeoPixel.h"
#include "animator.hpp"
#include "digit_manager.hpp"
#include "rtc_hal.hpp"
#include "settings.hpp"

enum ClockState_e
{
    STATE_NORMAL,
    STATE_SET_TIME,
    STATE_DISPLAY_VALUE,
};

class Clock
{
  private:
    enum DigitSeparatorLEDs_e
    {
        // for edge lit digits, use the two LEDs between rows 2/3 and 4/5
        BLINK_DIGIT_TYPE_1_LED_1 = 120,
        BLINK_DIGIT_TYPE_1_LED_2 = 121,

        // for pixel lit digits, use 4 LEDs in rows 2 and 4
        BLINK_DIGIT_TYPE_2_LED_1 = 25,
        BLINK_DIGIT_TYPE_2_LED_2 = 33,
        BLINK_DIGIT_TYPE_2_LED_3 = 65,
        BLINK_DIGIT_TYPE_2_LED_4 = 73,
    };

    Adafruit_NeoPixel &m_leds;
    DigitManager m_digitMgr;
    int m_lastRedrawTime{-1};
    ClockState_e &m_state;
    std::shared_ptr<Animator> m_animator;

    int m_millisSinceDisplayValueEntered{0};

  public:
    Clock(Adafruit_NeoPixel &leds, ClockState_e &state) : m_leds(leds), m_digitMgr(leds), m_state(state)
    {
        UseAnimation((AnimationType_e)Settings::Get(SETTING_ANIMATION_TYPE));
        Draw();
    }

    void UseAnimation(const AnimationType_e type)
    {
        m_animator = AnimatorFactory(m_digitMgr, type);
    }

    void ColorButtonPressed()
    {
        m_animator->ColorButtonPressed();
        Draw();
    }

    void ChangeDigitType()
    {
        m_digitMgr.CreateDigitDisplay();
    }

    void DisplayValue(unsigned int value)
    {
        m_state = STATE_DISPLAY_VALUE;
        m_millisSinceDisplayValueEntered = millis();
        UseAnimation(ANIM_NONE);

        /*xxx
        for (auto &num : m_digitMgr.numbers)
        {
            num = Digit::INVALID;
        }

        int digitNum = 5;
        do
        {
            m_digitMgr.numbers[digitNum--] = value % 10;
            value /= 10;
        } while (value && digitNum >= 0);
        */

        m_animator->Run();
    }

    void UpdateDigitsFromRTC()
    {
        std::vector<uint8_t> clockDigitValues;
        if (Settings::Get(SETTING_24_HOUR_MODE) == 1)
        {
            clockDigitValues.push_back(rtc_hal_hour() / 10);
            clockDigitValues.push_back(rtc_hal_hour() % 10);
        }
        else
        {
            clockDigitValues.push_back(rtc_hal_hourFormat12() / 10);
            clockDigitValues.push_back(rtc_hal_hourFormat12() % 10);

            if (clockDigitValues[0] == 0)
            {
                // disable leading 0 for 12 hour mode
                clockDigitValues[0] = Digit::INVALID;
            }
        }

        clockDigitValues.push_back(rtc_hal_minute() / 10);
        clockDigitValues.push_back(rtc_hal_minute() % 10);

        clockDigitValues.push_back(rtc_hal_second() / 10);
        clockDigitValues.push_back(rtc_hal_second() % 10);

        m_animator->UpdateClockDigitValues(clockDigitValues);

        BlinkDigitSeparators();
    }

    void Check()
    {
        bool update = false;

        if (m_state == STATE_DISPLAY_VALUE && ElapsedMsSinceDisplayModeStarted() >= 1000)
        {
            m_state = STATE_NORMAL;
            UseAnimation((AnimationType_e)Settings::Get(SETTING_ANIMATION_TYPE));
        }

        if (m_state == STATE_NORMAL)
        {
            rtc_hal_update();
        }
        else
        {
            // whenever not in normal mode, always update
            update = true;
        }

        if (rtc_hal_second() != m_lastRedrawTime)
        {
            update = true;
        }

        if (update || m_animator->IsFast())
        {
            m_lastRedrawTime = rtc_hal_second();
            Draw();
        }
    }

    void Draw()
    {
        if (m_state != STATE_DISPLAY_VALUE)
        {
            UpdateDigitsFromRTC();
        }

        m_animator->Run();

        m_leds.show();
    }

    void BlinkDigitSeparators()
    {
        if (Settings::Get(SETTING_BLINKING_SEPARATORS) != 1)
        {
            return;
        }

        int blinkColor = ColorWheel(Settings::Get(SETTING_COLOR));
        if (rtc_hal_second() % 2 != 0)
        {
            blinkColor = 0;
        }

        if (Settings::Get(SETTING_DIGIT_TYPE) == 1)
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_1_LED_2, blinkColor);
        }
        else
        {
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_1, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_2, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_3, blinkColor);
            m_leds.setPixelColor(BLINK_DIGIT_TYPE_2_LED_4, blinkColor);
        }
    }

  private:
    int ElapsedMsSinceDisplayModeStarted()
    {
        return (int)millis() - m_millisSinceDisplayValueEntered;
    }
};
