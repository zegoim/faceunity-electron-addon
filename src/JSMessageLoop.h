//
// NodeMessageLoop.h
//
// Copyright 2018年 Zego. All rights reserved.
//

#ifndef NodeMessageLoop_h__
#define NodeMessageLoop_h__

#include <mutex>
#include <string>
#include <nan.h>


namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        using std::string;


        typedef struct EventMsgInfo
        {
            // 在任务线程中定义，在uv事件循环中调用
            std::function<void()> get_data_and_notify_node_fun;

        } EventMsgInfo;


        class JSMessageLoop
        {
        public:
            static JSMessageLoop * Instance()
            {
                static JSMessageLoop n;
                return &n;
            }

            static NAUV_WORK_CB(MessageLoopProcess);

            void PostAsyncEventMsg(const EventMsgInfo& msg_info);

        private:
            JSMessageLoop();
            ~JSMessageLoop();

            std::mutex            msg_vec_mutex_;
            uv_async_t *          uv_async_;
            std::queue<EventMsgInfo> async_event_queue_;
        };

#define JSMessageLoopInstance JSMessageLoop::Instance

    }
}
#endif // NodeMessageLoop_h__

