#pragma once

#include <string>
#include <vector>

#include "JsCallBackInfo.h"

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        class GlobalConfig
        {
        public:

            static GlobalConfig* Instance()
            {
                static GlobalConfig g;
                return &g;
            }

            JsCallBackInfo init_cb_;

            std::vector<char> auth_package_data_;
            std::string v3_bundle_path_ = "";//"v3.bundle";
            std::string face_beauty_bundle_path_ = "";//"face_beautification.bundle";
            
        private:

            GlobalConfig(){}
            
        };

#define GlobalConfigInstance GlobalConfig::Instance

    }
}
