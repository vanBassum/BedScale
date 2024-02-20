#pragma once
#include "esp_lvgl.h"
#include "st7796s.h"
#include <functional>
#include "kernel.h"

class ST47796SAdapter : public ESP_LVGL::Display
{
	constexpr static const char* TAG = "ST7796SAdapter";
    std::shared_ptr<ST7796S> driver;

protected:
    virtual void test()override
    {

    }


	//void flushCallback(struct _lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) override
	//{
    //    driver->SetWindow(area->x1, area->y1, area->x2, area->y2);
    //    size_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    //    driver->WriteWindow((uint16_t*)px_map, size);
	//}
	
public:
    ST47796SAdapter(std::shared_ptr<ESP_LVGL::LVGLService> lvgl, std::shared_ptr<ST7796S> driver)
        : ESP_LVGL::Display(lvgl), driver(driver)
    {
        
    }
    
};
