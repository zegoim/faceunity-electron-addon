//
// VideoFilterProcessBase.h
//
// Copyright 2019年 Zego. All rights reserved.
// 

#ifndef VideoFilterProcessBase_h__
#define VideoFilterProcessBase_h__

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        // 视频滤镜处理类
        class VideoFilterProcessBase
        {
        public:
            VideoFilterProcessBase();
            virtual ~VideoFilterProcessBase();

            // 初始化滤镜
            virtual int InitFilter(){ return -1;};

            // 滤镜处理YUV I420 视频数据
            virtual void FilterProcessI420Data(unsigned char * data, int frame_len, int frame_w, int frame_h) {}

            // 滤镜处理RGB数据
            virtual void FilterProcessRGBAData(unsigned char * data, int frame_len, int frame_w, int frame_h) {}

            // 刷新滤镜处理等级，例如美颜等级
            virtual bool UpdateFilterLevel(int level){ return false; }

            virtual void Release() {}

            virtual bool SetParameter(const char *param) { return false;};
 
        };
    }
}

#endif // VideoFilterProcessBase_h__


