#pragma once

#include <memory>
#include <uc/util/noncopyable.h>
#include "uc_uwp_gx_submitable.h"

namespace uc
{
    namespace uwp
    {
        namespace gxu
        {
            struct initialize_context;
            struct render_context;
            struct update_context;
        }

        namespace overlay
        {
            class page : public util::noncopyable
            {
                public:

                page( gxu::initialize_context* ctx );

                virtual ~page();

                void update(gxu::update_context* ctx);
                std::unique_ptr< submitable >  render(gxu::render_context* ctx);

                private:

                virtual void do_update(gxu::update_context* ctx) = 0;
                virtual  std::unique_ptr< submitable >  do_render(gxu::render_context* ctx ) = 0;
            };
        }
    }

}