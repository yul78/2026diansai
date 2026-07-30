#include "key.h"

uint8_t AJ1_pressed = 0;
uint8_t AJ2_pressed = 0;
uint8_t AJ3_pressed = 0;
uint8_t AJ4_pressed = 0;


void AJ_Init(void)
{
    DL_GPIO_clearInterruptStatus(AJ_AJ1_PORT, AJ_AJ1_PIN);
    DL_GPIO_clearInterruptStatus(AJ_AJ2_PORT, AJ_AJ2_PIN);
    DL_GPIO_clearInterruptStatus(AJ_AJ2_PORT, AJ_AJ2_PIN);
    DL_GPIO_clearInterruptStatus(AJ_AJ2_PORT, AJ_AJ2_PIN);

    NVIC_EnableIRQ(AJ_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(GPIO_MULTIPLE_GPIOB_INT_IRQN);

}
uint8_t AJ1_IsPressed(void)
  {
      static uint8_t debounce_cnt = 0;

      if(AJ1_pressed)
      {
          if(!DL_GPIO_readPins(AJ_AJ1_PORT, AJ_AJ1_PIN))
          {
              if(++debounce_cnt >= 5)
              {
                  debounce_cnt = 0;
                  AJ1_pressed = 0;
                  return 1;
              }
          }
          else
          {
              debounce_cnt = 0;
              AJ1_pressed = 0;
          }
      }
      else
      {
          debounce_cnt = 0;
      }
      return 0;
  }

  uint8_t AJ2_IsPressed(void)
  {
      static uint8_t debounce_cnt = 0;

      if(AJ2_pressed)
      {
          if(!DL_GPIO_readPins(AJ_AJ2_PORT, AJ_AJ2_PIN))
          {
              if(++debounce_cnt >= 5)
              {
                  debounce_cnt = 0;
                  AJ2_pressed = 0;
                  return 1;
              }
          }
          else
          {
              debounce_cnt = 0;
              AJ2_pressed = 0;
          }
      }
      else
      {
          debounce_cnt = 0;
      }
      return 0;
  }

  uint8_t AJ3_IsPressed(void)
  {
      static uint8_t debounce_cnt = 0;

      if(AJ3_pressed)
      {
          if(!DL_GPIO_readPins(AJ_AJ3_PORT, AJ_AJ3_PIN))
          {
              if(++debounce_cnt >= 5)
              {
                  debounce_cnt = 0;
                  AJ3_pressed = 0;
                  return 1;
              }
          }
          else
          {
              debounce_cnt = 0;
              AJ3_pressed = 0;
          }
      }
      else
      {
          debounce_cnt = 0;
      }
      return 0;
  }

  uint8_t AJ4_IsPressed(void)
  {
      static uint8_t debounce_cnt = 0;

      if(AJ4_pressed)
      {
          if(!DL_GPIO_readPins(AJ_AJ4_PORT, AJ_AJ4_PIN))
          {
              if(++debounce_cnt >= 5)
              {
                  debounce_cnt = 0;
                  AJ4_pressed = 0;
                  return 1;
              }
          }
          else
          {
              debounce_cnt = 0;
              AJ4_pressed = 0;
          }
      }
      else
      {
          debounce_cnt = 0;
      }
      return 0;
  }
