// useful functions for passing messages to and parsing messages from an xbee radio

typedef struct {
  uint8_t frame_type;
  uint8_t frame_id;
  uint8_t destination_interface;
  uint8_t source_interface;
  uint8_t message[MESSAGE_BUFFER_SIZE];
  uint16_t message_length;
} APIFrame;

uint16_t parse_api_frame(uint16_t n_bytes, uint8_t *buffer, APIFrame *frame) 
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
    frame->frame_type = buffer[0];
    frame->source_interface = buffer[1];
    frame->message_length = n_bytes - 3; // number of bytes excluding frame type byte, source interface byte, and checksum byte
    memcpy(frame->message, &buffer[2], frame->message_length);
    return 1;
  }
}


uint16_t wait_for_api_frame(SoftwareSerial &ser, uint8_t *buffer) 
{
  uint16_t n_bytes = 0;
  
  // wait for start_delimiter
  while(1)
  {
    if (ser.available() > 0)
    {
      if (ser.read() == START_DELIMITER)
      {
        delay(1); // at 9600 bps a 1 byte (10 bits with start and stop bits) takes about 1 ms to arrive. 
        break;
      }
    }
    // delay(10); // short wait
  }

  uint16_t expected_length = 0;
  uint8_t retries = 0;

  // wait for next two bytes (length bytes) to arrive
  while (ser.available() < 2 && retries < N_RETRIES)
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
    uint8_t MSB = ser.read();
    uint8_t LSB = ser.read();
    expected_length = ((uint16_t)(MSB) << 8 | (uint16_t)(LSB)) + 1; // add one for checksum byte
    Serial.println("expected length");
    Serial.println(expected_length);

    retries = 0;
    while (n_bytes < expected_length && retries < N_RETRIES)
    {
      if (ser.available() > 0)
      {
        buffer[n_bytes] = ser.read();
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
      return n_bytes;
    }
  }
}




void construct_api_frame(uint16_t *n_bytes, uint8_t *frame_bytes, uint16_t n_chars, char *message) {

  memset(frame_bytes, 0, FRAME_BUFFER_SIZE); // clear out byte buffer, probably not nessecary

  frame_bytes[0] = START_DELIMITER; // start delimiter
  frame_bytes[1] = (uint8_t)((n_chars + 3) >> 8); // MSB length byte
  frame_bytes[2] = (uint8_t)((n_chars + 3) & 0x00FF); // LSB length byte
  
  // begin field data
  frame_bytes[3] = 0x2D; // frame type
  frame_bytes[4] = 0x00; // frame ID, 0x00 suppresses TX status response frame
  frame_bytes[5] = 0x02; // destination interface, 0x02 is micripython
  memcpy(&frame_bytes[6], message, n_chars); // message

  // calculate checksum
  uint8_t cs = 0;
  cs += frame_bytes[3];
  cs += frame_bytes[4];
  cs += frame_bytes[5];
  for (int i = 0; i < n_chars; i++) {
    cs += message[i];
  }
  cs = 0xFF - cs;

  frame_bytes[3 + 3 + n_chars] = cs;
  *n_bytes = 3 + 3 + n_chars + 1;

  return;
  
}
