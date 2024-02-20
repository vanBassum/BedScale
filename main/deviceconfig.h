#pragma once
#include "esp_drivers.h"



//----------------------------
//          ESP Gpio controller
//----------------------------

constexpr const DeviceProperty espGpio_0_config[] = {
    {"key",           DEVICE_PROP_STR("espGpio_0")},
    {"compatible",    DEVICE_PROP_STR("EspGpio")},
    DEVICE_END_MARKER // End marker
};

//----------------------------
//          SPI Bus
//----------------------------

constexpr const DeviceProperty spiBus_0_config[] = {
    {"key",           DEVICE_PROP_STR("spiBus_0")},
    {"compatible",    DEVICE_PROP_STR("espSpiBus")},
    {"host",          DEVICE_PROP_I32(HSPI_HOST)},
    {"dmaChannel",    DEVICE_PROP_I32(SPI_DMA_CH_AUTO)},
    {"mosi_io_num",   DEVICE_PROP_I32(GPIO_NUM_13)},
    {"miso_io_num",   DEVICE_PROP_I32(GPIO_NUM_NC)},
    {"sclk_io_num",   DEVICE_PROP_I32(GPIO_NUM_14)},
    {"max_transfer_sz", DEVICE_PROP_I32(1024)},
    DEVICE_END_MARKER // End marker
};


//----------------------------
//          ST7796S
//----------------------------
constexpr const DeviceProperty spiDevice_0_config[] = {     
    {"key",              DEVICE_PROP_STR("spiDevice_0")},
    {"compatible",       DEVICE_PROP_STR("espSpiDevice")},
    {"spiBus",           DEVICE_PROP_STR("spiBus_0")},
    {"clock_speed_hz",   DEVICE_PROP_I32(6 * 1000 * 1000)}, 
    {"spics_io_num",     DEVICE_PROP_I32(GPIO_NUM_NC)},
    {"queue_size",       DEVICE_PROP_I32(7)},
    {"spics_io_num",     DEVICE_PROP_I32(GPIO_NUM_15)},
    DEVICE_END_MARKER // End marker
};

constexpr const DeviceProperty st7796s_0_config[] = {
    {"key",              DEVICE_PROP_STR("st7796s_0")},
    {"compatible",       DEVICE_PROP_STR("ST7796S")},
    {"spiDevice",        DEVICE_PROP_STR("spiDevice_0")},
    {"dcPin",            DEVICE_PROP_STR("espGpio_0,2,5")},  // GPIO_NUM_21         2*8 + 5
    {"rstPin",           DEVICE_PROP_STR("espGpio_0,2,6")},  // GPIO_NUM_22         2*8 + 6
    {"blckPin",          DEVICE_PROP_STR("espGpio_0,2,7")},  // GPIO_NUM_23         2*8 + 7
    DEVICE_END_MARKER // End marker
};

constexpr const Device deviceTree[] = {
    espGpio_0_config,
    spiBus_0_config,
    spiDevice_0_config,
    st7796s_0_config,
    nullptr // End marker
};



/*
void ConfigSPIBus(std::shared_ptr<SPIBus> bus)
{
    assert(bus);
    bus->setConfig({
        .host = SPI2_HOST,
        .dmaChannel = 1,
        .config = {
            .mosi_io_num        = GPIO_NUM_13,
            .miso_io_num        = GPIO_NUM_NC,
            .sclk_io_num        = GPIO_NUM_14,
            .quadwp_io_num      = GPIO_NUM_NC,
            .quadhd_io_num      = GPIO_NUM_NC,
            .max_transfer_sz    = 40 * 320,
        },
    });

    bus->init();
    
}

void ConfigSPIDevice(std::shared_ptr<SPIDevice> device)
{
    assert(device);

    device->setConfig({
        .devConfig = {
            .command_bits = 0,
            .address_bits = 0,
            .dummy_bits = 0,
            .mode = 0,
            .duty_cycle_pos = 0,
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = 40 * 1000 * 1000,
            .input_delay_ns = 0,
            .spics_io_num = GPIO_NUM_15,
            .flags = 0,
            .queue_size = 50,
            .pre_cb = NULL,
            .post_cb = NULL,
        }
    });

    device->init();
}


void ConfigST7796S(std::shared_ptr<ST7796S> device)
{
    assert(device);
    device->setConfig({
        .dc       = GPIO_NUM_21,
        .rst      = GPIO_NUM_22,
        .blck     = GPIO_NUM_23,
        .hor_res  = 320,
        .ver_res  = 480,
    });

    device->init();

}


void ConfigLVGL(std::shared_ptr<ESP_LVGL::LVGLService> service)
{
    assert(service);
    service->setConfig({
        .timerIntervalms = 1,
        .taskIntervalms = 20,
    });
    service->init();
}

void ConfigST47796SAdapter(std::shared_ptr<ST47796SAdapter> adapter)
{
    assert(adapter);
    adapter->setConfig({
        .width = 320,
        .height = 480,
        .bufferSize = 320 * 480 / 20,
    });
    adapter->init();
}



*/

