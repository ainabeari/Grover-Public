# Grover: In-Situ Wildfire Detection
This repository contains the code used for our final year engineering capstone project. The goal of this project was to develop an in-situ wildfire detection device that utilizes the grounds naturally insulative properties to protect our sensors during fire and harsh Canadian winters. 

Authors:
- Arielle Ainabe [![Github Badge](https://img.shields.io/badge/GitHub-purple)](https://github.com/ainabeari)
- Mariel Serra [![Github Badge](https://img.shields.io/badge/GitHub-purple)](https://github.com/ainabeari)
- Nicole Rodriguez [![Github Badge](https://img.shields.io/badge/GitHub-purple)](https://github.com/ainabeari)

Libraries to install for BME688 Code:
- sdFat by Bill Greiman
- Bosch-BSEC2-LIbrary --> from Github, must be manually downloaded (https://github.com/boschsensortec/Bosch-BSEC2-Library/tags) tag v1.6.2400 was used
- BME68x Sensor Library
- ArduinoJson by Benoit Blanchon

Gas Sensor ESP32 Config:
- Huge partition scheme
- Adafruit ESP32 Feather

Electronics Diagram:
![image](https://github.com/user-attachments/assets/bab0c891-3b36-4843-b4e3-e7ec5828603e)

Communications Diagram (3-way BLE via ESP-NOW and Wired for the Arduino):
![image](https://github.com/user-attachments/assets/832eb301-5b20-4e04-890a-d80669169e72)

