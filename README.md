# DomoNode21 for ESPHome

Base files to load ESPHome on DomoNode 2.1

Tested in ESPHome module 1.12.0 for HomeAssistant.

Please note that it's the first time I write a module for ESPHome, so there
probably are better ways to do the same task. Patches (or integration in
ESPHome core!) are really welcome!

## Installation

Add a folder "DomoNode" to the folder where your devices yaml files reside (in
my case I created /usr/share/hassio/homeassistant/esphome/DomoNode/ ) and copy
there all files under DomoNode/ . Then use a device template like this for your
device:

```yaml
substitutions:
  devicename: mydevice
  upper_devicename: MyDevice

<<: !include DomoNode/DomoNode21.yaml

ota:
  password: "123456789abcdef0123456789abcdef0"

wifi:
  ssid: !secret wifi_main_ssid
  password: !secret wifi_main_pass
  fast_connect: true

web_server:
  port: 80
```

If you're using ESPHome module for HomeAssistant, it's better to keep the random
password it generated.

Now you can compile and install as usual. The first time you'll have to load it
via serial, unless you already loaded another OTA-enabled bin file.
