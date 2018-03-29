# GomeTestYuv
万能扫功能：实现编译so库，frameworkjava中引用jni引用so库，编译jni jar文件
1.编译so库 
~/codes/项目名称/packages/apps/GomeTestYuv/jni
执行 mm -B 编译到out下面的lib和lib64 目录libGomeTestYuv.so
2.编译jar包
~/codes/项目名称/packages/apps/GomeTestYuv/
执行 mm -B 编译到out下面的.../GomeTestYuv_intermediates/javalib.jar
将编译出来的 javalib.jar拷贝到自定义的目录F:\frameworks\base\libs\gomeTestYuv\libs\中
注：TestManager.java System.loadLibrary("GomeTestYuv");  动态加载libGomeTestYuv.so
还需要考虑如何将上述的libGomeTestYuv.so编译到rom，暂时push到手机system/lib和lib64目录
3.修改framework引用
/frameworks/base/Android.mk
在Android.mk 添加对上述拷贝过来的javalib.jar的引用
