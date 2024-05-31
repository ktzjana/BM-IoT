#include <iostream>
#include <mosquitto.h>
#include <jsoncpp/json/json.h>
#include <httplib.h>

const char *mqtt_host = "localhost";
const int mqtt_port = 1883;
const char *topic_servo = "actuators/servo";

void notifyEnvironment(const std::string &state) {
    httplib::Client cli("http://localhost:8080");
    httplib::Params params;
    params.emplace("state", state);
    auto res = cli.Post("/update_servo_state", params);
    if (res && res->status == 200) {
        std::cout << "Notified environment: Servo state " << state << std::endl;
    } else {
        std::cerr << "Failed to notify environment" << std::endl;
    }
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    std::string payload(static_cast<char *>(message->payload), message->payloadlen);
    std::cout << "Message received on topic: " << message->topic << std::endl;
    std::cout << "Payload: " << payload << std::endl;

    if (payload == "CURTAIN ACTIVE") {
        std::cout << "Servo actuator turned to CURTAIN ACTIVE" << std::endl;
        notifyEnvironment("CURTAIN ACTIVE");
    } else if (payload == "CURTAIN ON") {
        std::cout << "Servo actuator turned to CURTAIN ON" << std::endl;
        notifyEnvironment("CURTAIN ON");
    } else if (payload == "CURTAIN OFF") {
        std::cout << "Servo actuator turned to CURTAIN OFF" << std::endl;
        notifyEnvironment("CURTAIN OFF");
    }
}

int main() {
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        std::cerr << "Error: Unable to initialize MQTT client.\n";
        return 1;
    }

    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: Unable to connect to MQTT broker.\n";
        return 1;
    }

    mosquitto_subscribe(mosq, NULL, topic_servo, 0);

    int loop = mosquitto_loop_forever(mosq, -1, 1);
    if (loop != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: " << mosquitto_strerror(loop) << std::endl;
        return 1;
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

