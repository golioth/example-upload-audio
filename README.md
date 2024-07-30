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

## Building the Application and Assign Credentials

### Build for m5stack Core2

```
idf.py set-target esp32
idf.py build
idf.py flash monitor
```

### Build for m5stack CoreS3

```
idf.py set-target esp32s3
idf.py build
idf.py flash monitor
```

### Provisioning

```
esp32> settings set wifi/ssid <my-wifi-ssid>
esp32> settings set wifi/psk <my-wifi-psk>
esp32> settings set golioth/psk-id <my-psk-id@my-project>
esp32> settings set golioth/psk <my-psk>
esp32> kernel reboot cold
```

## Data Route Setup

- Create an Amazon S3 bucket and generate a credential that allows
  upload to it.
- Use add the contents of the YAML file in the pipelines directory of
  this repository to your Golioth project.
- Use your S3 credential to set up the follow secrets in your Golioth
  project:
  - AWS_S3_ACCESS_KEY
  - AWS_S3_ACCESS_SECRET
