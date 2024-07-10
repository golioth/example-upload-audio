# Golioth Audio Upload Example

## Overview

This application records audio and uploads it to and Amazon S3 bucket.
The Golioth stream service is used for the upload, with the Golioth
Pipeline service used to route the data to S3.

## Local Setup

```
git submodule add https://github.com/golioth/golioth-firmware-sdk.git submodules/golioth-firmware-sdk
cd example-upload-audio
git submodule update --init --recursive
```

## Note: Use Golioth Dev Server

This app is currently configured to connect to the Golioth dev server to
utilize a higher upload size limit.

## Building the Application and Assign Credentials

```
idf.py build
idf.py flash monitor
```

```
esp32> settings set wifi/ssid <my-wifi-ssid>
esp32> settings set wifi/psk <my-wifi-psk>
esp32> settings set golioth/psk-id <my-psk-id@my-project>
esp32> settings set golioth/psk <my-psk>
esp32> kernel reboot cold
```
