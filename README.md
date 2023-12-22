# smartParkingLot-stm32
임베디드시스템설계및실험 텀프로젝트로, stm32f107vc 보드를 이용하여 스마트주차장을 만들었습니다.
# requirements
- stm32f107vc development board
- J-link emulator
- RS232 to USB2.0 serial cable(if your board have RS232 serial port)
  - 보드에 RS232 시리얼 포트가 없을 경우 USB to TTL 시리얼 케이블 필요
- Photoresistor 3개
- Infrared distance sensor 3개
- RGB LED 3개
- bluetooth module(HC05, HC-06 or FB755AC)
- Android phone
# 부품 구매한 곳
- Photoresistor : [link](https://www.devicemart.co.kr/goods/view?no=1327438)
- Infrared distance sensor : [link](https://www.devicemart.co.kr/goods/view?no=11470)
- RGB LED : [link](https://www.devicemart.co.kr/goods/view?no=1287089)

# 설치 및 구동 방법
## IAR Embedded Workbench 설치
프로그램을 보드에 포팅하기 위해 IAR Embedded Workbench가 필요합니다. [제 블로그](https://minchocoin.github.io/stm32f107vc/1/)에서 설치방법을 볼 수 있습니다.
## STM32F10x standard peripheral library 다운로드
main.c 파일에는 ST에서 제공하는 STM32F10x standard peripheral library가 필요합니다. [여기](https://www.st.com/en/embedded-software/stsw-stm32054.html)서 다운받을 수 있습니다.
## IAR Embedded Workbench 설정
[제 블로그](https://minchocoin.github.io/stm32f107vc/2/) 에서 설정 방법을 볼 수 있습니다.
## J-link driver 설치
J-link를 컴퓨터와 연결하였는데 드라이버가 인식이 되지 않으면 드라이버를 수동 설치해야합니다. [제 블로그](https://minchocoin.github.io/microc-os-3-stm32/5/) 에서 설치 방법을 볼 수 있습니다.
## 부품과 보드 연결
|                 | parking1 | parking2 | parking3 |
|-----------------|----------|----------|----------|
| LED Red         | PD10     | PD13     | PD3      |
| LED Green       | PD11     | PD14     | PD4      |
| LED Blue        | PD12     | PD15     | PD5      |
| Photoresistor   | PB0      | PC0      | PC4      |
| distance sensor | PB1      | PC1      | PC5      |

(표1 : 부품과 보드 연결)

- USART1 : PA9(TX),PA10(RX) // if board has RS232 port, skip!
  - if your board doesn't have RS232 port, you should connect USB to TTL serial cable to these port(PA9 is connected to RX of USB-to-TTL, PA10 is connected to TX of USB-to-TTL)
- USART2 : PA2(TX)(Connected to bluetooth RX pin),PA3(RX)(Connected to bluetooth TX pin)
## 포팅
다운받은 stm32 library와 이 repository에 업로드되어 있는 main.c 파일을 IAR Embedded Workbench를 이용하여 보드에 포팅합니다.
## 블루투스와 스마트폰 연결
[Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=ko&gl=US) 앱을 스마트폰에 설치하고 블루투스 모듈과 연결해 통신을 시작합니다.

# 동작 영상(더빙)
사진을 클릭하면 동영상 링크가 클릭됩니다.

[![이미지](https://img.youtube.com/vi/imnr4QyE7CQ/0.jpg)](https://www.youtube.com/watch?v=imnr4QyE7CQ)

# 동작 영상(원본)
사진을 클릭하면 동영상 링크가 클릭됩니다.

[![이미지](https://img.youtube.com/vi/Hjx3WL4Kr9Q/0.jpg)](https://www.youtube.com/watch?v=Hjx3WL4Kr9Q)

# 기능
## 주차 시작 감지
- 바닥에 설치된 조도 센서 값을 읽어 조도 값이 어두워질 경우 차가 해당 주차칸에 들어오는 것으로 판단
- 해당 주차칸의 RGB LED의 색을 빨간색으로 변경 후 깜빡인다. 그리고 주차장 정보를 갱신한다(해당 주차칸 사용 불가)
- 갱신된 주차장 정보를 사용자(정보 받기를 설정한 경우)와 관리자에게 전송한다.
- 적외선 센서 값을 읽어 차와 벽 사이의 거리를 파악한다.
- 차와 벽 사이의 거리가 가까워진 상태로 일정시간 지나면 주차가 완료되었다고 판단, 해당 주차칸의 RGB LED의 색을 파란색으로 변경한다,
- 주차 시간을 기록한다.

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/e0014386-3a29-49e6-89fc-55a3a12a95f0)

(그림1 : 주차칸이 사용가능하면 LED는 초록색이다.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/14695d27-9caa-49dd-8916-9dd9e3769a02)

(그림2 : 주차칸에 차가 주차 중이면 LED는 적색으로 깜빡인다.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/f961729a-be74-4986-8889-70c790faa0a8)

(그림3 : 주차칸에 차가 주차 완료되면, LED는 파란색으로 변경된다.)

## 주차 종료
- 주차 완료 상태에서 해당 주차칸의 조도센서 값을 읽어 바닥이 밝아질 경우 해당 주차칸의 RGB LED를 녹색으로 변경
- 해당 주차칸의 현재 상태를 사용 가능으로 변경
- 갱신된 주차장 정보를 사용자(정보 받기를 설정한 경우)와 관리자에게 전송한다.
- (해당 주차칸과 사용자가 연결된 경우, 후술) 주차요금을 사용자에게 전송한 후, 사용자와 해당 주차칸의 연결을 해제한다.
- 해당 주차칸의 주차 요금을 관리자에게 전송한다.

## 주차 예약
- 사용자는 하나의 주차칸을 예약할 수 있다.
- 예약 시 해당 주차칸의 RGB LED를 흰색으로 변경한다.
- 예약된 주차칸에 주차가 이루어지면 해당 사용자에게 주차 시작/ 주차 완료 메시지를 전송한다.
- 주차칸을 예약할 경우 해당 주차칸과 사용자가 연결된다. 주차칸을 예약 후 해당 주차칸에 주차하면 'fee' 명령을 통해 현재 요금을 블루투스로 받을 수 있고, 출차 시 최종 요금을 블루투스를 통해 받게 된다.

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/cafc5031-0e12-45d4-a73d-506c422165c4)

(그림4 : 예약된 주차칸은 RGB LED가 흰색이다.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/bab6aa46-58b5-45a3-af3c-1027a633897d)

(그림5 : 'reserve 2' 명령을 통해 2번 주차칸을 예약한다. 2번 주차칸에 주차가 시작되면 'Parking start', 주차가 완료되면 'Parking completed' 메시지를 받는다. 이 명령을 통해 사용자와 2번 주차칸이 연결되어 있으므로, 'fee' 명령을 통해 2번 주차칸의 현재 요금을 받을 수 있다. 마지막으로 출차 시 최종 요금을 받게 된다.)


## 사용자가 사용할 수 있는 명령
- off : 주차자리에 대한 정보가 갱신되더라도 주차장에 대한 정보를 받지 않는다.
- on : 주차자리에 변화가 생기면 정보를 받는다.
- info : 현재 주차장의 정보를 출력한다. 정보의 형태는 주차칸이 사용가능이면 O, 사용불가능(예약, 주차 중)일 경우 X이다. 예를 들어 'O O X'는 1번과 2번 주차장은 사용가능한 상태이고, 3번 주차장은 사용 불가능을 의미한다.
- select n (n is one of 1,2,3) : n번 자리의 주차칸을 사용자와 연결한다. 이미 연결되어있거나, 해당 주차칸이 비어있는 경우 오류 메시지를 출력한다.
- reserve n (n is one of 1,2,3) : n번 자리의 주차칸을 예약한다(추가적으로 사용자와 n번 자리의 주차칸이 연결된다.) 예약하려는 주차칸에 차가 있는 경우 오류 메시지를 출력한다.
- fee : 해당 사용자와 연결된 주차칸이 있는 경우, 해당 주차칸의 현재 요금을 출력한다.
- help : 관리자와 메세지를 주고받을 수 있다. 이 명령을 입력하면 관리자에게 사용자와 통신을 시작한다고 알린다. 이때부터 관리자와 사용자가 입력하는 내용은 각각 관리자와 사용자에게 보낼 메시지로 간주하고, 입력받은 내용을 관리자(Admin : ), 사용자(User: )라는 표시와 함께 보낸다.
  - 단 예외적으로, 사용자가 'exit'를 입력하면 관리자와의 통신을 종료한다. 통신을 종료하면, 관리자와 사용자에게 서로 통신이 종료됨을 알린다.

## 관리자가 사용할 수 있는 명령
- clear : 주차장의 각종 정보를 받는 putty창을 클리어한다. 단, 실제로 클리어하는 것은 아니고, 개행을 30번 수행하여 오래된 내용을 보이지 않게 하였다.

# 팀원
- 김태훈
- 김찬호
- 이정민

# Reference
- [STM32F107 Datasheet](https://www.st.com/resource/en/datasheet/stm32f107vc.pdf)
- [STM32F107 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- STM32F107VCT6 schematic
- [FB755AC 사용자가이드](https://drive.google.com/file/d/1Br7Evx_k58qSpBgM7wPoJlqjDk6qLnjb/view)
- [FB755AC 환경설정 세부설명](https://drive.google.com/file/d/17wZyuzfTGCBFkGr4TZgauAfPMvzuwS7k/view)
- [FB755AC AT 명령어 세부설명 및 사용방법](https://drive.google.com/file/d/1hHo5eZwaaROHi40HOtSzdTMZp-Yqioum/view)
- [https://community.st.com/t5/stm32-mcus-products/dma-configuration-for-multiple-adc-channels-stm32f4/td-p/435988](https://community.st.com/t5/stm32-mcus-products/dma-configuration-for-multiple-adc-channels-stm32f4/td-p/435988)
# License
[Apache License v2.0](https://www.apache.org/licenses/LICENSE-2.0)
