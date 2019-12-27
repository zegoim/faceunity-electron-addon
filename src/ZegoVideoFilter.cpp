#include <nan.h>
#include <iostream>
#include <string>

#include "ZGExternalVideoFilterFactory.h"
#include "GlobalConfig.h"
#include "JsCallBackInfo.h"

ZEGO::VIDEO_BEAUTY_FILTER::ZGExternalVideoFilterFactory *FactoryInstance = nullptr;

std::string GetNodeStrParam(Nan::NAN_METHOD_ARGS_TYPE info, int index)
{
    v8::String::Utf8Value v8_str(Nan::To<v8::String>(info[index]).ToLocalChecked());
    std::string str = std::string(*v8_str);
    return str;
}

// 初始化 Fu SDK 配置
NAN_METHOD(InitFuBeautyConfig)
{
    if (info.Length() < 4)
    {
        info.GetReturnValue().Set(Nan::New(false));
        return;
    }

    if (!info[0]->IsArray()) 
    {
        info.GetReturnValue().Set(Nan::New(false));
        return ;
    }

    v8::Local<v8::Array> auth_package = v8::Local<v8::Array>::Cast(info[0]);

    ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->auth_package_data_.clear();

    for (int i = 0; i < auth_package->Length(); ++i)
    {
        ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->auth_package_data_.push_back(auth_package->Get(i)->Int32Value());
    }

    std::string v3_bundle_path = GetNodeStrParam(info, 1);
    std::string face_beautification_bundle_path = GetNodeStrParam(info, 2);

    ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->init_cb_.js_cb = std::shared_ptr<Nan::Callback>(new Nan::Callback(info[3].As<v8::Function>()));
    ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->init_cb_.iso = info.GetIsolate();

    ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->v3_bundle_path_ = v3_bundle_path;
    ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->face_beauty_bundle_path_ = face_beautification_bundle_path;

    if (ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->auth_package_data_.size() == 0
        || ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->v3_bundle_path_ == ""
        || ZEGO::VIDEO_BEAUTY_FILTER::GlobalConfigInstance()->face_beauty_bundle_path_ == "")
    {
        info.GetReturnValue().Set(Nan::New(false));
    }
    else {

        if (FactoryInstance == nullptr)
        {
            FactoryInstance = new ZEGO::VIDEO_BEAUTY_FILTER::ZGExternalVideoFilterFactory();
        }

        FactoryInstance->StartBeautyProcess();

        info.GetReturnValue().Set(Nan::New(true));
    }
}


// 定义函数 获取一个滤镜工厂
NAN_METHOD(GetVideoFilterFactory) 
{
    if (FactoryInstance == nullptr) 
    {
        FactoryInstance = new ZEGO::VIDEO_BEAUTY_FILTER::ZGExternalVideoFilterFactory();
    }
    
    int64_t fac = reinterpret_cast<int64_t>(FactoryInstance);
    //printf("GetVideoFilterFactory int64_t: %lld \n", fac);
    AVE::VideoFilterFactory* factoryBase = dynamic_cast<AVE::VideoFilterFactory*>(FactoryInstance);
    info.GetReturnValue().Set(Nan::New<v8::Number>(reinterpret_cast<int64_t>(factoryBase)));
}

// 定义函数 是否启用美颜滤镜
NAN_METHOD(EnableBeauty) 
{
    if (FactoryInstance == nullptr) 
    {
        info.GetReturnValue().Set(false);
        return;
    }

    bool enable = info[0]->BooleanValue();
    FactoryInstance->EnableBeauty(enable);

    info.GetReturnValue().Set(true);
}

// 定义函数 更新美颜滤镜等级
NAN_METHOD(UpdateBeautyLevel) 
{
    if (FactoryInstance == nullptr) 
    {
        info.GetReturnValue().Set(false);
        return;
    }
    int level = info[0]->Int32Value();
    bool ret = FactoryInstance->UpdateBeautyLevel(level);
    info.GetReturnValue().Set(ret);
}


// 定义模块初始化函数: 参数target相当于module.export
NAN_MODULE_INIT(ZegoVideoFilter_Init) 
{
#ifdef PLUGIN_VERSION

    printf("zego video filter version: %s\n", PLUGIN_VERSION);

#endif
    
    // 往target即module.export上挂东西
    Nan::SetMethod(target, "getVideoFilterFactory", GetVideoFilterFactory);
    Nan::SetMethod(target, "enableBeauty", EnableBeauty);
    Nan::SetMethod(target, "updateBeautyLevel", UpdateBeautyLevel);
    Nan::SetMethod(target, "initFuBeautyConfig", InitFuBeautyConfig);
}


// 告诉nodejs模块初始化函数在哪里
NODE_MODULE(ZegoVideoFilter, ZegoVideoFilter_Init)
