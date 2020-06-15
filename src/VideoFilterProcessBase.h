//
// VideoFilterProcessBase.h
//
// Copyright 2019�� Zego. All rights reserved.
// 

#ifndef VideoFilterProcessBase_h__
#define VideoFilterProcessBase_h__

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {
        // ��Ƶ�˾�������
        class VideoFilterProcessBase
        {
        public:
            VideoFilterProcessBase();
            virtual ~VideoFilterProcessBase();

            // ��ʼ���˾�
            virtual int InitFilter(){ return -1;};

            // �˾�����YUV I420 ��Ƶ����
            virtual void FilterProcessI420Data(unsigned char * data, int frame_len, int frame_w, int frame_h) {}

            // �˾�����RGB����
            virtual void FilterProcessRGBAData(unsigned char * data, int frame_len, int frame_w, int frame_h) {}

            // ˢ���˾�����ȼ����������յȼ�
            virtual bool UpdateFilterLevel(int level){ return false; }

            virtual void Release() {}

            virtual bool SetParameter(const char *param) { return false;};
 
        };
    }
}

#endif // VideoFilterProcessBase_h__


