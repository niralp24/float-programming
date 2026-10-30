#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>

// 🔒 Set your Wi-Fi Network Credentials here
const char* ssid = "ESP32";
const char* password = "12345678"; 

// Define static IP configuration (192.168.4.1)
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Instantiate the web server on port 80
WebServer server(80);

// HTML Template for the Main Dashboard (Displaying Data)
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Data Display</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f9; color: #333; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
        h2 { text-align: center; color: #007BFF; }
        .data-card { background: #f9f9f9; border: 1px solid #ddd; padding: 12px; margin-bottom: 15px; border-radius: 6px; }
        .data-label { font-size: 14px; color: #666; font-weight: bold; margin-bottom: 4px; }
        .data-value { font-size: 18px; color: #222; }
        .ota-link { display: block; text-align: center; margin-top: 20px; font-size: 12px; color: #666; text-decoration: none; }
        .ota-link:hover { text-decoration: underline; }
    </style>
</head>
<body>
    <div class="container">
        <h2>ESP32 Sensor Dashboard</h2>
        
        <div class="data-card">
            <div class="data-label">Data Field 1 (Uptime):</div>
            <div class="data-value">%VAL1%</div>
        </div>
        
        <div class="data-card">
            <div class="data-label">Data Field 2 (Signal RSSI):</div>
            <div class="data-value">%VAL2% dBm</div>
        </div>
        
        <div class="data-card">
            <div class="data-label">Data Field 3 (Status):</div>
            <div class="data-value">%VAL3%</div>
        </div>

        <a class="ota-link" href="/update">Firmware Update Page</a>
    </div>
</body>
</html>
)rawliteral";

// HTML for the Firmware Upload Page
const char UPDATE_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Firmware Update</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f4f9; color: #333; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); text-align: center; }
        h2 { color: #DC3545; }
        input[type="file"] { margin: 20px 0; }
        input[type="submit"] { width: 100%; padding: 10px; background-color: #DC3545; border: none; color: white; font-size: 16px; border-radius: 4px; cursor: pointer; }
        input[type="submit"]:hover { background-color: #bd2130; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Firmware OTA Update</h2>
        <p>Select your compiled <strong>.bin</strong> file to update the device.</p>
        <form method='POST' action='/update' enctype='multipart/form-data' id='upload_form'>
            <input type='file' name='update' accept='.bin' required><br>
            <input type='submit' value='Update Firmware'>
        </form>
    </div>
</body>
</html>
)rawliteral";

// Handler for the Root Page (Injects live ESP32 data into HTML placeholders)
void handleRoot() {
    // Generate real-time data or fetch from sensors
    //String val1 = String(get_depth(), 2) + " m";
    //String val2 = String(get_temp(), 2);
    //String val3 = String(get_lux(), 2);
    String val1 = String(rtc.getSecond() + "." + rtc.getMillis());
    String val2 = "38";
    String val3 = "50";

    // Read the PROGMEM HTML into a String
    String html = FPSTR(HTML_PAGE);

    // Replace placeholders with actual data
    html.replace("%VAL1%", val1);
    html.replace("%VAL2%", val2);
    html.replace("%VAL3%", val3);

    server.send(200, "text/html", html);
}

// Handlers for Firmware Updates
void handleUpdateRoot() {
    server.send(200, "text/html", UPDATE_PAGE);
}

void handleUpdateExecution() {
    bool success = !Update.hasError();
    String response = "<html><body style='font-family:Arial; text-align:center; margin-top:50px;'>";
    if (success) {
        response += "<h2>Update Success!</h2><p>Rebooting device now... Reconnect in 10 seconds.</p>";
    } else {
        response += "<h2>Update Failed!</h2><p>Please try rebuilding your binary file.</p>";
    }
    response += "</body></html>";
    
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", response);
    
    delay(1000);
    if (success) {
        ESP.restart();
    }
}

void handleUpdateUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update started. File: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.printf("Update Success: %u bytes.\nRebooting...\n", upload.totalSize);
        } else {
            Update.printError(Serial);
        }
    }
}

void setup() {
    Serial.begin(115200);
    rtc.setTime(0);
    delay(1000);

    Serial.println("Configuring Access Point...");
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password);

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Main Page Routing (GET only now since it displays data)
    server.on("/", HTTP_GET, handleRoot);

    // OTA Update Routing
    server.on("/update", HTTP_GET, handleUpdateRoot);
    server.on("/update", HTTP_POST, handleUpdateExecution, handleUpdateUpload);

    server.begin();
    Serial.println("HTTP Web Server started.");
}

void loop() {
    server.handleClient();
}