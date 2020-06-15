// FuBeautifyFilter.h
//
// Copyright 2019�� Zego. All rights reserved.
// 

#ifndef FUBeautifyFilter_h__
#define FUBeautifyFilter_h__

#include <vector>
#include <string>

#include "VideoFilterProcessBase.h"

#ifdef WIN32

#include< WinSock2.h> 
#include< WS2tcpip.h> 
#include <windows.h>

#endif

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        typedef struct FaceUnityBundle {
            std::string bundlePath = "";
            std::string bundleName = "";            
            std::string options = "";
            bool need_update = false;
            int bundle_handle = 0;
        }FaceUnityBundle;

        // fu���սӿ�ʵ��
        class FuBeautifyFilter : public VideoFilterProcessBase
        {
        public:
            FuBeautifyFilter();
            ~FuBeautifyFilter();

            // ��ʼ�������˾�
            virtual int InitFilter() override;

            // ���մ���YUV I420����
            virtual void FilterProcessI420Data(unsigned char * data, int frame_len, int frame_w, int frame_h) override;

            // �������յȼ�����Χ[0-10]
            virtual bool UpdateFilterLevel(int level) override;

            virtual void FilterProcessRGBAData(unsigned char * data, int frame_len, int frame_w, int frame_h) override;

            virtual void Release() override;

            virtual bool SetParameter(const char *param) override;

        protected:

            bool InitOpenGL();

            bool InitFuSdk();

            bool LoadFuResource();

            void UpdateBeautyParamIfNeeded();

            bool inited_ = false;

            int frame_id_ = 0;

            int beauty_handle_ = 0;

            std::vector<int> bundle_handles_;

            std::vector<FaceUnityBundle> face_unity_bundles_;

            bool need_load_bundles_ = false;

            bool need_update_bundles_ = false;

            
#ifdef WIN32
            HGLRC hglrc_ = nullptr;
#endif
        };

    }

}

#endif // FUBeautifyFilter_h__


