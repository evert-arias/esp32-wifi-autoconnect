/*
 *  Title:   ESP32 Wi-Fi Auto Connect Example 
 *  Version: 1.0.0
 *  Date:    02-10-2019
 *  Author:  Evert Arias | https://github.com/evert-arias
*/

#include <Arduino.h>
#include <WiFi.h>
#include <TaskScheduler.h>

// Wi-Fi Configurations
#define WIFI_SSID "GL-MT300N-V2-413" // Wi-Fi SSID
#define WIFI_PASS "goodlife"         // Wi-Fi Password
#define WIFI_CONNECT_INTERVAL 5000   // Connection retry interval in milliseconds
#define WIFI_WATCHDOG_INTERVAL 5000  // Wi-Fi watchdog interval in milliseconds

// Callback methods prototypes
void wifi_connect_cb();
void wifi_watchdog_cb();

// Tasks
Task wifi_connect_task(WIFI_CONNECT_INTERVAL, TASK_FOREVER, &wifi_connect_cb);
Task wifi_watchdog_task(WIFI_WATCHDOG_INTERVAL, TASK_FOREVER, &wifi_watchdog_cb);

// Task runner
Scheduler runner;

// Wi-Fi events
void _wifi_event(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_WIFI_READY:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi interface ready\n"));
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    // Serial.print(PSTR("[Wi-Fi] Event: Completed scan for access points\n"));
    break;
  case SYSTEM_EVENT_STA_START:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi client started\n"));
    break;
  case SYSTEM_EVENT_STA_STOP:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi clients stopped\n"));
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    // Serial.printf(PSTR("[Wi-Fi] Event: Connected to access point: %s \n"), WiFi.localIP().toString().c_str());
    wifi_connect_task.disable();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    // Serial.print(PSTR("[Wi-Fi] Event: Not connected to Wi-Fi network\n"));
    wifi_connect_task.enable();
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    // Serial.print(PSTR("[Wi-Fi] Event: Authentication mode of access point has changed\n"));
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    // Serial.printf(PSTR("[Wi-Fi] Event: Obtained IP address: %s\n"), WiFi.localIP().toString().c_str());
    wifi_watchdog_task.enable();
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    // Serial.print(PSTR("[Wi-Fi] Event: Lost IP address and IP address is reset to 0\n"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): succeeded in enrollee mode\n"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): failed in enrollee mode\n"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): timeout in enrollee mode\n"));
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi Protected Setup (WPS): pin code in enrollee mode\n"));
    break;
  case SYSTEM_EVENT_AP_START:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi access point started\n"));
    break;
  case SYSTEM_EVENT_AP_STOP:
    // Serial.print(PSTR("[Wi-Fi] Event: Wi-Fi access point  stopped\n"));
    break;
  case SYSTEM_EVENT_AP_STACONNECTED:
    // Serial.print(PSTR("[Wi-Fi] Event: Client connected\n"));
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    // Serial.print(PSTR("[Wi-Fi] Event: Client disconnected\n"));
    break;
  case SYSTEM_EVENT_AP_STAIPASSIGNED:
    // Serial.print(PSTR("[Wi-Fi] Event: Assigned IP address to client\n"));
    break;
  case SYSTEM_EVENT_AP_PROBEREQRECVED:
    // Serial.print(PSTR("[Wi-Fi] Event: Received probe request\n"));
    break;
  case SYSTEM_EVENT_GOT_IP6:
    // Serial.print(PSTR("[Wi-Fi] Event: IPv6 is preferred\n"));
    break;
  case SYSTEM_EVENT_ETH_START:
    // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_START\n"));
    break;
  case SYSTEM_EVENT_ETH_STOP:
    // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_STOP\n"));
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_CONNECTED\n"));
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_DISCONNECTED\n"));
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    // Serial.print(PSTR("[Wi-Fi] Event: SYSTEM_EVENT_ETH_GOT_IP\n"));
    break;
  case SYSTEM_EVENT_MAX:
    break;
  }
}

// Wi-Fi connect task
void wifi_connect_cb()
{
  // Disable this task to avoid further iterations
  wifi_connect_task.disable();

  Serial.println(PSTR("[Wi-Fi] Status: Connecting ..."));

  // Disconnect from Wi-Fi network
  WiFi.disconnect();

  // Connect to Wi-Fi network
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection result and capture the result code
  uint8_t result = WiFi.waitForConnectResult();

  // Serial.printf("[Wi-Fi] Connection Result: %d \n", result);

  if (result == WL_NO_SSID_AVAIL || result == WL_CONNECT_FAILED)
  {
    // Fail to connect or SSID no available
    Serial.println(PSTR("[Wi-Fi] Status: Could not connect to Wi-Fi AP"));
    // Wait and reenable this task to keep trying to connect
    wifi_connect_task.enableDelayed(1000);
  }
  else if (result == WL_IDLE_STATUS)
  {
    // Wi-Fi Idle. This means that it's connected to the AP but the DHCP has not assigned an IP yet
    Serial.println(PSTR("[Wi-Fi] Status: Idle | No IP assigned by DHCP Server"));
  }
  else if (result == WL_CONNECTED)
  {
    // Wi-Fi Connected
    Serial.printf(PSTR("[Wi-Fi] Status: Connected | IP: %s\n"), WiFi.localIP().toString().c_str());
  }
}

// Wi-Fi watchdog task
void wifi_watchdog_cb()
{
  Serial.println(PSTR("[Watchdog] Checking Wi-Fi ..."));
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(PSTR("[Watchdog] Wi-Fi Disconnected"));
    // Disconnect Wi-Fi
    WiFi.disconnect();
    // Disable watchdog task
    wifi_watchdog_task.disable();
    // Enable Wi-Fi connect task to reconnect to Wi-Fi AP
    wifi_connect_task.enableDelayed(2500);
    return;
  }
  Serial.println(PSTR("[Watchdog] Wi-Fi is connected!"));
}

// Setup
void setup()
{
  // Serial
  Serial.begin(115200);

  Serial.println();
  Serial.println(PSTR("### ESP32 WI-FI AUTO CONNECT EXAMPLE ###"));
  Serial.printf(PSTR("[Info] SSID: %s | Password: %s \n"), WIFI_SSID, WIFI_PASS);
  Serial.printf(PSTR("[Info] Connection Interval: %d(ms) \n"), WIFI_CONNECT_INTERVAL);
  Serial.printf(PSTR("[Info] Wi-Fi Watchdog Interval: %d(ms) \n"), WIFI_WATCHDOG_INTERVAL);

  // Prepare task runner
  runner.init();
  runner.addTask(wifi_connect_task);
  runner.addTask(wifi_watchdog_task);

  // Set Wi-Fi mode
  WiFi.mode(wifi_mode_t::WIFI_STA);

  // Wi-Fi events listener
  WiFi.onEvent(_wifi_event);

  // Wait and enable wifi connect task
  wifi_connect_task.enableDelayed(5000);
}

// Loop
void loop()
{
  runner.execute();
}