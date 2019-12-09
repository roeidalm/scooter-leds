/*
   This sketch can be used as a template to create your own dual core application.
   You can either pot code in codeForTask1, codeForTask2, or loop. Make sure, you
   place a delay(1000) in all unused tasks
*/
#include <Adafruit_NeoPixel.h>


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

    Adafruit_NeoPixel pixels(NUMPIXELS, *((int *)parameter), NEO_GRB + NEO_KHZ800);
    for (;;)
    {
        delay(1000);
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
        "led1Task",
        1000,
        (void *)&PINpixelsLeft,
        1,
        &Task1,
        1);

    delay(500); // needed to start-up task1

    xTaskCreatePinnedToCore(
        codeForTask1,
        "led2Task",
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
        0);
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