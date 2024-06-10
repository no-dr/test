#ifndef _I2S_RECORDER_H_
#define _I2S_RECORDER_H_

#include <dirent.h>     /* dir struct */
#include "driver/i2s.h"
#include "esp_spiffs.h"
#include "esp_vfs_fat.h"

#define SPI_DMA_CHAN            SPI_DMA_CH_AUTO
#define SD_MOUNT_POINT          "/spiffs"
#define DIR_NUM_MAX             (10)
#define DIR_NAM_LEN_MAX         (50)
#define WAV_LIST_MAX            (10)
#define I2S_REC_SAMPLE_RATE     (44100)
#define I2S_REC_BIT             (16)
#define I2S_SAMPLE_SIZE         (I2S_REC_BIT * 1024)
#define WAVE_HEADER_SIZE        (44)
#define I2S_PIN_BCK_GPIO        (41)
#define I2S_PIN_WS_GPIO         (40)
#define I2S_PIN_DATA_PLAYBACK   (42)

struct wavinfo
{
    char *name;         /* wav file path */
    bool status;        /* playing is 1, stop is 0 */
    int current_time;   /* last stop time */

    int bit;            /* bit */
    int channel;        
    int sample_rate;
    int while_time;     /* sample num */
    int format;
    struct wavinfo * next;
};

char dir_temp[DIR_NAM_LEN_MAX];
struct wavinfo list[WAV_LIST_MAX];
struct wavinfo current_wav_msg;
int16_t i2s_readraw_buff[I2S_SAMPLE_SIZE];
size_t bytes_read;
int current_list_num;

void dir_list(char *path);
void show_wav_list(void);
void init_i2s(void);

#ifdef __cplusplus
extern "C"
{

void playback_wav(char *wav_name);

}
#endif /* __cplusplus */

#endif /* _I2S_RECORDER_H_ */
