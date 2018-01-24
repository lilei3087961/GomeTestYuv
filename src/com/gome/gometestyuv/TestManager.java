package com.gome.gometestyuv;

import java.io.File;

import android.util.Log;

public class TestManager {
	public static String TAG = "lilei_TestManager";
	static {
        try {  
            System.loadLibrary("GomeTestYuv");  
        } catch (Exception e) {
        	e.printStackTrace();
        	Log.d(TAG,e.getMessage());
        }  		
		Log.d(TAG, "native_init");
		nativeInit();
	}
	
	public int add(int i,int j) {
		File f =new File("/sdcard/DCIM/camera.yuv");
		Log.d(TAG,"add f.exists():"+f.exists());
		return nativeAdd(i,j);
	}
	public byte[] mergeJpeg2Yuv(String jpeg_path,int width_small,int height_small,byte[] camera_buff,int width_big,int height_big){
		Log.i(TAG, "mergeJpeg2Yuv() jpeg_path:"+jpeg_path+" width_small:"+width_small+
			" camera_buff.length:"+camera_buff.length+" width_big:"+width_big+" height_big:"+height_big);
		byte[] result = nativeMergeJpg2Yuv(jpeg_path,width_small,height_small,camera_buff,width_big,height_big);
		return result;
	}
	private static native void nativeInit();
	private static native int nativeAdd(int i,int j);
	private static native byte[] nativeMergeJpg2Yuv(String jpeg_path,int width_small,int height_small,byte[] camera_buff,int width_big,int height_big);
}
