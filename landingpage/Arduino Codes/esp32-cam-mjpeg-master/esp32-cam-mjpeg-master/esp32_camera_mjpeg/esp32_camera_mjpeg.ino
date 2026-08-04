#include "src/OV2640.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

#define SSID1 "Kishore"
#define PWD1 "12345678"

OV2640 cam;
WebServer server(80);

void handle_jpg_stream(void) {
  char buf[32];
  int s;

  WiFiClient client = server.client();

  const char* HEADER = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: multipart/x-mixed-replace; boundary=123456789000000000000987654321\r\n";
  const char* BOUNDARY = "\r\n--123456789000000000000987654321\r\n";
  const char* CTNTTYPE = "Content-Type: image/jpeg\r\nContent-Length: ";
  int hdrLen = strlen(HEADER);
  int bdrLen = strlen(BOUNDARY);
  int cntLen = strlen(CTNTTYPE);

  client.write(HEADER, hdrLen);
  client.write(BOUNDARY, bdrLen);

  while (true) {
    if (!client.connected()) break;
    cam.run();
    s = cam.getSize();
    client.write(CTNTTYPE, cntLen);
    sprintf(buf, "%d\r\n\r\n", s);
    client.write(buf, strlen(buf));
    client.write((char*)cam.getfb(), s);
    client.write(BOUNDARY, bdrLen);
  }
}

void handle_jpg(void) {
  WiFiClient client = server.client();

  cam.run();
  if (!client.connected()) return;

  const char* JHEADER = "HTTP/1.1 200 OK\r\nContent-disposition: inline; filename=capture.jpg\r\nContent-type: image/jpeg\r\n\r\n";
  int jhdLen = strlen(JHEADER);

  client.write(JHEADER, jhdLen);
  client.write((char*)cam.getfb(), cam.getSize());
}

void handleNotFound() {
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  server.send(200, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Give some time to initialize Serial

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000; // 20 MHz
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters
config.frame_size = FRAMESIZE_QVGA; // Use lower resolution
config.jpeg_quality = 12; // Use medium quality
config.fb_count = 2; // Use fewer frame buffers


  Serial.printf("XCLK: %d, PCLK: %d, VSYNC: %d, HREF: %d, SIOD: %d, SIOC: %d\n",
                config.pin_xclk, config.pin_pclk, config.pin_vsync, config.pin_href,
                config.pin_sscb_sda, config.pin_sscb_scl);
  Serial.printf("D0: %d, D1: %d, D2: %d, D3: %d, D4: %d, D5: %d, D6: %d, D7: %d\n",
                config.pin_d0, config.pin_d1, config.pin_d2, config.pin_d3, config.pin_d4,
                config.pin_d5, config.pin_d6, config.pin_d7);
  Serial.printf("PWDN: %d, RESET: %d\n", config.pin_pwdn, config.pin_reset);

  // Initialize the camera
  esp_err_t err = cam.init(config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  IPAddress ip;

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID1, PWD1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(ip);
  Serial.print("Stream Link: http://");
  Serial.print(ip);
  Serial.println("/mjpeg/1");

  server.on("/mjpeg/1", HTTP_GET, handle_jpg_stream);
  server.on("/jpg", HTTP_GET, handle_jpg);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}
