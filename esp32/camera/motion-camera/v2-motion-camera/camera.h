#include "esp_camera.h"
#include "camera_pins.h"

#include "fd_forward.h"

void getSettings();

bool cameraOK = false;
bool *cameraInUse = new bool(false);

enum cameraModes { isReady, isStream, isCapture, isMotion };
cameraModes *cameraMode = new cameraModes;

bool initCamera() {
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
    //config.xclk_freq_hz = 20000000;
    config.xclk_freq_hz = 10000000;
    //config.xclk_freq_hz = 5000000;

    config.jpeg_quality = 10;
    config.fb_count = 1;

    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;

    cameraOK = esp_camera_init(&config) == ESP_OK;
    *cameraMode = isReady;
    *cameraInUse = false;
    return cameraOK;
}

void flash(bool on) {
    gpio_pad_select_gpio(GPIO_NUM_4);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_4, on ? 1 : 0);
}

void cameraControl(cameraModes current) {
    *cameraMode = current;
    *cameraInUse = true;
}

void cameraRelease(cameraModes current) {
    if (*cameraMode == current) *cameraMode = isReady;
    *cameraInUse = false;
}

void bufferRelease(camera_fb_t* fb) {
    uint8_t *buf;
    buf = fb->buf;
    *buf = NULL;
    esp_camera_fb_return(fb);
}

camera_fb_t* capture(uint8_t*& _jpg_buf, size_t& _jpg_buf_len) {
    _jpg_buf_len = 0;

    camera_fb_t *fb = esp_camera_fb_get();

    if (fb) {
        if(fb->format == PIXFORMAT_JPEG) {
            _jpg_buf = fb->buf;
            _jpg_buf_len = fb->len;
        } else frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);

    } else loggerln("Camera capture failed");

    return fb;
}

void updateParam(String param, int value) {
    loggerln(param);
    loggerln(value);

    sensor_t *s = esp_camera_sensor_get();

    if(param == "framesize") {
        s->set_framesize(s, (framesize_t) value);
    } else if (param ==  "hmirror") {
        s->set_hmirror(s, value);
    } else if (param == "vflip") {
        s->set_vflip(s, value);
    } else if (param == "quality") {
        s->set_quality(s, value);
    } else if (param == "contrast") {
        s->set_contrast(s, value);
    } else if (param == "brightness") {
        s->set_brightness(s, value);
    } else if (param == "saturation") {
        s->set_saturation(s, value);
    } else if (param == "gainceiling") {
        s->set_gainceiling(s, (gainceiling_t) value);
    } else if (param == "colorbar") {
        s->set_colorbar(s, value);
    } else if (param == "awb") {
        s->set_whitebal(s, value);
    } else if (param == "agc") {
        s->set_gain_ctrl(s, value);
    } else if (param == "aec") {
        s->set_exposure_ctrl(s, value);
    } else if (param == "awb_gain") {
        s->set_awb_gain(s, value);
    } else if (param == "agc_gain") {
        s->set_agc_gain(s, value);
    } else if (param == "aec_value") {
        s->set_aec_value(s, value);
    } else if (param == "aec2") {
       s->set_aec2(s, value);
    } else if (param == "dcw") {
        s->set_dcw(s, value);
    } else if (param == "bpc") {
        s->set_bpc(s, value);
    } else if (param == "wpc") {
        s->set_wpc(s, value);
    } else if (param == "lenc") {
        s->set_lenc(s, value);
    } else if (param == "raw_gma") {
        s->set_raw_gma(s, value);
    } else if (param == "special_effect") {
        s->set_special_effect(s, value);
    } else if (param == "wb_mode") {
        s->set_wb_mode(s, value);
    } else if (param == "ae_level") {
        s->set_ae_level(s, value);
    }
}

String configJSON() {
    StaticJsonDocument<1024> doc;
    JsonObject config = doc.to<JsonObject>();

    sensor_t *s = esp_camera_sensor_get();
    config["framesize"] = s->status.framesize;
    config["quality"] = s->status.quality;
    config["brightness"] = s->status.brightness;
    config["contrast"] = s->status.contrast;
    config["saturation"] = s->status.saturation;
    config["sharpness"] = s->status.sharpness;
    config["special_effect"] = s->status.special_effect;
    config["wb_mode"] = s->status.wb_mode;
    config["awb"] = s->status.awb;
    config["awb_gain"] = s->status.awb_gain;
    config["aec"] = s->status.aec;
    config["aec2"] = s->status.aec2;
    config["ae_level"] = s->status.ae_level;
    config["aec_value"] = s->status.aec_value;
    config["agc"] = s->status.agc;
    config["agc_gain"] = s->status.agc_gain;
    config["gainceiling"] = s->status.gainceiling;
    config["bpc"] = s->status.bpc;
    config["wpc"] = s->status.wpc;
    config["raw_gma"] = s->status.raw_gma;
    config["lenc"] = s->status.lenc;
    config["vflip"] = s->status.vflip;
    config["hmirror"] = s->status.hmirror;
    config["dcw"] = s->status.dcw;
    config["colorbar"] = s->status.colorbar;

    String json;
    serializeJson(config, json);
    return json;
}
