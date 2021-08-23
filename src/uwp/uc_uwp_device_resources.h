#pragma once

#include <memory>

#include <d3d12.h>
#include <dxgi1_5.h>

#include <wrl/client.h>

#include <uc/util/noncopyable.h>
#include <uc/gx/dx12/dx12.h>

#include "uc_uwp_renderer_impl_window.h"
#include "uc_uwp_device_swap_chain_resources.h"

#include <windows.foundation.h>


namespace uc
{
    namespace uwp
    {
        struct window_environment;
        class device_resources : public util::noncopyable
        {
            public:

            enum struct swap_chains : uint32_t
            {
                background  = 0,
                overlay     = 1
            };

            device_resources();
            ~device_resources();

            void set_window(const window_environment* environment);
            int add_swapchain(swap_chain::swap_chain_type type); // returns the id of the new swapchain

            void wait_for_gpu();
            void move_to_next_frame();
            void sync();
            void present(gx::dx12::gpu_command_queue::present_option o);

            swap_chain::resources* swap_chain(swap_chains s) const
            {
                return m_swap_chains[static_cast<uint32_t>(s)].get();
            }

            swap_chain::resources* swap_chain(int id) const
            {
                return m_swap_chains[id].get();
            }

            gx::dx12::gpu_back_buffer* back_buffer( swap_chains s ) const
            {
                return swap_chain(s)->back_buffer_dx12();
            }

            ID3D12Device* device_d2d12() const
            {
                return m_device.Get();
            }

            gx::dx12::gpu_command_context_allocator* direct_command_context_allocator( swap_chains s ) const
            {
                return swap_chain(s)->command_context_allocator();
            }

            gx::dx12::gpu_command_context_allocator* compute_command_context_allocator() const
            {
                return m_compute_context_allocator.get();
            }

            gx::dx12::gpu_command_context_allocator* copy_command_context_allocator() const
            {
                return m_copy_context_allocator.get();
            }

            gx::dx12::gpu_resource_create_context* resource_create_context() const
            {
                return m_resource_creator.get();
            }

            gx::dx12::gpu_command_queue* direct_queue( swap_chains s ) const
            {
                return swap_chain(s)->queue();
            }

            gx::dx12::gpu_command_queue* copy_queue() const
            {
                return this->m_copy_queue.get();
            }

            gx::dx12::gpu_command_queue* compute_queue() const
            {
                return this->m_compute_queue.get();
            }

            gx::dx12::gpu_upload_queue* upload_queue() const
            {
                return m_upload_queue.get();
            }


            private:

            void    create_d3d_12();

			Microsoft::WRL::ComPtr<ID3D12Debug1>										m_debug;
            Microsoft::WRL::ComPtr<ID3D12Device>                                        m_device;
            std::unique_ptr<gx::dx12::gpu_resource_create_context>                      m_resource_creator;
            std::unique_ptr<gx::dx12::gpu_command_queue>                                m_copy_queue;
            std::unique_ptr<gx::dx12::gpu_command_manager>                              m_copy_command_manager;
            std::unique_ptr<gx::dx12::gpu_command_context_allocator>                    m_copy_context_allocator;
            std::unique_ptr<gx::dx12::gpu_command_queue>                                m_compute_queue;
            std::unique_ptr<gx::dx12::gpu_command_manager>                              m_compute_command_manager;
            std::unique_ptr<gx::dx12::gpu_command_context_allocator>                    m_compute_context_allocator;
            std::unique_ptr<gx::dx12::gpu_upload_queue>                                 m_upload_queue;
            std::unique_ptr<gx::dx12::gpu_upload_queue>                                 m_upload_resource_queue;

            std::unique_ptr<swap_chain::resources>                                      m_swap_chains[2];
            uint32_t                                                                    m_swap_chain_count;

            //1. ui composition queue   -> fixed resolution when the program starts or window changes    -> xaml
            //2. hud composition queue  -> fixed resoultion when the program starts or window changes    -> hud
            //3. back buffer            -> might change resolution dynamically from the upper levels     -> game
        };
    }
    
}