
#include "OV5640Streamer.h"
#include <assert.h>



OV5640Streamer::OV5640Streamer(SOCKET aClient, OV5640 &cam) : CStreamer(aClient, cam.getWidth(), cam.getHeight()), m_cam(cam)
{
    printf("Created streamer width=%d, height=%d\n", cam.getWidth(), cam.getHeight());
    DEBUG_PRINT("Created streamer width=%d, height=%d\n", cam.getWidth(), cam.getHeight());
}

void OV5640Streamer::streamImage(uint32_t curMsec)
{
    m_cam.run();// queue up a read for next time

    BufPtr bytes = m_cam.getfb();
    //printf("get size-------%d\r\n", m_cam.getSize());
    streamFrame(bytes, m_cam.getSize(), curMsec);
    m_cam.done();
}
