#ifndef neobeemqtt_h
#define neobeemqtt_h

#include <Arduino.h>
#include "neobeeTypes.h"

// Data strucure to store relevant information
// between two deep sleep cycles
//
typedef struct {
  uint8_t flags;                    // Mqtt Flags
  char host_name[31];               // 0-terminated hostname or ip of the mqtt server
  uint16_t port;                    // mqtt port
} MqttServer;

#endif