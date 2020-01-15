
#include "FuBeautifyFilter.h"

#include <string>
#include <vector>
#include <fstream>
#include <thread>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include "GlobalConfig.h"

#include "funama.h"

#ifdef WIN32

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#else
#include <dlfcn.h>
#include <OpenGL/OpenGL.h>
#include <unistd.h>

#endif

using namespace rapidjson;

namespace ZEGO
{
    namespace VIDEO_BEAUTY_FILTER
    {

#define MAX_PROPERTY_NAME 256
#define MAX_PROPERTY_VALUE 512

        FuBeautifyFilter::FuBeautifyFilter()
        {
        }

        FuBeautifyFilter::~FuBeautifyFilter()
        {
        }

        size_t FileSize(std::ifstream &file)
        {
            std::streampos oldPos = file.tellg();
            file.seekg(0, std::ios::beg);
            std::streampos beg = file.tellg();
            file.seekg(0, std::ios::end);
            std::streampos end = file.tellg();
            file.seekg(oldPos, std::ios::beg);
            return static_cast<size_t>(end - beg);
        }

        bool LoadBundle(const std::string &filepath, std::vector<char> &data)
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
            fin.read(reinterpret_cast<char *>(&data[0]), size);

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

        void FuBeautifyFilter::FilterProcessI420Data(unsigned char *data, int frame_len, int frame_w, int frame_h)
        {
        }

        // https://github.com/Faceunity/FULivePC/blob/master/docs/Beautification_Filters_User_Specification.md
        // https://github.com/Faceunity/FULivePC/blob/master/docs/%E7%BE%8E%E9%A2%9C%E9%81%93%E5%85%B7%E5%8A%9F%E8%83%BD%E6%96%87%E6%A1%A3.md
        bool FuBeautifyFilter::UpdateFilterLevel(int level)
        {
            if (!inited_)
            {
                return false;
            }

            // 肤色检测开关，0为关，1为开
            fuItemSetParamd(beauty_handle_, "skin_detect", 1);

            // blur_level: 磨皮程度，取值范围0.0-6.0，默认6.0
            fuItemSetParamd(beauty_handle_, "blur_level", level * 6.0 / 10.0);

            // 美白 color_level 取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值0.2
            fuItemSetParamd(beauty_handle_, "color_level", level * 1.0 / 10.0);

            // 红润 red_level 取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值0.5
            fuItemSetParamd(beauty_handle_, "red_level", level * 1.0 / 10.0);

            // 亮眼 eye_bright   取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
            fuItemSetParamd(beauty_handle_, "eye_bright", level * 1.0 / 10.0);

            // 美牙 tooth_whiten   取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
            fuItemSetParamd(beauty_handle_, "tooth_whiten", level * 1.0 / 10.0);

            return true;
        }

        void FuBeautifyFilter::FilterProcessRGBAData(unsigned char *data, int frame_len, int frame_w, int frame_h)
        {
            if (inited_)
            {
                UpdateBeautyParamIfNeeded();

                fuRenderItemsEx2(FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int *>(data), FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int *>(data), frame_w, frame_h, frame_id_++, bundle_handles_.data(), bundle_handles_.size(), NAMA_RENDER_FEATURE_FULL, NULL);
            }
        }

        void FuBeautifyFilter::UpdateBeautyParamIfNeeded()
        {
            if (need_load_bundles_)
            {
                bundle_handles_.clear();

                for (int i = 0; i < face_unity_bundles_.size(); ++i)
                {
                    std::vector<char> prop_data;

                    if (false == LoadBundle(face_unity_bundles_[i].bundlePath + face_unity_bundles_[i].bundleName, prop_data))
                    {
                        continue;
                    }

                    //printf("loaded bundle path : %s \n", (face_unity_bundles_[i].bundlePath + face_unity_bundles_[i].bundleName).c_str());
                    face_unity_bundles_[i].bundle_handle = fuCreateItemFromPackage(&prop_data[0], (int)prop_data.size());

                    if(face_unity_bundles_[i].bundle_handle <= 0)
                    {
                        printf("loaded bundle path : %s, failed!!!\n", (face_unity_bundles_[i].bundlePath + face_unity_bundles_[i].bundleName).c_str());
                        continue;
                    }

                    bundle_handles_.push_back(face_unity_bundles_[i].bundle_handle);

                    if (face_unity_bundles_[i].bundleName == "face_beautification.bundle")
                    {
                        beauty_handle_ = face_unity_bundles_[i].bundle_handle;
                    }

                    Document d;
                    d.Parse(face_unity_bundles_[i].options.c_str());
                    for (Value::ConstMemberIterator itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
                    {
                        const char* propertyName = itr->name.GetString();

                        if (strlen(propertyName) > MAX_PROPERTY_NAME)
                        {
                            continue;
                        }

                        const Value& propertyValue = itr->value;
                        if (propertyValue.IsNumber())
                        {
                            int ret = fuItemSetParamd(face_unity_bundles_[i].bundle_handle, (char*)propertyName, propertyValue.GetDouble());

                            //printf("update handle = %d , ret = %d, name = %s, value = %.4f \n", face_unity_bundles_[i].bundle_handle, ret, propertyName, propertyValue.GetDouble());
                        }
                        else if (propertyValue.IsString())
                        {
                            if (strlen(propertyValue.GetString()) > MAX_PROPERTY_VALUE)
                            {
                                continue;
                            }
                            
                            int ret = fuItemSetParams(face_unity_bundles_[i].bundle_handle, (char*)propertyName, (char*)propertyValue.GetString());

                            //printf("update handle = %d , ret = %d, name = %s, value = %s \n", face_unity_bundles_[i].bundle_handle, ret , propertyName, propertyValue.GetString());
                        }
                    }
                }
                need_load_bundles_ = false;
            }

            if (need_update_bundles_)
            {
                printf("fuIsTracking() = %d \n", fuIsTracking());
                if(fuIsTracking() <= 0)
                {
                    printf("fuGetSystemError() = %d \n", fuGetSystemError());
                }
                for (int i = 0; i < face_unity_bundles_.size(); ++i)
                {
                    if (!face_unity_bundles_[i].need_update)
                    {
                        continue;
                    }

                    Document d;
                    d.Parse(face_unity_bundles_[i].options.c_str());
                    for (Value::ConstMemberIterator itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
                    {
                        const char* propertyName = itr->name.GetString();
                        if (strlen(propertyName) > MAX_PROPERTY_NAME)
                        {
                            continue;
                        }

                        const Value& propertyValue = itr->value;
                        int result = -1;
                        if (propertyValue.IsNumber())
                        {
                            int ret = fuItemSetParamd(face_unity_bundles_[i].bundle_handle, (char*)propertyName, propertyValue.GetDouble());

                            //printf("update handle = %d , ret = %d, name = %s, value = %.4f \n", bundles[i].bundle_handle, ret, propertyName, propertyValue.GetDouble());
                        }
                        else if (propertyValue.IsString())
                        {
                            if (strlen(propertyValue.GetString()) > MAX_PROPERTY_VALUE)
                            {
                                continue;
                            }
                            int ret = fuItemSetParams(face_unity_bundles_[i].bundle_handle, (char*)propertyName, (char*)propertyValue.GetString());

                            //printf("update handle = %d , ret = %d, name = %s, value = %s \n", bundles[i].bundle_handle, ret, propertyName, propertyValue.GetString());

                        }
                    }
                    face_unity_bundles_[i].need_update = false;
                }
                need_update_bundles_ = false;
            }
        }

        void FuBeautifyFilter::Release()
        {
            inited_ = false;
#ifdef WIN32
            if (hglrc_)
            {
                wglDeleteContext(hglrc_);
                hglrc_ = nullptr;
            }
#endif
            fuOnDeviceLost();
            fuDestroyAllItems();
        }

        bool FuBeautifyFilter::InitFuSdk()
        {

            printf("fu version = %s \n", fuGetVersion());

            // 初始化fu sdk
            std::vector<char> v3data;

            if (GlobalConfigInstance()->v3_bundle_path_ == "")
            {
                return false;
            }

            if (LoadBundle(GlobalConfigInstance()->v3_bundle_path_, v3data))
            {
                int result_code = fuSetup(reinterpret_cast<float *>(&v3data[0]), v3data.size(), NULL, GlobalConfigInstance()->auth_package_data_.data(), GlobalConfigInstance()->auth_package_data_.size());
                if (result_code == 0)
                {
                    return false;
                }
                return true;
            }
            else
            {
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
            beauty_handle_ = fuCreateItemFromPackage(&propData[0], propData.size());
            if(beauty_handle_ <= 0)
            {
                printf("loaded bundle path : %s, failed!!!\n", GlobalConfigInstance()->face_beauty_bundle_path_.c_str());
                return false;
            }
            bundle_handles_.push_back(beauty_handle_);

            FaceUnityBundle bundle;
            bundle.bundleName = "face_beautification.bundle";
            bundle.bundlePath = GlobalConfigInstance()->face_beauty_bundle_path_;
            bundle.bundle_handle = beauty_handle_;
            bundle.need_update = false;
            bundle.options = "";

            face_unity_bundles_.push_back(bundle);

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
                if (wglGetProcAddress("glGenFramebuffersARB") != nullptr || wglGetProcAddress("glGenFramebuffersOES") != nullptr || wglGetProcAddress("glGenFramebuffersEXT") != nullptr || wglGetProcAddress("glGenFramebuffers") != nullptr)
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }

#else

            CGLPixelFormatAttribute attrib[13] = { kCGLPFAOpenGLProfile,
                                                  (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy,
                                                  kCGLPFAAccelerated,
                                                  kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
                                                  kCGLPFAAlphaSize, (CGLPixelFormatAttribute)8,
                                                  kCGLPFADoubleBuffer,
                                                  kCGLPFASampleBuffers, (CGLPixelFormatAttribute)1,
                                                  kCGLPFASamples, (CGLPixelFormatAttribute)4,
                                                  (CGLPixelFormatAttribute)0 };
            CGLPixelFormatObj pixelFormat = NULL;
            GLint numPixelFormats = 0;
            CGLContextObj cglContext1 = NULL;
            CGLChoosePixelFormat(attrib, &pixelFormat, &numPixelFormats);
            CGLError err = CGLCreateContext(pixelFormat, NULL, &cglContext1);
            CGLSetCurrentContext(cglContext1);
            if (err)
            {
                return false;
            }
            return true;

#endif
        }

        bool FuBeautifyFilter::SetParameter(const char *param)
        {
            Document d;
            d.Parse(param);

            if (d.HasParseError())
            {
                return false;
            }

            if (d.HasMember("plugin.fu.bundles.load"))
            {
                face_unity_bundles_.clear();
                Value& bundlesData = d["plugin.fu.bundles.load"];
                if (!bundlesData.IsArray())
                {
                    return false;
                }

                int bundleLength = bundlesData.Capacity();
                for (int i = 0; i < bundleLength; i++)
                {
                    Value& bundleData = bundlesData[i];
                    if (!bundleData.HasMember("bundleName") || !bundleData.HasMember("bundleOptions") || !bundleData.HasMember("bundlePath"))
                    {
                        return false;
                    }
                    Value& bundleName = bundleData["bundleName"];
                    if (!bundleName.IsString())
                    {
                        return false;
                    }

                    Value& bundlePath = bundleData["bundlePath"];
                    if (!bundlePath.IsString())
                    {
                        return false;
                    }

                    FaceUnityBundle bundle;
                    bundle.bundleName = bundleName.GetString();
                    bundle.bundlePath = bundlePath.GetString();

                    StringBuffer sb;
                    Writer<StringBuffer> writer(sb);
                    bundleData["bundleOptions"].Accept(writer);

                    bundle.options = sb.GetString();

                    face_unity_bundles_.push_back(bundle);
                }
                need_load_bundles_ = true;
            }

            if (d.HasMember("plugin.fu.bundles.update"))
            {
                Value& updateBundleData = d["plugin.fu.bundles.update"];
                if (!updateBundleData.IsObject())
                {
                    return false;
                }

                if (!updateBundleData.HasMember("bundleName") || !updateBundleData.HasMember("bundleOptions"))
                {
                    return false;
                }

                std::string bundleName = updateBundleData["bundleName"].GetString();

                for (auto t = face_unity_bundles_.begin(); t != face_unity_bundles_.end(); ++t)
                {
                    if (bundleName == t->bundleName)
                    {
                        StringBuffer sb;
                        Writer<StringBuffer> writer(sb);
                        updateBundleData["bundleOptions"].Accept(writer);
                        t->options = sb.GetString();
                        t->need_update = true;
                    }
                }

                need_update_bundles_ = true;
            }

            return true;
        }


    } // namespace VIDEO_BEAUTY_FILTER
} // namespace ZEGO
