#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "i2s_recorder.h"

static char *char_change_A2a(char *raw_char)
{
    char *ret = raw_char;
    int len = strlen(raw_char);

    for (int i = 0; i < len; i++)
    {
        if ('A' <= ret[i] && ret[i] <= 'Z')
        {
            ret[i] = ret[i] + 32;
        }      
    }

    return ret;
}

static struct wavinfo get_wav_msg(char *wav_name)
{
    for (int i = 0; i < current_list_num; i++)
    {
        if (strstr(list[i].name, wav_name))
        {
            printf("has searched wav\n");
            printf("name: %s\n", list[i].name);
            return list[i];
        }
    }
    printf("not find wav %s\n", wav_name);
    return list[0];
}

void dir_list(char *path)
{
    char *dir_name = NULL;
    char *names[DIR_NUM_MAX];
    int count = 0;

    if (path != NULL)
        dir_name = path;
    else
        dir_name = SD_MOUNT_POINT;
    
    //printf("Opening dir %s\r\n", dir_name);
    DIR *dir = opendir(dir_name);

    while (1)
    {
        struct dirent *de = readdir(dir);
        if (!de)
        {
            break;
        }
        
        if (count >= DIR_NUM_MAX)
        {
            printf("file number exceed max!\r\n");
            break;
        }

        names[count] = char_change_A2a(de->d_name);
        //printf("file name: %s\n", names[count]);

        /* dir */
        if (de->d_type == 2)
        {
            strcpy(dir_temp, dir_name);
            strcat(dir_temp, "/");
            strcat(dir_temp, names[count]);
            printf("this is dir: %s\n", dir_temp);

            char *temp_cpr;
            temp_cpr = "system~1";

            if (strcmp(names[count], temp_cpr) == 0)
            {
                printf("ignore this dir\n");
            }
            else
            {
                dir_list(dir_temp);
            }
        }

        /* search wav file */ 
        char *search_key = ".wav";
        if (strstr(names[count], search_key) != NULL)
        {
            int file_name_len = strlen(dir_name) + strlen(names[count]);
            //printf("file name len = %d\n", file_name_len);
            char *temp_path = (char *)malloc(sizeof(char) * (file_name_len + 2));
            if (!temp_path)
            {
                printf("temp_path malloc failed!\r\n");
                return;
            }
            
            strcpy(temp_path, dir_name);
            strcat(temp_path, "/");
            strcat(temp_path, names[count]);
            //printf("file path = %s\n", temp_path);

            struct wavinfo *p1 ;
            p1 = &list[current_list_num];
            current_list_num++;
            p1->name = temp_path;
            //printf("p1->name = %s\n",p1->name);
        }

        ++count;
    }

    closedir(dir);
}

void show_wav_list(void)
{
    char *temp_path = SD_MOUNT_POINT;
    
    dir_list(temp_path);

    printf("--------- wav list --------: %d\n", current_list_num);
    for (int i = 0; i < current_list_num; i++)
    {
        printf("wav%d: %s\n", i, list[i].name);
    }
}

void playback_wav(char *wav_name)
{
    char wav_header_fmt_play[WAVE_HEADER_SIZE] = {0};
    char temp_cpr[8] = {0};

    //printf("wav name is %s\n", wav_name);
    FILE *f_wav_play = fopen(wav_name, "r");
    if (!f_wav_play)
    {
        printf("Failed to open file for writing!\r\n");
        return;
    }

    fread(wav_header_fmt_play, 1, WAVE_HEADER_SIZE, f_wav_play);
    printf("wav play head: %2x\n", wav_header_fmt_play[0]);
    strncpy(temp_cpr, wav_header_fmt_play, 4);

    if (strstr(temp_cpr, "RIFF"))
    {
        strncpy(temp_cpr, wav_header_fmt_play + 8, 8);
        if (strstr(temp_cpr, "WAVEfmt "))
        {
            int temp_current_time = 0;
            bytes_read = 0;

            printf("this is wav file!\n");
            current_wav_msg = get_wav_msg(wav_name);

            /* wav size */
            printf("wav size = %02x, %02x, %02x, %02x\n", wav_header_fmt_play[40], wav_header_fmt_play[41], wav_header_fmt_play[42], wav_header_fmt_play[43]);
            current_wav_msg.while_time = wav_header_fmt_play[40] + wav_header_fmt_play[41] * (1<<8) + wav_header_fmt_play[42] * (1<<16) + wav_header_fmt_play[43] * (1<<24);
            printf("while_time = %d\n", current_wav_msg.while_time);

            /* wav sample rate */
            current_wav_msg.sample_rate = wav_header_fmt_play[24] + wav_header_fmt_play[25] * (1<<8) + wav_header_fmt_play[26] * (1<<16) + wav_header_fmt_play[27] * (1<<24);
            printf("sample_rate = %d\n", current_wav_msg.sample_rate);

            /* Start playback */
            while (temp_current_time < current_wav_msg.while_time)
            {
                /* Read the RAW samples from the microphone (char *) */
                fread(i2s_readraw_buff, 1, bytes_read, f_wav_play);
                i2s_write(I2S_NUM_0, i2s_readraw_buff, I2S_SAMPLE_SIZE, &bytes_read, 100);
                temp_current_time += bytes_read;

                /* Write the samples to the WAV file */
                printf("temp_current_time = %d\n", temp_current_time);
            }
        }
    }

    fclose(f_wav_play);
    printf("Recording done!\r\n");
}

void init_i2s(void)
{
    // Set the I2S configuration as PDM and 16bits per sample
    i2s_config_t i2s_config = {
        .mode = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX,
        .sample_rate = I2S_REC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = 0,
    };

    // Set the pinout configuration (set using menuconfig)
    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_PIN_BCK_GPIO,
        .ws_io_num = I2S_PIN_WS_GPIO,
        .data_out_num = I2S_PIN_DATA_PLAYBACK,
        .data_in_num = I2S_PIN_NO_CHANGE,
    };

    // Call driver installation function before any I2S R/W operation.
    ESP_ERROR_CHECK( i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL) );
    ESP_ERROR_CHECK( i2s_set_pin(I2S_NUM_0, &pin_config) );
    ESP_ERROR_CHECK( i2s_set_clk(I2S_NUM_0, I2S_REC_SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO) );
}
