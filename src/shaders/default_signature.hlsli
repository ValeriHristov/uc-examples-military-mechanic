#ifndef __default_signature_hlsli__
#define __default_signature_hlsli__

#define MyRSDepth "RootFlags( 0 ), " \
              "RootConstants(num32BitConstants=32, b0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "CBV(b1, space = 0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "SRV(t0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "StaticSampler(s0)," \
              "StaticSampler(s1)," \
              "StaticSampler(s2, addressU = TEXTURE_ADDRESS_CLAMP,  addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_POINT )," \
              "StaticSampler(s3, addressU = TEXTURE_ADDRESS_CLAMP,  addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR )," \
              "StaticSampler(s4, addressU = TEXTURE_ADDRESS_WRAP,   addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  filter = FILTER_MIN_MAG_MIP_POINT )," \
              "StaticSampler(s5, addressU = TEXTURE_ADDRESS_WRAP,   addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  filter = FILTER_MIN_MAG_MIP_LINEAR )," \
              "StaticSampler(s6, addressU = TEXTURE_ADDRESS_BORDER, addressV = TEXTURE_ADDRESS_BORDER, addressW = TEXTURE_ADDRESS_BORDER, filter = FILTER_MIN_MAG_MIP_LINEAR, borderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK )," \
              "StaticSampler(s7, addressU = TEXTURE_ADDRESS_BORDER, addressV = TEXTURE_ADDRESS_BORDER, addressW = TEXTURE_ADDRESS_BORDER, filter = FILTER_MIN_MAG_MIP_LINEAR, borderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE )"

#define MyRS1 "RootFlags( 0 ), " \
              "RootConstants(num32BitConstants=32, b0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "CBV(b1, space = 0, visibility = SHADER_VISIBILITY_ALL), " \
              "SRV(t0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "DescriptorTable( CBV(b2, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL)," \
              "DescriptorTable( SRV(t1, numDescriptors = 3), visibility = SHADER_VISIBILITY_PIXEL)," \
              "CBV(b2, space = 0, visibility = SHADER_VISIBILITY_VERTEX), " \
              "StaticSampler(s0)," \
              "StaticSampler(s1)," \
              "StaticSampler(s2, addressU = TEXTURE_ADDRESS_CLAMP,  addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_POINT )," \
              "StaticSampler(s3, addressU = TEXTURE_ADDRESS_CLAMP,  addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR )," \
              "StaticSampler(s4, addressU = TEXTURE_ADDRESS_WRAP,   addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  filter = FILTER_MIN_MAG_MIP_POINT )," \
              "StaticSampler(s5, addressU = TEXTURE_ADDRESS_WRAP,   addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  filter = FILTER_MIN_MAG_MIP_LINEAR )," \
              "StaticSampler(s6, addressU = TEXTURE_ADDRESS_BORDER, addressV = TEXTURE_ADDRESS_BORDER, addressW = TEXTURE_ADDRESS_BORDER, filter = FILTER_MIN_MAG_MIP_LINEAR, borderColor = STATIC_BORDER_COLOR_OPAQUE_BLACK )," \
              "StaticSampler(s7, addressU = TEXTURE_ADDRESS_BORDER, addressV = TEXTURE_ADDRESS_BORDER, addressW = TEXTURE_ADDRESS_BORDER, filter = FILTER_MIN_MAG_MIP_LINEAR, borderColor = STATIC_BORDER_COLOR_OPAQUE_WHITE )"

#define MyRS2 "RootFlags( 0 ), " \
              "RootConstants(num32BitConstants=1, b0), " \
              "CBV(b1 , space = 0), " \
              "UAV(u0) " \

#endif

