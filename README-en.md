# smartParkingLot-stm32
a smart parking lot project using stm32f107 board for term project of 'Embedded System Design and Lab' subject
# requirements
- stm32f107vc development board
- J-link emulator
- RS232 to USB2.0 serial cable(if your board have RS232 serial port)
  - USB to TTL serial cable required if RS232 serial port is missing on board
- Photoresistor x3
- Infrared distance sensor x3
- RGB LED x3
- bluetooth module(HC05, HC-06 or FB755AC)
- Android phone
# I bought parts from...(korean webpage)
- Photoresistor : [link](https://www.devicemart.co.kr/goods/view?no=1327438)
- Infrared distance sensor : [link](https://www.devicemart.co.kr/goods/view?no=11470)
- RGB LED : [link](https://www.devicemart.co.kr/goods/view?no=1287089)

# how to install and run
## install IAR Embedded Workbench 
IAR Embedded Workbench is required to port the program to the board. you can read 'how to install IAR Embedded Workbench' on [my blog(in korean)](https://minchocoin.github.io/stm32f107vc/1/)
## download STM32F10x standard peripheral library 
To run the main.c file, it requires the STM32F10x standard peripheral library provided by ST. you can download it [here](https://www.st.com/en/embedded-software/stsw-stm32054.html)
## IAR Embedded Workbench setting
you can read 'how to set' on[my blog(in korean)](https://minchocoin.github.io/stm32f107vc/2/)
## install J-link driver
If you connect J-link to your computer and the driver is not recognized, you must install the driver manually. you can read 'how to install driver' on [my blog](https://minchocoin.github.io/microc-os-3-stm32/5/)
## connecting a board and parts
|                 | parking1 | parking2 | parking3 |
|-----------------|----------|----------|----------|
| LED Red         | PD10     | PD13     | PD3      |
| LED Green       | PD11     | PD14     | PD4      |
| LED Blue        | PD12     | PD15     | PD5      |
| Photoresistor   | PB0      | PC0      | PC4      |
| distance sensor | PB1      | PC1      | PC5      |

(table1 : connecting a board and parts)

- USART1 : PA9(TX),PA10(RX) // if board has RS232 port, skip!
  - if your board doesn't have RS232 port, you should connect USB to TTL serial cable to these port(PA9 is connected to RX of USB-to-TTL, PA10 is connected to TX of USB-to-TTL)
- USART2 : PA2(TX)(Connected to bluetooth RX pin),PA3(RX)(Connected to bluetooth TX pin)
## porting
Port the stm32 library and the main.c file to the board using IAR Embedded Workbench.
## connecting the board and smartphone via bluetooth
install [Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=ko&gl=US) app on the android smartphone, and starting communication

# demo video(dubbing in korean)
you can watch the video by clicking the link below

[![이미지](https://img.youtube.com/vi/imnr4QyE7CQ/0.jpg)](https://www.youtube.com/watch?v=imnr4QyE7CQ)

# 동작 영상(원본)
you can watch the video by clicking the link below

[![이미지](https://img.youtube.com/vi/Hjx3WL4Kr9Q/0.jpg)](https://www.youtube.com/watch?v=Hjx3WL4Kr9Q)

# functions
## Parking start detection
- reading the Photoresistor sensor(installed on the floor) value and if the illuminance value becomes low(the floor becomes dark), it is determined that the car enters the parking space
- Changing the color of the RGB LED of the corresponding parking space to red and blink, and updating the parking space information (after updating, the corresponding parking space will not be available)
- The updated parking space information is transmitted to the user (if the information is set to be received) and the manager.
- Read the infrared sensor values to determine the distance between the car and the wall.
- After a certain period of time with the distance between the car and the wall short, it is determined that parking is completed, and the color of the RGB LED of the corresponding parking space is changed to blue.
- Record the parking time.

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/e0014386-3a29-49e6-89fc-55a3a12a95f0)

(그림1 : The LED is green if the parking space is available.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/14695d27-9caa-49dd-8916-9dd9e3769a02)

(그림2 : When a car is being parked in the parking space, the LED flashes red.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/f961729a-be74-4986-8889-70c790faa0a8)

(그림3 : When the car is parked in the parking space, the LED changes to blue.)

## 주차 종료
- If It detect the floor is brightened by reading the photoresistor sensor value of the corresponding parking space after parking is complete, change the RGB LED of the corresponding parking lot to green
- Change the current state of the corresponding parking space to Available
- The updated parking space information is transmitted to the user (if the information is set to be received) and the manager.
- (if the parking space is connected to the user) After transmitting the parking fee to the user, disconnect the user and the parking space.
- The parking fee for the corresponding parking space is transmitted to the manager.

## 주차 예약
- The user can reserve one parking space.
- Change the RGB LED of the corresponding parking space to white when the user make a reservation.
- When parking is performed in the reserved parking space, a parking start/parking completion message is transmitted to the user.
- If you reserve a parking space, the parking space and the user will be connected. If you reserve a parking space and park in that space, you can get the current fee through Bluetooth through the "fee" command, and you will get the final fee through Bluetooth when you leave the car.

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/cafc5031-0e12-45d4-a73d-506c422165c4)

(그림4 : The reserved parking space has a white RGB LED.)

![image](https://github.com/minchoCoin/smartParkingLot-stm32/assets/62372650/bab6aa46-58b5-45a3-af3c-1027a633897d)

(그림5 : Reserve parking space 2 through the 'reserve 2' command. the user can get message 'Parking start' when the user is starting to park in parking space 2, and get 'Parking completed'  when the user parked in parking space 2. Since the user and the parking space 2 are connected through this command, the current fee for parking lot 2 can be received through the 'fee' command. Finally, the user will receive the final fee when you leave the car.)


## 사용자가 사용할 수 있는 명령
- off : Even if the information on the parking space is updated, the information of the parking space is not received.
- on : If there's a change in the parking space, you get information.
- info : It outputs information about the current parking space. The form of information is O if the parking space is available, and X if it is unavailable (reservation, parking). For example, 'O O X' means that parking space 1 and 2 are available, and parking space 3 is unavailable.
- select n (n is one of 1,2,3) : Connect the parking space n with the user. If it is already connected, or if the parking space is empty, an error message is occured.
- reserve n (n is one of 1,2,3) : Reserve a nth parking space  (additionally, the user and the nth parking space are connected.) If there is a car in the parking space you want to reserve, an error message is occured
- fee : If there is a parking space connected to the user, the current fee of the parking space is outputted.
- help : You can send and receive messages with the administrator. If you enter this command, the administrator is notified that you will start communicating with the administrator. From this point on, the command entered by the administrator and the user are regarded as messages to be sent to the administrator and the user. administrator's message is marked with 'Admin : ', user's message is marked with 'User:'
  - exceptionally, when the user enters 'exit', communication with the administrator is terminated. When communication is terminated, the administrator and the user are informed that communication with each other is terminated.

## 관리자가 사용할 수 있는 명령
- clear : Clear the putty window that receives various information from the parking lot. However, it was not actually cleared, but the old contents were not visible by breaking 30 lines.

# members
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
