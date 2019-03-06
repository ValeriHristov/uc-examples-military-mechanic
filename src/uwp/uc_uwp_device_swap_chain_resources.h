#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_5.h>
#include <wrl/client.h>

#include <uc/util/noncopyable.h>
#include <uc/gx/dx12/dx12.h>

#include "uc_uwp_renderer_impl_window.h"

namespace uc
{
    class gx::dx12::gpu_resource_create_context;

    namespace uwp
    {
        struct window_environment;

        namespace swap_chain
        {
            inline void throw_if_failed(HRESULT hr)
            {
                gx::dx12::throw_if_failed(hr);
            }

            struct factories : public util::noncopyable
            {
                //factory
                Microsoft::WRL::ComPtr<IDXGIFactory4>                           m_dxgi;
            };

            inline std::unique_ptr<factories> make_factories()
            {
                Microsoft::WRL::ComPtr<IDXGIFactory4> r0;
                throw_if_failed(CreateDXGIFactory1(IID_PPV_ARGS(&r0)));

                std::unique_ptr<factories> result = std::make_unique<factories>();

                result->m_dxgi = std::move(r0);
                return result;
            }

            enum struct swap_chain_type
            {
                background_core_window,
                background_composition,
				foreground_core_window,
				foreground_composition,
            };

            class resources : public util::noncopyable
            {
                public:

                resources(  ID3D12Device* device, gx::dx12::gpu_resource_create_context* resource_creator, swap_chain_type type );
                ~resources();

                void set_window( const window_environment* environment, gx::dx12::gpu_resource_create_context* resource_creator );

                gx::dx12::gpu_back_buffer* back_buffer_dx12() const
                {
                    return m_back_buffers[m_current_frame].get();
                }

                IDXGISwapChain3* swap_chain() const
                {
                    return m_swap_chain.Get();
                }

                ID3D12Device* device_d2d12() const
                {
                    return m_device_d3d_12.Get();
                }

                gx::dx12::gpu_command_context_allocator* direct_command_context_allocator() const
                {
                    return m_direct_context_allocator.get();
                }

                gx::dx12::gpu_command_queue* direct_queue() const
                {
                    return this->m_direct_queue.get();
                }

                gx::dx12::gpu_command_context_allocator* command_context_allocator() const
                {
                    return m_direct_context_allocator.get();
                }

                gx::dx12::gpu_command_queue* queue() const
                {
                    return this->m_direct_queue.get();
                }

                void set_source_size(uint32_t width, uint32_t height )
                {
                    throw_if_failed(m_swap_chain->SetSourceSize(width, height));
                }

                void wait_for_gpu();
                void move_to_next_frame();
                void present();
                void sync();

            private:

                std::unique_ptr<factories>                                      m_factories;

                //dx12 buffers
                Microsoft::WRL::ComPtr<ID3D12Device>                            m_device_d3d_12;
                std::unique_ptr<gx::dx12::gpu_back_buffer>                      m_back_buffers[3];

                Microsoft::WRL::ComPtr<IDXGISwapChain3>                         m_swap_chain;

                uint64_t                                                        m_current_frame = 0;

                //submission control
                std::unique_ptr<gx::dx12::gpu_command_queue>                    m_direct_queue;
                std::unique_ptr<gx::dx12::gpu_command_manager>                  m_direct_command_manager;
                std::unique_ptr<gx::dx12::gpu_command_context_allocator>        m_direct_context_allocator;

                void create_d3d_11();
                void create_d2d_3();
                swap_chain_type                                                 m_type;
            };
        }
    }
}