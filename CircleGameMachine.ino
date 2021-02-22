#include "HardwareConfiguration.h"
#include "StateMachine.h"

void setup(void) 
{
  SetupHardware();
  InitializeInterrupts();
  IRQ_CoinInterruptHandler = &IRQHand_Coin;
  IRQ_ButtonInterruptHandler = &IRQHand_Button;
}

void loop(void)
{
  if (nextMode != NONE && currentMode == NONE)
  {
    currentMode = nextMode;
    CallModeHandler();  
  }
}

void CallModeHandler()
{
  switch (currentMode)
  {
    case STANDBY:
      StandbyLoop();
      break;
    case GAME_MODE:
      GameLoop();
      break;
  }
}
