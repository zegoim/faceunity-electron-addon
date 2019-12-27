
#include "FuBeautifyFilter.h"

#include <string>
#include <vector>
#include <fstream>
#include <thread>

#include "GlobalConfig.h"

#include "funama.h"


#ifdef WIN32

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#endif



namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {

        FuBeautifyFilter::FuBeautifyFilter()
        {

        }


        FuBeautifyFilter::~FuBeautifyFilter()
        {

        }

        size_t FileSize(std::ifstream& file)
        {
            std::streampos oldPos = file.tellg();
            file.seekg(0, std::ios::beg);
            std::streampos beg = file.tellg();
            file.seekg(0, std::ios::end);
            std::streampos end = file.tellg();
            file.seekg(oldPos, std::ios::beg);
            return static_cast<size_t>(end - beg);
        }

        bool LoadBundle(const std::string& filepath, std::vector<char>& data)
        {
            std::ifstream fin(filepath, std::ios::binary);
            if (false == fin.good())
            {
                fin.close();
                return false;
            }
            size_t size = FileSize(fin);
            if (0 == size)
            {
                fin.close();
                return false;
            }
            data.resize(size);
            fin.read(reinterpret_cast<char*>(&data[0]), size);

            fin.close();
            return true;
        }

        bool FuBeautifyFilter::InitFilter()
        {
            if (!InitOpenGL())
            {
                inited_ = false;
                return false;
            }

            if (!InitFuSdk())
            {
                inited_ = false;
                return false;
            }

            if (!LoadFuResource())
            {
                inited_ = false;
                return false;
            }

            // 设置默认美颜等级
            UpdateFilterLevel(5);

            inited_ = true;

            return true;
        }

        void FuBeautifyFilter::FilterProcessI420Data(unsigned char * data, int frame_len, int frame_w, int frame_h)
        {

        }

        bool FuBeautifyFilter::UpdateFilterLevel(int level)
        {            
            if (!inited_)
            {
                return false;
            }

            // 肤色检测开关，0为关，1为开
            fuItemSetParamd(beauty_handles_, "skin_detect", 1);

            // blur_level: 磨皮程度，取值范围0.0-6.0，默认6.0
            fuItemSetParamd(beauty_handles_, "blur_level", level * 6.0 / 10.0);

            // 美白 color_level 取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值0.2
            fuItemSetParamd(beauty_handles_, "color_level", level * 1.0 / 10.0);

            // 红润 red_level 取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值0.5
            fuItemSetParamd(beauty_handles_, "red_level", level * 1.0 / 10.0);

            // 亮眼 eye_bright   取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
            fuItemSetParamd(beauty_handles_, "eye_bright", level * 1.0 / 10.0);

            // 美牙 tooth_whiten   取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
            fuItemSetParamd(beauty_handles_, "tooth_whiten", level * 1.0 / 10.0);

            return true;
        }

        void FuBeautifyFilter::FilterProcessRGBAData(unsigned char * data, int frame_len, int frame_w, int frame_h)
        {
            if (inited_)
            {
                fuRenderItemsEx2(FU_FORMAT_RGBA_BUFFER, reinterpret_cast<int*>(data), FU_FORMAT_RGBA_BUFFER, reinterpret_cast<int*>(data), frame_w, frame_h, frame_id_++, handles_.data(), handles_.size(), NAMA_RENDER_FEATURE_FULL, NULL);
            }
        }

        void FuBeautifyFilter::Release()
        {
            inited_ = false;
            if (hglrc_)
            {
                wglDeleteContext(hglrc_);
                hglrc_ = nullptr;
            }
            fuOnDeviceLost();
            fuDestroyAllItems();
        }

        bool FuBeautifyFilter::InitFuSdk()
        {
            // 初始化fu sdk
            std::vector<char> v3data;

            if (GlobalConfigInstance()->v3_bundle_path_ == "")
            {
                return false;
            }

            if(LoadBundle(GlobalConfigInstance()->v3_bundle_path_, v3data))
            {
                int result_code = fuSetup(reinterpret_cast<float*>(&v3data[0]), v3data.size(), NULL, GlobalConfigInstance()->auth_package_data_.data(), GlobalConfigInstance()->auth_package_data_.size());
                if (result_code == 0)
                {
                    return false;
                }
                return true;
            }
            else {
                return false;
            }
        }

        bool FuBeautifyFilter::LoadFuResource()
        {
            //读取美颜道具    
            std::vector<char> propData;
            if (false == LoadBundle(GlobalConfigInstance()->face_beauty_bundle_path_, propData))
            {
                return false;
            }

            beauty_handles_ = fuCreateItemFromPackage(&propData[0], propData.size());
            handles_.push_back(beauty_handles_);
            return true;
        }


        bool FuBeautifyFilter::InitOpenGL()
        {

        #ifdef WIN32
            PIXELFORMATDESCRIPTOR pfd = {
                sizeof(PIXELFORMATDESCRIPTOR),
                1u,
                PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW,
                PFD_TYPE_RGBA,
                32u,
                0u, 0u, 0u, 0u, 0u, 0u,
                8u,
                0u,
                0u,
                0u, 0u, 0u, 0u,
                24u,
                8u,
                0u,
                PFD_MAIN_PLANE,
                0u,
                0u, 0u };

            HWND hw = CreateWindowExA(
                0, "EDIT", "", ES_READONLY,
                0, 0, 1, 1,
                NULL, NULL,
                GetModuleHandleA(NULL), NULL);

            HDC hgldc = GetDC(hw);
            int spf = ChoosePixelFormat(hgldc, &pfd);
            if (!spf)
            {
                return false;
            }
            int ret = SetPixelFormat(hgldc, spf, &pfd);

            hglrc_ = wglCreateContext(hgldc);
            wglMakeCurrent(hgldc, hglrc_);

            if (hglrc_ != nullptr)
            {
                //OpenGL函数的地址，如果4个中任意一个不为空值，则OpenGL环境是可用的
                //此后的所有Nama SDK调用都会基于这个context
                //如果客户端有其他绘制创建了另外的OpenGL context，那么请确保调用Nama接口时一直是同一个context
                if (wglGetProcAddress("glGenFramebuffersARB") != nullptr
                    || wglGetProcAddress("glGenFramebuffersOES") != nullptr
                    || wglGetProcAddress("glGenFramebuffersEXT") != nullptr
                    || wglGetProcAddress("glGenFramebuffers") != nullptr)
                {
                    return true;
                }
                else {
                    return false;
                }
            }
            else {
                return false;
            }

        #else

            return false;

        #endif 

        }

    }
}
