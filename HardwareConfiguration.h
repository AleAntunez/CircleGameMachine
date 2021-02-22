
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#define LED_PIN    44

#define LED_COUNT 60
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define COIN_PIN 20
#define BUTTON_PIN 21

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void (*IRQ_CoinInterruptHandler)() = 0;
void (*IRQ_ButtonInterruptHandler)() = 0;

void SetupLCD()
{
  tft.reset();
  uint16_t identifier = tft.readID();
  if(identifier==0x0101)
  {     
    identifier=0x9341;
  }
  else
  {
    identifier=0x9341;
  }
  tft.begin(identifier);
  tft.setRotation(3);
}

void SetupLEDStrip()
{
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void SetupHardware()
{
  SetupLCD();
  pinMode(22, INPUT);
  Serial.begin(9600);
  SetupLEDStrip();
}

static unsigned long coin_last_interrupt_time = 0;
static unsigned long button_last_interrupt_time = 0;

void CoinInterrupt()
{
 unsigned long interrupt_time = millis();
 if (interrupt_time - coin_last_interrupt_time > 300) 
 {
   IRQ_CoinInterruptHandler();
 }
 coin_last_interrupt_time = interrupt_time;
}

void ButtonInterrupt()
{
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - button_last_interrupt_time > 200) 
 {
   IRQ_ButtonInterruptHandler();
 }
 button_last_interrupt_time = interrupt_time;
}

void InitializeInterrupts()
{
    attachInterrupt(digitalPinToInterrupt(COIN_PIN), CoinInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), ButtonInterrupt, CHANGE);
}
