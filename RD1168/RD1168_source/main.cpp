// ----------------------------------------------------------------------------
// Copyright 2016-2018 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "mbed.h"
#include "simple-mbed-cloud-client.h"
//#include "SDBlockDevice.h"
//#include "FATFileSystem.h"
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
#include "WicedInterface.h"

// An event queue is a very useful structure to debounce information between contexts (e.g. ISR and normal threads)
// This is great because things such as network operations are illegal in ISR, so updating a resource in a button's fall() function is not allowed
EventQueue eventQueue;

// Storage implementation definition, currently using SDBlockDevice (SPI flash is also available)
//SDBlockDevice sd(P_4, P_7, P_6, P_5);
SPIFBlockDevice sd(P_4, P_7, P_6, P_33);
//FATFileSystem fs("sd", &sd);
LittleFileSystem fs("sd", &sd);

// Declaring pointers for access to Mbed Cloud Client resources outside of main()
MbedCloudClientResource *button_res;
MbedCloudClientResource *pattern_res;

// This function gets triggered by the timer. It's easy to replace it by an InterruptIn and fall() mode on a real button
void fake_button_press() {
    int v = button_res->get_value_int() + 1;

    button_res->set_value(v);

    printf("Simulated button clicked %d times\n", v);
}

/**
 * PUT handler
 * @param resource The resource that triggered the callback
 * @param newValue Updated value for the resource
 */
void pattern_updated(MbedCloudClientResource *resource, m2m::String newValue) {
    printf("PUT received, new value: %s\n", newValue.c_str());
}

/**
 * POST handler
 * @param resource The resource that triggered the callback
 * @param buffer If a body was passed to the POST function, this contains the data.
 *               Note that the buffer is deallocated after leaving this function, so copy it if you need it longer.
 * @param size Size of the body
 */
void blink_callback(MbedCloudClientResource *resource, const uint8_t *buffer, uint16_t size) {
    printf("POST received. Going to blink LED pattern: %s\n", pattern_res->get_value().c_str());

    static DigitalOut augmentedLed(LED1); // LED that is used for blinking the pattern

    // Parse the pattern string, and toggle the LED in that pattern
    string s = std::string(pattern_res->get_value().c_str());
    size_t i = 0;
    size_t pos = s.find(':');
    while (pos != string::npos) {
        wait_ms(atoi(s.substr(i, pos - i).c_str()));
        augmentedLed = !augmentedLed;

        i = ++pos;
        pos = s.find(':', pos);

        if (pos == string::npos) {
            wait_ms(atoi(s.substr(i, s.length()).c_str()));
            augmentedLed = !augmentedLed;
        }
    }
}

/**
 * Notification callback handler
 * @param resource The resource that triggered the callback
 * @param status The delivery status of the notification
 */
void button_callback(MbedCloudClientResource *resource, const NoticationDeliveryStatus status) {
    printf("Button notification, status %s (%d)\n", MbedCloudClientResource::delivery_status_to_string(status), status);
}

/**
 * Registration callback handler
 * @param endpoint Information about the registered endpoint such as the name (so you can find it back in portal)
 */
void registered(const ConnectorClientEndpointInfo *endpoint) {
    printf("Connected to Mbed Cloud. Endpoint Name: %s\n", endpoint->internal_endpoint_name.c_str());
}

int main(void) {
    printf("Starting Simple Mbed Cloud Client example\n");
    printf("Connecting to the network using WiFi...\n");

    //Erase SPI Flash
    printf("Init\n");
    int init_status = sd.init();
    printf("Init status = %d\n", init_status);
    if(init_status == 0) {
        printf("spif size: %llu\n",         sd.size());
        printf("spif read size: %llu\n",    sd.get_read_size());
        printf("spif erase size: %llu\n",   sd.get_erase_size());
    }
    //Un-comment the below block if forcible flash erase is required.
    /*
    int erase_status = sd.erase(0, sd.size());
    if (erase_status == 0) {
         printf("SPIF erased\n");
    }
    */

    // Connect to the internet (DHCP is expected to be on)
    WicedInterface net;
    nsapi_error_t status = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);

    if (status != 0) {
        printf("Connecting to the network failed %d!\n", status);
        return -1;
    }

    printf("Connected to the network successfully. IP address: %s\n", net.get_ip_address());

    // SimpleMbedCloudClient handles registering over LwM2M to Mbed Cloud
    
    //Un-comment the below line while demonstrating remote firmware update. You will need to re-build the application
    //and use the newly built binary for the update.
    printf("\r\n*********This line demonstrates a firmware update*********\r\n");
    
    //create SMCC obj here---------------
    SimpleMbedCloudClient client(&net, &sd, &fs);
    int client_status = client.init();
    printf("client_status = %d\n", client_status);
    if (client_status != 0) {
        printf("Initializing Mbed Cloud Client failed (%d)\n", client_status);
        return -1;
    }

    // Creating resources, which can be written or read from the cloud
    button_res = client.create_resource("3200/0/5501", "button_count");
    button_res->set_value(0);
    button_res->methods(M2MMethod::GET);
    button_res->observable(true);
    button_res->attach_notification_callback(button_callback);

    pattern_res = client.create_resource("3201/0/5853", "blink_pattern");
    pattern_res->set_value("500:500:500:500:500:500:500:500");
    pattern_res->methods(M2MMethod::GET | M2MMethod::PUT);
    pattern_res->attach_put_callback(pattern_updated);

    MbedCloudClientResource *blink_res = client.create_resource("3201/0/5850", "blink_action");
    blink_res->methods(M2MMethod::POST);
    blink_res->attach_post_callback(blink_callback);

    printf("Initialized Mbed Cloud Client. Registering...\n");

    // Callback that fires when registering is complete
    client.on_registered(&registered);

    // Register with Mbed Cloud
    client.register_and_connect();

    // Placeholder for callback to update local resource when GET comes.
    // The timer fires on an interrupt context, but debounces it to the eventqueue, so it's safe to do network operations
    Ticker timer;
    timer.attach(eventQueue.event(&fake_button_press), 5.0);

    // You can easily run the eventQueue in a separate thread if required
    eventQueue.dispatch_forever();
}
