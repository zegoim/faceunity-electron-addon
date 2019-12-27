#pragma once

#include "nan.h"

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        class JsCallBackInfo
        {
        public:
            std::shared_ptr<Nan::Callback> js_cb = nullptr;
            v8::Isolate *iso = nullptr;

        };
    }

}