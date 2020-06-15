
本工程为ZEGO FaceUnity 的美颜插件工程，使用本工程编译出来的插件，在结合ZEGO Electron SDK使用，即可在Electron 平台使用美颜功能。


注：需要客户先了解基础的nodejs扩展开发环境配置 

Windows 平台如果没安装VS2017,请先安装VS2017。
Mac 平台如果没有安装XCode,请先安装XCode。

1. 安装依赖

```
cnpm install nan
cnpm install -g node-gyp
```

2. Windows 平台编译插件

参考 build_x86.bat 文件编译32位版本的插件

例如编译：Electron 5.0.12 版本的32位插件如下：
```
set x64=false & set plugin_version="1.0.0" & node-gyp rebuild --target=5.0.12 --arch=ia32 --dist-url=https://atom.io/download/electron
```

参考 build_x64.bat 文件编译64位版本的插件

例如编译：Electron 5.0.12 版本的64位插件如下：
```
set x64=true & set plugin_version="1.0.0" & node-gyp rebuild --target=5.0.12 --arch=x64 --dist-url=https://atom.io/download/electron
```

编译其它Electron版本的插件，只需要修改--target=后面的版本号，重新执行命令编译即可

编译成功后，在当前目录下会生成 electron_libs_out 和 electron_libs_pdbs

在 electron_libs_out 目录下的x86或者x64目录会生成以下3个文件：

```
libsgemm.dll
nama.dll
ZegoVideoFilter.node
```

3. Mac 平台编译插件

参考build_mac.sh 文件编译mac 版本的插件。

```
export plugin_version=\"1.0.0\" && node-gyp rebuild --target=5.0.12 --arch=x64 --dist-url=https://atom.io/download/electron
```
编译其它Electron版本的插件，只需要修改--target=后面的版本号，重新执行命令编译即可。

编译成功后，在build/Rease目录下会生成 ZegoVideoFilter.node 文件。

4.使用插件

Windows平台先拷贝 libsgemm.dll、nama.dll和ZegoVideoFilter.node 到工程目录的 node_modules/zegoliveroom 目录下。

Mac 平台先拷贝ZegoVideoFilter.node 到工程目录的 node_modules/zegoliveroom 目录下。
然后在工程中进行引入：

```
// 引入FaceUnity的滤镜插件
var ZegoVideoFilterDemo = require("zegoliveroom/ZegoVideoFilter.node");

// 获取外部滤镜插件工厂
let fac = ZegoVideoFilterDemo.getVideoFilterFactory()

// 设置外部滤镜工厂，必须在initSdk之前调用
// zegoClient 是 ZEGO Electron SDK ZegoLiveRoom 的实例
zegoClient.setVideoFilterFactory({factory: fac})

// 初始化FaceUnity美颜滤镜的参数
// 参数1：填FaceUnity 的license ，形式为 [-123,23,34,-34,45] 
// 参数2：填FaceUnity 的资源文件 v3.bundle 的路径
// 参数3：填FaceUnity 的资源文件 face_beautification.bundle 的路径
// 参数4：初始化回调，返回结果对象中error_code 为 0 - 成功， -1 - 失败
// 返回值：false - 参数无效，true - 正在异步初始化美颜库
// 
let init_fu_sdk_ret = ZegoVideoFilterDemo.initFuBeautyConfig([此处填写FaceUnity的license，形式一个数组], "此处填写FaceUnity 的 v3.bundle的文件路径", "此处填写FaceUnity 的 face_beautification.bundle的文件路径", function(rs){
      console.log(rs)
	  // -2 初始化openGL失败
	  // -3 初始化fu sdk 失败，通常可能是FaceUnity 的license填得不对
	  // -4 加载fu资源失败，通常可能是设置的FaceUnity 的资源路径有问题
      if(rs.error_code == 0)
      {
          ZegoVideoFilterDemo.enableBeauty(true);
          
          let fu_config_ret = ZegoVideoFilterDemo.updateBeautyLevel(10);
          
          if(fu_config_ret == true)
          {
              console.log("美颜配置成功");
          }else{
              console.log("美颜配置失败");
          }
      }
    }
  );
      
if(!init_fu_sdk_ret)
{
  console.log("美颜配置失败");
}

```

5. 相关接口说明

```
// 美颜开关
// true - 打开美颜，false - 关闭美颜
ZegoVideoFilterDemo.enableBeauty(true);


// 调节美颜等级，参数范围为 1到10，1美颜程度最轻，10程度最大
// 返回值，true-成功，false-失败
ZegoVideoFilterDemo.updateBeautyLevel(10);


// 美颜精细参数控制

// filter_level 取值范围 0.0-1.0, 0.0为无效果，1.0为最大效果，默认值1.0
var filter_level = 1.0
// filter_name 取值为一个字符串，默认值为 “origin” ，origin即为使用原图效果
var filter_name = "origin"
// 美白
// color_level 取值范围 0.0-1.0, 0.0为无效果，1.0为最大效果，默认值0.2
var color_level = 0.2
// 红润
// red_level 取值范围 0.0-1.0, 0.0为无效果，1.0为最大效果，默认值0.5
var red_level = 0.5
// 磨皮
// 控制磨皮的参数有四个：blur_level，skin_detect，nonskin_blur_scale，heavy_blur，blur_type
// blur_level: 磨皮程度，取值范围0.0-6.0，默认6.0
var blur_level = 1
// skin_detect:肤色检测开关，0为关，1为开
var skin_detect = 1
// nonskin_blur_scale:肤色检测之后非肤色区域的融合程度，取值范围0.0-1.0，默认0.45
var nonskin_blur_scale = 0.45
// heavy_blur: 朦胧磨皮开关，0为清晰磨皮，1为朦胧磨皮
var heavy_blur = 0
// blur_type：此参数优先级比heavy_blur低，在使用时要将heavy_blur设为0，0 清晰磨皮  1 朦胧磨皮  2精细磨皮
var blur_type = 2
// 亮眼 eye_bright        取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
var eye_bright = 1.0
// 美牙 tooth_whiten      取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
var tooth_whiten = 1.0
// 美型 face_shape_level   取值范围 0.0-1.0,0.0为无效果，1.0为最大效果，默认值1.0
var face_shape_level = 1.0
// 美型的渐变由 change_frames 参数控制
// change_frames 0为关闭，大于0开启渐变，值为渐变所需要的帧数
var change_frames = 0
// 美型的种类主要由 face_shape 参数控制
// face_shape: 变形取值 0:女神变形 1:网红变形 2:自然变形 3:默认变形 4:精细变形
var face_shape = 3
// face_shape 为0 1 2 3时
// 对应0：女神 1：网红 2：自然 3：默认
// 可以使用参数
// eye_enlarging:     默认0.5, 大眼程度范围0.0-1.0
// cheek_thinning:    默认0.0, 瘦脸程度范围0.0-1.0

// face_shape 为4时，为用户自定义的精细变形，开放了脸型相关参数，添加了窄脸小脸参数
// eye_enlarging:     默认0.5, 大眼程度范围0.0-1.0
// cheek_thinning:    默认0.0, 瘦脸程度范围0.0-1.0
// cheek_v:           默认0.0, v脸程度范围0.0-1.0
// cheek_narrow:      默认0.0, 窄脸程度范围0.0-1.0
// cheek_small:       默认0.0, 小脸程度范围0.0-1.0
// intensity_nose:    默认0.0, 瘦鼻程度范围0.0-1.0
// intensity_forehead:默认0.5, 额头调整程度范围0.0-1.0，0-0.5是变小，0.5-1是变大
// intensity_mouth:   默认0.5, 嘴巴调整程度范围0.0-1.0，0-0.5是变小，0.5-1是变大
// intensity_chin:    默认0.5, 下巴调整程度范围0.0-1.0，0-0.5是变小，0.5-1是变大
var eye_enlarging = 0.5
var cheek_thinning = 0.0
var cheek_v = 0.0
var cheek_narrow = 0.0
var cheek_small = 0.0
var intensity_nose = 0.0
var intensity_forehead = 0.5
var intensity_mouth = 0.5
var intensity_chin = 0.5

let update_param_obj = {"plugin.fu.bundles.update": {
                    bundleName: "face_beautification.bundle",
                    bundleOptions: {
                      "filter_level":filter_level,
                      "filter_name":filter_name,
                      "color_level": color_level,
                      "red_level": red_level,
                      "blur_level": blur_level,
                      "skin_detect": skin_detect,
                      "heavy_blur": heavy_blur,
                      "blur_type": blur_type,
                      "eye_bright": eye_bright,
                      "tooth_whiten": tooth_whiten,
                      "face_shape_level": face_shape_level,
                      "change_frames": change_frames,
                      "face_shape": face_shape,
                      "eye_enlarging": eye_enlarging,
                      "cheek_thinning": cheek_thinning,
                      "cheek_v": cheek_v,
                      "cheek_narrow": cheek_narrow,
                      "cheek_small": cheek_small,
                      "intensity_nose": intensity_nose,
                      "intensity_forehead": intensity_forehead,
                      "intensity_mouth": intensity_mouth,
                      "intensity_chin": intensity_chin
                    }
                  }}
                  
let ret = ZegoVideoFilterDemo.setParameter(JSON.stringify(update_param_obj))

```

6. 参考demo

请参考zego-electron-quick-start的 fu_beauty 分支，链接为：

https://github.com/zegoim/zego-electron-quick-start/tree/fu_beauty


7. 获取FaceUnity license

    拨打电话 0571-89774660

    发送邮件至 marketing@faceunity.com 进行咨询。
    
    相关美颜参考
    
    http://www.faceunity.com/
    
    https://github.com/Faceunity/FULivePC/blob/master/docs/Beautification_Filters_User_Specification.md
    
    https://github.com/Faceunity/FULivePC/blob/master/docs/%E7%BE%8E%E9%A2%9C%E9%81%93%E5%85%B7%E5%8A%9F%E8%83%BD%E6%96%87%E6%A1%A3.md
    

8. 资源文件[v3.bundle](https://github.com/zegoim/faceunity-electron-addon/tree/master/sdk/fusdk/assets), [face_beautification.bundle](https://github.com/zegoim/faceunity-electron-addon/tree/master/sdk/fusdk/assets)





























