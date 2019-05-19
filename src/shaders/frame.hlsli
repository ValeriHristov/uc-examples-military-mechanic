#ifndef __per_frame_hlsli__
#define __per_frame_hlsli__

#include "interop.h"

cbuffer per_frame : register(b1)
{
    interop::frame m_frame;
};

#endif
