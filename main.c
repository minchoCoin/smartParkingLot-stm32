#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
/**
 * @brief RCC를 설정하는 함수
 * @param none
 * @return none
*/
void RCC_Configure(void);
/**
 * @brief GPIO를 설정하는 함수
 * @param none
 * @return none
*/
void GPIO_Configure(void);
/**
 * @brief ADC를 설정하는 함수
 * @param none
 * @return none
*/
void ADC_Configure(void);
/**
 * @brief NVIC를 설정하는 함수
 * @param none
 * @return none
*/
void NVIC_Configure(void);
//void ADC1_2_IRQHandler(void);
/**
 * @brief DMA1 Channel ISR 함수
 * @param none
 * @return none
*/
void DMA1_Channel1_IRQHandler(void);
/**
 * @brief DMA를 설정하는 함수
 * @param none
 * @return none
*/
void DMA_Configure();
/**
 * @brief USART1(putty)를 설정하는 함수
 * @param none
 * @return none
*/
void USART1_Init(void);
/**
 * @brief USART2(bluetooth)를 설정하는 함수
 * @param none
 * @return none
*/
void USART2_Init(void);
/**
 * @brief USART1(putty)에서 받은 문자열을 처리하는 함수
 * @param none
 * @return none
*/
void end_usart1();
/**
 * @brief USART2(bluetooth)에서 받은 문자열을 처리하는 함수
 * @param none
 * @return none
*/
void end_usart2();
/**
 * @brief USART2(bluetooth)로 문자열을 보내는 함수
 * @param char* 문자열
 * @return none
*/
void send_msg_to_bluetooth();
/**
 * @brief USART1(putty)로 문자열을 보내는 함수
 * @param char* 문자열
 * @return none
*/
void send_msg_to_putty();
/**
 * @brief 일정시간 기다리는 함수
 * @param int 이 값이 클수록 많이 기다림
 * @return none
*/
void delay(int);
/**
 * @brief 현재 parking lot의 정보를 출력한다. O는 비어있다는 것이고, X는 주차되어있다는 뜻이다. 
 * @param none
 * @return none
*/
void print_parking_lot(void);


int delay_value[3] = {10000000, 10000000, 10000000}; //주차공간1, 주차공간2, 주차공간3의 LED의 delay value이다. 이 값을 이용하여 각 주차공간의 LED를 깜빡인다.

int parking_lot[3] = {0, 0, 0};//parking lot의 현재 상태이다. -1 예약되어있음, 0 비어있음, 1 주차 진행 중, 2 주차 완료
char usart1_msg[50]; // usart1(putty)에서 메시지를 받을 때 메세지를 저장할 버퍼이다.
char usart2_msg[50]; // usart2(bluetooth)에서 메시지를 받을 때 메시지를 저장할 버퍼이다.
int usart1_index = 0;//usart1_msg 버퍼에 다음으로 문자가 들어갈 인덱스이다.
int usart2_index = 0;//usart2_msg 버퍼에 다음으로 문자가 들어갈 인덱스이다.
int help = 0;//help=1이면 관리자와 사용자 간의 통신을 한다. 즉 입력한 정보는 명령이 아니라 관리자(또는 사용자)에게 보낼 메시지이다.
int communication = 0;      //0이면 주차장 관련 정보를 블루투스로 받는다. 1이면 주차장 관련 정보를 블루투스로 받지 않는다.
int stable_count[3] = {0, 0, 0};//각 주차공간에 주차가 시작된 후(즉 벽과의 거리가 일정 거리 이하) 경과된 시간을 의미한다. 이 값이 25000이상이면 주차가 완료된다. 
                                //만약 이 값이 25000이 되지 않았는데 벽과 거리가 일정 거리 이상으로 벌어지면 주차가 중단되었다고 판단하고 이 값을 0으로 설정한다.
int user_lot = -1;//사용자와 연결된 주차공간이다. 0,1,2가 각각 주차공간1, 주차공간2, 주차공간3을 의미한다.
volatile uint32_t value[6];//센서로부터 온 아날로그 값을 저장하는 공간이다. ADC와 DMA를 통해 센서값을 이 공간에 저장한다.
int dv = 1000000; //이 값을 이용하여 각 주차공간의 LED를 깜빡인다.
clock_t start_time[3];//주차가 시작되었을 때 시간이다.
char buffer[50];//sprintf를 이용하여 정수를 문자열로 변환할 때 버퍼로 쓰인다.

/*
pin connected
      주차1  주차2  주차3
Red : PD10 / PD13 / PD3
Green: PD11 / PD14 / PD4
Blue : PD12 /PD15/ PD5
photoresistor: PB0, PC0, PC4
distance : PB1, PC1, PC5

-------------------------
USART1 : PA9(TX),PA10(RX)
USART2 : PA2(TX)(Connected bluetooth RX pin),PA3(RX)(Connected bluetooth TX pin)
*/
int main() {
  SystemInit();
  RCC_Configure();
  GPIO_Configure();
  ADC_Configure();
  USART1_Init();      // pc
  USART2_Init();      // bluetooth
  NVIC_Configure();
  DMA_Configure();
   


 //주차공간1의 LED를 녹색으로 초기화한다.
  GPIO_ResetBits(GPIOD,GPIO_Pin_10);  
  GPIO_SetBits(GPIOD,GPIO_Pin_11);  
  GPIO_ResetBits(GPIOD,GPIO_Pin_12); 

//주차공간2의 LED를 녹색으로 초기화한다.
  GPIO_ResetBits(GPIOD,GPIO_Pin_13);  
  GPIO_SetBits(GPIOD,GPIO_Pin_14);  
  GPIO_ResetBits(GPIOD,GPIO_Pin_15); 

//주차공간2의 LED를 녹색으로 초기화한다.
  GPIO_ResetBits(GPIOD,GPIO_Pin_3);  
  GPIO_SetBits(GPIOD,GPIO_Pin_4);  
  GPIO_ResetBits(GPIOD,GPIO_Pin_5);  

  while(1){
    if (parking_lot[0] == 1) {
        //주차공간1이 주차중 상태일 경우 주차공간1에 연결된 빨강색 LED를 깜빡인다.
        GPIO_SetBits(GPIOD,GPIO_Pin_10);            // 빨강 ON
        if(delay_value[0]!=0)
            delay(dv);//delay
        if(delay_value[0]!=0){
            GPIO_ResetBits(GPIOD,GPIO_Pin_10);      // 빨강 OFF
            delay(dv);
        }
    }

    if (parking_lot[1] == 1) {
        //주차공간2가 주차중 상태일 경우 주차공간2에 연결된 빨강색 LED를 깜빡인다.
        GPIO_SetBits(GPIOD,GPIO_Pin_13);            // 빨강 ON
        if(delay_value[1]!=0)
            delay(dv);//delay
        if(delay_value[1]!=0){
            GPIO_ResetBits(GPIOD,GPIO_Pin_13);      // 빨강 OFF
            delay(dv);
        }
    }
   
    if (parking_lot[2] == 1) {
        //주차공간3이 주차중 상태일 경우 주차공간3에 연결된 빨강색 LED를 깜빡인다.
        GPIO_SetBits(GPIOD,GPIO_Pin_3);            // 빨강 ON
        if(delay_value[2]!=0)
            delay(dv);//delay
        if(delay_value[2]!=0){
            GPIO_ResetBits(GPIOD,GPIO_Pin_3);      // 빨강 OFF
            delay(dv);
        }
    }
  }
}

void RCC_Configure(void)
{
    /* PC0,PC1핀(주차공간2의 센서)을 사용하기 위해 RCC GPIOC 클럭 인가 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    /*LED를 사용하기 위해 RCC GPIOD 클럭 인가 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    /* 아날로그 센서값 읽기 위해 ADC에 클럭 인가 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    // DMA에 클럭 인가
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

       /* USART1, USART2 TX/RX port clock enable */

   // USART1 : PA9, PA10 // USART2 : PA2, PA3
   //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

   /* USART1, USART2 clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
   
   /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);


}


void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure1;

    /* ADC1 Channel8 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    /* ADC1_2 CHannel9*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    /* ADC1 Channel10 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* ADC1 Channel11 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /*ADC Channel14*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

     /* ADC Channel15*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
   
    /*GPIO LED*/
    GPIO_InitStructure1.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure1);

      /* USART1 pin setting */
    //TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // Tx -> PA9
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50Mhz
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate function
   GPIO_Init(GPIOA, &GPIO_InitStructure); // PA9  설정
   
   //RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // Rx -> PA10
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50Mhz
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // Pull-Up
   GPIO_Init(GPIOA, &GPIO_InitStructure); // PA10 설정
   
    /* USART2 pin setting */
    //TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; // Tx -> PA2
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50Mhz
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Alternate function
   GPIO_Init(GPIOA, &GPIO_InitStructure); // PA2  설정
   
   //RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // Rx -> PA3
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50Mhz
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // Pull-Up
   GPIO_Init(GPIOA, &GPIO_InitStructure); // PA3  설정

}


void ADC_Configure()
{
    ADC_InitTypeDef ADC_InitStructure;

    // ADC1 Configuration
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;//ADC1과 ADC2는 독립적으로 움직인다
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;//여러 센서를 측정해야하므로, ScanConvMode 활성화
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//한번만 변환하는 것이 아니라, 계속 변환
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//외부 trigger 미사용
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//12비트 아날로그 값을 16비트 레지스터(ADC->DR)에 오른쪽 정렬하여 저장한다.
    ADC_InitStructure.ADC_NbrOfChannel = 6;//측정할 센서는 6개다.
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);        //pb0 조도1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 2, ADC_SampleTime_239Cycles5);        //pb1 거리1

    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_239Cycles5);        //pc0 조도2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 4, ADC_SampleTime_239Cycles5);        //pc1 거리2
   
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_239Cycles5);        //pc4 조도3
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 6, ADC_SampleTime_239Cycles5);        //pc5 거리3

    // Enable interrupt
    //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
    //Enable ADC to DMA
    ADC_DMACmd(ADC1,ENABLE);

    ADC_ResetCalibration(ADC1);

    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while(ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}


void USART1_Init(void)
{
    USART_InitTypeDef USART1_InitStructure;

   // Enable the USART1 peripheral
   USART_Cmd(USART1, ENABLE);
   
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
   USART1_InitStructure.USART_BaudRate = 9600;
   USART1_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART1_InitStructure.USART_StopBits = USART_StopBits_1;
   USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // CTS, RTS x
   USART1_InitStructure.USART_Parity = USART_Parity_No;
   USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART1, &USART1_InitStructure); //          
   
   // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // Rx -> interrupt enable

}

void USART2_Init(void)
{
    USART_InitTypeDef USART2_InitStructure;

   // Enable the USART2 peripheral
   USART_Cmd(USART2, ENABLE);
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
   USART2_InitStructure.USART_BaudRate = 9600;
   USART2_InitStructure.USART_WordLength = USART_WordLength_8b;
   USART2_InitStructure.USART_StopBits = USART_StopBits_1;
   USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // CTS, RTS x
   USART2_InitStructure.USART_Parity = USART_Parity_No;
   USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
   USART_Init(USART2, &USART2_InitStructure); //          
   
   // TODO: Enable the USART2 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
   USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // Rx -> interrupt enable
}


void NVIC_Configure(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    // NVIC Line DMA_Channel1
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

     // USART1
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // USART2
    // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // TODO
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void USART1_IRQHandler() {      // PUTTY -> Phone
    uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
        // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
        //받은 word에서 뒤 8비트만 추출
        word=word&0xFF;
        
        if (word != '\n' && word!='\r')
            //받은 word가 개행문자 등 특수문자가 아닐 경우 usart1_msg 버퍼에 받은 문자를 넣는다.
            usart1_msg[usart1_index++] = word;


        if (word=='\r'){
            //받은 word가 특수문자일 경우 널 문자를 넣고 end_usart()를 호출하여 받은 문자열을 판단한다.
             usart1_msg[usart1_index++] = '\0';
            end_usart1();
        }

        // clear 'Read data register not empty' flag
       USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void USART2_IRQHandler() {      // Phone -> PUTTY
    uint16_t word;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET){
        // the most recent received data by the USART2 peripheral
        word = USART_ReceiveData(USART2);
        //받은 word에서 뒤 8비트만 추출
        word=word&0xFF;
        // TODO implement
        if (word != '\n' && word!='\r')
        //받은 word가 개행문자 등 특수문자가 아닐 경우 usart1_msg 버퍼에 받은 문자를 넣는다.
            usart2_msg[usart2_index++] = word;


        if (word=='\r'){
            //받은 word가 특수문자일 경우 널 문자를 넣고 end_usart()를 호출하여 받은 문자열을 판단한다.
             usart2_msg[usart2_index++] = '\0';
            end_usart2();
        }
           
        // clear 'Read data register not empty' flag
       USART_ClearITPendingBit(USART2,USART_IT_RXNE);
    }
}

void end_usart1() {
    //putty로 부터 받은 문자열을 검사한다. 문자열은 usart1_msg에 저장되어있다.
    if (help == 0) {
        //현재 사용자가 도움을 요청하지 않은 상태이면 명령어 입력 상태이다. 관리자는 clear를 입력하여 버퍼에 clear 문자열이 있다면 putty창을 클리어할 수 있다.
        //clear는 개행을 30번하여 수행한다.
        if (strcmp(usart1_msg, "clear") == 0) {
            for (int i=0; i<30; i++) {
                send_msg_to_putty("\n\r");
            }
        }
    }
    else {
        //현재 사용자가 도움을 요청하지 않은 상태이면 사용자와 메시지를 주고받아야하므로, 메시지 입력 상태이다. 받은 문자열(즉, usart1_msg 버퍼에 저장된 문자열)을 admin이라는 표시와 함께 그대로 블루투스로 보낸다.
        usart1_msg[usart1_index] = '\n';
        send_msg_to_bluetooth("Admin: ");
        send_msg_to_bluetooth(usart1_msg);
        send_msg_to_bluetooth("\n");
    }
    usart1_index = 0;//버퍼를 클리어한다.

}

void end_usart2() {
    //bluetooth로부터 받은 문자열을 처리한다. 문자열은 usart2_msg에 저장되어있다.
    if (help == 0) {    // 도움 아님
        //현재 사용자가 도움을 요청하지 않은 상태이면 명령어 입력 상태이다.
        if (strcmp(usart2_msg, "on") == 0) {
            //주차장 상태가 변할 때 주차장 정보를 받음  (기본값)
            communication = 0;
            send_msg_to_bluetooth("Receive update message\n");
        }
        else if (strcmp(usart2_msg, "off") == 0) {
            //주차장 상태가 변할 때 주차장 정보를 받지 않음
            communication = 1;  
            send_msg_to_bluetooth("Do not receive update message\n");
        }
        else if (strcmp(usart2_msg, "help") == 0) {
            //관리자에게 도움 요청, 관리자(PUTTY)와 1:1 채팅
            send_msg_to_bluetooth("Starting chat with the admin\n");
            send_msg_to_putty("Starting chat with a user\n\r");
            help = 1;
        }
        else if (strcmp(usart2_msg, "info") == 0) {
            //현재 주차장의 상태를 출력
            send_msg_to_bluetooth("Current state of parking lot\n");
            for (int i=0; i<3; i++) {
                if (parking_lot[i] == 0) {
                    send_msg_to_bluetooth("O ");
                }
                else {
                    send_msg_to_bluetooth("X ");
                }
            }
            send_msg_to_bluetooth("\n");
        }
        else if (strcmp(usart2_msg, "fee") == 0) {
            //사용자의 주차 요금을 출력
          if (user_lot == -1) { // 사용자가 주차 중이 아닐 경우
            send_msg_to_bluetooth("You are not parking\n");
          }
          else {                // 사용자가 주차 중인 경우
            clock_t cur = clock();                                                  // 현재 시간 측정
            int fee = ((int)(cur - start_time[user_lot])) / CLOCKS_PER_SEC;         // 주차 시작 시간을 통해 주차 중인 시간 계산 후 요금 출력
            sprintf(buffer, "%d", fee);
            send_msg_to_bluetooth("Current fee: ");
            send_msg_to_bluetooth(buffer);
            send_msg_to_bluetooth("\n");
          }
        }                       
        else if(strncmp(usart2_msg, "select", 6) == 0 && strlen(usart2_msg) > 7) {
            //주차 후 주차한 칸을 선택하는 기능
            //칸 번호 입력이 필요해 길이로 추가적인 조건 적용 ex) select 2 
            int lot = usart2_msg[7]-'0';    // 형 변환 
            if (user_lot != -1) {           // 사용자가 이미 다른 칸을 사용 중인 경우, 사용자는 한 개의 칸만 사용 가능
                send_msg_to_bluetooth("You have already selected another lot\n");
            }
            else {                          // 사용자가 한 칸도 사용하고 있지 않음, 선택 가능
                if (1 <= lot && lot <= 3) { // 칸 번호의 범위 무결성 확인
                    if (parking_lot[lot-1] == 2) {  // 선택하려는 주차칸이 점유 중이어야 함
                      user_lot = lot-1;             // 사용자의 칸으로 설정 후 완료 메시지 출력
                      send_msg_to_bluetooth("Selected parking lot: ");
                      USART_SendData(USART2, usart2_msg[7]);
                      for(int i=0; i<10000; i++) {}
                      send_msg_to_bluetooth("\n");
                    }
                    else {                          // 선택하려는 주차칸이 비어있음, 주차 후 선택이므로 비어있으면 안됨
                      send_msg_to_bluetooth("This lot is empty\n");
                    }
                }
                else {                      // 칸 번호가 범위를 벗어남
                    send_msg_to_bluetooth("Invalid parking lot number\n");
                }
            }
        }
        else if(strncmp(usart2_msg, "reserve", 7) == 0 && strlen(usart2_msg) > 8) {
            // 주차하기 전 비어있는 주차칸을 에약하는 기능
            int lot = usart2_msg[8]-'0';    // 형 변환 

            if (user_lot != -1) {           // 사용자가 이미 다른 칸을 사용 중인 경우, 사용자는 한 개의 칸만 사용 가능
                send_msg_to_bluetooth("You have already selected another lot\n");
            }
            else {                          // 사용자가 한 칸도 사용하고 있지 않음, 예약 가능
                if (1 <= lot && lot <= 3) { // 칸 번호의 범위 무결성 확인
                      if (parking_lot[lot-1] == 0) {    // 주차칸이 비어있다면 예약
                        parking_lot[lot-1] = -1;
                        user_lot = lot-1;
                        if (lot == 1) {     
                        // 녹색 -> 흰색
                          GPIO_SetBits(GPIOD,GPIO_Pin_10);  // 빨강 ON
                          GPIO_SetBits(GPIOD,GPIO_Pin_12);  // 파랑 ON
                          send_msg_to_bluetooth("Reserved parking lot: 1 \n");
                        }
                        else if (lot == 2) {
                          GPIO_SetBits(GPIOD,GPIO_Pin_13);  // 빨강 ON
                          GPIO_SetBits(GPIOD,GPIO_Pin_15);  // 파랑 ON
                          send_msg_to_bluetooth("Reserved parking lot: 2 \n");
                        }
                        else {
                          GPIO_SetBits(GPIOD,GPIO_Pin_3);   // 빨강 ON
                          GPIO_SetBits(GPIOD,GPIO_Pin_5);   // 파랑 ON
                          send_msg_to_bluetooth("Reserved parking lot: 3 \n");
                        }
                        print_parking_lot();
                    }
                    else {                          // 주차칸이 비어있지 않음
                      send_msg_to_bluetooth("This parking lot is not available\n");
                    }
                }
                else {                      // 칸 번호가 범위를 벗어남
                    send_msg_to_bluetooth("Invalid parking lot number\n");
                }
            }
        }
        else {  // 입력받은 명령어가 잘못된 형식
            send_msg_to_bluetooth("Invalid command\n");
        }
    }
    else {                                                  // 도움 관리자와 1:1 채팅   
        if (strcmp(usart2_msg, "exit") == 0) {              // 1:1 채팅 중 명령어 exit 입력 시 1:1 채팅 종료
            send_msg_to_bluetooth("Ending a chat with the admin\n");
            send_msg_to_putty("Ending a chat with a user\n\r");
            help = 0;
        }
        else {                                              // 그 외의 메시지는 PUTTY로 전송
            usart2_msg[usart2_index] = '\n';
            send_msg_to_putty("User: ");
            send_msg_to_putty(usart2_msg);
            send_msg_to_putty("\n\r");
        }
    }

    usart2_index = 0;                                       // Usart2 메시지 초기화
    usart2_msg[0] = '\0';
}


void print_parking_lot() {
    // 주차장의 상태를 출력, 변화가 발생했을 때 실행됨
    // 예약되어 있지 않고 비어있을 경우 O, 이외의 경우 X
    if (communication == 0) {   // 사용자가 수신 동의 상태라면 사용자에게 출력 
        // 블루투스로 출력
        send_msg_to_bluetooth("Changes occur in the parking lot\n");
        for (int i=0; i<3; i++) {
            //주차공간1, 주차공간2, 주차공간3의 현재 상태를 확인한다.
            if (parking_lot[i] == 0) {
                //현재 주차공간의 상태가 0, 즉 비어있으면 O를 보낸다.
                send_msg_to_bluetooth("O ");
            }
            else {
                //현재 주차공간의 상태가 비어있지 않으면(예약되어있거나 주차 중이거나 주차완료) X를 보낸다.
                send_msg_to_bluetooth("X ");
            }
        }
        send_msg_to_bluetooth("\n");//개행 문자를 출력한다.
        //example 주차공간1, 주차공간2가 비어있고 주차공간3에 차가 있으면 O O X를 출력한다
    }
    // PUTTY로 출력하여 관리자에게 주차장 정보 전송
    send_msg_to_putty("Changes occur in the parking lot\n\r");
    for (int i=0; i<3; i++) {
        if (parking_lot[i] == 0) {
            send_msg_to_putty("O ");
        }
        else {
            send_msg_to_putty("X ");
        }
    }
    send_msg_to_putty("\n\r");
}

// 인자의 문자열을 블루투스로 전송
void send_msg_to_bluetooth(char* buf){
    for (int i=0;; i++) {       
        if (buf[i] == '\0')             // 문자열의 끝이라면 무한 반복 종료
            break;
        USART_SendData(USART2, buf[i]); // 한글자씩 전송
        for(int k=0;k<50000;++k);       // 전송 후 조금 대기
       
    }
}

// 인자의 문자열을 PUTTY로 전송
void send_msg_to_putty(char* buf){
    for (int i=0;; i++) {
        if (buf[i] == '\0')             // 문자열의 끝이라면 무한 반복 종료
            break;
        USART_SendData(USART1, buf[i]); // 한글자씩 전송
        for(int k=0;k<50000;++k);       // 전송 후 조금 대기
    }
}

void DMA1_Channel1_IRQHandler() {
    //ADC 값들이 DMA를 통해 메모리에 다 저장이 되면 인터럽트가 발생한다.
    if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET){
        /*
        value[0] : 주차공간1의 조도센서값이다. 값이 클수록 어둡다.
        value[1] : 주차공간1의 거리센서값이다. 값이 클수록 가깝다.
        value[2] : 주차공간2의 조도센서값이다. 값이 클수록 어둡다.
        value[3] : 주차공간2의 거리센서값이다. 값이 클수록 가깝다.
        value[4] : 주차공간3의 조도센서값이다. 값이 클수록 어둡다.
        value[5] : 주차공간3의 거리센서값이다. 값이 클수록 가깝다.
        */

        // value[0]>2300: 조도, 조도센서가 어두워졌을 때
        // parking_lot[0] <= 0: 주차칸이 비어있을 때, 0 이하인 이유는 예약되었을 시 -1이기 때문
        //주차공간1이 비어있었거나 예약된 상태인 상태에서 조도센서1이 어두워지면 주차공간1의 주차가 시작된 것이다.
        if (value[0] > 2300 && parking_lot[0] <= 0) {   //주차1 시작 감지
            // 녹색(예약 X) or 흰색(예약 O) -> 빨강
            GPIO_SetBits(GPIOD,GPIO_Pin_10);    // 빨강 on
            GPIO_ResetBits(GPIOD,GPIO_Pin_11);  // 초록 off
            GPIO_ResetBits(GPIOD,GPIO_Pin_12);  // 파랑 off
            parking_lot[0] = 1;                 // 주차칸의 상태를 주차 진행 중으로 변경
           
            if (user_lot == 0)                  // 사용자가 예약한 칸이라면 추가적인 메시지 출력
                send_msg_to_bluetooth("Parking starts\n");
        }
        else if (parking_lot[0] == 1) {         // 주차1 진행 중
            // value[1] =
            if(value[1] < 3700 && value[1] > 2300){ // 차와 벽 사이의 거리가 일정 범위에 도달 후(즉 가까워지면) 일정 시간이 지나면 주차 완료로 판단
                stable_count[0] ++;
            }
            else{
                stable_count[0] = 0;                //만약 stable_count[0]이 25000이 되지 않았는데 벽과 차 사이의 거리가 일정이상 벌어진다면 stable_count[0]을 초기화한다.
            }
           
            if (stable_count[0] > 25000){           //거리가 가까워진 상태로 일정 시간 지나면 주차가 완료된 것으로 본다.
                start_time[0] = clock();            // 현재 시각을 측정해 저장 (주차 시작 시간 저장)
                print_parking_lot();                //주차공간에 변화가 생겼기 때문에 현재 주차공간 정보를 출력한다.
                stable_count[0] = 0;                // 초기화
                parking_lot[0] = 2;                 // 주차칸을 주차 완료로 변경
                // 빨강 -> 파랑
                GPIO_ResetBits(GPIOD,GPIO_Pin_10);  // 빨강 off
                GPIO_SetBits(GPIOD,GPIO_Pin_12);    // 파랑 on
                if (user_lot == 0)                  // 사용자가 예약한 칸이라면 사용자에게 주차가 완료되었음을 블루투스로 알린다.
                    send_msg_to_bluetooth("Parking completed\n");            
            }
        }
        else if (parking_lot[0] == 2 && value[0] < 2300) {  // 주차 종료, 주차 완료된 차량이 있던 칸의 조도 센서가 밝아짐
            // 파랑 -> 녹색
            GPIO_ResetBits(GPIOD,GPIO_Pin_12);      // 파랑 off
            GPIO_SetBits(GPIOD,GPIO_Pin_11);        // 초록 on
            parking_lot[0] = 0;                     // 주차칸을 비어있음으로 변경
            print_parking_lot();                    //주차장의 상태가 변했음으로 현재 주차장 상태를 출력한다.

            clock_t end = clock();                  // 현재 시각을 측정
            int fee = ((int)(end - start_time[0])) / CLOCKS_PER_SEC;    // 주차 시작 시간과 비교해 요금 계산 
            sprintf(buffer, "%d", fee);             // fee를 정수에서 문자열로 변경하여 buffer에 저장
            send_msg_to_putty("lot 1's fee: ");     // 나갈 때 요금 PUTTY로 출력하여 관리자에게 알림
            send_msg_to_putty(buffer);
            send_msg_to_putty("\n\r");
            if (user_lot == 0) {                    // 사용자가 예약하거나 선택한 칸이라면 사용자에게도 블루투스를 통해요금 출력
              send_msg_to_bluetooth("Your fee: ");
              send_msg_to_bluetooth(buffer);
              send_msg_to_bluetooth("\n");
              user_lot = -1;                        // 사용자의 칸 초기화(즉 사용자와 해당 주차공간의 연결을 해제)
            }
        }

        // 위 알고리즘과 같음
        // 핀의 이름이 달라 재작성
        // 주차공간2
        if (value[2] > 3500 && parking_lot[1] <= 0) {   //주차2 시작 감지
            // 녹색 -> 빨강
            GPIO_SetBits(GPIOD,GPIO_Pin_13);  
            GPIO_ResetBits(GPIOD,GPIO_Pin_14);
            GPIO_ResetBits(GPIOD,GPIO_Pin_15);
            parking_lot[1] = 1;
           
            if (user_lot == 1)
                send_msg_to_bluetooth("Parking starts\n");
        }
        else if (parking_lot[1] == 1) {
            if(value[3] < 3800 && value[3] > 2500){
                stable_count[1] ++;
            }
            else{
                stable_count[1] = 0;
            }
           
            if (stable_count[1] > 25000){
                start_time[1] = clock();
                print_parking_lot();
                stable_count[1] = 0;
                parking_lot[1] = 2;
                // 빨강 -> 파랑
                GPIO_ResetBits(GPIOD,GPIO_Pin_13);
                GPIO_SetBits(GPIOD,GPIO_Pin_15);      
                if (user_lot == 1)
                  send_msg_to_bluetooth("Parking completed\n");          
            }
        }
        else if (parking_lot[1] == 2 && value[2] < 3500) {
            // 파랑 -> 녹색
            GPIO_ResetBits(GPIOD,GPIO_Pin_15);
            GPIO_SetBits(GPIOD,GPIO_Pin_14);  
            parking_lot[1] = 0;
            print_parking_lot();

            clock_t end = clock();
            int fee = ((int)(end - start_time[1])) / CLOCKS_PER_SEC;
            sprintf(buffer, "%d", fee);
            // 나갈 때 요금
            send_msg_to_putty("lot 2's fee: ");
            send_msg_to_putty(buffer);
            send_msg_to_putty("\n\r");
            if (user_lot == 1) {
              send_msg_to_bluetooth("Your fee: ");
              send_msg_to_bluetooth(buffer);
              send_msg_to_bluetooth("\n");
              user_lot = -1;
            }
        }

        // 위 알고리즘과 같음
        // 핀의 이름이 달라 재작성
        // 주차공간3
        if (value[4] > 2600 && parking_lot[2] <= 0) {   //주차3 시작 감지
            // 녹색 -> 빨강
            GPIO_SetBits(GPIOD,GPIO_Pin_3);  
            GPIO_ResetBits(GPIOD,GPIO_Pin_4);
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            parking_lot[2] = 1;
           
            if (user_lot == 2)
                send_msg_to_bluetooth("Parking starts\n");
        }
        else if (parking_lot[2] == 1) {
            if(value[5] < 3800 && value[5] > 2700){
                stable_count[2] ++;
            }
            else{
                stable_count[2] = 0;
            }
           
            if (stable_count[2] > 25000){
                start_time[2] = clock();
                print_parking_lot();
                stable_count[2] = 0;
                parking_lot[2] = 2;
                // 빨강 -> 파랑
                GPIO_ResetBits(GPIOD,GPIO_Pin_3);
                GPIO_SetBits(GPIOD,GPIO_Pin_5);
                if (user_lot == 2)
                    send_msg_to_bluetooth("Parking completed\n");            
            }
        }
        else if (parking_lot[2] == 2 && value[4] < 2600) {
            // 파랑 -> 녹색
            GPIO_ResetBits(GPIOD,GPIO_Pin_5);
            GPIO_SetBits(GPIOD,GPIO_Pin_4);  
            parking_lot[2] = 0;
            print_parking_lot();
            clock_t end = clock();
            int fee = ((int)(end - start_time[2])) / CLOCKS_PER_SEC;
            sprintf(buffer, "%d", fee);
            // 나갈 때 요금
            send_msg_to_putty("lot 3's fee: ");
            send_msg_to_putty(buffer);
            send_msg_to_putty("\n\r");
            if (user_lot == 2) {
              send_msg_to_bluetooth("Your fee: ");
              send_msg_to_bluetooth(buffer);
              send_msg_to_bluetooth("\n");
              user_lot = -1;
            }
        }
        DMA_ClearITPendingBit(DMA1_IT_TC1);
    }
   
}


void DMA_Configure(void){
  // 구조체를 사용하여 DMA1 채널 1의 초기화 설정
  DMA_InitTypeDef DMA_Instructure;

   DMA_Instructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR; //메모리로 옮길 값을 ADC1->DR 즉 변환된 아날로그 값이 저장되는 레지스터로 지정한다.         
   DMA_Instructure.DMA_MemoryBaseAddr = (uint32_t)&value[0]; // ADC_Value배열 주소로 설정
   DMA_Instructure.DMA_DIR = DMA_DIR_PeripheralSRC;
   DMA_Instructure.DMA_BufferSize = 6;   // 전송 버퍼 크기 6 설정                              
   DMA_Instructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   // 주변 기기 주소의 증가 비활성화
   DMA_Instructure.DMA_MemoryInc = DMA_MemoryInc_Enable;              
   DMA_Instructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;  
   DMA_Instructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;      
   DMA_Instructure.DMA_Mode = DMA_Mode_Circular;   // 순환 모드 설정
   DMA_Instructure.DMA_Priority = DMA_Priority_High;   // 우선순위 높음 설정            
   DMA_Instructure.DMA_M2M = DMA_M2M_Disable;   // 메모리간 전송 비활성화      
   
   // DMA 초기화, DMA 전송 활성화
   DMA_Init(DMA1_Channel1, &DMA_Instructure);                              
   DMA_Cmd(DMA1_Channel1, ENABLE);  
   //DMA_IT_TC 인터럽트 활성화    
   DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE);      
}


void delay(int delay_value) {
    for (int i=0; i<delay_value; i++) {}
}