#include "main.h"
#include "cmsis_gcc.h"
#include "config.h"
#include "dsp_pipeline.h"
#include "stm32u0xx_hal.h"
#include "stm32u0xx_hal_adc.h"
#include "stm32u0xx_hal_gpio.h"
#include "stm32u0xx_hal_tim.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

/* Private defines -----------------------------------------------------------*/
typedef enum {
    STATE_LISTEN,
    STATE_COMPUTE,
    STATE_ACTUATE,
    STATE_SETTLE,
    STATE_OUT_OF_BOUNDS
} system_state_t;

typedef enum {
    RED,
    GREEN,
    BLUE,
    OFF
} led_state_t;


/* Setup variables -----------------------------------------------------------*/
COM_InitTypeDef BspCOMInit;
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;


/* Function prototypes -------------------------------------------------------*/
static void stm32cubeMX_setup(void);
static void set_led_state(led_state_t color);
static void split_raw_data(void);
static float compute_angle_offset(int16_t sample_offset);
static void set_servo_angle(float angle);


/* Variables -----------------------------------------------------------------*/
// global state machine tracker
volatile system_state_t system_state = STATE_LISTEN;
volatile bool out_of_bound_detected = false;

// audio buffers
uint16_t adc_dma_buffer[TOTAL_DMA_BUFFER_SIZE];
uint16_t left_channel_buffer[AUDIO_BUFFER_SIZE];
uint16_t right_channel_buffer[AUDIO_BUFFER_SIZE];



/* ============================== MAIN LOOP ================================= */

int main(void)
{
  // variables
  float current_servo_angle = SERVO_CENTER_ANGLE; // Start at center
  float angle_offset;
  int16_t sample_offset;
  uint32_t last_sound_time = HAL_GetTick(); // for inactivity timeout

  // wrapper function to hide stm32cubeMX generated code from the main function
  stm32cubeMX_setup();

  // Turn on the Servo PWM Timer (Timer 2)
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  // Turn on the ADC Trigger Timer (Timer 3)
  HAL_TIM_Base_Start(&htim3);

  set_servo_angle(SERVO_CENTER_ANGLE);
  HAL_Delay(BEAM_SETUP_DELAY); // Allow time for the servo to reach center position and center the beam

  while (1){
    switch(system_state){

      case STATE_LISTEN:

            set_led_state(BLUE);

            // Start the DMA transferl
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, TOTAL_DMA_BUFFER_SIZE);

            // Sleep CPU until DMA interrupt wakes it up and changes state to COMPUTE
            while(system_state == STATE_LISTEN) __WFI();
            break;


      case STATE_COMPUTE:

            set_led_state(BLUE);

            // Split the raw interleaved DMA data into clean left/right channels
            split_raw_data();
            
            // Call DSP pipeline and check if a sound was detected
            if(!DSP_pipeline(&sample_offset, QUIET_ROOM_TRESHOLD , left_channel_buffer, right_channel_buffer, AUDIO_BUFFER_SIZE, MAX_SAMPLE_LAG)){
              
              // If no sound detected, check for inactivity timeout
              if((HAL_GetTick() - last_sound_time > INACTIVITY_TIMEOUT) && (fabs(current_servo_angle - SERVO_CENTER_ANGLE) > 0.01f)){
                current_servo_angle = SERVO_CENTER_ANGLE; // Set angle to center
                last_sound_time = HAL_GetTick(); // Reset the timer so it doesn't constantly trigger
                system_state = STATE_ACTUATE;    // Go to actuate state to move the motor
              }
              else system_state = STATE_LISTEN;     // Otherwise, keep listening
              break; 
            }

            last_sound_time = HAL_GetTick(); // Update the sound tracker if a sound was detected

            // Compute the angle offset
            angle_offset = compute_angle_offset(sample_offset);

            // Set the new servo angle and check for boundary conditions
            if(current_servo_angle + angle_offset > SERVO_MAX_ANGLE){
              current_servo_angle = SERVO_MAX_ANGLE;
              out_of_bound_detected = true;
            }
            else if(current_servo_angle + angle_offset < SERVO_MIN_ANGLE){
              current_servo_angle = SERVO_MIN_ANGLE;
              out_of_bound_detected = true;
            } 
            else current_servo_angle += angle_offset;

            // DEBUG
            
            char boot_msg[] = "MCU Booted Successfully!\r\n";
            extern UART_HandleTypeDef hcom_uart[];
            HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t*)boot_msg, sizeof(boot_msg)-1, 1000);


            // go to next state
            system_state = STATE_ACTUATE;
            break;


      case STATE_ACTUATE:

            set_led_state(GREEN);
            
            // update servo position
            set_servo_angle(current_servo_angle);

            // change state
            if(out_of_bound_detected) system_state = STATE_OUT_OF_BOUNDS;
            else system_state = STATE_SETTLE;
            break;


      case STATE_SETTLE:

            // delay for servo mechanical noise
            set_led_state(GREEN);
            HAL_Delay(SETTLING_TIME);

            system_state = STATE_LISTEN;
            break;


      case STATE_OUT_OF_BOUNDS:

            // reset out of bound flag
            out_of_bound_detected = false;
            // Flash Red LED for 2 seconds
            int count = OUT_OF_BOUNDS_DELAY / 500;
            if(count < 1) count = 1; // Ensure at least one flash
            for(int i = 0; i < count; i++){
              set_led_state(RED);
              HAL_Delay(250);
              set_led_state(OFF);
              HAL_Delay(250);
            }
            // change state
            system_state = STATE_LISTEN;
            break;
    }
  }
}



/* ============================== FUNCTIONS ================================== */

/**
* @brief change the RGB LED state
* @param[in] color the desired color [RED, GREEN, BLUE] or [OFF] to turn it off
* @retval none
*/
static void set_led_state(led_state_t color){
  switch(color){
    case RED:
      HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
      break;
    
    case GREEN:
      HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
      break;

    case BLUE:
      HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
      break;

    case OFF:
      HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
      break;
  }
}


/**
* @brief split the raw data collected by the ADC into a left and right channel buffers
* @retval none
*/
static void split_raw_data(void){

  for(uint16_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    left_channel_buffer[i]  = adc_dma_buffer[2 * i];
    right_channel_buffer[i] = adc_dma_buffer[2 * i + 1];
  }
}


/**
* @brief compute the angle offset of the sound source based on the number of sample
*        offset between the two channels
* @param[in] sample_offset the offset between the two channels recordings
* @retval angle offset in radiants 
*/
static float compute_angle_offset(int16_t sample_offset){

  // compute the time delay of arrival
  float TDOA = (float) sample_offset / SAMPLE_FREQUENCY_HZ;

  // compute the angle offset applying the formula seen in the lectures
  float asin_arg = TDOA * SPEED_OF_SOUND / MIC_DISTANCE;

  // clamp the computer value for safety
  if(asin_arg > 1.0f) asin_arg = 1.0f;
  else if(asin_arg < -1.0f) asin_arg = -1.0f;

  float angle_offset = asinf(asin_arg);

  return angle_offset;
}


/**
* @brief set the servo motor to the desired angle within the servo limitations
* @param[in] angle the desired angle in radiants
* @retval none
*/
static void set_servo_angle(float angle){

  // convert angle to PWM
  int pwm_value = angle * (SERVO_MAX_PWM - SERVO_MIN_PWM)/(SERVO_MAX_ANGLE - SERVO_MIN_ANGLE) + SERVO_MIN_PWM;

  // clamp PWM value for safety reasons
  if(pwm_value > SERVO_MAX_PWM) pwm_value = SERVO_MAX_PWM;
  else if(pwm_value < SERVO_MIN_PWM) pwm_value = SERVO_MIN_PWM;

  // set PWM value
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_value);
}



/* ============================== INTERRUPT CALLBACKS ======================== */

/**
* @brief Conversion complete ISR invoked automatically when the ADC DMA buffer is completely filled with audio samples.
* @param[in, out] hadc Pointer to the ADC handle structure
* @retval none
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
  if(hadc->Instance == ADC1){
    // stop the ADC
    HAL_ADC_Stop_DMA(hadc);
    // update FSM state to COMPUTE
    system_state = STATE_COMPUTE;
  } 
}



/* ============================== HARDWARE SETUP ============================= */

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);

/**
  * @brief wrapper function to hide stm32cubeMX generated code from the main function
  * @return None
  */
static void stm32cubeMX_setup(void)
{
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIG_T3_TRGO;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 15;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 319;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_R_Pin|LED_G_Pin|LED_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_R_Pin LED_G_Pin LED_B_Pin */
  GPIO_InitStruct.Pin = LED_R_Pin|LED_G_Pin|LED_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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

}
#endif /* USE_FULL_ASSERT */
