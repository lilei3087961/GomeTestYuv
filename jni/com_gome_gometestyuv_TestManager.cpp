#include "android_runtime/AndroidRuntime.h"
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include "com_gome_gometestyuv_TestManager.h"
#include <android/log.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus 
extern "C" { 
#endif
#include "gomeMultiScan.h"
#ifdef __cplusplus
}
#endif

#define LOG_TAG "Test_JNI"

JNIEXPORT void JNICALL Java_com_gome_gometestyuv_TestManager_nativeInit
  (JNIEnv *env, jclass)  {
	ALOGD("lilei call Java_com_gome_gometestyuv_TestManager_nativeInit");
}

/*
 * Class:     Java_com_example_gomeAccountClient_TestManager
 * Method:    native_add
 * Signature: (II)I
 */
JNIEXPORT jint JNICALL Java_com_gome_gometestyuv_TestManager_nativeAdd
  (JNIEnv *env, jclass, jint num1, jint num2) {
	gome_testYuv();
	ALOGD("lilei call Java_com_gome_gometestyuv_TestManager_nativeAdd num1:%d num2:%d",num1,num2);
	return num1+num2;
}
/*
 * Class:     com_gome_gometestyuv_TestManager
 * Method:    nativeMergeJpg2Yuv
 * Signature: (II)I
 */
JNIEXPORT jbyteArray JNICALL Java_com_gome_gometestyuv_TestManager_nativeMergeJpg2Yuv
   (JNIEnv *env, jclass, jstring jpeg_path,jint width_small,jint height_small,jbyteArray byte_array_camera_buff,jint width_big,jint height_big){
	const char *jpegPath = env->GetStringUTFChars(jpeg_path, NULL);
	jbyte * jbyteCameraBuff = env->GetByteArrayElements(byte_array_camera_buff, 0);
	unsigned char *cameraBuff = (unsigned char *)jbyteCameraBuff;
	ALOGD("lilei call Java_com_gome_gometestyuv_TestManager_nativeMergeJpg2Yuv() jpegPath:%s width_small:%d height_small:%d cameraBuff:%p width_big:%d height_big:%d"
			,jpegPath,width_small,height_small,cameraBuff,width_big,height_big);
    if(jpegPath == NULL) {
        return NULL; /* OutOfMemoryError already thrown */
    }
    gome_merge_jpeg2yuv_for_nv21(jpegPath,(int)width_small,(int)height_small,cameraBuff,(int)width_big,(int)height_big);
    //生成返回类型
    int bufferSize = width_big*height_big*3/2;
    jbyteArray byteArray =env->NewByteArray(bufferSize);//创建一个byte数组
    env->SetByteArrayRegion(byteArray, 0, bufferSize, (jbyte*)cameraBuff);

    env->ReleaseStringUTFChars(jpeg_path, jpegPath); //release jinbuff
    env->ReleaseByteArrayElements(byte_array_camera_buff,jbyteCameraBuff,0);
    return byteArray;
}
