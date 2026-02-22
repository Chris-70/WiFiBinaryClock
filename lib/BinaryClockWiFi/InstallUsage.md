# BinaryClockWiFi Library - Installation and Usage Guide

This document provides detailed instructions on how to install and use the BinaryClockWiFi library, which is part of the WiFi Binary Clock project. It covers installation steps for both PlatformIO and Arduino IDE, as well as examples of how to utilize the library's features for WiFi connectivity, SNTP time synchronization, WPS setup, and persistent settings management.

## Additional Documents
- [**`ClassDiagram.md`**][CLASS_DIAGRAM] (GitHub: [`ClassDiagram.md`][CLASS_DIAGRAM_GIT]) - Class diagram and structure of the **BinaryClockWiFi library**.
- [**`README.md`**][README] (GitHub: [`README.md`][README_Git]) - Overview and usage instructions for the **BinaryClockWiFi library**.
---

## Installation

### PlatformIO

Add to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/Chris-70/WiFiBinaryClock.git#lib/BinaryClockWiFi
```

### Arduino IDE

1. **Download the Repository**:
   - Go to https://github.com/Chris-70/WiFiBinaryClock
   - Click the green **Code** button and select **Download ZIP**
   - Extract the ZIP file to a temporary location

2. **Install the BinaryClockWiFi Library**:
   - Open Arduino IDE
   - Go to **Sketch → Include Library → Add .ZIP Library...**
   - Navigate to the extracted folder: `WiFiBinaryClock-main/lib/BinaryClockWiFi`
   - Compress the `BinaryClockWiFi` folder into a ZIP file if needed
   - Select the ZIP file and click **Open**

3. **Install Dependencies**:
   Repeat step 2 for each required dependency:
   - `lib/BCGlobalDefines` (from the same repository)
   - `lib/BinaryClock` (from the same repository)
   - `lib/RTClibPlus` (from the same repository)

4. **Install External Libraries**:
   - Go to **Sketch → Include Library → Manage Libraries...**
   - The BinaryClockWiFi library requires the ESP32 Arduino Core, which provides WiFi, NVS, and SNTP support

5. **Verify Installation**:
   - Go to **File → Examples → BinaryClockWiFi**
   - Open an example sketch and compile to verify installation

**Note:** For easier dependency management and better build configuration, we recommend using PlatformIO instead of Arduino IDE.

### Dependencies

This library requires:
- **`BCGlobalDefines`**: Global structures and interfaces
- **`BinaryClock`**: Implementation of the `IBinaryClock` interface
- **`ESP32 Arduino Core`**: WiFi, NVS, SNTP support

## Quick Start

### Basic WiFi Connection

```cpp
#include <BinaryClock.h>
#include <BinaryClockWAN.h>

BinaryClock& clock = BinaryClock::get_Instance();
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();

void setup() {
    Serial.begin(115200);
    clock.setup();
    
    // Begin WiFi with auto-connect
    if (wan.Begin(clock, true)) {
        Serial.println("WiFi connected!");
        Serial.println(wan.get_LocalIP());
    }
}

void loop() {
    clock.loop();
}
```

### WPS Setup

```cpp
#include <BinaryClockWPS.h>
#include <BinaryClockSettings.h>

void setupWiFiWithWPS() {
    BinaryClockWPS& wps = BinaryClockWPS::get_Instance();
    BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
    
    Serial.println("Press WPS button on your router...");
    
    WPSResult result = wps.ConnectWPS();
    
    if (result.success) {
        Serial.println("WPS connection successful!");
        Serial.print("SSID: ");
        Serial.println(result.credentials.ssid);
        
        // Save credentials for future use
        settings.AddWiFiCreds(result.credentials);
        settings.Save();
    } else {
        Serial.print("WPS failed: ");
        Serial.println(result.errorMessage);
    }
}
```

### Manual Credential Entry

```cpp
#include <BinaryClockSettings.h>

void addWiFiCredentials() {
    BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
    settings.Begin();
    
    // Add WiFi credentials
    uint8_t id = settings.AddWiFiCreds(
        "YourSSID",           // SSID
        "YourPassword",       // Password
        "AA:BB:CC:DD:EE:FF"   // BSSID (optional, use "" for any)
    );
    
    settings.Save();
    Serial.print("Credential ID: ");
    Serial.println(id);
}
```

## Core Classes

### BinaryClockWAN

The central coordinator managing all WiFi operations.

#### Key Methods

```cpp
// Get singleton instance
static BinaryClockWAN& get_Instance();

// Initialize and optionally auto-connect
bool Begin(IBinaryClock& clock, bool autoConnect = true);

// Connect to stored credentials
bool ConnectLocal();

// Connect with specific credentials
bool Connect(APCreds& creds);

// Setup SNTP service
bool ConnectSNTP();

// Force time synchronization
bool UpdateTime();

// End WiFi and optionally save settings
void End(bool save = false);

// Scan for available networks
static std::vector<WiFiInfo> GetAvailableNetworks();
```

#### Properties

```cpp
// Get connection status
bool get_IsConnected();

// Get assigned IP address
IPAddress get_LocalIP();

// Get/Set timezone string (POSIX format)
String get_Timezone();
void set_Timezone(String tz);

// Get current WiFi credentials
APCreds get_WiFiCreds();

// Get/Set NTP servers
std::vector<String> get_NtpServers();
void set_NtpServers(std::vector<String> servers);
```

#### Example: Connection with Status

```cpp
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();

if (wan.Begin(clock)) {
    if (wan.get_IsConnected()) {
        Serial.print("Connected to: ");
        Serial.println(wan.get_WiFiCreds().ssid);
        Serial.print("IP Address: ");
        Serial.println(wan.get_LocalIP());
        
        // Force time sync
        if (wan.UpdateTime()) {
            Serial.println("Time synchronized!");
        }
    }
}
```

---

### BinaryClockNTP

Manages SNTP time synchronization with multiple server support.

#### Key Methods

```cpp
// Get singleton instance
static BinaryClockNTP& get_Instance();

// Initialize SNTP service
void Begin(std::vector<String> servers, 
           size_t serverCount = 3, 
           bool startSyncing = true);

// End SNTP service
void End();

// Force synchronization
NTPResult SyncTime();

// Static one-off NTP query
static NTPResult SyncTime(String server, uint16_t port = 123);

// Register callback for sync events
bool RegisterSyncCallback(std::function<void(DateTime&)> callback);

// Check sync status
bool isTimeSynchronized();
sntp_sync_status_t get_SyncStatus();
```

#### Properties

```cpp
// Get current NTP time
DateTime get_LocalNtpTime();    // Local timezone
DateTime get_CurrentNtpTime();  // UTC

// Get/Set NTP servers
std::vector<String> get_NtpServers();
void set_NtpServers(std::vector<String> servers);

// Get/Set sync interval (milliseconds)
unsigned long get_SyncInterval();
void set_SyncInterval(unsigned long ms);

// Get/Set timeout (milliseconds)
uint32_t get_Timeout();
void set_Timeout(uint32_t ms);

// Timezone (POSIX format)
static void set_Timezone(char* tz);
static char* get_Timezone();
```

#### Example: NTP Configuration

```cpp
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();

// Configure NTP servers
std::vector<String> servers = {
    "time.nrc.ca",
    "pool.ntp.org",
    "time.nist.gov"
};

ntp.Begin(servers, 3, true);

// Set timezone (Eastern Time with DST)
ntp.set_Timezone("EST+5EDT,M3.2.0/2,M11.1.0/2");

// Set sync interval to 1 hour
ntp.set_SyncInterval(3600000);

// Register callback
ntp.RegisterSyncCallback([](DateTime& time) {
    Serial.print("Time synced: ");
    char buf[32];
    Serial.println(time.toString(buf, 32, "YYYY-MM-DD HH:mm:ss"));
});
```

#### Timezone Format

Uses POSIX timezone strings:
```
std offset [dst [offset][,start[/time],end[/time]]]
```

**Examples:**
- `"EST+5EDT,M3.2.0/2,M11.1.0/2"` - US Eastern Time
- `"PST+8PDT,M3.2.0/2,M11.1.0/2"` - US Pacific Time
- `"CET-1CEST,M3.5.0,M10.5.0/3"` - Central European Time
- `"GMT0"` - UTC
- `"JST-9"` - Japan Standard Time (no DST)

---

### BinaryClockWPS

Handles WPS (WiFi Protected Setup) push-button connections.

#### Key Methods

```cpp
// Get singleton instance
static BinaryClockWPS& get_Instance();

// Start WPS connection (blocking)
WPSResult ConnectWPS();

// Cancel ongoing WPS attempt
void CancelWPS();
```

#### Properties

```cpp
// Check if WPS is in progress
bool get_IsConnecting();

// Get/Set WPS timeout (milliseconds)
uint32_t get_Timeout();
void set_Timeout(uint32_t ms);
```

#### WPSResult Structure

```cpp
struct WPSResult {
    bool success;              // Connection success flag
    APCreds credentials;       // Retrieved credentials
    String errorMessage;       // Error details if failed
    uint32_t connectionTimeMs; // Time taken to connect
};
```

#### Example: WPS Connection Flow

```cpp
BinaryClockWPS& wps = BinaryClockWPS::get_Instance();
BinaryClockSettings& settings = BinaryClockSettings::get_Instance();

// Set 2-minute timeout
wps.set_Timeout(120000);

Serial.println("Starting WPS...");
Serial.println("Press WPS button on router within 2 minutes");

WPSResult result = wps.ConnectWPS();

if (result.success) {
    Serial.printf("Connected in %d ms\n", result.connectionTimeMs);
    Serial.printf("SSID: %s\n", result.credentials.ssid.c_str());
    
    // Save for future connections
    settings.AddWiFiCreds(result.credentials);
    settings.Save();
} else {
    Serial.printf("WPS failed: %s\n", result.errorMessage.c_str());
}
```

---

### BinaryClockSettings

Manages persistent storage of WiFi credentials and settings in ESP32 NVS.

#### Key Methods

```cpp
// Get singleton instance
static BinaryClockSettings& get_Instance();

// Initialize NVS
void Begin();

// Save all settings to NVS
bool Save();

// End and optionally save
void End(bool save = true);

// Clear all stored settings
void Clear();

// Add WiFi credentials
uint8_t AddWiFiCreds(String ssid, String password, String bssid = "");
uint8_t AddWiFiCreds(APCreds& creds);

// Retrieve credentials
APCredsPlus GetWiFiAP(uint8_t id);
APCredsPlus GetWiFiAP(APNames& ap);
std::vector<APCredsPlus> GetWiFiAPs(String ssid);

// Delete/Undelete credentials
bool DeleteID(uint8_t id);
bool UndeleteID(uint8_t id);

// Get credential ID
uint8_t GetID(APNames& ap);
```

#### Properties

```cpp
// Get/Set timezone
String get_Timezone();
void set_Timezone(String tz);

// Check if settings have been modified
bool get_Modified();
```

#### Example: Credential Management

```cpp
BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
settings.Begin();

// Add multiple networks
uint8_t homeId = settings.AddWiFiCreds("HomeNetwork", "password123");
uint8_t workId = settings.AddWiFiCreds("WorkNetwork", "work_pass");
uint8_t coffeeId = settings.AddWiFiCreds("CoffeeShop", "");  // Open network

// Retrieve all stored APs
auto homeAP = settings.GetWiFiAP(homeId);
Serial.printf("Home AP: %s (ID: %d)\n", homeAP.ssid.c_str(), homeAP.id);

// Delete a credential
settings.DeleteID(coffeeId);

// Save changes
if (settings.get_Modified()) {
    settings.Save();
}
```

#### Multiple SSIDs with Different BSSIDs

The library supports storing multiple credentials for the same SSID (e.g., multiple routers with same name):

```cpp
// Same SSID, different BSSIDs
uint8_t router1 = settings.AddWiFiCreds("MyNetwork", "pass", "AA:BB:CC:DD:EE:FF");
uint8_t router2 = settings.AddWiFiCreds("MyNetwork", "pass", "11:22:33:44:55:66");

// Retrieve all "MyNetwork" credentials
auto networkCreds = settings.GetWiFiAPs("MyNetwork");
Serial.printf("Found %d credentials for MyNetwork\n", networkCreds.size());
```

## Advanced Usage

### Time Synchronization with Callbacks

```cpp
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();

// Register sync callback
ntp.RegisterSyncCallback([](DateTime& syncTime) {
    Serial.println("=== Time Synchronized ===");
    
    char buf[32];
    Serial.print("UTC Time: ");
    Serial.println(syncTime.toString(buf, 32, "YYYY-MM-DD HH:mm:ss"));
    
    // Update displays, save to RTC, etc.
});

// Start SNTP with callbacks enabled
std::vector<String> servers = {"pool.ntp.org"};
ntp.Begin(servers, 1, true);
```

### FreeRTOS EventGroup Integration

```cpp
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

EventGroupHandle_t myEventGroup = xEventGroupCreate();

// Configure NTP event bits
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();
NTPEventBits ntpBits(8);  // Start at bit 8

ntp.set_NtpEventGroup(myEventGroup);
ntp.set_NtpEventBits(&ntpBits);

// Wait for sync in another task
EventBits_t bits = xEventGroupWaitBits(
    myEventGroup,
    ntpBits.get_SyncedMask(),
    pdTRUE,   // Clear on exit
    pdFALSE,  // Wait for any bit
    pdMS_TO_TICKS(30000)
);

if (bits & ntpBits.get_SyncedMask()) {
    Serial.println("NTP sync completed!");
}
```

### Network Scanning and Selection

```cpp
BinaryClockSettings& settings = BinaryClockSettings::get_Instance();

// Scan for available networks
auto availableAPs = BinaryClockWAN::GetAvailableNetworks();

Serial.println("Available Networks:");
for (const auto& ap : availableAPs) {
    Serial.printf("  %s (RSSI: %d, Ch: %d)\n", 
                  ap.ssid.c_str(), ap.rssi, ap.channel);
}

// Match against stored credentials
auto matches = settings.GetWiFiAPs(availableAPs);

for (const auto& [creds, info] : matches) {
    Serial.printf("Stored: %s (RSSI: %d)\n", 
                  creds.ssid.c_str(), info.rssi);
}

// Connect to best match (highest RSSI)
if (!matches.empty()) {
    auto best = std::max_element(matches.begin(), matches.end(),
        [](const auto& a, const auto& b) {
            return a.second.rssi < b.second.rssi;
        });
    
    BinaryClockWAN& wan = BinaryClockWAN::get_Instance();
    wan.Connect(best->first);
}
```

### Custom NTP Server Configuration

```cpp
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();

// Use custom NTP servers
std::vector<String> customServers = {
    "192.168.1.100",      // Local NTP server
    "time.cloudflare.com", // Cloudflare NTP
    "time.google.com"     // Google NTP
};

ntp.set_NtpServers(customServers);

// Configure sync parameters
ntp.set_SyncInterval(3600000);  // Sync every hour
ntp.set_Timeout(5000);          // 5-second timeout

// One-off query to specific server
NTPResult result = BinaryClockNTP::SyncTime("time.windows.com");
if (result.success) {
    char buf[32];
    Serial.print("Time from Windows: ");
    Serial.println(result.dateTime.toString(buf, 32, "HH:mm:ss"));
}
```

### Persistent Settings with Validation

```cpp
BinaryClockSettings& settings = BinaryClockSettings::get_Instance();

void saveSettings() {
    // Add credentials
    uint8_t id = settings.AddWiFiCreds("MyNetwork", "password");
    
    // Set timezone
    settings.set_Timezone("PST+8PDT,M3.2.0/2,M11.1.0/2");
    
    // Check if modified
    if (settings.get_Modified()) {
        if (settings.Save()) {
            Serial.println("Settings saved successfully");
        } else {
            Serial.println("Failed to save settings");
        }
    }
}

void loadAndValidate() {
    settings.Begin();
    
    // Retrieve and validate
    APCredsPlus creds = settings.GetWiFiAP(1);
    if (!creds.ssid.isEmpty()) {
        Serial.printf("Loaded: %s\n", creds.ssid.c_str());
    } else {
        Serial.println("No credentials stored with ID 1");
    }
    
    String tz = settings.get_Timezone();
    Serial.printf("Timezone: %s\n", tz.c_str());
}
```

## Data Structures

### WiFiInfo
Scanned network information:
```cpp
struct WiFiInfo {
    String ssid;          // Network name
    int32_t rssi;         // Signal strength
    String ipAddress;     // IP if connected
    String macAddress;    // MAC address
    uint8_t channel;      // WiFi channel
    String bssid;         // Router MAC (BSSID)
};
```

### APCreds
Basic WiFi credentials:
```cpp
struct APCreds : public APNames {
    String ssid;          // Network name
    String password;      // WiFi password
    // Inherited: MAC address fields
};
```

### APCredsPlus
Extended credentials with metadata:
```cpp
struct APCredsPlus : public APCreds {
    uint8_t priority;     // Connection priority
    uint8_t channel;      // Preferred channel
    String bssid;         // Specific router MAC
    uint8_t id;           // Unique identifier
};
```

### NTPResult
Result from NTP synchronization:
```cpp
struct NTPResult {
    bool success;         // Sync success flag
    DateTime dateTime;    // Synchronized time
    String serverUsed;    // Server that responded
    String errorMessage;  // Error if failed
    NtpPacket packet;     // Full NTP packet
};
```

### WPSResult
Result from WPS connection:
```cpp
struct WPSResult {
    bool success;              // Connection success
    APCreds credentials;       // Retrieved credentials
    String errorMessage;       // Error description
    uint32_t connectionTimeMs; // Connection duration
};
```

## Design Patterns

### Singleton Pattern
All classes use thread-safe singletons:
```cpp
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();
BinaryClockWPS& wps = BinaryClockWPS::get_Instance();
BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
```

### Dependency Injection
`BinaryClockWAN` accepts `IBinaryClock` interface:
```cpp
class MyCustomClock : public IBinaryClock {
    // Implementation
};

MyCustomClock customClock;
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();
wan.Begin(customClock);  // Injected dependency
```

### Repository Pattern
`BinaryClockSettings` abstracts NVS storage:
- Add/retrieve/delete operations
- Validation and serialization
- Automatic persistence

### Event-Driven Communication
FreeRTOS EventGroups for async coordination:
- NTP sync notifications
- Connection state changes
- Task synchronization

## Best Practices

### Initialization Order
```cpp
void setup() {
    Serial.begin(115200);
    
    // 1. Initialize clock
    BinaryClock& clock = BinaryClock::get_Instance();
    clock.setup();
    
    // 2. Load settings
    BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
    settings.Begin();
    
    // 3. Start WiFi (auto-connects)
    BinaryClockWAN& wan = BinaryClockWAN::get_Instance();
    wan.Begin(clock, true);
}
```

### Error Handling
```cpp
BinaryClockWAN& wan = BinaryClockWAN::get_Instance();

if (!wan.Begin(clock)) {
    Serial.println("WiFi initialization failed");
    // Try WPS setup
    setupWiFiWithWPS();
}

if (!wan.get_IsConnected()) {
    Serial.println("Not connected to WiFi");
    // Fallback: AP mode, manual entry, etc.
}
```

### Resource Management
```cpp
void shutdown() {
    BinaryClockWAN& wan = BinaryClockWAN::get_Instance();
    BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();
    BinaryClockSettings& settings = BinaryClockSettings::get_Instance();
    
    // Cleanup in reverse order
    wan.End(true);          // Save settings
    ntp.End();
    settings.End(false);    // Already saved
}
```

## Troubleshooting

### WiFi Won't Connect
```cpp
// Check credentials
APCredsPlus creds = settings.GetWiFiAP(1);
Serial.printf("SSID: %s\n", creds.ssid.c_str());
Serial.printf("Password: %s\n", creds.password.c_str());

// Scan for network
auto networks = BinaryClockWAN::GetAvailableNetworks();
bool found = false;
for (const auto& net : networks) {
    if (net.ssid == creds.ssid) {
        found = true;
        Serial.printf("Network found! RSSI: %d\n", net.rssi);
    }
}
if (!found) Serial.println("Network not in range");
```

### NTP Sync Fails
```cpp
BinaryClockNTP& ntp = BinaryClockNTP::get_Instance();

// Check status
auto status = ntp.get_SyncStatus();
Serial.println(ntp.SyncStatusToString(status));

// Verify timezone
Serial.printf("Timezone: %s\n", ntp.get_Timezone());

// Test with known-good server
NTPResult result = BinaryClockNTP::SyncTime("pool.ntp.org");
if (!result.success) {
    Serial.println(result.errorMessage);
}
```

### WPS Timeout
```cpp
BinaryClockWPS& wps = BinaryClockWPS::get_Instance();

// Increase timeout
wps.set_Timeout(180000);  // 3 minutes

// Monitor status
Serial.println("Starting WPS...");
WPSResult result = wps.ConnectWPS();
Serial.printf("Time taken: %d ms\n", result.connectionTimeMs);
```

## Performance Considerations

### Memory Usage
- **Flash**: ~40-50 KB (all four classes)
- **RAM**: ~5-10 KB (vectors, buffers, state)
- **NVS**: ~2 KB per stored AP credential

### WiFi Scan Time
- Typical: 1-3 seconds
- Depends on: Number of channels, APs in range

### NTP Sync Time
- Typical: 100-500 ms
- Depends on: Network latency, server load
- Timeout configurable (default: 5 seconds)

### WPS Connection Time
- Typical: 15-60 seconds
- Maximum: Configurable timeout (default: 120 seconds)

## Version History

- **v0.9.4** - Current version
  - Full WiFi, WPS, SNTP, and Settings integration
  - FreeRTOS EventGroup support
  - Multiple NTP server support
  - Persistent NVS storage
  - Timezone management

## Author

**Chris-70** (2025)

Created for the WiFi Binary Clock Shield project.

## License

Part of the WiFi Binary Clock project. See project repository for license details.

## Links

- **WiFi Binary Clock Project**: https://github.com/Chris-70/WiFiBinaryClock
- **BinaryClockWiFi Library**: https://github.com/Chris-70/WiFiBinaryClock/tree/main/lib/BinaryClockWiFi
- **Class Diagram**: [BinaryClockWiFi_LibClassDiagram.md](BinaryClockWiFi_LibClassDiagram.md)
- **Dependencies**:
  - [BCGlobalDefines](../BCGlobalDefines)
  - [BinaryClock](../BinaryClock)
  - [RTClibPlus](../RTClibPlus)

---

**Stay connected. Stay synchronized. ⏰📡**

<!-- Reference Links -->
[CLASS_DIAGRAM]: ClassDiagram.md
[CLASS_DIAGRAM_GIT]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClockWiFi/ClassDiagram.md
[README]: README.md
[README_Git]: https://github.com/Chris-70/WiFiBinaryClock/blob/main/lib/BinaryClockWiFi/README.md
