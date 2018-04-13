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

引用方法如下patch:
lilei@gome07-OptiPlex-7040:~/codes/gm_7.0_mtk6750_develop_verify/frameworks/base$ git diff
diff --git a/Android.mk b/Android.mk
index 95a6aa1..9066045 100755
--- a/Android.mk
+++ b/Android.mk
@@ -43,6 +43,14 @@ gome_res_source_path := APPS/gome-res_intermediates/src
 # embedded builds use nothing in frameworks/base
 ifneq ($(ANDROID_BUILD_EMBEDDED),true)
 
+
+
+##################################################for test yuv by lilei
+include $(CLEAR_VARS)
+LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libGomeTestYuv:libs/gomeTestYuv/libs/javalib.jar
+include $(BUILD_MULTI_PREBUILT)
+##################################################
+
 include $(CLEAR_VARS)
 
 # FRAMEWORKS_BASE_SUBDIRS comes from build/core/pathmap.mk
@@ -623,6 +631,8 @@ ifeq ($(MTK_3GDONGLE_SUPPORT),yes)
 else
 # LOCAL_STATIC_JAVA_LIBRARIES += viatelecomjar
 endif
+#test by lilei
+LOCAL_STATIC_JAVA_LIBRARIES += libGomeTestYuv
 
 # M: need to explicitly declare these required shared libraries
 LOCAL_REQUIRED_MODULES := libRS librs_jni
diff --git a/core/java/android/hardware/Camera.java b/core/java/android/hardware/Camera.java
old mode 100644
new mode 100755
index 347eb21..01a5e10
--- a/core/java/android/hardware/Camera.java
+++ b/core/java/android/hardware/Camera.java
@@ -70,7 +70,7 @@ import android.hardware.camera2.CameraCharacteristics;
 
 
 import static android.system.OsConstants.*;
-
+import com.gome.gometestyuv.TestManager;
