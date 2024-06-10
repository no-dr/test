#include "CRtspSession.h"
#include "OV5640Streamer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "i2s_recorder.h"
#include "hcsr04.h"
#include "oled.h"

#define PORT    3333

#ifdef CONFIG_FRAMESIZE_QVGA
#define CAM_FRAMESIZE FRAMESIZE_QVGA
#endif
#ifdef CONFIG_FRAMESIZE_CIF
#define CAM_FRAMESIZE FRAMESIZE_CIF
#endif
#ifdef CONFIG_FRAMESIZE_VGA
#define CAM_FRAMESIZE FRAMESIZE_VGA
#endif
#ifdef CONFIG_FRAMESIZE_SVGA
#define CAM_FRAMESIZE FRAMESIZE_SVGA
#endif
#ifdef CONFIG_FRAMESIZE_XGA
#define CAM_FRAMESIZE FRAMESIZE_XGA
#endif
#ifdef CONFIG_FRAMESIZE_SXGA
#define CAM_FRAMESIZE FRAMESIZE_SXGA
#endif
#ifdef CONFIG_FRAMESIZE_UXGA
#define CAM_FRAMESIZE FRAMESIZE_UXGA
#endif

const char *person = "/spiffs/person.wav";
const char *car = "/spiffs/car.wav";
const char *bike = "/spiffs/bike.wav";
const char *block = "/spiffs/block.wav";
const char *green = "/spiffs/green.wav";
const char *lamp = "/spiffs/lamp.wav";
const char *red = "/spiffs/red.wav";
const char *tree = "/spiffs/tree.wav";

//Run `idf.py menuconfig` and set various other options in "ESP32CAM Configuration"

OV5640 cam;

//lifted from Arduino framework
unsigned long millis()
{
    return (unsigned long) (esp_timer_get_time() / 1000ULL);
}

void delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

void udp_server(void* server)
{
    int len = 0;
    char rx_buffer[128];
    char addr_str[128];
    char *wav_play = NULL;
    struct sockaddr_in dest_addr;
    uint64_t timer_counter_value = 0;
    uint64_t timer_counter_update = 0;
    float length_m = 0;
    int err = 0;
    static uint16_t count = 0;

    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        printf("sock create failed!");
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    err = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (err < 0)
    {
        printf("set sock opt failed!--------\r\n");
    }

    err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0)
    {
        printf("Socket unable to bind\r\n");
    }
    printf("Socket bound, port %d\r\n", PORT);

    while (1)
    {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);
        printf("--------sock: %d\r\n", sock);

        memset(rx_buffer, 0, sizeof(rx_buffer));
        len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
        printf("-------len: %d\r\n", len);
        
        count++;
        printf("count: %d\r\n", count);
        if (len > 0 && count % 10 == 0) /* 数据长度大于0，说明接收有数据 */
        {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
            printf("Received %d bytes from:%s\r\n", len, addr_str);
            printf("buf: %s\r\n", rx_buffer);
            
            hcsr04_timer_init(); /* 初始化 */
 
            gpio_set_level(TIRG_PIN, 0);
            hcsr04_delay_us(20);
            gpio_set_level(TIRG_PIN, 1); /* 然后拉高Trig至少10us以上 */
            ets_delay_us(10);
            gpio_set_level(TIRG_PIN, 0); /* 再拉低Trig，完成一次声波发出信号 */
    
            //检测Echo引脚，一直为低电平就一直在等待
            while (gpio_get_level(ECHO_PIN) == 0);
            timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_counter_value);
            while (gpio_get_level(ECHO_PIN) == 1);
            timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_counter_update);

            //计算Echo引脚高电平持续的时间
            if (timer_counter_update - timer_counter_value < 130000)
            {
                char distance[10] = {0};
                //length_mm = 0.0425*(timer_counter_update - timer_counter_value);//counter * 340000 / 4000000 / 2
                length_m = 0.17 * (timer_counter_update - timer_counter_value) / 1000;//counter * 340000 / 1000000 / 2
                //printf("HCSR04 length: %.2f m\r\n", length_m);
                snprintf(distance, sizeof(distance), "%.2f%s", length_m, "m");
                //printf("distance: %s\r\n", distance);
                OLED_ShowString(0, 4, "distance:", 16);
                OLED_ShowString(0, 6, (char *)distance, 16);
            } 
            else /* 131989 value for error */
            {
                printf("HCSR04 length error!\r\n");
            }

            vTaskDelay(200 / portTICK_PERIOD_MS);

            if (length_m - 0.3 < 0)
            {
                if (!strncmp("person", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)person;
                }
                else if (!strncmp("car", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)car;
                }
                else if (!strncmp("tree", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)tree;
                }
                else if (!strncmp("pillar", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)block;
                }
                else if (!strncmp("bike", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)bike;
                }
                else if (!strncmp("lamppost", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)lamp;
                }
                else if (!strncmp("go", (const char *)rx_buffer, strlen(rx_buffer)))
                {
                    wav_play = (char *)green;
                }
                else
                {
                    wav_play = (char *)red;
                }
                
                playback_wav(wav_play);
            }

            printf("----finish!---\r\n");

            if (count > 65500)
            {
                count = 0;
            }
        }
    }

    // Stop I2S driver and destroy
    ESP_ERROR_CHECK(i2s_driver_uninstall(I2S_NUM_0));
}

void client_worker(void * client)
{
    //esp_task_wdt_add(NULL);

    OV5640Streamer * streamer = new OV5640Streamer((SOCKET)client, cam);
    CRtspSession * session = new CRtspSession((SOCKET)client, streamer);

    unsigned long lastFrameTime = 0;
    const unsigned long msecPerFrame = (1000 / CONFIG_CAM_FRAMERATE);

    while (session->m_stopped == false)
    {
        session->handleRequests(0);

        unsigned long now = millis();
        if ((now > (lastFrameTime + msecPerFrame)) || (now < lastFrameTime))
        {
            //printf("------------------start take picture------\r\n");
            session->broadcastCurrentFrame(now);
            lastFrameTime = now;
        }
        else
        {
            //let the system do something else for a bit
            vTaskDelay(1);
        }
        //esp_task_wdt_reset();
    }

#ifdef CONFIG_SINGLE_CLIENT_MODE
    esp_restart();
#else
    //shut ourselves down
    delete streamer;
    delete session;
    //esp_task_wdt_delete(NULL);
    vTaskDelete(NULL);
#endif
}

extern "C" void rtsp_server(void);
void rtsp_server(void)
{
    SOCKET server_socket;
    SOCKET client_socket;
    sockaddr_in server_addr;
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    TaskHandle_t xHandle = NULL;

    //note that I am adding this task watchdog, but I never feed it
    //if a client doesn't connect in 60 seconds, we reset
    //I assume by that time that something is wrong with the wifi and reset will clear it up
    //if a client connects, we'll delete this watchdog
    //esp_task_wdt_add(NULL);

    camera_config_t config = esp32cam_aithinker_config;
    //config.frame_size = CAM_FRAMESIZE;
    //config.jpeg_quality = CAM_QUALITY;
    cam.init(config);

    sensor_t * s = esp_camera_sensor_get();
    #ifdef CONFIG_CAM_HORIZONTAL_MIRROR
        s->set_hmirror(s, 0);
    #endif
    #ifdef CONFIG_CAM_VERTICAL_FLIP
        s->set_vflip(s, 0);
    #endif
    
    //setup other camera options
    // s->set_brightness(s, 0);     // -2 to 2
    // s->set_contrast(s, 2);       // -2 to 2
    // s->set_saturation(s, 1);     // -2 to 2
    // s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    //s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    //s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    //s->set_ae_level(s, 0);       // -2 to 2
    s->set_aec_value(s, 300);    // 0 to 1200
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    //s->set_agc_gain(s, 0);       // 0 to 30
    //s->set_gainceiling(s, (gainceiling_t)2);  // 0 to 6
    //s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    //s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(8554); // listen on RTSP port 8554
    server_socket               = socket(AF_INET, SOCK_STREAM, 0);

    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        printf("setsockopt(SO_REUSEADDR) failed! errno=%d\n", errno);
    }

    // bind our server socket to the RTSP port and listen for a client connection
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("bind failed! errno=%d\n", errno);
    }

    if (listen(server_socket, 5) != 0)
    {
        printf("listen failed! errno=%d\n", errno);
    }

    printf("\n\nrtsp://%s.localdomain:8554/mjpeg/1\n\n", CONFIG_LWIP_LOCAL_HOSTNAME);

    // loop forever to accept client connections
    while (true)
    {
        printf("Minimum free heap size: %u bytes\n", esp_get_minimum_free_heap_size());
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        printf("Client connected from: %s\n", inet_ntoa(client_addr.sin_addr));
        //esp_task_wdt_delete(NULL);
        //xTaskCreate(client_worker, "client_worker", 3584, (void*)client_socket, tskIDLE_PRIORITY, &xHandle);
    
        xTaskCreate(client_worker, "client_worker", 4096, (void*)client_socket, 9, &xHandle);

        xTaskCreate(udp_server, "recv_server", 4096, NULL, 10, NULL);
    }

    close(server_socket);
}

