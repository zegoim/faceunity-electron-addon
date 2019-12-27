#include "JSMessageLoop.h"

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {

        JSMessageLoop::JSMessageLoop()
        {
            uv_async_ = new uv_async_t;
            uv_async_init(uv_default_loop(), uv_async_, &JSMessageLoop::MessageLoopProcess);
        }

        void DestroyAsyncHandle(uv_handle_t *handle)
        {
            delete handle;
        }

        JSMessageLoop::~JSMessageLoop()
        {
            if (!uv_is_closing(reinterpret_cast<uv_handle_t*>(uv_async_)))
            {
                uv_close(reinterpret_cast<uv_handle_t*>(uv_async_), DestroyAsyncHandle);
            }
        }

        void JSMessageLoop::PostAsyncEventMsg(const EventMsgInfo& msg_info)
        {
            std::lock_guard<std::mutex> lock(this->msg_vec_mutex_);
            if (!uv_async_)
            {
                return;
            }
            async_event_queue_.push(msg_info);

            uv_async_->data = this;
            uv_async_send(uv_async_);

        }

        NAUV_WORK_CB(JSMessageLoop::MessageLoopProcess)
        {
            Nan::HandleScope scope;

            JSMessageLoop* pthis = reinterpret_cast<JSMessageLoop*>(async->data);

            std::queue<EventMsgInfo> tmp_event_queue;
            {
                std::lock_guard<std::mutex> lock(pthis->msg_vec_mutex_);
                tmp_event_queue = pthis->async_event_queue_;
                while (!pthis->async_event_queue_.empty())
                {
                    pthis->async_event_queue_.pop();
                }
            }
    
            while (!tmp_event_queue.empty())
            {
                if (tmp_event_queue.front().get_data_and_notify_node_fun)
                {
                    tmp_event_queue.front().get_data_and_notify_node_fun();
                }

                tmp_event_queue.pop();
            }
        }

    }
}