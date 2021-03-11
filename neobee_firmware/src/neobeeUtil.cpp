#include "neobeeUtil.h"

#include <Arduino.h>


void writeInt32(int32_t value, uint8_t* dst) {
  boolean NEGFLAG(value < 0);

  // If value is negative, write the
  // positive value and set the negative
  // flag
  if (NEGFLAG) {
    value *= -1;
  } 
  dst[0] = (value >> 24) & 0xff;
  dst[1] = (value >> 16) & 0xff;
  dst[2] = (value >> 8) & 0xff;
  dst[3] = value & 0xff;
  // If the original value was negative, 
  // set the highest bit to one to 
  // indicate a negative value.
  if (NEGFLAG) {
    dst[0] |= 0b10000000;
  }
}

String stringFromByteAray(const uint8_t* src, const uint8_t size) {
  String dest = String();
  for (uint8_t i=0; i< size; i++) {
    if (src[i] == 0) break;
    dest += char(src[i]);
  };
  return dest;
}

String toHex8String(uint8_t *data, uint8_t size)
{
    // Create a buffer for storing the hex characters plus one 
    char tmp[(size << 1) + 1];
    for (byte i=0; i<size; i++) {
            sprintf(tmp+(i<<1), "%.2X",data[i]);
    }
    tmp[size<<1] = '\0';
    return String(tmp);
}

void writeFloat100(float val, uint8_t* dest) {
  writeInt32(int(val * 100.f + 0.5f), dest);
}  

void writeDouble100(double val, uint8_t* dest) {
  writeInt32(int(val * 100. + 0.5), dest);
}  

