/* USER CODE BEGIN Header */
      /**
         ******************************************************************************
         * @file           : main.c
         * @brief          : Main program body
         ******************************************************************************
         * @attention
         *
         * Copyright (c) 2026 STMicroelectronics.
         * All rights reserved.
         *
         * This software is licensed under terms that can be found in the LICENSE file
         * in the root directory of this software component.
         * If no LICENSE file comes with this software, it is provided AS-IS.
         *
         ******************************************************************************
         */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
      #include "stm32f1xx_hal.h"
      #include "i2c-lcd.h"
      #include <stdio.h>
      #include <string.h>

      #include "stm32f1xx_hal_iwdg.h"  
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

      GPIO_TypeDef* IR_PORT  = GPIOB;
      #define GREEN_NORMAL_TIME   6000
      #define GREEN_LONG_TIME     9000
      #define GREEN_SHORT_TIME    3000
      #define YELLOW_TIME         2000
      #define IR_DEBOUNCE_MS      20
      #define IR_CONFIRM_MS       2000
      #define UPDATE_INTERVAL     100
      #define LCD_UPDATE_SEC      1000

      // ===== CĂ¡c háº±ng sá»‘ trÆ°á»›c Ä‘Ă¢y lĂ  "magic number" náº±m rá»i ráº¡c trong code =====
      #define IR_POLL_INTERVAL_MS     20   // táº§n suáº¥t quĂ©t cáº£m biáº¿n IR (trÆ°á»›c: sá»‘ 20 tráº§n trong checkIR)
      #define IR_CONFIRM_VEHICLE_MS   150  // thá»i gian giá»¯ tĂ­n hiá»‡u Ä‘á»ƒ xĂ¡c nháº­n cĂ³ xe tháº­t, lá»c nhiá»…u (trÆ°á»›c: sá»‘ 150 tráº§n)
      #define NIGHTMODE_LOOP_DELAY_MS 10   // delay HAL_Delay trong vĂ²ng láº·p night mode (trÆ°á»›c: sá»‘ 10 tráº§n)
      #define NORMAL_LOOP_DELAY_MS    5    // delay HAL_Delay trong vĂ²ng láº·p chĂ­nh (trÆ°á»›c: sá»‘ 5 tráº§n)
      #define NIGHTMODE_BLINK_MS      400  // chu ká»³ nháº¥p nhĂ¡y Ä‘Ă¨n vĂ ng á»Ÿ night mode (trÆ°á»›c: sá»‘ 400 tráº§n)

      // Háº¡n cháº¿ pháº§n cá»©ng cáº§n ghi rĂµ trong bĂ¡o cĂ¡o: má»—i lĂ n chá»‰ cĂ³ 1 cáº£m biáº¿n
      // há»“ng ngoáº¡i (IR) Ä‘áº·t cá»‘ Ä‘á»‹nh táº¡i 1 Ä‘iá»ƒm trĂªn Ä‘Æ°á»ng, nĂªn há»‡ thá»‘ng CHá»ˆ
      // phĂ¡t hiá»‡n Ä‘Æ°á»£c "cĂ³ xe Ä‘i qua Ä‘iá»ƒm Ä‘Ă³ hay khĂ´ng táº¡i 1 thá»i Ä‘iá»ƒm", vĂ 
      // Ä‘áº¿m Ä‘Æ°á»£c Sá» XE Ä‘Ă£ Ä‘i qua Ä‘iá»ƒm Ä‘Ă³ theo thá»i gian â€” KHĂ”NG thá»ƒ biáº¿t
      // chĂ­nh xĂ¡c Ä‘ang cĂ³ bao nhiĂªu xe Ä‘ang xáº¿p hĂ ng/chá» Ä‘Ă¨n Ä‘á» phĂ­a sau
      // Ä‘iá»ƒm cáº£m biáº¿n. Máº­t Ä‘á»™ xe dÆ°á»›i Ä‘Ă¢y Ä‘Æ°á»£c suy ra giĂ¡n tiáº¿p báº±ng cĂ¡ch
      // Ä‘áº¿m sá»‘ xe Ä‘i qua cáº£m biáº¿n trong má»™t khung thá»i gian gáº§n nháº¥t
      // (sliding window) lĂ m Ä‘áº¡i diá»‡n cho "lÆ°u lÆ°á»£ng" cá»§a lĂ n Ä‘Ă³, khĂ´ng
      // pháº£i con sá»‘ tuyá»‡t Ä‘á»‘i "Ä‘ang cĂ³ bao nhiĂªu xe chá»".

      // ===== Äáº¿m máº­t Ä‘á»™ xe theo cá»­a sá»• trÆ°á»£t (sliding window) =====
      // Thay cho viá»‡c chá»‰ biáº¿t "cĂ³/khĂ´ng cĂ³ xe" táº¡i 1 thá»i Ä‘iá»ƒm, há»‡ thá»‘ng
      // Ä‘áº¿m sá»‘ xe Ä‘Ă£ Ä‘i qua má»—i lĂ n trong VEHICLE_WINDOW_MS gáº§n nháº¥t, rá»“i
      // dĂ¹ng tá»‰ lá»‡ sá»‘ xe giá»¯a 2 cáº·p lĂ n Ä‘á»ƒ tĂ­nh thá»i gian xanh tÆ°Æ¡ng á»©ng.
      #define VEHICLE_WINDOW_MS       15000 // cá»­a sá»• thá»i gian tĂ­nh máº­t Ä‘á»™ (15s gáº§n nháº¥t)
      #define VEHICLE_WINDOW_SLOTS    15    // chia cá»­a sá»• thĂ nh cĂ¡c "Ă´" 1s Ä‘á»ƒ dá»… trÆ°á»£t cá»­a sá»•
      #define VEHICLE_SLOT_MS         (VEHICLE_WINDOW_MS / VEHICLE_WINDOW_SLOTS)

      // ===== Tá»± Ä‘á»™ng chuyá»ƒn Night Mode khi Ä‘Æ°á»ng váº¯ng lĂ¢u =====
      // Náº¿u cáº£ 4 lĂ n khĂ´ng phĂ¡t hiá»‡n xe nĂ o liĂªn tá»¥c trong khoáº£ng thá»i gian
      // nĂ y, há»‡ thá»‘ng tá»± chuyá»ƒn sang cháº¿ Ä‘á»™ nháº¥p nhĂ¡y vĂ ng (giá»‘ng night mode
      // thá»§ cĂ´ng) mĂ  khĂ´ng cáº§n ngÆ°á»i báº¥m nĂºt â€” phĂ¹ há»£p vá»›i tinh tháº§n "Ä‘Ă¨n
      // giao thĂ´ng thĂ´ng minh" Ä‘Ă£ Ä‘áº·t tĂªn cho dashboard.
      #define AUTO_NIGHTMODE_IDLE_MS  120000 // 2 phĂºt khĂ´ng cĂ³ xe á»Ÿ cáº£ 4 lĂ n -> tá»± vĂ o night mode

      // ===== Watchdog =====
      // IWDG (Independent Watchdog) dĂ¹ng Ä‘á»ƒ tá»± reset MCU náº¿u vĂ²ng láº·p chĂ­nh
      // bá»‹ treo (deadlock, vĂ´ tĂ¬nh rÆ¡i vĂ o loop vĂ´ háº¡n...), trĂ¡nh Ä‘Ă¨n giao
      // thĂ´ng bá»‹ "Ä‘á»©ng yĂªn á»Ÿ 1 mĂ u" mĂ£i náº¿u pháº§n má»m gáº·p lá»—i khĂ´ng lÆ°á»ng
      // trÆ°á»›c. Pháº£i gá»i HAL_IWDG_Refresh() Ä‘á»u trong while(1) trÆ°á»›c khi
      // timeout, náº¿u khĂ´ng MCU sáº½ tá»± reset.
      #define IWDG_TIMEOUT_MS         2000  // náº¿u quĂ¡ 2s khĂ´ng "refresh" -> tá»± reset MCU

      // ===== Checksum UART =====
      // ThĂªm 1 byte checksum (XOR Ä‘Æ¡n giáº£n) á»Ÿ cuá»‘i má»—i frame UART Ä‘á»ƒ ESP32
      // cĂ³ thá»ƒ loáº¡i bá» cháº¯c cháº¯n cĂ¡c frame bá»‹ nhiá»…u/Ä‘á»©t giá»¯a Ä‘Æ°á»ng truyá»n,
      // thay vĂ¬ chá»‰ dá»±a vĂ o sscanf Ä‘áº¿m Ä‘á»§ sá»‘ field (vá»‘n váº«n cĂ³ thá»ƒ "vĂ´ tĂ¬nh"
      // khá»›p Ä‘á»§ field vá»›i dá»¯ liá»‡u sai do nhiá»…u).

      // ===== PIN DEFINES =====
      #define L1_G GPIO_PIN_0
      #define L1_Y GPIO_PIN_1
      #define L1_R GPIO_PIN_2

      #define L2_G GPIO_PIN_3
      #define L2_Y GPIO_PIN_4
      #define L2_R GPIO_PIN_5

      #define L3_G GPIO_PIN_6
      #define L3_Y GPIO_PIN_7
      #define L3_R GPIO_PIN_15

      #define L4_G GPIO_PIN_1   
      #define L4_Y GPIO_PIN_10  
      #define L4_R GPIO_PIN_11 

      #define IR1 GPIO_PIN_12
      #define IR2 GPIO_PIN_13
      #define IR3 GPIO_PIN_14
      #define IR4 GPIO_PIN_15
      
      #define BTN_RESET  GPIO_PIN_5
      #define BTN_NIGHT  GPIO_PIN_0  // nĂºt Night Mode (PB0, pull-up)
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
      uint8_t nightMode = 0;
      uint32_t lastButtonPress = 0;
      uint32_t lastBlink = 0;
      uint8_t blinkState = 0;
      uint16_t lane1_count = 0;
      uint16_t lane2_count = 0;
      uint16_t lane3_count = 0;
      uint16_t lane4_count = 0;
      
      uint8_t counted[4] = {0};
      
      uint8_t laneState[4];
         
      //  STATE ENUMS 
      typedef enum
      {
            RED = 0,
            YELLOW,
            GREEN
      } State;

      typedef enum
      {
            MODE_NORMAL = 0,
            MODE_FAVOR_13,
            MODE_FAVOR_24
      } TrafficMode;

      //  GLOBAL VARIABLES 
      uint8_t currentPair = 0;
      State currentState = GREEN;
      uint32_t stateStart = 0;
      uint32_t stateDuration = 0;
      TrafficMode requestedMode = MODE_NORMAL;
      TrafficMode activeMode = MODE_NORMAL;

      uint8_t  isDetecting[4]  = {0};
      uint32_t detectStart[4]  = {0};
      uint8_t  sensorActive[4] = {0};

      uint32_t lastIRCheck = 0;
      uint32_t lastLCDUpdate = 0;
      uint32_t lastMainUpdate = 0;

      // Chá»‘ng nhiá»…u khi chá»n requestedMode: chá»‰ Ä‘á»•i mode Ä‘á» xuáº¥t khi tĂ­n hiá»‡u
      // favor13/favor24 giá»¯ á»”N Äá»NH liĂªn tá»¥c trong MODE_STABLE_MS, trĂ¡nh viá»‡c
      // activeMode bá»‹ chá»‘t sai do Ä‘Ăºng lĂºc chuyá»ƒn pha cáº£m biáº¿n Ä‘ang "nhiá»…u"
      // (vĂ­ dá»¥ xe vá»«a rá»i khá»i cáº£m biáº¿n ngay khoáº£nh kháº¯c GREEN->YELLOW).
      #define MODE_STABLE_MS 300
      TrafficMode pendingMode = MODE_NORMAL;
      uint32_t    pendingModeStart = 0;

      // Non-blocking "SYSTEM RESET" message trĂªn LCD (thay cho HAL_Delay(800) cÅ©)
      uint8_t  resetMsgShowing = 0;
      uint32_t resetMsgStart   = 0;
      #define RESET_MSG_DURATION 800

      // ===== Sliding window Ä‘áº¿m máº­t Ä‘á»™ xe theo tá»«ng lĂ n =====
      // vehicleSlots[lane][slot]: sá»‘ xe Ä‘i qua lĂ n Ä‘Ă³ trong Ă´ thá»i gian (slot) tÆ°Æ¡ng á»©ng.
      // Má»—i VEHICLE_SLOT_MS, há»‡ thá»‘ng "trÆ°á»£t" cá»­a sá»•: bá» Ă´ cÅ© nháº¥t, má»Ÿ Ă´ má»›i.
      // vehicleDensity[lane] = tá»•ng sá»‘ xe trong toĂ n bá»™ cá»­a sá»• VEHICLE_WINDOW_MS gáº§n nháº¥t.
      uint16_t vehicleSlots[4][VEHICLE_WINDOW_SLOTS] = {{0}};
      uint8_t  currentSlotIndex = 0;
      uint32_t lastSlotShiftTime = 0;
      uint16_t vehicleDensity[4] = {0};

      // ===== Auto night mode khi Ä‘Æ°á»ng váº¯ng lĂ¢u =====
      uint32_t lastVehicleSeenTime = 0; // má»‘c thá»i gian gáº§n nháº¥t CĂ“ xe á»Ÿ báº¥t ká»³ lĂ n nĂ o
      uint8_t  autoNightMode = 0;       // 1 = Ä‘ang á»Ÿ night mode do há»‡ thá»‘ng Tá»° kĂ­ch hoáº¡t (khĂ´ng pháº£i ngÆ°á»i báº¥m nĂºt)


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

      // SET TRAFFIC LIGHTS

      void setPair(State s)
      {
            currentState = s;

            // Cáº­p nháº­t laneState[] NGAY Äáº¦U HĂ€M Ä‘á»ƒ UART gá»­i luĂ´n Ä‘á»“ng bá»™
            // vá»›i LED thá»±c táº¿ (trÆ°á»›c Ä‘Ă¢y cáº­p nháº­t cuá»‘i hĂ m â†’ cĂ³ thá»ƒ gá»­i
            // 1 frame "cÅ©" trong vĂ i ms trÆ°á»›c khi setPair() káº¿t thĂºc).
            if(currentPair == 0)
            {
                  laneState[0] = s;
                  laneState[2] = s;
                  laneState[1] = RED;
                  laneState[3] = RED;
            }
            else
            {
                  laneState[1] = s;
                  laneState[3] = s;
                  laneState[0] = RED;
                  laneState[2] = RED;
            }

            // Táº¯t toĂ n bá»™ LED
            HAL_GPIO_WritePin(GPIOA, 
                  GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
                  GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|
                  GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15,
                  GPIO_PIN_RESET);

            HAL_GPIO_WritePin(GPIOB, 
                  GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11,
                  GPIO_PIN_RESET);

            if(currentPair == 0)
            {
                  // L1 + L3 cháº¡y
                  if(s == GREEN)
                        HAL_GPIO_WritePin(GPIOA, L1_G | L3_G, GPIO_PIN_SET);

                  else if(s == YELLOW)
                        HAL_GPIO_WritePin(GPIOA, L1_Y | L3_Y, GPIO_PIN_SET);

                  else // RED
                        HAL_GPIO_WritePin(GPIOA, L1_R | L3_R, GPIO_PIN_SET);

                  // L2 + L4 luĂ´n Ä‘á»
                  HAL_GPIO_WritePin(GPIOA, L2_R, GPIO_PIN_SET);
                  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); // L4_R
            }
            else
            {
                  // L2 + L4 cháº¡y
                  if(s == GREEN)
                  {
                        HAL_GPIO_WritePin(GPIOA, L2_G, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); // L4_G
                  }
                  else if(s == YELLOW)
                  {
                        HAL_GPIO_WritePin(GPIOA, L2_Y, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET); // L4_Y
                  }
                  else // RED
                  {
                        HAL_GPIO_WritePin(GPIOA, L2_R, GPIO_PIN_SET);
                        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET); // L4_R
                  }

                  // L1 + L3 luĂ´n Ä‘á»
                  HAL_GPIO_WritePin(GPIOA, L1_R | L3_R, GPIO_PIN_SET);
            }
      }
      // Forward declaration: getVehicleRatio Ä‘Æ°á»£c Ä‘á»‹nh nghÄ©a sau checkIR(),
      // nhÆ°ng getGreenDuration() cáº§n gá»i nĂ³ trÆ°á»›c Ä‘Ă³ trong file.
      float getVehicleRatio(uint8_t pair);

      // GET GREEN DURATION
      // TrÆ°á»›c Ä‘Ă¢y: chá»‰ cĂ³ 2 má»©c cá»‘ Ä‘á»‹nh GREEN_LONG_TIME/GREEN_SHORT_TIME,
      // khĂ´ng phĂ¢n biá»‡t Ä‘Æ°á»£c "1 xe" vĂ  "20 xe" cĂ¹ng á»Ÿ tráº¡ng thĂ¡i cĂ³ xe.
      // Giá»: thá»i gian xanh Ä‘Æ°á»£c ná»™i suy tuyáº¿n tĂ­nh theo Tá»ˆ Lá»† máº­t Ä‘á»™ xe
      // thá»±c táº¿ (Ä‘áº¿m trong VEHICLE_WINDOW_MS gáº§n nháº¥t) giá»¯a 2 cáº·p lĂ n,
      // náº±m trong khoáº£ng [GREEN_SHORT_TIME, GREEN_LONG_TIME].
      // VĂ­ dá»¥: cáº·p Ä‘ang xĂ©t chiáº¿m 80% tá»•ng lÆ°u lÆ°á»£ng -> gáº§n vá»›i GREEN_LONG_TIME;
      // chiáº¿m 20% -> gáº§n vá»›i GREEN_SHORT_TIME; 50/50 -> Ä‘Ăºng giá»¯a hai má»©c.
      uint32_t getGreenDuration(uint8_t pair, uint8_t mode)
      {
            if (mode == MODE_NORMAL)
                  return GREEN_NORMAL_TIME;

            float ratio = getVehicleRatio(pair); // 0.0 - 1.0, tá»‰ lá»‡ lÆ°u lÆ°á»£ng cá»§a "pair" nĂ y
            uint32_t duration = GREEN_SHORT_TIME +
                  (uint32_t)(ratio * (float)(GREEN_LONG_TIME - GREEN_SHORT_TIME));

            // Káº¹p giĂ¡ trá»‹ trong khoáº£ng an toĂ n [GREEN_SHORT_TIME, GREEN_LONG_TIME]
            if (duration < GREEN_SHORT_TIME) duration = GREEN_SHORT_TIME;
            if (duration > GREEN_LONG_TIME)  duration = GREEN_LONG_TIME;

            return duration;
      }

      // IR SENSOR CHECK

void checkIR(void)
{
    uint32_t now = HAL_GetTick();
    if (now - lastIRCheck < IR_POLL_INTERVAL_MS) return; // tÄƒng tá»‘c Ä‘á»c IR
    lastIRCheck = now;

    // ----- TrÆ°á»£t cá»­a sá»• Ä‘áº¿m máº­t Ä‘á»™ xe (sliding window) -----
    // Má»—i VEHICLE_SLOT_MS, má»Ÿ 1 "Ă´" má»›i vĂ  xoĂ¡ Ă´ cÅ© nháº¥t (cĂ¡ch Ä‘Ă¢y
    // VEHICLE_WINDOW_MS) ra khá»i tá»•ng máº­t Ä‘á»™, Ä‘á»ƒ vehicleDensity[] luĂ´n
    // pháº£n Ă¡nh Ä‘Ăºng "sá»‘ xe trong N giĂ¢y gáº§n nháº¥t" chá»© khĂ´ng tĂ­ch lÅ©y mĂ£i.
    if (now - lastSlotShiftTime >= VEHICLE_SLOT_MS)
    {
        lastSlotShiftTime = now;
        currentSlotIndex = (currentSlotIndex + 1) % VEHICLE_WINDOW_SLOTS;
        for (int lane = 0; lane < 4; lane++)
        {
            vehicleDensity[lane] -= vehicleSlots[lane][currentSlotIndex]; // bá» Ă´ cÅ© nháº¥t ra khá»i tá»•ng
            vehicleSlots[lane][currentSlotIndex] = 0; // má»Ÿ Ă´ má»›i sáº¡ch Ä‘á»ƒ Ä‘áº¿m tiáº¿p
        }
    }

    uint16_t irPins[4] = {IR1, IR2, IR3, IR4};

    for (int i = 0; i < 4; i++)
    {
        GPIO_PinState state = HAL_GPIO_ReadPin(IR_PORT, irPins[i]);

        //  XE PHĂT HIá»†N 
        if (state == GPIO_PIN_RESET)
        {
            if (isDetecting[i] == 0)
            {
                isDetecting[i] = 1;
                detectStart[i] = now;
            }

            
            if ((now - detectStart[i]) >= IR_CONFIRM_VEHICLE_MS)
            {
                sensorActive[i] = 1;
            }
        }
        else
        {
            // XE Rá»œI KHá»I Cáº¢M BIáº¾N 
            isDetecting[i] = 0;
            sensorActive[i] = 0;
            counted[i] = 0;
        }

        // Äáº¾M XE 
        if (sensorActive[i] && !counted[i])
        {
            switch (i)
            {
                case 0: lane1_count++; break;
                case 1: lane2_count++; break;
                case 2: lane3_count++; break;
                case 3: lane4_count++; break;
            }
            counted[i] = 1;

            // Ghi nháº­n xe nĂ y vĂ o Ă´ hiá»‡n táº¡i cá»§a sliding window + cá»™ng vĂ o tá»•ng máº­t Ä‘á»™
            vehicleSlots[i][currentSlotIndex]++;
            vehicleDensity[i]++;
        }
    }

    // ----- Auto night mode: theo dĂµi má»‘c thá»i gian gáº§n nháº¥t cĂ³ xe -----
    // DĂ¹ng sensorActive[] (tráº¡ng thĂ¡i "Ä‘ang cĂ³ xe" tá»•ng quĂ¡t cá»§a tá»«ng lĂ n,
    // khĂ´ng chá»‰ Ä‘Ăºng khoáº£nh kháº¯c vá»«a xĂ¡c nháº­n xe má»›i) Ä‘á»ƒ má»‘c thá»i gian
    // luĂ´n Ä‘Æ°á»£c cáº­p nháº­t chá»«ng nĂ o cĂ²n xe á»Ÿ báº¥t ká»³ lĂ n nĂ o.
    uint8_t anyVehiclePresent = sensorActive[0] || sensorActive[1] ||
                                 sensorActive[2] || sensorActive[3];
    if (anyVehiclePresent)
    {
        lastVehicleSeenTime = now;

        // CĂ³ xe trá»Ÿ láº¡i sau khi há»‡ thá»‘ng Tá»° vĂ o night mode -> tá»± thoĂ¡t,
        // tráº£ láº¡i quyá»n Ä‘iá»u khiá»ƒn Ä‘Ă¨n bĂ¬nh thÆ°á»ng. Náº¿u ngÆ°á»i dĂ¹ng tá»± báº¥m
        // nĂºt vĂ o night mode (nightMode=1, autoNightMode=0) thĂ¬ KHĂ”NG tá»±
        // thoĂ¡t á»Ÿ Ä‘Ă¢y, pháº£i Ä‘á»£i ngÆ°á»i dĂ¹ng báº¥m nĂºt láº¡i.
        if (autoNightMode)
        {
            autoNightMode = 0;
            nightMode = 0;
        }
    }
    else if (!nightMode && (now - lastVehicleSeenTime >= AUTO_NIGHTMODE_IDLE_MS))
    {
        // ÄÆ°á»ng váº¯ng Ä‘á»§ lĂ¢u vĂ  Ä‘ang KHĂ”NG á»Ÿ night mode -> tá»± Ä‘á»™ng vĂ o
        // night mode. ÄĂ¡nh dáº¥u autoNightMode=1 Ä‘á»ƒ phĂ¢n biá»‡t vá»›i viá»‡c ngÆ°á»i
        // dĂ¹ng tá»± báº¥m nĂºt (autoNightMode=0), nhá» Ä‘Ă³ cĂ³ xe quay láº¡i sáº½ tá»±
        // thoĂ¡t mĂ  khĂ´ng cáº§n ngÆ°á»i dĂ¹ng pháº£i báº¥m nĂºt thĂªm láº§n ná»¯a.
        nightMode = 1;
        autoNightMode = 1;
    }

    //CHá»ŒN MODE - dá»±a trĂªn Máº¬T Äá»˜ xe (sliding window) thay vĂ¬ chá»‰ cĂ³/khĂ´ng cĂ³ xe.
    // TrÆ°á»›c Ä‘Ă¢y: chá»‰ cáº§n "cĂ³ xe" á»Ÿ lĂ n nĂ o lĂ  Æ°u tiĂªn cá»‘ Ä‘á»‹nh cho cáº·p Ä‘Ă³,
    // khĂ´ng phĂ¢n biá»‡t 1 xe hay 20 xe. Giá» dĂ¹ng tá»•ng máº­t Ä‘á»™ 2 cáº·p lĂ n trong
    // VEHICLE_WINDOW_MS gáº§n nháº¥t Ä‘á»ƒ quyáº¿t Ä‘á»‹nh mode VĂ€ má»©c Ä‘á»™ Æ°u tiĂªn.
    uint16_t density13 = vehicleDensity[0] + vehicleDensity[2]; // cáº·p L1+L3
    uint16_t density24 = vehicleDensity[1] + vehicleDensity[3]; // cáº·p L2+L4

    TrafficMode instantMode;
    if (density13 > density24)
        instantMode = MODE_FAVOR_13;
    else if (density24 > density13)
        instantMode = MODE_FAVOR_24;
    else
        instantMode = MODE_NORMAL;

    if (instantMode != pendingMode)
    {
        // TĂ­n hiá»‡u vá»«a Ä‘á»•i -> báº¯t Ä‘áº§u láº¡i Ä‘áº¿m thá»i gian á»•n Ä‘á»‹nh
        pendingMode = instantMode;
        pendingModeStart = now;
    }
    else if ((now - pendingModeStart) >= MODE_STABLE_MS)
    {
        // TĂ­n hiá»‡u Ä‘Ă£ á»•n Ä‘á»‹nh Ä‘á»§ lĂ¢u -> má»›i cho phĂ©p chá»‘t vĂ o requestedMode
        requestedMode = pendingMode;
    }
}

// ===== TĂ­nh tá»‰ lá»‡ máº­t Ä‘á»™ xe giá»¯a 2 cáº·p lĂ n =====
// Tráº£ vá» tá»‰ lá»‡ (0.0 - 1.0) cá»§a cáº·p "pair" trong tá»•ng máº­t Ä‘á»™ 2 cáº·p, dĂ¹ng Ä‘á»ƒ
// chia thá»i gian xanh theo tá»‰ lá»‡ thá»±c táº¿ lÆ°u lÆ°á»£ng, thay vĂ¬ chá»‰ 2 má»©c cá»‘
// Ä‘á»‹nh "dĂ i/ngáº¯n" nhÆ° cĂ´ng thá»©c GREEN_LONG_TIME/GREEN_SHORT_TIME cÅ©.
float getVehicleRatio(uint8_t pair)
{
    uint16_t density13 = vehicleDensity[0] + vehicleDensity[2];
    uint16_t density24 = vehicleDensity[1] + vehicleDensity[3];
    uint16_t total = density13 + density24;

    if (total == 0) return 0.5f; // khĂ´ng cĂ³ dá»¯ liá»‡u -> chia Ä‘á»u 50/50

    uint16_t ownDensity = (pair == 0) ? density13 : density24;
    return (float)ownDensity / (float)total;
}


      // STATE MACHINE

      void updateState(void)
      {
            uint32_t now = HAL_GetTick();
            if(now - stateStart < stateDuration) return;
            
            if(currentState == GREEN)
            {
                  setPair(YELLOW);
                  stateStart = now;
                  stateDuration = YELLOW_TIME;
            }
            else
            {
                  currentPair = !currentPair;
                  activeMode = requestedMode;
                  setPair(GREEN);
                  stateStart = now;
                  stateDuration = getGreenDuration(currentPair, activeMode);
            }
      }


      // LCD UPDATE 

      void LCD_L1_L3(uint32_t time, uint8_t state)
      {
            uint8_t col = (state == GREEN) ? 0 : (state == YELLOW) ? 1 : 2;
            char digit = '0' + (time % 10);
            
            lcd_put_cur(1, col);      // L1
            lcd_send_data(digit);
            lcd_put_cur(1, col + 8);  // L3
            lcd_send_data(digit);
      }

      void LCD_L2_L4(uint32_t time, uint8_t state)
      {
            uint8_t col = (state == GREEN) ? 4 : (state == YELLOW) ? 5 : 6;
            char digit = '0' + (time % 10);
            lcd_put_cur(1, col);      // L2
            lcd_send_data(digit);
            lcd_put_cur(1, col + 8);  // L4
            lcd_send_data(digit);
      }

      void updateLCD(void)
      {
            uint32_t now = HAL_GetTick();

            // ----- Xá»­ lĂ½ thĂ´ng bĂ¡o "SYSTEM RESET" khĂ´ng cháº·n (non-blocking) -----
            // Thay cho HAL_Delay(800) cÅ©: chá»‰ giá»¯ thĂ´ng bĂ¡o trĂªn LCD trong
            // RESET_MSG_DURATION ms rá»“i tá»± Ä‘á»™ng váº½ láº¡i mĂ n hĂ¬nh bĂ¬nh thÆ°á»ng,
            // khĂ´ng lĂ m dá»«ng state machine / IR / UART trong lĂºc Ä‘Ă³.
            if(resetMsgShowing)
            {
                  if(now - resetMsgStart >= RESET_MSG_DURATION)
                  {
                        resetMsgShowing = 0;
                        lcd_clear();
                        lcd_put_cur(0,0);
                        lcd_send_string("XVD XVD XVD XVD");
                  }
                  else
                  {
                        return; // váº«n Ä‘ang hiá»ƒn thá»‹ "SYSTEM RESET", chÆ°a váº½ láº¡i UI thÆ°á»ng
                  }
            }

            uint32_t elapsed = now - stateStart;
            
            if(now - lastLCDUpdate < LCD_UPDATE_SEC) return;
            lastLCDUpdate = now;
            
            uint32_t remainingMs = (elapsed >= stateDuration) ? 0 : stateDuration - elapsed;
            uint32_t remainingSec = (remainingMs + 999) / 1000;
            
            lcd_put_cur(1, 0);
            lcd_send_string("                ");

            // ----- Thá»i gian cĂ²n láº¡i cá»§a cáº·p Ä‘Ă¨n Äá»I DIá»†N (Ä‘ang RED) -----
            // Cáº·p Ä‘á»‘i diá»‡n luĂ´n Ä‘á» trong khi cáº·p hiá»‡n táº¡i GREEN/YELLOW.
            // - Náº¿u cáº·p hiá»‡n táº¡i Ä‘ang GREEN: cáº·p Ä‘á»‘i diá»‡n cĂ²n pháº£i Ä‘á»£i
            //   háº¿t pháº§n GREEN cĂ²n láº¡i + cáº£ pha YELLOW sáº¯p tá»›i -> remainingSec + YELLOW_TIME.
            // - Náº¿u cáº·p hiá»‡n táº¡i Ä‘ang YELLOW: cáº·p Ä‘á»‘i diá»‡n chá»‰ cĂ²n pháº£i
            //   Ä‘á»£i háº¿t Ä‘Ăºng pháº§n YELLOW cĂ²n láº¡i -> remainingSec.
            uint32_t oppositeRemaining;
            if(currentState == GREEN)
            {
                  oppositeRemaining = remainingSec + (YELLOW_TIME / 1000);
            }
            else // YELLOW
            {
                  oppositeRemaining = remainingSec;
            }

         if(nightMode)
      {
            lcd_put_cur(1,0);
            lcd_send_string("                ");
         
            lcd_put_cur(0,0);
            lcd_send_string("                ");
         
            lcd_put_cur(1,0);
            lcd_send_string("   NIGHT MODE   ");
            return;
      }
            if(!nightMode)
      {
            lcd_put_cur(0, 0);
            lcd_send_string("XVD XVD XVD XVD");
      }

            if(currentPair == 0)
            {
                  LCD_L1_L3(remainingSec, currentState);
                  LCD_L2_L4(oppositeRemaining, RED);
            }
            else
            {
                  LCD_L1_L3(oppositeRemaining, RED);
                  LCD_L2_L4(remainingSec, currentState);
            }
      }

      void checkButton(void)
      {
            // Edge-detect (cáº¡nh xuá»‘ng): chá»‰ toggle 1 láº§n khi vá»«a nháº¥n,
            // dĂ¹ ngÆ°á»i dĂ¹ng cĂ³ GIá»® nĂºt bao lĂ¢u cÅ©ng khĂ´ng bá»‹ láº·p toggle.
            static uint8_t lastNightBtnState = 1; // pull-up -> máº·c Ä‘á»‹nh nháº£ = 1 (SET)
            uint32_t now = HAL_GetTick();
            GPIO_PinState btnState = HAL_GPIO_ReadPin(GPIOB, BTN_NIGHT);

            if(btnState == GPIO_PIN_RESET && lastNightBtnState == GPIO_PIN_SET)
            {
                  if(now - lastButtonPress > IR_DEBOUNCE_MS) // debounce theo cáº¡nh, khĂ´ng theo giá»¯ nĂºt
                  {
                        nightMode = !nightMode;
                        lastButtonPress = now;

                        // NgÆ°á»i dĂ¹ng báº¥m nĂºt thá»§ cĂ´ng -> chuyá»ƒn toĂ n quyá»n kiá»ƒm soĂ¡t
                        // vá» ngÆ°á»i dĂ¹ng, khĂ´ng cĂ²n lĂ  night mode do há»‡ thá»‘ng Tá»°
                        // kĂ­ch hoáº¡t ná»¯a (trĂ¡nh trÆ°á»ng há»£p cĂ³ xe cháº¡y qua láº¡i vĂ´
                        // tĂ¬nh tá»± thoĂ¡t night mode mĂ  ngÆ°á»i dĂ¹ng vá»«a báº­t báº±ng tay).
                        autoNightMode = 0;
                        lastVehicleSeenTime = now;

                              // reset toĂ n bá»™ LED
            HAL_GPIO_WritePin(GPIOA,
            GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
            GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|
            GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_15,
            GPIO_PIN_RESET);

            HAL_GPIO_WritePin(GPIOB,
            GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11,
            GPIO_PIN_RESET);
                  }
            }

            lastNightBtnState = btnState;
      } 
      void resetSystem(void)
{
    // RESET COUNTER
    lane1_count = 0;
    lane2_count = 0;
    lane3_count = 0;
    lane4_count = 0;

    //  RESET SENSOR 
    for(int i = 0; i < 4; i++)
    {
        counted[i] = 0;
        isDetecting[i] = 0;
        sensorActive[i] = 0;
        detectStart[i] = 0;
    }

    //  RESET MODE
    requestedMode = MODE_NORMAL;
    activeMode = MODE_NORMAL;
          //  reset sliding window
    memset(vehicleSlots, 0, sizeof(vehicleSlots));
    memset(vehicleDensity, 0, sizeof(vehicleDensity));
    currentSlotIndex = 0;
    lastSlotShiftTime = HAL_GetTick();

    // reset auto night mode state
    autoNightMode = 0;
    lastVehicleSeenTime = HAL_GetTick();

    //  RESET TRAFFIC
    currentPair = 0;

    setPair(GREEN);

    stateStart = HAL_GetTick();
    stateDuration = getGreenDuration(currentPair, activeMode);

    // RESET LCD - hiá»ƒn thá»‹ thĂ´ng bĂ¡o KHĂ”NG cháº·n (non-blocking).
    // updateLCD() sáº½ tá»± xoĂ¡ thĂ´ng bĂ¡o nĂ y sau RESET_MSG_DURATION ms.
    lcd_clear();
    lcd_put_cur(0,0);
    lcd_send_string("SYSTEM RESET");

    resetMsgShowing = 1;
    resetMsgStart   = HAL_GetTick();

    // Báº¯t buá»™c láº§n update LCD káº¿ tiáº¿p Ä‘Æ°á»£c váº½ láº¡i ngay
    lastLCDUpdate = HAL_GetTick() - LCD_UPDATE_SEC;
}
      
      void checkResetButton(void)
{
    // Edge-detect (cáº¡nh xuá»‘ng): chá»‰ reset 1 láº§n má»—i láº§n nháº¥n,
    // trĂ¡nh giá»¯ nĂºt > 300ms gá»i resetSystem() láº·p láº¡i nhiá»u láº§n.
    static uint8_t lastResetBtnState = 1; // pull-up -> nháº£ = 1 (SET)

    GPIO_PinState btnState = HAL_GPIO_ReadPin(GPIOB, BTN_RESET);

    if(btnState == GPIO_PIN_RESET && lastResetBtnState == GPIO_PIN_SET)
    {
        resetSystem();
    }

    lastResetBtnState = btnState;
}
      
      void runNightMode(void)
      {
            uint32_t now = HAL_GetTick();
         
         laneState[0] = YELLOW;
         laneState[1] = YELLOW;
         laneState[2] = YELLOW;
         laneState[3] = YELLOW;

            if(now - lastBlink >= NIGHTMODE_BLINK_MS) // nhap nhay nightmode
            {
                  lastBlink = now;
                  blinkState ^= 1;

                  // L1, L2, L3 (PORT A)
            HAL_GPIO_WritePin(GPIOA, L1_Y | L2_Y | L3_Y,
               
            blinkState ? GPIO_PIN_SET : GPIO_PIN_RESET);

                  // L4 (PORT B)
            HAL_GPIO_WritePin(GPIOB, L4_Y,
               
            blinkState ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
      }
      
void ESP32_SendData(void)
{
      char buffer[160];
      char dataPart[130];
    // ThĂªm trÆ°á»ng M:%d (activeMode) - trÆ°á»›c Ä‘Ă¢y KHĂ”NG gá»­i mode qua UART
    // nĂªn dashboard ESP32 khĂ´ng bao giá» cáº­p nháº­t Ä‘Æ°á»£c badge "BINH THUONG /
    // UU TIEN 1-3 / UU TIEN 2-4".
sprintf(dataPart,
    "N:%d;M:%d;"
    "S1:%d;C1:%d;D1:%d;"
    "S2:%d;C2:%d;D2:%d;"
    "S3:%d;C3:%d;D3:%d;"
    "S4:%d;C4:%d;D4:%d",
    nightMode, (int)activeMode,
    laneState[0], lane1_count, vehicleDensity[0],
    laneState[1], lane2_count, vehicleDensity[1],
    laneState[2], lane3_count, vehicleDensity[2],
    laneState[3], lane4_count, vehicleDensity[3]
);

    // ----- Checksum UART (XOR Ä‘Æ¡n giáº£n) -----
    // TĂ­nh XOR trĂªn toĂ n bá»™ kĂ½ tá»± cá»§a dataPart, gá»­i kĂ¨m dÆ°á»›i dáº¡ng hex 2 kĂ½
    // tá»± (CK:%02X) á»Ÿ cuá»‘i frame. PhĂ­a ESP32 tĂ­nh láº¡i checksum trĂªn pháº§n dá»¯
    // liá»‡u nháº­n Ä‘Æ°á»£c vĂ  so sĂ¡nh, nhá» Ä‘Ă³ loáº¡i bá» cháº¯c cháº¯n cĂ¡c frame bá»‹
    // nhiá»…u/Ä‘á»©t giá»¯a Ä‘Æ°á»ng truyá»n, thay vĂ¬ chá»‰ dá»±a vĂ o sscanf Ä‘áº¿m Ä‘á»§ sá»‘
    // field (vá»‘n váº«n cĂ³ thá»ƒ "vĂ´ tĂ¬nh" khá»›p Ä‘á»§ field vá»›i dá»¯ liá»‡u sai do nhiá»…u).
    uint8_t checksum = 0;
    for (size_t i = 0; i < strlen(dataPart); i++)
    {
        checksum ^= (uint8_t)dataPart[i];
    }

    sprintf(buffer, "%s;CK:%02X\r\n", dataPart, checksum);

    HAL_UART_Transmit(
        &huart1,
        (uint8_t*)buffer,
        strlen(buffer),
        100
    );
}
      /* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
            lcd_init();
            lcd_clear();
            lcd_put_cur(0, 0);
            lcd_send_string("XVD XVD XVD XVD");
            
            currentPair = 0;
            activeMode = MODE_NORMAL;
            requestedMode = MODE_NORMAL;
            
            setPair(GREEN);
            stateStart = HAL_GetTick();
            stateDuration = getGreenDuration(currentPair, activeMode);
            
            lastIRCheck = HAL_GetTick();
            lastLCDUpdate = HAL_GetTick();
            lastMainUpdate = HAL_GetTick();
            

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
         while (1)
         {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
               uint32_t now = HAL_GetTick();

         checkButton();

         // Refresh watchdog má»—i vĂ²ng láº·p: náº¿u vĂ²ng láº·p chĂ­nh bá»‹ treo
         // (deadlock, vĂ´ tĂ¬nh rÆ¡i vĂ o loop vĂ´ háº¡n á»Ÿ Ä‘Ă¢u Ä‘Ă³...), dĂ²ng nĂ y
         // sáº½ khĂ´ng Ä‘Æ°á»£c gá»i ná»¯a -> IWDG tá»± reset MCU sau IWDG_TIMEOUT_MS,
         // trĂ¡nh Ä‘Ă¨n giao thĂ´ng bá»‹ "Ä‘á»©ng yĂªn á»Ÿ 1 mĂ u" mĂ£i.
         HAL_IWDG_Refresh(&hiwdg);

         if(nightMode)
         {
               checkIR();
               runNightMode();
               updateLCD();
               checkResetButton(); 
               if(now - lastMainUpdate >= UPDATE_INTERVAL)  // â† giá»›i háº¡n 10 frame/giĂ¢y
               {
                     ESP32_SendData();
                     lastMainUpdate = now;
               }

               HAL_Delay(NIGHTMODE_LOOP_DELAY_MS);
               continue;
         }

         // NORMAL MODE
         updateState();

         if(now - lastMainUpdate >= UPDATE_INTERVAL)
         {
               checkIR();
               updateLCD();
              ESP32_SendData(); 
               lastMainUpdate = now;
            checkResetButton();
         }
         
         HAL_Delay(NORMAL_LOOP_DELAY_MS);

         }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 1250;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3
                           PA4 PA5 PA6 PA7
                           PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB12 PB13 PB14
                           PB15 PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB10 PB11 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
         /* User can add his own implementation to report the HAL error return state */
         __disable_irq();
         while (1)
         {
         }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
         /* User can add his own implementation to report the file name and line number,
             ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */