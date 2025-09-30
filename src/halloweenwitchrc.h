#ifndef _HALLOWEENWITCHRC_H_
#define _HALLOWEENWITCHRC_H_

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <sabre/factory.hpp>
#include <sabre/gpio/input_gpio.hpp>
#include <sabre/service/service.hpp>
#include <string>

class HalloweenWitchRc
{
public:
    HalloweenWitchRc(sabre::FactorySharedPtr factory,
                     sabre::InputGPIOSharedPtr button);
    void start();

private:
    void _init_button();
    void _init_esp_now();
    void _button_press();
    void _send_esp_now_message(const std::string message);
    void _isr_button(int);
    void _service_send_message();

    sabre::InputGPIOSharedPtr _button;
    sabre::FactorySharedPtr _factory;
    sabre::ServiceSharedPtr _service;
    QueueHandle_t _queue;
    int64_t _last_press_time;
    uint8_t _broadcast_mac[6];
};

#endif // _HALLOWEENWITCHRC_H_