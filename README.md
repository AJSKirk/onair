# On Air Sign

During the COVID-19 pandemic I built an ESP8266 powered LED light in an old school radio recording booth "On Air" sign to hang outside my home office. The idea is to have the sign ping various calendar APIs to automatically indicate if I'm on a call, so others in the house know not to barge into the room. In the end I only built a Google Calendar interface, as all my personal commitments are synched to a single Google Calendar anyway, and privacy restrictions on my work calendar prevented me from being able to access it via API.

`esp8266/` code contains Arduino C code for running on the embedded devices, `python/` contains code for prototyping API creds and calls.

### Building Your Own

This is a pretty simple project, and a great one to replicate! Steps to do so are:

1. Buy Hardware, totalling $10-$20. You'll need
   - A NodeMCU ESP8266
     - You could also use other ESP8266 boards, but might need to modify some code
     - Nearly all NodeMCU boards you can buy are v0.9 or v1.0. Get the v1.0 if you can, driver setup is far easier. See [this page](https://cityos-air.readme.io/docs/1-mac-os-usb-drivers-for-nodemcu) for an easy guide to telling the difference.
   - Some short breadboard wires
   - A way to connect wires, best choice is either a breadboard or some 2.54mm pluggable terminal blocks
   - A light (I just bought a manual LED lightbox off Amazon)
     - The NodeMCU has a builtin LED if all you want to do is test the API interface, but it won't be a great signal
     - This README will assume you're using a low-power LED strip that can be powered by the 3.3V from the NodeMCU, but it isn't hard to extend it to switch a transistor/relay in order to control a higher power signal
   - A power source, either a regular USB output wall adapter or a 5-10V battery setup with connector
   - A "good" micro USB cable. I don't know what "good" really covers here, but I had to go through about four until one was finally able to connect
2. Install (free) software. You'll need
   - Arduino IDE
     - And the ESP8266 Board Manager extension, which can be installed from within Arduino IDE
   - Python 3.x
   - A NodeMCU and UART driver, see [this post](https://cityos-air.readme.io/docs/1-mac-os-usb-drivers-for-nodemcu) for info
   - [NodeMCU PyFlasher](https://github.com/marcelstoer/nodemcu-pyflasher/releases)
3. Flash the NodeMCU according to the instructions in the PyFlasher repo.
3. Register a Google Cloud Platform app with the `calendar.readonly` sensitive scope. You'll need to set it up with an API Key as well as OAuth 2.0 Credentials
   - Go ahead and "publish" your app. Google will give some popup warnings to tell you that you'll need to go through a lengthy verification process, but ignore them. Once your app is published you can actually connect up to 100 users before Google makes you click the "verify" button.
4. Modify the python script in this repository with your API keys and call with `python google_calendar.py <your_email@domain.com>` - This should open a webpage for you to sign in with Gmail and click through multiple "Warning: Unverified App" alerts, which will then save your personal OAuth token details to a token.pickle file.
5. Take the `REFRESH_TOKEN` value from that pickle file and add it, alongside your Google Cloud Platform app creds and WiFi details, to the top of the `light_node_mcu.ino` file and upload it to your NodeMCU
   - You *might* need to modify the timezone adjustment in the `configTime` call, the `gcal_req_body` function, or both. I think it's correct as it is, but one of the quirks of living in GMT is that it becomes impossible to test the difference between "correctly accounting for timezones" and "not at all accounting for timezones".
6. On the NodeMCU, wire the following connections
   - Pin D0 (GPIO16) to RST - This sends the 'wake' signal for the NodeMCU to return from deep sleep
   - Pin D1 to the positive terminal of your light
   - Any GND pin to the negative terminal of your light
   - Wire a battery source to Vin and GND, or just plug in the USB if you're powering from mains
7. Plug everything in, hit the RST switch, and you're done!
