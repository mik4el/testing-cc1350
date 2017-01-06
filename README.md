# Testing cc1350
Testing communication between two cc1350 based on the original TI examples of dual mode Node and Concentrator. This is a slightly different setup than the original to test sending more data, e.g. internal temperature, with the long range mode. 

# How to setup
1. Clone repo
1. Open CCS and set workspace to the repo directory
1. Build
1. Debug!

# Todos
* Concentrator posts Node data on BLE viewable via TI Sensor tag app
* Concentrator posts Node data on BLE viewable via standard bluetooth on raspi 
* Node reads data from LMT70
* Encrypted communication between Node and Concentrator
