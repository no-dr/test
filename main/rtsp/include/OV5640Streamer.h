#pragma once

#include "CStreamer.h"
#include "OV5640.h"

class OV5640Streamer : public CStreamer
{
    bool m_showBig;
    OV5640 &m_cam;

public:
    OV5640Streamer(SOCKET aClient, OV5640 &cam);

    virtual void streamImage(uint32_t curMsec) override;
};
