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
#include <stdint.h>
#include "driver/uart.h"
#include "datacollector.h"
#include "serializer.h"

#define UART_TX2_PIN    GPIO_NUM_17
#define UART_RX2_PIN    GPIO_NUM_16
#define UARTNUM         UART_NUM_1
#define BUF_SIZE        128
Task rxTask;
DataCollector collector;
Serializer Serializer;

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



void SendAndReceive(const char* message)
{
    uart_write_bytes(UARTNUM, message, strlen(message));
}


extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "Started");
    uart_init();
    rxTask.Init("RxTask", 7, 1024 * 2);
    rxTask.SetHandler([&](){
        uint8_t data[BUF_SIZE];
        int len;
        while(1)
        {
            len = uart_read_bytes(UARTNUM, data, BUF_SIZE, pdMS_TO_TICKS(100));
            if (len > 0) {
                data[len-1] = 0; // Add null-terminator to use with printf
                ESP_LOGI("MAIN", "Received: %s", data);
            }
        }
    });
    rxTask.Run();

    int i = 0;
    while (1) {
        if(i%10 == 0)
        {
            SendAndReceive("0_FF_0_CGID_1234\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        SendAndReceive("0_1_0_DRAW_1234\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        i++;
        //SendAndReceive("0_2_0_DVER_1234\n");
        //vTaskDelay(1000);
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

