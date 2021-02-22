
enum MACH_MODE
{
  NONE,
  STANDBY,
  GAME_MODE,
  LOOPING
};


MACH_MODE currentMode = NONE;
MACH_MODE nextMode = STANDBY;
volatile int credits = 0;
volatile bool coinLcdRefresh = true;

void DrawCoinScreen()
{
  tft.fillRect(0,185, 320, 5, YELLOW);
  tft.fillRect(0,190, 320, 50, RED);
  tft.setCursor(10,200);
  tft.setTextSize(3);
  tft.print(credits);
  tft.print(" creditos");
}

void CommonLoop()
{
  Serial.println(digitalRead(22));
  if (coinLcdRefresh)
  {
    DrawCoinScreen();
    coinLcdRefresh = false;
  }
}
/* Standby Mode */

///
/// It draws the introduction screen
///
void DrawIntroScreen()
{
  tft.fillScreen(BLACK);
}

void StandbyChase(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<30; a++) {  // Repeat 30 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

bool StbLcdRefresh = true;
void StandbyLoop()
{
  DrawIntroScreen();
  StbLcdRefresh = true;
  while (currentMode == STANDBY)
  {
    CommonLoop();

    /* Center text to play */
    if (StbLcdRefresh)
    {
      tft.fillRect(0,0, 320, 185, BLACK);
      tft.setCursor(20, (230/2)-20);
      tft.setTextSize(2);
      if (credits > 0)
      {
            tft.print(" PULSA ROJO PARA JUGAR");
      }
      else
      {
            tft.print(" INTRODUCE UN CREDITO");
      }
      StbLcdRefresh = false;
    }

    /* Led animation */

    StandbyChase(1);
    strip.show();
  }
}

int pixelNdx;
int GameDebounceCont;
int GameTarget;
bool GameRotating;

int WinStatus;
int ronda;
int level;
bool StdUpdateStatus;

const int maxNivels = 3;
const int maxRondas = 5;
const int debouncingVal = 15;
int numVueltas = 0;
int numHalfVueltas = 0;
volatile byte inhibitInterrupt = LOW;
volatile byte interruptPending = LOW;


inline void IRQ_PrizeHandler()
{
    GameRotating = false;
    
    if (pixelNdx == GameTarget)
    {
      WinStatus = 1;
    }
    else
    {
      WinStatus = 0;
    }
}


void GameLoop()
{
  
  tft.fillRect(0,0, 320, 185, BLACK);
  GameTarget = -1;
  GameDebounceCont = debouncingVal;
  pixelNdx = 0;
  GameRotating = true;
  WinStatus = -1;
  ronda = 1;
  level = 1;
  StdUpdateStatus = true;
  numVueltas = 0;
  

      
  while (currentMode == GAME_MODE)
  {
    CommonLoop();

    if (StdUpdateStatus)
    {
        tft.setTextSize(2);
        tft.setCursor(70, 5);
        tft.fillRect(70, 5, 350 ,20, BLACK);
        tft.print("Nivel ");
        tft.print(level);
        tft.print(", Ronda ");
        tft.print(ronda);
        StdUpdateStatus = false;
    }
    
    if (GameTarget == -1)
    {
      GameTarget = rand() % 60;
    }
    
    if (GameRotating)
    {
      inhibitInterrupt = HIGH;
      interruptPending = LOW;
      for (int i = 0; i < 60; i++)
      {
        strip.setPixelColor(i, strip.Color(0x12, 0x00, 0x5e));
      }

      pixelNdx++;
  
      if (pixelNdx > 59)
      {
        pixelNdx = 0;
        numVueltas++;
      }

      if (pixelNdx > 30)
      {
        numHalfVueltas++;
      }

      strip.setPixelColor(GameTarget-1, strip.Color(0x87, 0, 0));
      strip.setPixelColor(GameTarget, strip.Color(0x87, 0, 0));
      strip.setPixelColor(GameTarget+1, strip.Color(0x87, 0, 0));
      
      strip.setPixelColor(pixelNdx, strip.Color(0xff, 0xff, 0xff));
      strip.show();
      inhibitInterrupt = LOW;
      if (interruptPending)
      {
        IRQ_PrizeHandler();
        interruptPending = LOW; 
      }
      
      /* Logica dependiente del nivel */
      if (numVueltas == 1)
      {
        if (ronda == 2 || ronda == 4)
          GameTarget++;
        numVueltas = 0;
      }

      
      if (numHalfVueltas == 1)
      {
        if (ronda == 5)
        {
          GameTarget++;
          numHalfVueltas = 0;
        }
        
      }

      if (GameTarget > 59)
      {
        GameTarget = 0;
      }

      delay( 20/level);
      
    }
    else
    {
      if (WinStatus != -1)
      {
        numVueltas = 0;
        numHalfVueltas = 0;
        if (WinStatus == 0) // Perder
        {
          credits -= 20;
        }
        else if (WinStatus == 1) // Ganar
        {
          credits = credits; // Do nothing for now
          StdUpdateStatus = true;
          ronda++;
        }
        coinLcdRefresh = true;
        WinStatus = -1;

        if (ronda == 3 || ronda == 4 || ronda == 5)
        {
          GameTarget = -1;
        }

        if (ronda == maxRondas+1)
        {
          ronda = 1;
          level++;
        }
          
        if (credits == 0)
        {    
          currentMode = NONE;
          nextMode = STANDBY;
        }
        else // Sigues con creditos
        {
          tft.setTextSize(2);
          tft.setCursor(0, (230/2)-20);
          tft.print("PULSA VERDE PARA CONTINUAR");          
        }

      }

      
      byte val = digitalRead(22);
      if (val == 0)
      {
        GameDebounceCont--;
        Serial.println(val);
        if (GameDebounceCont == 0)
        {
          GameDebounceCont = debouncingVal;
          GameRotating = true;
          tft.fillRect(0, (230/2)-20, 350 ,20, BLACK);
        }
      }
      else
      {
        GameDebounceCont = debouncingVal;
      }
      
    }
    
  }
}


///
/// Interrupt Handlers
///
void IRQHand_Coin()
{
  credits+=100;
  coinLcdRefresh = true;
  if (credits == 100)
  StbLcdRefresh = true;
}

void IRQHand_Button()
{
  if (currentMode == STANDBY && credits > 0)
  {
    currentMode = NONE;
    nextMode = GAME_MODE;
  }
  else if (currentMode == GAME_MODE && GameRotating)
  {
    if (inhibitInterrupt){
      interruptPending = HIGH;
      return;
    }
    
    IRQ_PrizeHandler();
    
  }
}
