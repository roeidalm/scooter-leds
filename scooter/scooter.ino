// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 5 // On Trinket or Gemma, suggest changing this to 1
//in wemos d1 mini it's pin D1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 60 // Popular NeoPixel ring size

//for the yellow run led it's only index for it
int ledrun = 0;

//gyro data
const int MPU = 0x68;
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

//debug mode
bool debugmode = true;

//Interrupt data
const byte ledPin = 13;
const byte interruptPin = 2;
volatile byte state = LOW;

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixelsLeft(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsRight(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 1 // Time (in milliseconds) to pause between pixels

void setup()
{
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
  Serial.begin(9600);
  Serial.println("Start");

  //gyro setup
  // gyroSetUp();
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  //interrupt setup
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blinkLed, CHANGE);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixelsLeft.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixelsRight.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
}
void gyroSetUp()
{
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void loop()
{
  Serial.println("loop");
  //pixels.clear(); // Set all pixel colors to 'off'

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  //for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

  // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
  // Here we're using a moderately bright green color:
  //pixels.setPixelColor(i, pixels.Color(0, 150, 0));

  //pixels.show();   // Send the updated pixel colors to the hardware.

  //delay(DELAYVAL); // Pause before next pass through loop
  //}
  // rainbow(DELAYVAL,pixelsLeft);
  //theaterChase(pixelsLeft);
  //  colorWipe();
  blinkLed();
  movetoSide(pixelsLeft);
  blinkLed();

  readGyro();
  
  canvertGyroData();

  if (debugmode)
  {
    printDebug();
  }
  delay(333);
}
void printDebug()
{
  Serial.print("AcX = ");
  Serial.print(AcX);
  Serial.print(" | AcY = ");
  Serial.print(AcY);
  Serial.print(" | AcZ = ");
  Serial.print(AcZ);
  Serial.print(" | Tmp = ");
  Serial.print(Tmp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = ");
  Serial.print(GyX);
  Serial.print(" | GyY = ");
  Serial.print(GyY);
  Serial.print(" | GyZ = ");
  Serial.println(GyZ);
}

void colorWipe(uint32_t color, int wait, Adafruit_NeoPixel pixels)
{
  for (int i = 0; i < pixels.numPixels(); i++)
  {                                 // For each pixel in strip...
    pixels.setPixelColor(i, color); //  Set pixel's color (in RAM)
    pixels.show();                  //  Update strip to match
    delay(wait);                    //  Pause for a moment
  }
}
void blinkLed()
{
  pixelsLeft.clear();  // Set all pixel colors to 'off'
  pixelsRight.clear(); // Set all pixel colors to 'off'
  for (int i = 0; i < pixelsLeft.numPixels(); i++)
  {
    pixelsLeft.setPixelColor(i, pixelsLeft.Color(255, 0, 0));
    pixelsRight.setPixelColor(i, pixelsRight.Color(255, 0, 0));
  }
  pixelsLeft.show();  // Send the updated pixel colors to the hardware.
  pixelsRight.show(); // Send the updated pixel colors to the hardware.
  delay(100);
  for (int i = 0; i < pixelsLeft.numPixels(); i++)
  {
    pixelsLeft.setPixelColor(i, pixelsLeft.Color(0, 0, 0));
    pixelsRight.setPixelColor(i, pixelsRight.Color(0, 0, 0));
  }
  pixelsLeft.show();
  pixelsRight.show();
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

    pixelsLeft.clear(); // Set all pixel colors to 'off' or this
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

void readGyro()
{
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);      // request a total of 14 registers
  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
}

void canvertGyroData()
{
  if (GyY < 45)
  {
    //right
    movetoSide(pixelsLeft);
  }
  else if (GyY < -45)
  {
    //left
    movetoSide(pixelsRight);
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait, Adafruit_NeoPixel pixels)
{
  for (int a = 0; a < 10; a++)
  { // Repeat 10 times...
    for (int b = 0; b < 3; b++)
    {                 //  'b' counts from 0 to 2...
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
    {                 //  'b' counts from 0 to 2...
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
