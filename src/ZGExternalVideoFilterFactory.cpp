#include "ZGExternalVideoFilterFactory.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "nan.h"

#include "FuBeautifyFilter.h"
#include "JSMessageLoop.h"
#include "GlobalConfig.h"

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {

        void ZGLog(const char * format, ...)
        {
//             printf(format);
//             printf("\n");
#ifdef WIN32
            char log_buf[1024] = { 0 };
            va_list la;
            va_start(la, format);
            vsprintf_s(log_buf, format, la);
            va_end(la);

            OutputDebugStringA(log_buf);
#endif
        }

        #define ZGENTER_FUN_LOG ZGLog("==>%s",__FUNCTION__);
        #define ZGLEAVE_FUN_LOG ZGLog("<==%s",__FUNCTION__);

        ZGExternalVideoFilterFactory::ZGExternalVideoFilterFactory()
        {
            ZGENTER_FUN_LOG;
            for (int i = 0; i < MAX_FILTER_FRAME_COUNT; ++i)
            {
                std::unique_ptr<VideoFilterBuffer> fb(new VideoFilterBuffer());
                fb->width_ = 0;
                fb->height_ = 0;
                fb->stride_ = 0;
                fb->data_ = nullptr;
                filter_data_list_.push_back(std::move(fb));
            }

            // ���տ���
            filter_process_ = std::shared_ptr<VideoFilterProcessBase>(new FuBeautifyFilter());

        }


        ZGExternalVideoFilterFactory::~ZGExternalVideoFilterFactory()
        {
            ZGENTER_FUN_LOG;

            is_exit_ = true;

            if (process_thread_.joinable())
            {
                process_thread_.join();
            }
        }


        void ZGExternalVideoFilterFactory::EnableBeauty(bool enable)
        {
            ZGENTER_FUN_LOG;
            enable_beautify_ = enable;
        }

        bool ZGExternalVideoFilterFactory::UpdateBeautyLevel(int level)
        {
            ZGENTER_FUN_LOG;
            if (filter_process_ != nullptr)
            {
                return filter_process_->UpdateFilterLevel(level);
            }
            return false;
        }

        void ZGExternalVideoFilterFactory::StartBeautyProcess()
        {
            if (beauty_process_thread_have_start_)
            {
                CallToJSFun(GlobalConfigInstance()->init_cb_, 0);
                return;
            }

            beauty_process_thread_have_start_ = true;

            // ���մ����߳�
            process_thread_ = std::thread(&ZGExternalVideoFilterFactory::BeautifyProcess, this);

        }

        bool ZGExternalVideoFilterFactory::SetParameter(std::string parameter)
        {
            ZGENTER_FUN_LOG;
            if (filter_process_ != nullptr)
            {
                return filter_process_->SetParameter(parameter.c_str());
            }
            return false;
        }

        AVE::VideoFilter* ZGExternalVideoFilterFactory::Create()
        {
            ZGENTER_FUN_LOG;            
            return this;
        }

        void ZGExternalVideoFilterFactory::CallToJSFun(JsCallBackInfo cb, int error_code)
        {
            if (!cb.js_cb)
            {
                return;
            }

            EventMsgInfo event_info = {};

            event_info.get_data_and_notify_node_fun = [=]()->void
            {
                v8::Local<v8::Object> ret_obj = v8::Object::New(cb.iso);

                ret_obj->Set(Nan::New("error_code").ToLocalChecked(), Nan::New(error_code));

                v8::Local<v8::Value> ret_data[1] = { ret_obj };

                Nan::MakeCallback(Nan::GetCurrentContext()->Global(), cb.js_cb->GetFunction(), 1, ret_data);
            };

            JSMessageLoopInstance()->PostAsyncEventMsg(event_info);
        }

        // �˾�������ʼ��
        void ZGExternalVideoFilterFactory::AllocateAndStart(Client* client)
        {
            ZGENTER_FUN_LOG;
            {
                std::lock_guard<std::mutex> lock(client_mutex_);
                client_ = client;
            }

            write_index_ = 0;
            read_index_ = 0;
            pending_count_ = 0;

        }

        void ZGExternalVideoFilterFactory::StopAndDeAllocate()
        {
            ZGENTER_FUN_LOG;
   
            std::lock_guard<std::mutex> lock(client_mutex_);
            if (client_ != nullptr)
            {
                client_->Destroy();
                client_ = nullptr;
            }
        }

        // sdk �ص���1���� ����buffer����
        AVE::VideoBufferType ZGExternalVideoFilterFactory::SupportBufferType()
        {
            //ZGENTER_FUN_LOG;

            // rgb ֡��С
            // rgb frame_len = w * h * 4
             cal_frame_factor_ = 4;
             return  AVE::BUFFER_TYPE_MEM;

            // yuv֡��С
            // yuv i420 frame_len = w * h * 1.5
            //cal_frame_factor_ = 1.5f;
            //return AVE::BUFFER_TYPE_ASYNC_I420_MEM;
        }

        // sdk �ص���2���������ȡ�����ӿ�
        void* ZGExternalVideoFilterFactory::GetInterface()
        {
            //ZGENTER_FUN_LOG;
            return (AVE::VideoBufferPool*)this;
        }

        // sdk �ص���3������sdkһ�������ţ�sdk��һ��ʹ�����������ȡbuffer��ַ��׼���������ݵ��õ�ַ
        int ZGExternalVideoFilterFactory::DequeueInputBuffer(int width, int height, int stride)
        {
            //ZGENTER_FUN_LOG;

            if (!have_start_)
            {
                //ZGLog("have not start");
                return -1;
            }

            if (pending_count_ >= MAX_FILTER_FRAME_COUNT)
            {                
                //ZGLog("pending_count_ >= MAX_FILTER_FRAME_COUNT, return");
                return -1;
            }

            //filter_data_list_[write_index_]->cur_time = GetTickCount();

            //ZGLog("width = %d, height = %d, stride = %d ", width, height, stride);

            if (filter_data_list_[write_index_]->width_ != width
                || filter_data_list_[write_index_]->height_ != height
                || filter_data_list_[write_index_]->stride_ != stride)
            {
                //ZGLog("reset buffer , index = %d", write_index_);
                std::unique_ptr<VideoFilterBuffer> fb(new VideoFilterBuffer());
                fb->width_ = width;
                fb->height_ = height;
                fb->stride_ = stride;
                fb->data_ = new unsigned char[width * height * cal_frame_factor_];
                //ZGLog("filter_data_list_ size = %d ", filter_data_list_.size());
                if (write_index_ < filter_data_list_.size())
                {
                    filter_data_list_[write_index_] = std::move(fb);
                }
                else {
                    //ZGLog("write_index_ error");
                }
            }
            return write_index_;
        }

        // sdk �ص���4����sdkͨ��������ȡbuffer��ַ��sdk�´�����ݵ��ṩ�ĵ�ַ��
        void* ZGExternalVideoFilterFactory::GetInputBuffer(int index)
        {
            //ZGENTER_FUN_LOG;

            if (index < 0 || index >= MAX_FILTER_FRAME_COUNT)
            {
                ZGLog("index invalid !!!");
                return nullptr;
            }            

            return filter_data_list_[write_index_]->data_;
        }

        // sdk �ص���5����sdk�Ѿ��������ݵ�ָ����ַ��ϣ������˾������buffer
        void ZGExternalVideoFilterFactory::QueueInputBuffer(int index, int width, int height, int stride, unsigned long long timestamp_100n)
        {
            //ZGENTER_FUN_LOG;

            if (write_index_ != index)
            {
                ZGLog("write_index_ != index, return");
                return;
            }

            //ZGLog("index=%d, width=%d, height=%d, stride=%d, timestamp=%ld", index, width, height, stride, timestamp_100n);

            filter_data_list_[write_index_]->width_ = width;
            filter_data_list_[write_index_]->height_ = height;
            filter_data_list_[write_index_]->timestamp_100n_ = timestamp_100n;
            filter_data_list_[write_index_]->stride_  = stride;
            filter_data_list_[write_index_]->len_ = width * height * cal_frame_factor_;
            write_index_ = (write_index_ + 1) % MAX_FILTER_FRAME_COUNT;

            pending_count_++;

            //ZGLog("pending_count_++, pending_count_ = %d", pending_count_);
        }


        // �˾�������
        void ZGExternalVideoFilterFactory::BeautifyProcess()
        {
            ZGENTER_FUN_LOG;

            //std::cout << "ZGExternalVideoFilterFactory::BeautifyProcess";
			int init_filter_ret = filter_process_->InitFilter();
            if (init_filter_ret == 0)
            {
                CallToJSFun(GlobalConfigInstance()->init_cb_, 0);

                have_start_ = true;

                while (!is_exit_)
                {

                    while (pending_count_ > 0)
                    {
                        unsigned char* src_data = filter_data_list_[read_index_]->data_;

                        // �˾�����ͼ������
                        if (enable_beautify_ && filter_process_ != nullptr)
                        {
                            filter_process_->FilterProcessRGBAData(filter_data_list_[read_index_]->data_, filter_data_list_[read_index_]->len_, filter_data_list_[read_index_]->width_, filter_data_list_[read_index_]->height_);
                        }

                        // �˾�������ϵ���Ƶ����������sdk
                        {
                            std::lock_guard<std::mutex> lock(client_mutex_);
                            if(client_)
                            {
                                VideoBufferPool* pool = (VideoBufferPool*)client_->GetInterface();
                                int index = pool->DequeueInputBuffer(filter_data_list_[read_index_]->width_, filter_data_list_[read_index_]->height_, filter_data_list_[read_index_]->stride_);

                                if (index >= 0)
                                {
                                    unsigned char* dst = (unsigned char*)pool->GetInputBuffer(index);

                                    memcpy(dst, src_data, filter_data_list_[read_index_]->width_*filter_data_list_[read_index_]->height_ * cal_frame_factor_);

                                    pool->QueueInputBuffer(index, filter_data_list_[read_index_]->width_, filter_data_list_[read_index_]->height_, filter_data_list_[read_index_]->stride_, filter_data_list_[read_index_]->timestamp_100n_);

                                    //DWORD elapse_time = GetTickCount() - filter_data_list_[read_index_]->cur_time;
                                    //ZGLog("elapse time = %d", elapse_time);
                                }
                            }
                        }

                        read_index_ = (read_index_ + 1) % MAX_FILTER_FRAME_COUNT;
                        pending_count_--;

                        if (is_exit_)
                        {
                            break;
                        }
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }

                filter_process_->Release();

            }
            else {
                CallToJSFun(GlobalConfigInstance()->init_cb_, init_filter_ret);
            }

            have_start_ = false;

            beauty_process_thread_have_start_ = false;

            ZGLEAVE_FUN_LOG;

        }

        void ZGExternalVideoFilterFactory::Destroy(VideoFilter *vf)
        {
            ZGENTER_FUN_LOG;
            //ZGLog("ZGExternalVideoFilterFactory::Destroy");
            {
                std::lock_guard<std::mutex> lock(client_mutex_);
                client_ = nullptr;
            }
            
            write_index_ = 0;
            read_index_ = 0;
            pending_count_ = 0;    
        }

    }
}