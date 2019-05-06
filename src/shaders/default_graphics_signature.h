#pragma once

//autogenerated code

#include <uc/gx/dx12/dx12.h>

namespace uc
{
    namespace gx
    {
        namespace dx12
        {
            struct default_graphics_signature : public gpu_root_signature_blob 
            {
                using base = gpu_root_signature_blob;
                default_graphics_signature( );
            };

            ID3D12RootSignature* create_default_graphics_signature(ID3D12Device* d);
        }
    }
}
