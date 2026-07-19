#pragma once

#include <Arduino.h>
#include <time.h>

namespace MeterConfig {

namespace Wifi {
constexpr char SSID[] = "<Wi-Fi SSID>";
constexpr char PASSWORD[] = "<Wi-Fi password>";
constexpr char HOSTNAME[] = "ESP32-PM5350";
constexpr unsigned long RECONNECT_INTERVAL_MS = 10000;
constexpr bool SLEEP_ENABLED = false;
}

namespace Http {
constexpr uint16_t PORT = 80;
}

namespace Ntp {
constexpr char TIMEZONE[] = "UTC0";
constexpr char PRIMARY_SERVER[] = "pool.ntp.org";
constexpr char SECONDARY_SERVER[] = "time.nist.gov";
constexpr time_t MIN_VALID_EPOCH = 1704067200;  // 2024-01-01T00:00:00Z
}

namespace Logging {
constexpr unsigned long BAUD_RATE = 9600;
}

namespace Service {
constexpr unsigned long READ_INTERVAL_MS = 5000;
constexpr uint32_t READ_TASK_STACK_SIZE = 6144;
constexpr UBaseType_t READ_TASK_PRIORITY = 1;
}

namespace Pm5350 {
constexpr uint8_t SLAVE_ID = 2;
constexpr int RX_PIN = 16;
constexpr int TX_PIN = 17;
constexpr int DE_PIN = 19;
constexpr int RE_PIN = 18;
constexpr unsigned long BAUD_RATE = 9600;
constexpr uint32_t SERIAL_FORMAT = SERIAL_8E1;
constexpr char SERIAL_FORMAT_NAME[] = "8E1";
constexpr uint8_t MAX_READ_ATTEMPTS = 2;
constexpr uint8_t MAX_VALUE_READ_ATTEMPTS = 2;
constexpr unsigned long RTU_SILENT_INTERVAL_MS = 5;
constexpr unsigned long RETRY_DELAY_MS = 20;
constexpr unsigned long VALUE_RETRY_DELAY_MS = 500;
constexpr float MIN_POWER_FACTOR_ACTIVE_POWER_KW = 0.02f;
}

namespace Am2320 {
constexpr int SDA_PIN = 32;
constexpr int SCL_PIN = 33;
constexpr uint32_t I2C_CLOCK_HZ = 100000;
constexpr uint8_t MAX_READ_ATTEMPTS = 3;
constexpr unsigned long RETRY_DELAY_MS = 20;
}

}  // namespace MeterConfig
