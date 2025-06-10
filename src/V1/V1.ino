#include <Adafruit_NeoPixel.h>
#include <driver/i2s.h>

// --- LED setup ---
#define LED_PIN 48
#define NUM_PIXELS 1
Adafruit_NeoPixel pixels(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- I2S Mic setup ---
#define I2S_WS 45
#define I2S_SD 37
#define I2S_SCK 36
#define I2S_PORT I2S_NUM_0

// --- Clap Detection Parameters ---
const float CLAP_THRESHOLD = 0.5;
unsigned long lastClapTime = 0;
bool ledOn = false;
bool testMode = false; // Set to false for normal operation

// --- I2S Config ---
const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

const i2s_pin_config_t pin_config = {
    .mck_io_num = I2S_PIN_NO_CHANGE,
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD};

void setup()
{
  Serial.begin(115200);

  // Init LED
  pixels.begin();
  pixels.setBrightness(30);
  pixels.show();

  // Diagnostic LED sequence
  for (int i = 0; i < 3; i++)
  {
    pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Blue
    pixels.show();
    delay(300);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // Off
    pixels.show();
    delay(300);
  }

  // Init I2S
  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK)
  {
    Serial.printf("I2S driver install failed: %d\n", err);
    while (1)
    {
      // Flash red to indicate I2S failure
      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.show();
      delay(200);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
      delay(200);
    }
  }

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK)
  {
    Serial.printf("I2S pin config failed: %d\n", err);
    while (1)
    {
      // Flash yellow to indicate pin config failure
      pixels.setPixelColor(0, pixels.Color(255, 255, 0));
      pixels.show();
      delay(200);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.show();
      delay(200);
    }
  }

  // Green LED indicates successful initialization
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();
  delay(1000);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();

  Serial.println("System initialized. Ready for clap detection.");
}

void loop()
{
  int32_t samples[1024];
  size_t bytesRead;

  // Read audio samples
  esp_err_t err = i2s_read(I2S_PORT, samples, sizeof(samples), &bytesRead, portMAX_DELAY);
  if (err != ESP_OK)
  {
    Serial.printf("I2S read error: %d\n", err);
    return;
  }

  int sampleCount = bytesRead / sizeof(int32_t);
  float maxSample = 0;
  float avgSample = 0;

  // Analyze samples
  for (int i = 0; i < sampleCount; i++)
  {
    float sample = (float)samples[i] / INT32_MAX;
    avgSample += abs(sample);
    if (abs(sample) > maxSample)
    {
      maxSample = abs(sample);
    }
  }
  avgSample /= sampleCount;

  // Testing mode - continuously show audio levels
  if (testMode)
  {
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 100)
    { // Print every 100ms
      Serial.printf("Max: %.4f | Avg: %.4f | Threshold: %.2f\n", maxSample, avgSample, CLAP_THRESHOLD);
      lastPrint = millis();

      // Visual feedback - LED brightness proportional to sound level
      int brightness = min(255, (int)(maxSample * 255 * 2));
      pixels.setPixelColor(0, pixels.Color(brightness, brightness, brightness));
      pixels.show();
    }

    // In test mode, don't do clap detection
    return;
  }

  // Normal clap detection mode
  if (maxSample > CLAP_THRESHOLD)
  {
    unsigned long now = millis();
    if (now - lastClapTime > 100)
    { // Debounce
      Serial.printf("Clap detected! Amplitude: %.2f\n", maxSample);

      ledOn = !ledOn;
      if (ledOn)
      {
        pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // ON
      }
      else
      {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0)); // OFF
      }
      pixels.show();

      lastClapTime = now;
    }
  }

  delay(10);
}