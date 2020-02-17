#include "neobeeCmd.h"
#include "neobeeContext.h"
#include "neobeeUtil.h"
#include "neobeeVersion.h"

//static const char *ssid PROGMEM = "NeoBee";
//const char *password = "sumsum";

#define HEADSIZE 2

NeoBeeCmd::NeoBeeCmd(
    Context& ctx, 
    NeoBeeScale& scale, 
    NeoBeeTemperature& temp, 
    uint16_t port): 

    m_ctx(ctx), 
    m_port(port),
    m_scale(scale),
    m_temperature(temp),
    m_started(false)
{};

NeoBeeCmd::~NeoBeeCmd() {
    free(m_buffer);
};

void NeoBeeCmd::begin()
{   
    if (m_started) return;

    m_buffer = (uint8_t*) malloc(BUFFER_SIZE);
    m_data_space = m_buffer + HEADSIZE;
    m_data_space_size = BUFFER_SIZE - HEADSIZE;
    wifiServer = new WiFiServer(m_port);
    wifiServer->begin();
    m_started = true;
}

void NeoBeeCmd::sendResponse(WiFiClient& client, bool flush) {
    if (!m_started) return;
    client.write(m_buffer, BUFFER_SIZE);
    client.flush();
    clearBuffer();
}

void NeoBeeCmd::handleCommand(WiFiClient& client) {
    if (!m_started) return;
    
    CmdCode cmd = getCommand();
    RequestMethod method = getMethod();

    switch (cmd) {
        case CmdCode::NOP:
            #ifdef DEBUG 
            Serial.println("NOP Request");
            #endif
            clearBuffer(CmdCode::NOP, StatusCode::OK); 
            break;

        case CmdCode::INFO:
            // New command since firmware version 0.1.2
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::INFO, StatusCode::OK);
                    m_data_space[0] = MAJOR_VERSION;
                    m_data_space[1] = MINOR_VERSION;
                    m_data_space[2] = BUILD_VERSION;
                    m_data_space[3] = 0;
                    bitWrite(m_data_space[3], 0, m_ctx.hasName());
                    bitWrite(m_data_space[3], 1, m_ctx.wifi_network.hasSSID());
                    bitWrite(m_data_space[3], 2, m_ctx.wifi_network.hasPassword());
                    bitWrite(m_data_space[3], 3, m_ctx.mqttServer.hostnameSet());
                    bitWrite(m_data_space[3], 4, m_ctx.mqttServer.portSet());
                    bitWrite(m_data_space[3], 5, m_ctx.mqttServer.loginSet());
                    bitWrite(m_data_space[3], 6, m_ctx.mqttServer.passwordSet());
                    
                    bitWrite(m_data_space[4], 0, m_ctx.scale.hasOffset());
                    bitWrite(m_data_space[4], 1, m_ctx.scale.hasFactor());
                    bitWrite(m_data_space[4], 2, m_ctx.scale.hasGain());
                    //TODO: Mayby encode which channels are in use in two bits
                    m_data_space[5] = m_temperature.getDeviceCount();
                    m_ctx.scale.printOffset(m_data_space+6);
                    m_ctx.scale.printFactor(m_data_space+10);
                    break;
                    
                default:
                    clearBuffer(CmdCode::INFO, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::NAME:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::NAME, StatusCode::OK);
                    if (m_ctx.hasName()) {
                        m_ctx.copyNameTo(m_data_space);
                        setStatus(StatusCode::OK);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    }
                    break;

                case RequestMethod::PUT:
                    m_ctx.setName(m_data_space);
                    clearBuffer(CmdCode::NAME, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::NAME, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        /********************************************************
         Managing the Scale Offset.
         Supported Methods: GET, PUT, DELETE
         ********************************************************/
        case CmdCode::SCALE_OFFSET:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::SCALE_OFFSET, StatusCode::OK);
                    if (m_scale.hasOffset()) {
                        writeDouble100(m_scale.getOffset(), m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    }
                    break;

                case RequestMethod::PUT:
                    m_scale.setOffset((double) (readInt32(m_data_space) / 100.f));
                    clearBuffer(CmdCode::SCALE_OFFSET, StatusCode::OK);
                    break;
                
                case RequestMethod::DELETE:
                    m_scale.setOffset(0.);
                    clearBuffer(CmdCode::SCALE_OFFSET, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::SCALE_OFFSET, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        /********************************************************
         Requesting the MAC Address.
         Supported Methods: GET

         Sends the MAC address back to the client. The mac
         address has six bytes and it will by stored in the first
         sic bytes of the data space of the response.
         ********************************************************/
        case CmdCode::GET_MAC_ADDRESS:
            #ifdef DEBUG
            Serial.println("---- GET_MAC_ADDRESS ----");
            #endif
            
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::GET_MAC_ADDRESS, StatusCode::OK);
                    WiFi.macAddress(m_data_space);
                    break;

                default:
                    clearBuffer(CmdCode::GET_MAC_ADDRESS, StatusCode::BAD_METHOD);
                    break;
            };
            break;
            
        case CmdCode::SCALE_FACTOR:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::SCALE_FACTOR, StatusCode::OK);
                    if (m_ctx.scale.hasFactor()) {
                        writeFloat100(m_ctx.scale.getFactor(), m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:
                    m_scale.setFactor(readInt32(m_data_space) / 100.f);
                    clearBuffer(CmdCode::SCALE_FACTOR, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::SCALE_FACTOR, StatusCode::BAD_METHOD);
                    break;
            };            
            break;

        case CmdCode::TARE:
            /**
             * TARE
             * 
             * Data Byte 0:  ntime. Number of times to measure
             */
            #ifdef DEBUG 
            Serial.println("TARE Request");
            #endif
            if (m_data_space[0] == 0) {
                clearBuffer(CmdCode::TARE, StatusCode::BAD_REQUEST);
            } else {
                m_scale.begin();
                m_scale.tare(m_data_space[0]);
                
                clearBuffer(CmdCode::TARE, StatusCode::OK);
                writeDouble100(m_scale.getOffset(), m_data_space);
                writeFloat100(m_scale.getFactor(), m_data_space+4);
            };            
            break;

        case CmdCode::CALIBRATE:
            /**
             * CALIBRATE command
             * 
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : HIGH byte of the reference weight
             * 1 : LOW byte of the reference weight
             * 2 : Number of times to measure to build an average 
             **/
            {
                uint16_t ref_weight((m_data_space[0] << 8) | m_data_space[1]);

                #ifdef DEBUG 
                Serial.println("CALIBRATE Request");
                Serial.print("Reference weight is ");
                Serial.println(ref_weight);
                #endif
                
                if (m_data_space[2] == 0 || ref_weight == 0) {
                    clearBuffer(CmdCode::CALIBRATE, StatusCode::BAD_REQUEST);    
                } else {
                    m_scale.begin();
                    if (m_scale.calibrate(ref_weight, m_data_space[2])) {
                        clearBuffer(CmdCode::CALIBRATE, StatusCode::OK);
                        writeDouble100(m_ctx.scale.getOffset(), m_data_space);
                        writeFloat100(m_ctx.scale.getFactor(), m_data_space + 4);
                    } else {
                        clearBuffer(CmdCode::CALIBRATE, StatusCode::BAD_REQUEST);
                    };
                };
            };
            break;

        case CmdCode::GET_VERSION:
            /**
             * GET_VERSION command
             * 
             * Requesting the version of the firmware. The version follows
             * roughly the rules of semantic versioning.
             *  
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : Version major number
             * 1 : Version number
             * 2 : Version build number
             **/
            clearBuffer(CmdCode::GET_VERSION, StatusCode::OK);
            m_data_space[0] = MAJOR_VERSION;
            m_data_space[1] = MINOR_VERSION;
            m_data_space[2] = BUILD_VERSION;
            break;

        case CmdCode::IDLE_TIME:
            switch (method) {
                case RequestMethod::PUT:
                    m_ctx.setDeepSleepSeconds(m_data_space[0], m_data_space[1]);
                    clearBuffer(CmdCode::IDLE_TIME, StatusCode::OK);
                    m_data_space[0] = highByte(m_ctx.getDeepSleepSeconds()); // HIGH BYTE
                    m_data_space[1] = lowByte(m_ctx.getDeepSleepSeconds());  // LOW BYTE
                    break;

                case RequestMethod::GET:
                    clearBuffer(CmdCode::IDLE_TIME, StatusCode::OK);
                    m_data_space[0] = highByte(m_ctx.getDeepSleepSeconds()); // HIGH BYTE
                    m_data_space[1] = lowByte(m_ctx.getDeepSleepSeconds());  // LOW BYTE
                    break;

                default:
                    clearBuffer(CmdCode::IDLE_TIME, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::GET_WEIGHT:
            /**
             * GET_WEIGHT command
             * 
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : Number of times to measure to build an average weight.
             * 1 : Mesure Method
             */
            {
                float weight;
                uint8_t ntimes(m_data_space[0]);
                WeightMethod weight_method(static_cast<WeightMethod>(m_data_space[1])); 

                if (ntimes == 0) {
                    clearBuffer(CmdCode::GET_WEIGHT, StatusCode::BAD_REQUEST);
                } else {
                    clearBuffer(CmdCode::GET_WEIGHT, StatusCode::OK);
                    weight = m_scale.getWeight(ntimes, weight_method);
                    writeFloat100(weight, m_data_space);            
                };
            };
            break;

        case CmdCode::SAVE_SETTINGS:
            /**
             * SAVE_SETTINGS command
             * 
             * Saves the settings to EEPROM without any checks,
             * if data is valid.
             **/
            saveContext(&m_ctx);
            break;

        case CmdCode::RESET_SETTINGS:
            resetContext(&m_ctx);
            break;

        case CmdCode::ERASE_SETTINGS:
            eraseContext(&m_ctx);
            break;

        case CmdCode::SSID:
            #ifdef DEBUG
            Serial.println("SSID ");
            #endif
            switch (method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::SSID, StatusCode::OK);
                    if (m_ctx.wifi_network.hasSSID()) {
                        m_ctx.wifi_network.getSSID(m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:
                    m_ctx.wifi_network.setSSID(m_data_space);
                    clearBuffer(CmdCode::SSID, StatusCode::OK);
                    break;

                case RequestMethod::DELETE:
                    clearBuffer(CmdCode::SSID, StatusCode::OK);
                    m_ctx.wifi_network.clearSSID();
                    break;

                default:
                    clearBuffer(CmdCode::SSID, StatusCode::BAD_METHOD);
                    m_data_space[0] = static_cast<uint8_t>(method);
                    break;
            };
            break;   

        case CmdCode::PASSWORD:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::PASSWORD, StatusCode::OK);
                    if (m_ctx.wifi_network.hasPassword()) {
                        m_ctx.wifi_network.getPassword(m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    }
                    break;

                case RequestMethod::PUT:        
                    m_ctx.wifi_network.setPassword(m_data_space);
                    clearBuffer(CmdCode::PASSWORD, StatusCode::OK);
                    break;
                
                case RequestMethod::DELETE:
                    clearBuffer(CmdCode::PASSWORD, StatusCode::OK);
                    m_ctx.wifi_network.clearPassword();
                    break;

                default:
                    clearBuffer(CmdCode::PASSWORD, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::MQTT_HOST:
            switch (method) {
                case RequestMethod::GET:            
                    clearBuffer(CmdCode::MQTT_HOST, StatusCode::OK);
                    if (m_ctx.mqttServer.hostnameSet()) {
                        m_ctx.mqttServer.copyHostnameTo(m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:
                    m_ctx.mqttServer.setHostname(m_data_space);
                    clearBuffer(CmdCode::MQTT_HOST, StatusCode::OK);
                    break;

                case RequestMethod::DELETE:
                    m_ctx.mqttServer.clearHostname();
                    clearBuffer(CmdCode::MQTT_HOST, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::MQTT_HOST, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::MQTT_PORT:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::MQTT_PORT, StatusCode::OK);
                    if (m_ctx.mqttServer.portSet()) {
                        m_data_space[0] = highByte(m_ctx.mqttServer.port);
                        m_data_space[1] = lowByte(m_ctx.mqttServer.port);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:
                    m_ctx.mqttServer.setPort((m_data_space[0] << 8) | m_data_space[1]);
                    clearBuffer(CmdCode::MQTT_PORT, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::MQTT_PORT, StatusCode::BAD_METHOD);
                    break;
            };
            break;
            
        case CmdCode::MQTT_LOGIN:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::MQTT_LOGIN, StatusCode::OK);
                    if (m_ctx.mqttServer.loginSet()) {
                        m_ctx.mqttServer.copyLoginTo(m_data_space);
                        #ifdef DEBUG
                        Serial.println(stringFromByteAray((uint8_t*) (m_ctx.mqttServer.login),30));
                        #endif
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:        
                    m_ctx.mqttServer.setLogin(m_data_space);
                    clearBuffer(CmdCode::MQTT_LOGIN, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::MQTT_LOGIN, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::MQTT_PASSWORD:
            switch(method) {
                case RequestMethod::GET:
                    clearBuffer(CmdCode::MQTT_PASSWORD, StatusCode::OK);
                    if (m_ctx.mqttServer.passwordSet()) {
                        m_ctx.mqttServer.copyPasswordTo(m_data_space);
                    } else {
                        setStatus(StatusCode::NOT_FOUND);
                    };
                    break;

                case RequestMethod::PUT:
                    m_ctx.mqttServer.setPassword(m_data_space);
                    clearBuffer(CmdCode::MQTT_PASSWORD, StatusCode::OK);
                    break;

                default:
                    clearBuffer(CmdCode::MQTT_PASSWORD, StatusCode::BAD_METHOD);
                    break;
            };
            break;

        case CmdCode::GET_TEMPERATURE:
            #ifdef DEBUG
            Serial.println("GET_TEMPERATURE Request");
            #endif
            m_temperature.begin();
            clearBuffer(CmdCode::GET_TEMPERATURE, StatusCode::OK);
            {
                float c = m_temperature.getCTemperatureByIndex();
                #ifdef DEBUG
                Serial.print("Temperature: ");
                Serial.println(c);
                #endif
                writeFloat100(c, m_data_space);
            }
            break;

        case CmdCode::RESET_ESP:
            clearBuffer(CmdCode::RESET_ESP, StatusCode::OK);
            sendResponse(client);
            ESP.restart();
            break;
            

        default:
            #ifdef DEBUG 
            Serial.println("Unknown Command");
            #endif
            clearBuffer(CmdCode::NOP, StatusCode::BAD_REQUEST);
            break;
    }
    sendResponse(client);
}

/**
 * Checks, if a client has established an connection and 
 * collects the incoming data, until we have received
 * one bytes for one command (32-Bytes). After receiving
 * the bytes, the handleCommand methos is called for
 * command execution.
 **/
void NeoBeeCmd::checkForCommands() {
    if (!m_started) return;
    WiFiClient client = wifiServer->available();

    if (client) {
        
        while (client.connected()) {
            uint8_t index = 0;
            clearBuffer();
            while (client.available()>0) {
                m_buffer[index++] = client.read();
                if (index == 32) {
                    handleCommand(client);
                    clearBuffer();
                    index = 0;
                }
                delay(10);
            }
        };
        client.stop();
    }
}