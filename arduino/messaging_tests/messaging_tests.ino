
// User Data Relay and XBee API Frame Parsing Example

#include<ArduinoJson.h>
#include"XBeeAPI.h"

#define JSON_BUFFER_SIZE 256


// helper function to print byte arrays to serial console
void print_byte_array(uint16_t n_bytes, uint8_t *byte_array){
  for (int i = 0; i < n_bytes; i++) {
    Serial.print(byte_array[i], HEX); Serial.print(" ");
  }
  Serial.println();
  return;
}


char json_buffer[JSON_BUFFER_SIZE]; // define a character buffer to use for json construction

JsonDocument data;
XBeeAPI xAPI(2,3);

void setup() {

  Serial.begin(9600); // start coms with computer
  xAPI.serial_begin(9600); // start coms with XBee radio
  while (!Serial) {
    // wait;
    delay(100);
  }

}

void loop() {


  data["temp"] = random(30);
  data["pH"] = random(14);
  data["DO"] = random(10);
  data["ORP"] = random(60);
  data["cond"] = random(100);

  memset(json_buffer, '\0', JSON_BUFFER_SIZE); // clear buffer
  uint16_t json_length  = serializeJson(data, json_buffer, JSON_BUFFER_SIZE);
 
  Serial.println("JSONified Data...");
  Serial.println(json_buffer);
  Serial.println("Making API Frame...");
  xAPI.construct_api_frame(json_buffer, json_length);
  print_byte_array(xAPI.n_bytes, xAPI.buffer);
  Serial.println("Sending Data to XBee...");
  xAPI.make_post_request();
  Serial.println("Waiting for Reply...");
  xAPI.wait_for_api_frame();
  Serial.println("Parsing Response(s)...");
  xAPI.parse_api_frame();
  print_byte_array(xAPI.frame.message_length, xAPI.frame.message);
  Serial.println("Checking for Additonal Responses...");
  while (1) 
  {
    if (xAPI.check_for_api_frame() == 1) {
      xAPI.read_api_frame();
      xAPI.parse_api_frame();
      print_byte_array(xAPI.frame.message_length, xAPI.frame.message);
    }
    else {
      Serial.println("None Found...");
      break;
    }
  }

  delay(60000); // wait 1 minute

}
