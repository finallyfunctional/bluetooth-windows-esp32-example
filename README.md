# Bluetooth Windows Esp32 Example

This repository is a sample on how to connect from a Windows 10 PC to an ESP32 via bluetooth and windows sockets. You should pair your PC with the ESP32 first. The console application will loop through the BT devices connected to the PC and find the ESP32, connect to it, send a message, and recieve a message. The Arduino.ino file should be loaded onto the ESP32. The Visual Studio project should be built (I used Visual Studio 2019). The actual source code is just contained in the .ino file and single .cpp file. The code should be simple to follow, unlike most of the examples and documentation I came across.


## References

Helpful examples on getting connected devices - https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancedotherprotocol4k.html
Bluetooth APIs - https://docs.microsoft.com/en-us/windows/win32/api/bluetoothapis/
Microsoft example - https://github.com/microsoftarchive/msdn-code-gallery-microsoft/blob/master/Official%20Windows%20Platform%20Sample/Bluetooth%20connection%20sample/%5BC%2B%2B%5D-Bluetooth%20connection%20sample/C%2B%2B/bthcxn.cpp
Bluetooth programming with Windows sockets - https://docs.microsoft.com/en-us/windows/win32/bluetooth/bluetooth-programming-with-windows-sockets