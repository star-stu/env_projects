#include "XBeeAPI.h"


XBeeAPI::XBeeAPI(uint8_t RX, uint8_t TX)
{

  xbee = new SoftwareSerial(RX, TX);
  
}

uint16_t XBeeAPI::serial_begin(uint32_t baud) {
  xbee->begin(baud);
  return 1;
}


uint16_t XBeeAPI::read_api_frame()
{
  n_bytes = 0;
  
  uint16_t expected_length = 0;
  uint8_t retries = 0;

  // wait for next two bytes (length bytes) to arrive
  while (xbee->available() < 2 && retries < N_RETRIES)
  {
    retries++;
    delay(1);
  }

  if (retries == N_RETRIES)
  {
    return 0; // ERROR
  }
  else 
  {
    uint8_t MSB = xbee->read();
    uint8_t LSB = xbee->read();
    expected_length = ((uint16_t)(MSB) << 8 | (uint16_t)(LSB)) + 1; // add one for checksum byte

    retries = 0;
    while (n_bytes < expected_length && retries < N_RETRIES)
    {
      if (xbee->available() > 0)
      {
        buffer[n_bytes] = xbee->read();
        n_bytes++;
        delay(1);
      }
      else 
      {
        retries++;
        delay(1);
      }
    }
    if (retries == N_RETRIES)
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }
}


uint16_t XBeeAPI::wait_for_api_frame() 
{
  
  // wait for start_delimiter
  while(1)
  {
    if (xbee->available() > 0)
    {
      if (xbee->read() == START_DELIMITER)
      {
        delay(1); 
        break;
      }
    }
  }
  delay(1); // at 9600 bps a 1 byte (10 bits with start and stop bits) takes about 1 ms to arrive. 
  return read_api_frame();
}

uint16_t XBeeAPI::check_for_api_frame() 
{
  // check for start_delimeter
  while (xbee->available() > 0)
  {
    if (xbee->read() == START_DELIMITER)
    {
      return 1;
    }
    delay(1); 
  }
  return 0;

}

uint16_t XBeeAPI::parse_api_frame() 
{
  // check length
  if (n_bytes < 4) // must have at lease frame type byte, source interface byte, one data byte, and checksum byte. 
  {
    return 0;
  }
  // validate checksum
  uint8_t sum = 0;
  for (int i = 0; i < n_bytes; i++) 
  {
    sum += buffer[i];
  }
  if (sum != 0xFF) {
    return 0;
  }
  else {
    frame.frame_type = buffer[0];
    frame.source_interface = buffer[1];
    frame.message_length = n_bytes - 3; // number of bytes excluding frame type byte, source interface byte, and checksum byte
    memcpy(frame.message, &buffer[2], frame.message_length);
    return 1;
  }
}

uint16_t XBeeAPI::construct_api_frame(char *MESSAGE, uint16_t MESSAGE_LENGTH) {

  // add message and construct API frame. 
  memcpy(&frame.message[0], MESSAGE, MESSAGE_LENGTH);
  frame.message_length = MESSAGE_LENGTH;

  memset(buffer, 0, BUFFER_SIZE); // clear out byte buffer, probably not nessecary

  buffer[0] = START_DELIMITER; // start delimiter
  buffer[1] = (uint8_t)((frame.message_length + 3) >> 8); // MSB length byte
  buffer[2] = (uint8_t)((frame.message_length + 3) & 0x00FF); // LSB length byte
  
  // begin field data
  buffer[3] = 0x2D; // frame type
  buffer[4] = 0x00; // frame ID, 0x00 suppresses TX status response frame
  buffer[5] = 0x02; // destination interface, 0x02 is micripython
  memcpy(&buffer[6], frame.message, frame.message_length); // message

  // calculate checksum
  uint8_t cs = 0;
  cs += buffer[3];
  cs += buffer[4];
  cs += buffer[5];
  for (int i = 0; i < frame.message_length; i++) {
    cs += frame.message[i];
  }
  cs = 0xFF - cs;

  buffer[3 + 3 + frame.message_length] = cs;
  n_bytes = 3 + 3 + frame.message_length + 1;

  return 1;
  
}


uint16_t XBeeAPI::make_post_request() 
{
  xbee->write(buffer, n_bytes);
  return 1;

}
