#pragma once
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <cstdio>

class Serializer
{
    constexpr static const char seperationChar = '_';
    constexpr static const char endOfFrameChar = '\n';
    constexpr static const uint16_t crcPolynomial = 0x1021;
    constexpr static const uint16_t crcInitialRemainder = 0xFFFF;
    constexpr static const uint16_t crcFinalXORValue = 0x0000;

    uint16_t calculateCRC(const char* start, const char* end)
    {
        uint16_t remainder = crcInitialRemainder;
        for (const char* current = start; current < end; ++current) {
            uint8_t data = *current;
            for (uint8_t bit = 0; bit < 8; ++bit) {
                if ((data & 0x01) ^ (remainder & 0x01)) {
                    remainder = (remainder >> 1) ^ crcPolynomial;
                }
                else {
                    remainder >>= 1;
                }
                data >>= 1;
            }
        }
        remainder = ~remainder;
        remainder ^= crcFinalXORValue;
        return remainder;
    }
public:
    int Deserialize(const char* src, size_t size, uint8_t* source, uint8_t* destination, uint8_t* hopCount, const char** payload) {
        // Ensure src is not NULL and size is sufficient for at least one character
        if (src == NULL || size < 1) {
            //printf("Error: Invalid source string\n");
            return -1;
        }

        // Extract individual components from the source string
        const char* arg0 = src;
        const char* arg1 = arg0 ? strchr(arg0, seperationChar) + 1 : NULL;
        const char* arg2 = arg1 ? strchr(arg1, seperationChar) + 1 : NULL;
        const char* arg3 = arg2 ? strchr(arg2, seperationChar) + 1 : NULL;
        const char* arg4 = arg3 ? strchr(arg3, seperationChar) + 1 : NULL;
        const char* arg5 = arg4 ? strchr(arg4, endOfFrameChar) + 1 : NULL;

        // Check if all components are found and in correct order
        if (arg0 && arg1 && arg2 && arg3 && arg4 && arg5) {
            // Parse and store individual components
            *source = (uint8_t)strtol(arg0, NULL, 16);
            *destination = (uint8_t)strtol(arg1, NULL, 16);
            *hopCount = (uint8_t)strtol(arg2, NULL, 16);
            *payload = arg3;
            uint16_t crc = (uint16_t)strtol(arg4, NULL, 16);
            uint16_t calcCrc = calculateCRC(arg0, arg4);

            if (crc == calcCrc) {
                return arg5 ? (arg5 - src) + 1 : -1;
            }
            else {
                //printf("Error: CRC invalid\n");
                return -2;
            }
        }
        else {
            //printf("Error: Invalid format\n");
            return -3;
        }
    }

    int Serialize(char* dst, size_t size, uint8_t source, uint8_t destination, uint8_t hopCount, const char* payload) {

        // Ensure data is valid:
        if (strchr(payload, seperationChar) != NULL)
        {
            //printf("Error: payload contains '%c' char\n", seperationChar);
            return -1;
        }

        if (strchr(payload, endOfFrameChar) != NULL)
        {
            //printf("Error: payload contains '%c' char\n", endOfFrameChar);
            return -1;
        }

        // Calculate the length of the string including dummy crc
        size_t stringLength = snprintf(NULL, 0, "%02X,%02X,%s,%04X\n", destination, hopCount, payload, 0x0000);
        if (stringLength + 1 > size) { //+1 for null terminator
            return -1;
        }

        size_t idx = 0;
        idx += snprintf(&dst[idx], size - idx, "%02X", source);
        dst[idx++] = seperationChar;
        idx += snprintf(&dst[idx], size - idx, "%02X", destination);
        dst[idx++] = seperationChar;
        idx += snprintf(&dst[idx], size - idx, "%02X", hopCount);
        dst[idx++] = seperationChar;
        idx += snprintf(&dst[idx], size - idx, "%s", payload);
        dst[idx++] = seperationChar;

        // Calculate CRC
        uint16_t crc_value = calculateCRC(dst, dst + idx - 1);
        idx += snprintf(&dst[idx], size - idx, "%04X", crc_value);
        dst[idx++] = endOfFrameChar;
        dst[idx++] = '\0';
        return idx;
    }
};