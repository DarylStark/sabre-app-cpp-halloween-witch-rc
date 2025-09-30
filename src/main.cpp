#include "halloweenwitchrc.h"
#include <iostream>

#include <sabre_esp32/factory.hpp>

HalloweenWitchRc *app;

extern "C"
{
    void app_main(void)
    {

        // Configure button
        sabre::FactorySharedPtr factory =
            std::make_shared<sabre::esp32::Factory>();
        sabre::InputGPIOSharedPtr button = factory->create_input_gpio(8);
        button->enable_pullup();
        button->set_inverse_level(true);

        // Create the application
        app = new HalloweenWitchRc(factory, button);

        // Start the application
        app->start();

        // Done!
        return;
    }
}