
本工程为ZEGO FaceUnity 的美颜插件工程，使用本工程编译出来的插件，在结合ZEGO Electron SDK使用，即可在Electron 平台使用美颜功能。


注：需要客户先了解基础的nodejs扩展开发环境配置 

如果没安装VS2017,请先安装VS2017。

1. 安装依赖

```
cnpm install nan
cnpm install -g node-gyp
```

2. 编译插件

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

3. 编译成功后，在当前目录下会生成 electron_libs_out 和 electron_libs_pdbs

在 electron_libs_out 目录下的x86或者x64目录会生成以下3个文件：

```
libsgemm.dll
nama.dll
ZegoVideoFilter.node
```

4.使用插件

先拷贝 libsgemm.dll、nama.dll和ZegoVideoFilter.node 到工程目录的 node_modules/zegoliveroom 目录下

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
```

6. 参考demo

请参考zego-electron-quick-start的 fu_beauty 分支，链接为：

https://github.com/zegoim/zego-electron-quick-start/tree/fu_beauty


7. 获取FaceUnity license

    拨打电话 0571-89774660

    发送邮件至 marketing@faceunity.com 进行咨询。
    
    相关美颜参考
    
    http://www.faceunity.com/
    

8. 资源文件[v3.bundle](https://github.com/zegoim/faceunity-electron-addon/tree/master/sdk/fusdk/assets), [face_beautification.bundle](https://github.com/zegoim/faceunity-electron-addon/tree/master/sdk/fusdk/assets)





























