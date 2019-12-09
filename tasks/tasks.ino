/*
   This sketch can be used as a template to create your own dual core application.
   You can either pot code in codeForTask1, codeForTask2, or loop. Make sure, you
   place a delay(1000) in all unused tasks
*/
#include <Adafruit_NeoPixel.h>

#define DELAYVAL 1 // Time (in milliseconds) to pause between pixels

//for the yellow run led it's only index for it
int ledrun = 0;

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.

long loops1 = 1000;
long loops2 = 1000;
long qq;
float t1;
int t2, t3;

TaskHandle_t Task1, Task2, Task3;
SemaphoreHandle_t baton;

// Adafruit_NeoPixel pixelsLeft(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// Adafruit_NeoPixel pixelsRight(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int PINpixelsLeft = 5;  // On Trinket or Gemma, suggest changing this to 1
int PINpixelsRight = 6; // On Trinket or Gemma, suggest changing this to 1
int NUMPIXELS = 60;     // On Trinket or Gemma, suggest changing this to 1

void artificialLoad()
{
    for (long i = 0; i < loops1; i++)
    {
        for (long j = 1; j < loops2; j++)
        {
            qq++;
            t1 = 5000.0 * i;
            t2 = 150 * 1234 * i;
            t3 = j % 554;
        }
    }
}

void codeForTask1(void *parameter)
{
    int pinumber = *((int *)parameter);
    Serial.print("pin number is: ");
    Serial.println(pinumber);
    Adafruit_NeoPixel pixels(NUMPIXELS, pinumber, NEO_GRB + NEO_KHZ800);
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

    for (;;)
    {
        pixels.clear();
        delay(3000);
        Serial.print("Finish Loop Task which runs on Core ");
        Serial.println(xPortGetCoreID());

        blinkLed(pixels);
        movetoSide(pixels);
        blinkLed(pixels);
        rainbow(DELAYVAL, pixels);
        rainbow(DELAYVAL, pixels);
        rainbow(DELAYVAL, pixels);
    }
}

void codeForTask2(void *parameter)
{
    for (;;)
    {
        delay(1000);
    }
}

void codeForTask3(void *parameter)
{
    for (;;)
    {
        delay(1000);
    }
}
// the setup function runs once when you press reset or power the board
void setup()
{
    Serial.begin(115200);

    baton = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        codeForTask1,
        "leftLed",
        1000,
        (void *)&PINpixelsLeft,
        1,
        &Task1,
        1);

    delay(500); // needed to start-up task1

    xTaskCreatePinnedToCore(
        codeForTask1,
        "rightLed",
        1000,
        (void *)&PINpixelsRight,
        1,
        &Task2,
        0);

    delay(500); // needed to start-up task2

    xTaskCreatePinnedToCore(
        codeForTask3,
        "led3Task",
        1000,
        NULL,
        1,
        &Task3,
        1);
}

void loop()
{
    long start = millis();

    artificialLoad();

    Serial.print("Finish Loop Task which runs on Core ");
    Serial.print(xPortGetCoreID());
    Serial.print(" Time ");
    Serial.println(millis() - start);
    delay(10);
}

void colorWipe(uint32_t color, int wait, Adafruit_NeoPixel pixels)
{
    for (int i = 0; i < pixels.numPixels(); i++)
    {                                   // For each pixel in strip...
        pixels.setPixelColor(i, color); //  Set pixel's color (in RAM)
        pixels.show();                  //  Update strip to match
        delay(wait);                    //  Pause for a moment
    }
}
void blinkLed(Adafruit_NeoPixel pixels)
{
    pixels.clear(); // Set all pixel colors to 'off'
    for (int i = 0; i < pixels.numPixels(); i++)
    {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
    }
    pixels.show(); // Send the updated pixel colors to the hardware.
    delay(100);
    for (int i = 0; i < pixels.numPixels(); i++)
    {
        pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    delay(250);
}

void movetoSide(Adafruit_NeoPixel strip)
{
    Serial.println("movetoside");

    ledrun = 0;
    strip.clear(); // Set all pixel colors to 'off'
    for (int i = 0; i < (strip.numPixels() + 10) * 2; i++)
    {
        ledrun = i % strip.numPixels();
        Serial.println("ledrun");
        Serial.println(ledrun);

        strip.clear(); // Set all pixel colors to 'off' or this
        for (int j = ledrun; j < ledrun + 10; j++)
        {
            Serial.println("j");
            Serial.println(j);
            strip.setPixelColor(j, strip.Color(255, 240, 0));
            strip.show();
        }
        //pixels.setPixelColor(ledrun, pixels.Color(0, 0, 0)); //withou this
        delay(20);
        Serial.println("show");
    }
    delay(1500);
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait, Adafruit_NeoPixel pixels)
{
    for (int a = 0; a < 10; a++)
    { // Repeat 10 times...
        for (int b = 0; b < 3; b++)
        {                   //  'b' counts from 0 to 2...
            pixels.clear(); //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in steps of 3...
            for (int c = b; c < pixels.numPixels(); c += 3)
            {
                pixels.setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            pixels.show(); // Update strip with new contents
            delay(wait);   // Pause for a moment
        }
    }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait, Adafruit_NeoPixel pixels)
{
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
    {
        for (int i = 0; i < pixels.numPixels(); i++)
        { // For each pixel in strip...
            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            int pixelHue = firstPixelHue + (i * 65536L / pixels.numPixels());
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue)));
        }
        pixels.show(); // Update strip with new contents
        delay(wait);   // Pause for a moment
    }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait, Adafruit_NeoPixel pixels)
{
    int firstPixelHue = 0; // First pixel starts at red (hue 0)
    for (int a = 0; a < 30; a++)
    { // Repeat 30 times...
        for (int b = 0; b < 3; b++)
        {                   //  'b' counts from 0 to 2...
            pixels.clear(); // Set all pixel colors to 'off'
            // 'c' counts up from 'b' to end of strip in increments of 3...
            for (int c = b; c < pixels.numPixels(); c += 3)
            {
                // hue of pixel 'c' is offset by an amount to make one full
                // revolution of the color wheel (range 65536) along the length
                // of the strip (strip.numPixels() steps):
                int hue = firstPixelHue + c * 65536L / pixels.numPixels();
                uint32_t color = pixels.gamma32(pixels.ColorHSV(hue)); // hue -> RGB
                pixels.setPixelColor(c, color);                        // Set pixel 'c' to value 'color'
            }
            pixels.show();               // Send the updated pixel colors to the hardware.
            delay(wait);                 // Pause for a moment
            firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
        }
    }
}
