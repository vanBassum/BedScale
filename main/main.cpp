/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#define CONFIG_LV_COLOR_16_SWAP 1
//#define CONFIG_LV_THEME_DEFAULT_DARK 1
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_drivers.h"
#include "deviceconfig.h"
#include <math.h>
#pragma once
#include <stdint.h>
#include "driver/uart.h"

#define UART_TX2_PIN (GPIO_NUM_17)
#define UART_RX2_PIN (GPIO_NUM_16)

#define BUF_SIZE (1024)

#define UARTNUM UART_NUM_1

void uart_init() {
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Configure UART parameters
    uart_param_config(UARTNUM, &uart_config);
    
    // Set UART pins
    uart_set_pin(UARTNUM, UART_TX2_PIN, UART_RX2_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Install UART driver using an event queue here.
    uart_driver_install(UARTNUM, BUF_SIZE * 2, 0, 0, NULL, 0);
}

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

class DataCollector
{
    constexpr static const char endOfFrameChar = '\n';
    char buffer[128];
    size_t idx = 0;
public:
    void (*OnFrame)(char* frame, size_t size) = NULL;

    void HandleData(const char* data, size_t size)
    {
        for(int i = 0; i < size; i++)
        {
            buffer[idx++] = data[i];
            if(idx >= 128)
                return;
            if(data[i] == endOfFrameChar)
            {
                if(OnFrame)
                    OnFrame(buffer, idx);
                idx = 0;
            }
        }
    }
};




void HandleCommand(uint8_t source, uint8_t destination, uint8_t hopCount, const char* payload)
{
  hopCount++;
  if(strlen(payload) < 4)
  {
    SendCommand(myAddress, 0, hopCount, "NACKPayload to short");
    return;
  }

  bool forMaster = destination == 0;
  if (forMaster)
  {
    SendCommand(source, destination, hopCount, payload);                // Forward to next node!
    return;
  }

  bool isBroadcast = destination == 0xFF;                               // Broadcast message
  if (isBroadcast)
  {
    ExecuteCommand(source, destination, hopCount, payload);             // Execute command
    delay(10);
    SendCommand(source, destination, hopCount, payload);                // Forward to next node!
    return;
  }

  bool hasAddress = (myAddress != 0);   
  if (!hasAddress)
  {
    SendCommand(source, destination, hopCount, payload);                // Forward to next node!
    delay(10);
    SendCommand(hopCount, 0, hopCount, "DGID");                         // Ask for ID
    return;
  }

  bool addressMatch = (destination == myAddress);                       // Message directed to me
  if (addressMatch)
  {
    ExecuteCommand(source, destination, hopCount, payload);             // Execute command
    return;
  }
  else
  {
    SendCommand(source, destination, hopCount, payload);                // Forward to next node!
  }
}





void SendAndReceive(const char* message)
{
    uint8_t data[BUF_SIZE];
    int len;

    ESP_LOGW("MAIN", "Send data: %s", message);
    uart_write_bytes(UARTNUM, message, strlen(message));
    vTaskDelay(10); 
    // Read data from UART
    len = uart_read_bytes(UARTNUM, data, BUF_SIZE, 100);
    if (len > 0) {
        data[len] = 0; // Add null-terminator to use with printf
        ESP_LOGI("MAIN", "Received: %s", data);
    }
}


extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "Started");
    uart_init();

    const char *message = "00_00_00_BGID_ECC2\n";


    while (1) {
        SendAndReceive("0_FF_0_CGID_1234\n");
        vTaskDelay(1000);
        
        SendAndReceive("0_1_0_DVER_1234\n");
        vTaskDelay(1000);
        
        SendAndReceive("0_2_0_DVER_1234\n");
        vTaskDelay(1000);
    }
}









/*
float Scale(float raw_measurement)
{
    float x1 = 122708;
    float y1 = 0.0f;
    float x2 = 122612;
    float y2 = 143.52f;

    float scaled_measurement = y1 + (raw_measurement - x1) * (y2 - y1) / (x2 - x1);
    return scaled_measurement;
}

extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "Started");


    std::shared_ptr<DriverRegistry> driverRegistery = std::make_shared<DriverRegistry>();
    driverRegistery->RegisterDriver<SpiBus>("espSpiBus");
    driverRegistery->RegisterDriver<SpiDevice>("espSpiDevice");
    driverRegistery->RegisterDriver<EspGpio>("EspGpio");
    driverRegistery->RegisterDriver<HX711>("HX711");

    std::shared_ptr<DeviceManager> deviceManager = std::make_shared<DeviceManager>(driverRegistery);
    deviceManager->init();
    deviceManager->RegisterDetector(std::make_shared<DeviceTreeDetector>(deviceTree)); 





    int num = 10;
    std::shared_ptr<HX711> hx711 = nullptr;
    uint32_t measurements[num];
    memset(measurements, 0, sizeof(measurements)); // Clear measurements array
    uint32_t i = 0;
    while (1)
    {   
        if(hx711 == nullptr)
        {
            hx711 = deviceManager->getDeviceByKey<HX711>("hx711_0");
            //if(hx711!= nullptr)
            //    hx711->power_up();
        }
        else
        {
            hx711->read(&measurements[i % num]);
            if((i + 1) % num == 0) // Check if we've gathered 100 measurements
            {
                uint64_t value = 0;
                for(int m = 0; m < num; m++)
                    value += measurements[m];

                uint32_t raw = value / num;
                float scaled = Scale(raw);

                ESP_LOGI("MAIN", "%08d  -  %.3f", (int)raw, scaled);
                memset(measurements, 0, sizeof(measurements)); // Clear measurements array
            }            
        }

        vTaskDelay(pdMS_TO_TICKS(10));
        i++;
    }
*/

    /*
    builder.Services.addService<SPIBus>().Config(ConfigSPIBus);
    builder.Services.addService<SPIDevice>(builder.Services.getService<SPIBus>()).Config(ConfigSPIDevice);
    builder.Services.addService<ST7796S>(builder.Services.getService<SPIDevice>()).Config(ConfigST7796S);
    builder.Services.addService<ESP_LVGL::LVGLService>().Config(ConfigLVGL);
    builder.Services.addService<ESP_LVGL::Display, ST47796SAdapter>(builder.Services.getService<ESP_LVGL::LVGLService>(), builder.Services.getService<ST7796S>()).Config(ConfigST47796SAdapter);


    auto lvgl = builder.Services.getService<ESP_LVGL::LVGLService>();
    auto display = builder.Services.getService<ESP_LVGL::Display>();

    assert(display);

    ESP_LVGL::Screen screen(lvgl);
    display->showScreen(screen);

    ESP_LVGL::Button button(screen);

    button.SetSize(200, 50);

    ESP_LVGL::Label label(screen);

    label.SetPosition(10, 300);
    label.SetText("Hello world");

    while(1)
        vTaskDelay(pdMS_TO_TICKS(1000));
    */
//}

