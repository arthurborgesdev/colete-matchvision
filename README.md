# colete-matchvision (Electronic jacket for trainning soccer players)


## What it is for?

To be used in soccer trainings with the purpose of improving players' visual acuity


## How it works?

The equipment consists in an ESP8266 connected to a Lithium battery and LED strips of model WS2812b. These strips have a feature that LEDs can be controlled individually. Then, it was possible to build 7 segment displays to cover the whole breastplate of the soccer player.

Accordingly to the doctor's research who requested the development of the prototype, when you change the pattern in the players' t-shirt, you force the players to not look to the ball (forcing them to look at heir colleagues t-shirts), then, improving their performance.

The device is programmed to alternate the numbers in the jacket from time to time. As part of the training exercise, the players should have to tell the number in their colleagues jackets when they received the ball pass and also when they kicked it. This forced them to look always to the front, and not to the ground.


## MAin files and its functionalities

**/espColorWebServerWs2812b-mod.ino**

This unique file is compiled and transfered to the ESP8266. It contains the import of libraries, like, for example, the one to control the LEDs and the one to control the Wifi of the board. It also contains the definition of variables and the webpage that runs in a webserver in the board, allowing to customize the patterns in the jacket through a web broweser.

It contains the function to store data in the EEPROM and set up the time. Then it contains the main functions: setup() and loop().

## Built With

- ESP8266
- WS2812b LED strips
- Arduino C/C++
- VSCode
- Ubuntu 20.04.3


## Setup

- Get the link of the repository: `git@github.com:arthurborgesdev/colete-matchvision.git`
- Clone it as `git@github.com:arthurborgesdev/colete-matchvision.git` on a Terminal

## Usage

- This project runs on physical devices that does not exist anymore. But the codebase is a reference for future Arduino embedded projects.


## Author

👤 **Arthur Borges**

- GitHub: [@arthuborgesdev](https://github.com/arthurborgesdev)
- Twitter: [@arthurmoises](https://twitter.com/arthurmoises)
- LinkedIn: [Arthur Borges](https://linkedin.com/in/arthurmoises)


## 🤝 Contributing

Contributions, issues, and feature requests are welcome!

## Show your support

Give a ⭐️ if you like this project!

## Acknowledgments

- RYD Engenharia and all the people related
- Lots and lots of Stack Overflow questions and answers