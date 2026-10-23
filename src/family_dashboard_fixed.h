{
  "version": 1,
  "editor": "wokwi",
  "parts": [
    { "type": "wokwi-breadboard-half", "id": "bb1", "top": 35.4, "left": -755.6, "attrs": {} },
    { "type": "board-esp32-s2-devkitm-1", "id": "esp1", "top": 60, "left": 33.54, "attrs": {} },
    {
      "type": "board-ssd1306",
      "id": "oled1",
      "top": 336.25,
      "left": 10,
      "attrs": { "i2cAddress": "0x3c" }
    },
    { "type": "wokwi-dht22", "id": "dht1", "top": -124.5, "left": -15, "attrs": {} },
    { "type": "wokwi-dht22", "id": "dht2", "top": -124.5, "left": 129, "attrs": {} },
    {
      "type": "wokwi-pushbutton",
      "id": "btn1",
      "top": 447.8,
      "left": -86.4,
      "attrs": { "color": "green", "xray": "1" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn2",
      "top": 447.8,
      "left": 124.8,
      "attrs": { "color": "green", "xray": "1" }
    },
    {
      "type": "wokwi-pushbutton",
      "id": "btn3",
      "top": 438.2,
      "left": 19.2,
      "attrs": { "color": "green", "xray": "1" }
    },
    {
      "type": "wokwi-text",
      "id": "text1",
      "top": 499.2,
      "left": -67.2,
      "attrs": { "text": "Up" }
    },
    {
      "type": "wokwi-text",
      "id": "text2",
      "top": 499.2,
      "left": 144,
      "attrs": { "text": "Down" }
    },
    {
      "type": "wokwi-text",
      "id": "text3",
      "top": 489.6,
      "left": 28.8,
      "attrs": { "text": "Enter\n" }
    },
    { "type": "wokwi-potentiometer", "id": "pot1", "top": 27.5, "left": -115.4, "attrs": {} },
    { "type": "wokwi-vcc", "id": "vcc1", "top": 192.76, "left": -806.4, "attrs": {} },
    {
      "type": "wokwi-buzzer",
      "id": "bz1",
      "top": -132,
      "left": -132.6,
      "attrs": { "volume": "0.1" }
    },
    { "type": "wokwi-gas-sensor", "id": "gas1", "top": 31.5, "left": 227.8, "attrs": {} }
  ],
  "connections": [
    [ "oled1:VCC", "esp1:5V", "red", [ "v-38.4", "h-47.85", "v-38.4" ] ],
    [ "oled1:GND", "esp1:GND.1", "black", [ "v-38.4", "h-38.4", "v-28.8" ] ],
    [ "oled1:SDA", "esp1:21", "green", [ "v-28.8", "h76.87", "v-57.6" ] ],
    [ "oled1:SCL", "esp1:26", "green", [ "v-48", "h86.7", "v-57.6" ] ],
    [ "esp1:5V", "bb1:bp.25", "red", [ "h0" ] ],
    [ "esp1:GND.1", "bb1:bn.25", "black", [ "h0" ] ],
    [ "dht1:VCC", "bb1:bp.24", "red", [ "v19.2", "h-212.8" ] ],
    [ "dht1:GND", "bb1:bn.24", "black", [ "v19.2", "h-241.6" ] ],
    [ "dht1:SDA", "esp1:17", "green", [ "v0" ] ],
    [ "dht2:VCC", "bb1:bp.23", "red", [ "v19.2", "h-326.4", "v220.8" ] ],
    [ "dht2:GND", "bb1:bn.23", "black", [ "v19.2", "h-345.6" ] ],
    [ "dht2:SDA", "esp1:16", "green", [ "v19.2", "h-105.5", "v230.4" ] ],
    [ "btn1:2.l", "bb1:bn.22", "green", [ "h-124.8", "v-211", "h-76.8" ] ],
    [ "btn1:1.l", "esp1:15", "green", [ "h9.6", "v-220.8" ] ],
    [ "btn2:1.l", "esp1:14", "green", [ "h-19.2", "v-28.8", "h-134.4", "v-192" ] ],
    [ "btn3:2.l", "btn1:2.r", "green", [ "h0" ] ],
    [ "btn2:2.l", "btn3:2.r", "green", [ "h0" ] ],
    [ "btn3:1.l", "esp1:13", "green", [ "h-67.2", "v-153.6" ] ],
    [ "pot1:VCC", "bb1:bp.22", "red", [ "v0" ] ],
    [ "pot1:GND", "bb1:bn.21", "black", [ "v0" ] ],
    [ "vcc1:VCC", "bb1:bp.1", "red", [ "v0" ] ],
    [ "pot1:SIG", "esp1:10", "green", [ "v28.8", "h86", "v64.31" ] ],
    [ "bz1:1", "bb1:bn.21", "green", [ "v57.6", "h-136" ] ],
    [ "bz1:2", "esp1:33", "green", [ "v57.6", "h278", "v217.91" ] ],
    [ "gas1:GND", "bb1:bn.17", "black", [ "h28.8", "v229.6", "h-683.2" ] ],
    [ "gas1:VCC", "bb1:bp.17", "red", [ "h28.8", "v219.9", "h-683.2" ] ],
    [ "gas1:AOUT", "esp1:18", "green", [ "h28.8", "v230.4" ] ],
    [ "gas1:DOUT", "esp1:34", "green", [ "h28.8", "v124.5" ] ]
  ],
  "dependencies": {}
}