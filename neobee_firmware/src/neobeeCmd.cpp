#include "neobeeCmd.h"
#include "neobeeContext.h"
#include "neobeeUtil.h"


const char *ssid = "NeoBee";
const char *password = "sumsum";

#define HEADSIZE 2

NeoBeeCmd::NeoBeeCmd(
    Context& ctx, 
    NeoBeeScale& scale, 
    NeoBeeTemperature& temp, 
    uint16_t port): 

    m_ctx(ctx), 
    m_port(port),
    m_scale(scale),
    m_temperature(temp)
{};

NeoBeeCmd::~NeoBeeCmd() {
    free(m_buffer);
};

void NeoBeeCmd::begin()
{   
    m_buffer = (uint8_t*) malloc(BUFFER_SIZE);
    m_data_space = m_buffer + HEADSIZE;
    m_data_space_size = BUFFER_SIZE - HEADSIZE;
    wifiServer = new WiFiServer(m_port);
    wifiServer->begin();
}

void NeoBeeCmd::sendResponse(WiFiClient& client, bool flush) {
    client.write(m_buffer, BUFFER_SIZE);
    client.flush();
    clearBuffer();
}

void NeoBeeCmd::handleCommand(WiFiClient& client) {
    printByteArray(m_buffer);
    CmdCode cmd = getCommand();
    switch (cmd) {
        case CmdCode::NOP:
            #ifdef DEBUG 
            Serial.println("NOP Request");
            #endif
            clearBuffer(CmdCode::NOP, StatusCode::OK); 
            break;

        case CmdCode::GET_NAME:
            #ifdef DEBUG 
            Serial.println("GET_NAME Request");
            printByteArray(m_ctx.name, 20);
            #endif
            clearBuffer(CmdCode::GET_NAME, StatusCode::OK);
            if (m_ctx.hasName()) {
                memcpy(m_data_space, m_ctx.name, sizeof(m_ctx.name));
                setStatus(StatusCode::OK);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            }
            break;

        case CmdCode::SET_NAME:
            #ifdef DEBUG 
            Serial.println("SET_NAME Request");
            #endif
            printByteArray(m_ctx.name, 20);
            m_ctx.setName(m_data_space);
            printByteArray(m_ctx.name, 20);
            clearBuffer(CmdCode::SET_NAME, StatusCode::OK);
            break;

        case CmdCode::GET_SCALE_OFFSET:
            #ifdef DEBUG 
            Serial.println("GET_SCALE_OFFSET Request");
            #endif
            clearBuffer(CmdCode::GET_SCALE_OFFSET, StatusCode::OK);
            if (m_ctx.hasOffset()) {
                #ifdef DEBUG
                Serial.println("Write offset to data space");
                #endif
                writeInt32(int(m_scale.getOffset() * 100.0 + 0.5), m_data_space);
                printByteArray(m_buffer);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            }
            break;

        case CmdCode::SET_SCALE_OFFSET:
            #ifdef DEBUG 
            Serial.println("SET_SCALE_OFFSET Request");
            #endif
            m_scale.setOffset(readInt32(m_data_space) / 100.f);
            clearBuffer(CmdCode::SET_SCALE_OFFSET, StatusCode::OK);
            #ifdef DEBUG 
            Serial.print("New offset is:");
            Serial.println(m_scale.getOffset());            
            #endif
            break;

        /* ------------------------------------
         * GET_MAC_ADDRESS
         */    
        case CmdCode::GET_MAC_ADDRESS:
            #ifdef DEBUG 
            Serial.println("GET_MAC_ADDRESS Request");
            #endif
            clearBuffer(CmdCode::GET_MAC_ADDRESS, StatusCode::OK);
            WiFi.macAddress(m_data_space);
            break;

        case CmdCode::GET_FLAGS:
            #ifdef DEBUG 
            Serial.println("GET_FLAGS Request");
            #endif
            clearBuffer(CmdCode::GET_FLAGS, StatusCode::OK);
            m_data_space[0] = m_ctx.flags;
            break;

        case CmdCode::GET_SCALE_FACTOR:
            #ifdef DEBUG 
            Serial.println("GET_SCALE_FACTOR Request");
            #endif
            clearBuffer(CmdCode::GET_SCALE_FACTOR, StatusCode::OK);
            if (m_ctx.hasFactor()) {
                writeInt32((int)(m_ctx.scale.factor * 100.0 + 0.5), m_data_space);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            };
            break;

        case CmdCode::SET_SCALE_FACTOR:
            #ifdef DEBUG 
            Serial.println("SET_SCALE_FACTOR Request");
            #endif
            m_scale.setFactor(readInt32(m_data_space) / 100.f);
            clearBuffer(CmdCode::SET_SCALE_FACTOR, StatusCode::OK);
            #ifdef DEBUG 
            Serial.println(m_scale.getFactor());
            #endif
            
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
            clearBuffer(CmdCode::TARE, StatusCode::OK);
            m_scale.begin();
            m_scale.tare(m_data_space[0]);
            writeInt32(int(m_scale.getOffset() * 100 + 0.5), m_data_space);
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
            uint16_t ref_weight;

            #ifdef DEBUG 
            Serial.println("CALIBRATE Request");
            #endif
            clearBuffer(CmdCode::CALIBRATE, StatusCode::OK);
            m_scale.begin();
            
            ref_weight = ((m_data_space[0] << 8) + m_data_space[1]);
            m_scale.calibrate(ref_weight, m_data_space[2]);
        
            writeInt32(int(m_ctx.scale.offset * 100 + 0.5), m_data_space);
            writeInt32(int(m_ctx.scale.factor * 100 + 0.5), m_data_space + 4);
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
            m_data_space[0] = 0;
            m_data_space[1] = 1;
            m_data_space[2] = 0;
            break;

        /**
         * Setting the idle time in seconds (duration for deep sleep).
         * The value is a two byte value. High-Byte in the first and
         * low-byte in th second byte of the data space of the command.
         */
        case CmdCode::SET_IDLE_TIME:

            /**
             * SET_IDLE_TIME command
             * 
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : HIGH byte of the idle time
             * 1 : LOW byte of the idle time
             **/
            clearBuffer(CmdCode::SET_IDLE_TIME, StatusCode::OK);
            m_ctx.deep_sleep_seconds = (m_data_space[0] << 8) | m_data_space[1];
            break;

        case CmdCode::GET_IDLE_TIME:

            /**
             * GET_IDLE_TIME command
             * 
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : HIGH byte of the idle time
             * 1 : LOW byte of the idle time
             **/
            clearBuffer(CmdCode::GET_IDLE_TIME, StatusCode::OK);
            m_data_space[0] = (m_ctx.deep_sleep_seconds >> 8) & 0xff; // HIGH BYTE
            m_data_space[1] = m_ctx.deep_sleep_seconds & 0xff;        // LOW BYTE
            break;

        case CmdCode::GET_WEIGHT:
            /**
             * GET_WEIGHT command
             * 
             * Data bytes
             * ---------------------------------------------------------------
             * 0 : Number of times to measure to build an average weight.
             */
            uint8_t ntimes;
            float weight;

            clearBuffer(CmdCode::GET_WEIGHT, StatusCode::OK);
            ntimes = m_data_space[0];
            weight = m_scale.getWeight(ntimes);
            writeInt32(weight, m_data_space);
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

        case CmdCode::GET_SSID:
            #ifdef DEBUG
            Serial.println("GET SSID");
            Serial.print("Flags : ");
            Serial.println(m_ctx.wifi_network.flags, BIN);
            #endif
            clearBuffer(CmdCode::GET_SSID, StatusCode::OK);
            if (m_ctx.wifi_network.hasSSID()) {
                m_ctx.wifi_network.getSSID(m_data_space);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            };
            break;

        case CmdCode::SET_SSID:
            m_ctx.wifi_network.setSSID(m_data_space);
            clearBuffer(CmdCode::SET_SSID, StatusCode::OK);
            break;

        case CmdCode::CLEAR_SSID:
            clearBuffer(CmdCode::CLEAR_SSID, StatusCode::OK);
            m_ctx.wifi_network.clearSSID();
    	    break;

        case CmdCode::GET_PASSWORD:
            clearBuffer(CmdCode::GET_PASSWORD, StatusCode::OK);
            if (m_ctx.wifi_network.hasPassword()) {
                m_ctx.wifi_network.getPassword(m_data_space);
            } else {
                setStatus(StatusCode::NOT_FOUND);
            }
            break;
        
        case CmdCode::SET_PASSWORD:
            m_ctx.wifi_network.setPassword(m_data_space);
            clearBuffer(CmdCode::SET_PASSWORD, StatusCode::OK);
            break;

        case CmdCode::CLEAR_PASSWORD:
            clearBuffer(CmdCode::CLEAR_PASSWORD, StatusCode::OK);
            m_ctx.wifi_network.clearPassword();
            break;

        case CmdCode::SET_WIFI_ACTIVE:
            bitWrite(
                m_ctx.wifi_network.flags, 
                int(WifiFlags::FLAG_ACTIVE), 
                m_data_space[0] & 1); 
            clearBuffer(CmdCode::SET_WIFI_ACTIVE, StatusCode::OK);
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