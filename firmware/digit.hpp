#pragma once
#include <Wire.h>

#include "Adafruit_NeoPixel.h"

// returns a color transitioning from r -> g -> b and back to r
// lightly modified from Adafruit NeoPixel strand test
static inline uint32_t ColorWheel(uint8_t pos)
{
    pos = 255 - pos;
    if (pos < 85)
    {
        return Adafruit_NeoPixel::Color(255 - pos * 3, 0, pos * 3);
    }

    if (pos < 170)
    {
        pos -= 85;
        return Adafruit_NeoPixel::Color(0, pos * 3, 255 - pos * 3);
    }

    pos -= 170;
    return Adafruit_NeoPixel::Color(pos * 3, 255 - pos * 3, 0);
}

static inline int ScaleBrightness(const int color, const float brightness)
{
    const float r = ((color & 0xFF0000) >> 16) * brightness;
    const float g = ((color & 0x00FF00) >> 8) * brightness;
    const float b = (color & 0x0000FF) * brightness;
    return Adafruit_NeoPixel::Color(r, g, b);
}

class Digit
{
  public:
    enum
    {
        LEDS_PER_DIGIT = 20,
        OFF_COLOR = 0x000000,
        INVALID = 0xFF,
    };

  protected:
    Adafruit_NeoPixel &leds;
    int m_first;

    // m_colors[x] is the color we should draw digit x (where 0 <= x <= 9)
    std::vector<uint8_t> m_colors{0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

  public:
    Digit(Adafruit_NeoPixel &strip, const int firstLED, const int onColor)
        : leds(strip), m_first(firstLED)
    {
    }

    virtual void Draw() = 0;

    void AllOff()
    {
        for (int i = 0; i < LEDS_PER_DIGIT; ++i)
        {
            leds.setPixelColor(m_first + i, OFF_COLOR);
        }
    }

    void SetValue(uint8_t value, const uint32_t newColor)
    {
        m_colors[value] = newColor;
    }
};

// lights two LEDs at a time under each numeral
class EdgeLitDigit : public Digit
{
  private:
    using Digit::Digit;

  public:
    virtual void Draw()
    {
        //leds.setPixelColor(m_first + 2, 0xffffff);
        //return;
        //char s[100];
        //sprintf(s, "drawing %i\n", num);
        //Serial.print(s);

        AllOff();
        for (uint8_t num = 0; num < 10; num++)
        {
            const int row = 10 - num;
            leds.setPixelColor(m_first + (row * 2) - 2, m_colors[num]);
            leds.setPixelColor(m_first + (row * 2) - 1, m_colors[num]);
        }
    }
};

// draws an entire number on the display using the available LEDs
class DisplayDigit : public Digit
{
  private:
    using Digit::Digit;

  public:
    virtual void Draw()
    {
        auto num = 1; //xxx hack to compile
        if (num >= 0 && num <= 9)
        {
            const uint8_t *data = NUMBERS + (num * LEDS_PER_DIGIT);
            for (int i = 0; i < LEDS_PER_DIGIT; ++i)
            {
                const int col = (data[i] == 0 ? OFF_COLOR : m_colors[num]);
                leds.setPixelColor(m_first + i, col);
            }
        }
        else
        {
            AllOff();
        }
    }

  private:
    const uint8_t NUMBERS[10 * LEDS_PER_DIGIT] = {
        // clang-format off

        // 0
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 1
            1,      0,
        1,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,
            1,      0,
        0,      0,

        // 2
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            1,      0,
        1,      0,
            0,      0,
        1,      1,
            0,      0,
        0,      0,

        // 3
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            1,      0,
        0,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 4
            0,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,

        // 5
            0,      0,
        1,      1,
            0,      0,
        1,      0,
            1,      0,
        0,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 6
            1,      0,
        1,      0,
            0,      0,
        1,      0,
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 7
            1,      0,
        1,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,
            0,      0,
        0,      1,

        // 8
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      0,

        // 9
            1,      0,
        1,      1,
            0,      0,
        1,      1,
            1,      0,
        0,      1,
            0,      0,
        0,      1,
            1,      0,
        0,      0,

        // clang-format on
    };
};
