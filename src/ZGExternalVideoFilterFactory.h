
// ZGExternalVideoFilterFactory.h
//
// Copyright 2019å¹? Zego. All rights reserved.
// 

#ifndef ZGExternalFilterDemo_h__
#define ZGExternalFilterDemo_h__

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "ZegoLiveRoom/zego-api-defines.h"
#include "ZegoLiveRoom/video_capture.h"

#include "VideoFilterProcessBase.h"
#include "JsCallBackInfo.h"


namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        using namespace ZEGO;
        using std::vector;

        class VideoFilterBuffer
        {
        public:
            VideoFilterBuffer()
            {
            }
            ~VideoFilterBuffer()
            {
                if (data_ != nullptr)
                {
                    delete[]data_;
                    data_ = nullptr;
                }
            }

            int width_ = 0;
            int height_ = 0;
            int len_ = 0;
            int stride_ = 0;
            unsigned long long timestamp_100n_ = 0;
            unsigned char * data_ = nullptr;
            //DWORD cur_time = 0;

        };

        class ZGExternalVideoFilterFactory :
            public AVE::VideoFilter,
            public AVE::VideoBufferPool,
            public AVE::VideoFilterFactory
        {
        public:
            ZGExternalVideoFilterFactory();
            ~ZGExternalVideoFilterFactory();

        public:

            void EnableBeauty(bool enable);

            bool UpdateBeautyLevel(int level);

            void StartBeautyProcess();

            bool SetParameter(std::string parameter);

        protected:


            std::atomic<bool> beauty_process_thread_have_start_ = { false };

            virtual void AllocateAndStart(Client* client) override;


            virtual void StopAndDeAllocate() override;


            virtual AVE::VideoBufferType SupportBufferType() override;


            virtual void* GetInterface() override;


            virtual int DequeueInputBuffer(int width, int height, int stride) override;


            virtual void* GetInputBuffer(int index) override;


            virtual void QueueInputBuffer(int index, int width, int height, int stride, unsigned long long timestamp_100n) override;

            void BeautifyProcess();
            virtual VideoFilter* Create() override;


            void CallToJSFun(JsCallBackInfo cb, int error_code);

            virtual void Destroy(VideoFilter *vf) override;


            const int MAX_FILTER_FRAME_COUNT = 2;

            std::atomic<int> pending_count_ = {0};

            std::atomic<bool> is_exit_ = {false};

            float cal_frame_factor_ = 4.0f;

            vector<std::unique_ptr<VideoFilterBuffer> > filter_data_list_;
            
            Client* client_ = nullptr;
            
            int write_index_ = 0;
            
            int read_index_ = 0;

            const int TICK_PERIOD_MS = 20;

            std::atomic<bool> enable_beautify_ = {true};

            std::shared_ptr<VideoFilterProcessBase> filter_process_ = nullptr;

            std::thread process_thread_;

            std::atomic<bool> have_start_ = {false};
    
            std::condition_variable task_condition_var_;
    
            std::mutex task_condition_var_mutex_;

            std::atomic<bool> have_task_in_queue_ = {false};



        };

}
}

#endif // ZGExternalFilterDemo_h__

