#include "halloweenwitchrc.h"
#include "esp_timer.h"
#include <cstring>
#include <esp_now.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <functional>
#include <iostream>
#include <sabre/factory.hpp>
#include <sabre_esp32/service/service.hpp>

#include <nvs_flash.h>

HalloweenWitchRc::HalloweenWitchRc(sabre::FactorySharedPtr factory,
                                   sabre::InputGPIOSharedPtr button)
    : _button(button), _factory(factory),
      _last_press_time(0), _broadcast_mac{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
{
    _queue = xQueueCreate(10, sizeof(int));
    _service = std::make_shared<sabre::esp32::Service>(
        std::bind(&HalloweenWitchRc::_service_send_message, this));
}

void HalloweenWitchRc::start()
{
    _service->start();
    _init_button();
    _init_esp_now();
}

void HalloweenWitchRc::_init_button()
{
    _button->add_interrupt_handler(
        std::bind(&HalloweenWitchRc::_isr_button, this, std::placeholders::_1),
        sabre::ISRTrigger::FALLING);
}

void HalloweenWitchRc::_init_esp_now()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi in STA mode
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    // Initialize ESP-NOW after Wi-Fi is started
    ESP_ERROR_CHECK(esp_now_init());

    // Register for the broadcast address
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, _broadcast_mac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
}

void HalloweenWitchRc::_isr_button(int)
{
    int value = 1;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(_queue, &value, &xHigherPriorityTaskWoken);
}

void HalloweenWitchRc::_service_send_message()
{
    int msg;
    while (true)
        if (xQueueReceive(_queue, &msg, portMAX_DELAY) == pdTRUE)
            _button_press();
}

void HalloweenWitchRc::_button_press()
{
    int64_t current_time = esp_timer_get_time();
    if (current_time - _last_press_time < 1000000)
    {
        std::cout << "Not sending message" << std::endl;
        return;
    }
    _send_esp_now_message("PRESS");
    std::cout << "Send ESP-NOW message" << std::endl;
    _last_press_time = current_time;
}

void HalloweenWitchRc::_send_esp_now_message(const std::string message)
{
    ESP_ERROR_CHECK(esp_now_send(
        _broadcast_mac, (const uint8_t *)message.c_str(), message.length()));
}