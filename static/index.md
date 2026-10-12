# About

This project lets you use an ESP32 device with a CAN interface to interact with the Zehnder ComfoAir Q ventilation unit.

# Installation

Choose your board to install its pre-built firmware via USB from the browser.
AtomS3 Lite and Waveshare both use an ESP32-S3 but require different CAN pins and flash settings.

<esp-web-install-button manifest="./zehnder-comfoair-q-m5stack-atom/install.json">
  <button slot="activate">Install M5Stack Atom Lite</button>
</esp-web-install-button>

<esp-web-install-button manifest="./zehnder-comfoair-q-m5stack-atoms3/install.json">
  <button slot="activate">Install M5Stack AtomS3 Lite</button>
</esp-web-install-button>

<esp-web-install-button manifest="./zehnder-comfoair-q-m5stack-nanoc6/install.json">
  <button slot="activate">Install M5Stack NanoC6</button>
</esp-web-install-button>

<esp-web-install-button manifest="./zehnder-comfoair-q-waveshare-esp32-s3-rs485-can/install.json">
  <button slot="activate">Install Waveshare ESP32-S3-RS485-CAN</button>
</esp-web-install-button>

<script type="module" src="https://unpkg.com/esp-web-tools@9/dist/web/install-button.js?module"></script>
