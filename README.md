# smartblinds
Smart blinds project for CPSC 498, Spring 2020

Set up using NodeMCU, Raspberry Pi 3B, HassIO, CloudMQTT.

# Helpful Info

Upload the blinds.ino file to the NodeMCU. You may have to install a couple plugins to the Arduino IDE to get the code compiled.

Copy/paste the text of my automations.yaml and configuration.yaml files into your own. This currently sets up three (3) automation events and contains all of the code necessary to set up the integration for the MQTT server, just make sure you are inputting your own information where applicable.

If setting up with HassIO and Raspberry Pi, make sure to update the DNS settings. Log in directly to the Raspberry Pi (yes, with a monitor and keyboard, no SSH) with "root" as the user, and type `ha dns update` when logged in. Restart the HomeAssistant server and ensure that weather data is being properly read from the default weather UI.

Use the default user/pass given to you by CloudMQTT in all configuration files. 

Set up USER: homeassistant PASS: homeassistant with ACL # - readwrite in the "Users & ACL" tab of CloudMQTT. Do not use this login information in any config files.

Enable the Developer Tools tab by clicking on your user profile in the bottom left, and toggling "Advanced Options".

Integrate the MQTT server manually by going to Configuration -> Integrations, adding a new integration, and searching for "MQTT". Input your default user/pass from CloudMQTT, the server address (broker), and the default port. Enable discovery, and your device should show up under Developer Tools -> States.
