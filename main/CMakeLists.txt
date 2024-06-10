
idf_component_register(SRCS "main.c" 
                            "rtsp/esp32cam.cpp" 
                            "rtsp/src/CRtspSession.cpp" 
                            "rtsp/src/CStreamer.cpp" 
                            "rtsp/src/OV5640.cpp" 
                            "rtsp/src/OV5640Streamer.cpp" 
                            "i2s/src/i2s_recorder.c" 
                            "hcsr04/src/hcsr04.c"
                            "oled/src/oled.c"
                       INCLUDE_DIRS "rtsp/include"
                                    "i2s/include"
                                    "hcsr04/include"
                                    "oled/include")

spiffs_create_partition_image(storage ../data FLASH_IN_PROJECT)
