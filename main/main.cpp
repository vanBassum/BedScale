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


extern "C" void app_main(void)
{
    ESP_LOGI("MAIN", "Started");

    std::shared_ptr<DriverRegistry> driverRegistery = std::make_shared<DriverRegistry>();
    driverRegistery->RegisterDriver<SpiBus>("SpiBus");
    driverRegistery->RegisterDriver<SpiDevice>("SpiDevice");
    driverRegistery->RegisterDriver<ESPUart>("ESPUart");
    
    

    std::shared_ptr<DeviceManager> deviceManager = std::make_shared<DeviceManager>(driverRegistery);
    deviceManager->init();
    deviceManager->RegisterDetector(std::make_shared<DeviceTreeDetector>(deviceTree)); 


    while(1)
        vTaskDelay(pdMS_TO_TICKS(1000));



}

