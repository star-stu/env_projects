#ifndef XBEEAPI_H
#define XBEEAPI_H

#include <Arduino.h>
#include <SoftwareSerial.h>


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

    uint8_t buffer[BUFFER_SIZE];


  public:

    uint16_t parse_api_frame(uint16_t n_bytes, uint8_t *buffer, APIFrame *frame);
    uint16_t wait_for_api_frame(SoftwareSerial &ser, uint8_t *buffer);
    void construct_api_frame(uint16_t *n_bytes, uint8_t *frame_bytes, uint16_t n_chars, char *message);


};



#endif
