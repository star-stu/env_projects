#ifndef XBEEAPI_H
#define XBEEAPI_H

#include <Arduino.h>
#include <SoftwareSerial.h>

// A class to facilitate sending and receiving API frames between an Arduino and an XBee module.
// Jeff Groff, Starlight Studio, 2025

/*

API Frame Structure (AP mode 1 - no escape characters):

Start Delimiter: 0x7E
Length (2 bytes, MSB LSB): contains the number of bytes in the Frame Data Field
Frame Data Field: the data is our message
Checksum (1 byte): calculated as 0xFF - (8-bit sum of all bytes of the Frame Data Field)

Frame Data Field Structure: 

Frame Type: 0x2D
Frame ID: 0x00 (this value disables the return of a TX status frame)
Destination Interface: 0x02 (micropython)
Message: the data we want to relay to the XBee

*/

#define MESSAGE_BUFFER_SIZE 256
#define BUFFER_SIZE 512

#define START_DELIMITER 0x7E

#define N_RETRIES 10

typedef struct {
  uint8_t frame_type;
  uint8_t frame_id;
  uint8_t destination_interface;
  uint8_t source_interface;
  uint8_t message[MESSAGE_BUFFER_SIZE];
  uint16_t message_length;
} APIFrame;

class XBeeAPI {

  private:
    SoftwareSerial *xbee;

  public:
    uint8_t buffer[BUFFER_SIZE];
    uint16_t n_bytes;
    XBeeAPI(uint8_t RX, uint8_t TX);
    APIFrame frame;
    uint16_t serial_begin(uint32_t baud);
    uint16_t wait_for_api_frame();
    uint16_t parse_api_frame();
    uint16_t construct_api_frame(char *MESSAGE, uint16_t MESSAGE_LENGTH);
    uint16_t make_post_request();
    uint16_t check_for_api_frame();
    uint16_t read_api_frame();

};

#endif
