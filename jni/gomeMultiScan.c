#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/videodev2.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "utils/Log.h"
#include <cutils/properties.h>
//下面测试可用打开log方式
//#include <android/log.h>
//#define LOG_TAG "libGomeMultiScan"
//#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, __VA_ARGS__)
//#undef NDEBUG
#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"
#include "gomeMultiScan.h"

#ifdef __cplusplus
}
#endif

#define WIDTH		320
#define	HIGHT		240
#define WIDTH_BIG       1920
#define HIGHT_BIG       1080   
#define FILE_VIDEO  "/dev/video0"
#define JPG "/sdcard/DCIM/image_%s"
typedef struct{
    void *start;
	int length;
}BUFTYPE;
BUFTYPE *usr_buf;

static unsigned int n_buffer = 0;  
unsigned char* mjpeg_buff;
unsigned char* yuyv_buff;
unsigned char* yuv420_buff;
unsigned char* yuv420_buff_for_yv12;
unsigned char* yuv420_buff_for_nv21;
unsigned char* yuv420_buff_big;
unsigned char* nv21_test_buff;
unsigned char* nv21_test_buff_big;

int gome_dump_yuv(void *addr, int length,char* name)
{
	FILE *fp;
	static int num = 0;
	//char image_name[20];
	
	//sprintf(image_name, JPG, name);
	ALOGD("libGomeMultiScan ##gome_dump_yuv 111");
	if((fp = fopen(name, "w")) == NULL)
	{
		perror("gome_dump_yuv libGomeMultiScan Fail to fopen JPG");
		exit(EXIT_FAILURE);
	}
	//fwrite(addr, WIDTH*HIGHT*2 , 1, fp);
        fwrite(addr, length , 1, fp);
	usleep(500);
	fclose(fp);
	return 0;
}
void getQrcodeOffset(int *x,int *y){
	char MODEL_U9[] ="gm22a";
	char MODEL_U7[]="gm12b";
	char WECHAT[]="com.tencent.mm";
	char ALIPAY[]="com.eg.android.AlipayGphone";
	char OFO[]="so.ofo.labofo";
	char MOBIKE[] = "com.mobike.mobikeapp";
	char scanApp[PROPERTY_VALUE_MAX];
	char model[PROPERTY_VALUE_MAX];
	property_get("persist.sys.multiscan_app",scanApp,"null");
	property_get("ro.product.model",model,"null");

	if(0 == strcmp(model,MODEL_U9)){  //u9
		if(0 == strcmp(scanApp,WECHAT)){ //微信864*480 300*300
			*x = 0;
			*y = 0;
		}else if(0 == strcmp(scanApp,ALIPAY)){ //支付宝1920*1080 600*600
			*x = -30;
			*y = 10;
		}else if(0 == strcmp(scanApp,OFO)){  //ofo 1280*720 400*400
			*x = -65;
			*y = 0;
		}else if(0 == strcmp(scanApp,MOBIKE)){ //mobike 1280*720 400*400
			*x = 5;
			*y = 0;
		}
	}else if(0 == strcmp(model,MODEL_U7)){  //u7
		if(0 == strcmp(scanApp,WECHAT)){ //微信1280*640 400*400
			*x = 0;
			*y = 0;
		}else if(0 == strcmp(scanApp,ALIPAY)){ //支付宝 1280*720 400*400
			*x = -20;
			*y = 0;
		}else if(0 == strcmp(scanApp,OFO)){  //ofo 1280*720 400*400
			*x = -65;
			*y = 0;
		}else if(0 == strcmp(scanApp,MOBIKE)){ //mobike 400*400
			*x = 5;
			*y = 0;
		}
	}
	//for test begin
	char testCharOffsetX[PROPERTY_VALUE_MAX];
	char testCharOffsetY[PROPERTY_VALUE_MAX];
	property_get("persist.sys.multiscan_testOffsetX",testCharOffsetX,"0");
	property_get("persist.sys.multiscan_testOffsetY",testCharOffsetY,"0");
	int testOffsetX = atoi(testCharOffsetX);
	int testOffsetY = atoi(testCharOffsetY);
	if(testOffsetX != 0 || testOffsetY != 0){
		*x = testOffsetX;
		*y = testOffsetY;
	}
	//for test end
	ALOGD("libGomeMultiScan getQrcodeOffset() scanApp:%s,model:%s *x:%d,*y:%d,strcmp(scanApp,WECHAT):%d,testOffsetX:%d,testOffsetY:%d",scanApp,model,*x,*y,strcmp(scanApp,WECHAT),testOffsetX,testOffsetY);

}
//两个nv21图片合成
void merge_two_yuv_for_nv21(unsigned char *big_buff,int width_big,int height_big,unsigned char * small_buff,int width_small,int height_small)
{
    ALOGD("libGomeMultiScan merge_two_yuv_for_nv21 big_buff:%p,width_big:%d height_big:%d small_buff:%p width_small:%d height_small:%d \n",big_buff,width_big,height_big,small_buff,width_small,height_small);
    if(width_big<width_small || height_big < height_small)
    {
        ALOGD("libGomeMultiScan merge_two_yuv_for_nv21 error width_big<width_small || height_big < height_small \n");
        return;
    }
    int small_vu_distance = width_small*height_small;
    int large_vu_distance = width_big*height_big;
    //for test
    //gome_dump_yuv(big_buff,width_big*height_big*3/2,"/sdcard/DCIM/image_nv21_big_raw.yuv");
    //return;
    int offset_horizontal = 0; //基于小图居中横向偏移量
    int offset_vertical = 0;   //基于小图居中纵向偏移量
    getQrcodeOffset(&offset_horizontal,&offset_vertical);

    int y_offset = width_big*offset_vertical+offset_horizontal;//基于小图居中后y偏移量
    int vu_offset = width_big*offset_vertical/2 +offset_horizontal;//基于小图居中后vu偏移量

    int y_center_offset = ((height_big - height_small)/2)*width_big + (width_big-width_small)/2;
    int vu_center_offset = ((height_big - height_small)/4)*width_big + (width_big-width_small)/2;
    unsigned char * big_buff_y =big_buff + y_center_offset+y_offset;
    unsigned char * big_buff_vu = big_buff+large_vu_distance+vu_center_offset+vu_offset;
    unsigned char * small_buff_y = small_buff; 
    unsigned char * small_buff_vu = small_buff + small_vu_distance;
    ALOGD("libGomeMultiScan ##merge_two_yuv_for_nv21  large_vu_distance:%d,small_vu_distance:%d,offset_horizontal:%d,offset_vertical:%d \n",large_vu_distance,small_vu_distance,offset_horizontal,offset_vertical);
    for(int i=0;i<height_small;i++)
    {  
        memcpy(big_buff_y,small_buff_y,width_small);
        if(i%2 == 0){
		memcpy(big_buff_vu,small_buff_vu,width_small);
		big_buff_vu += width_big; 
        }
        big_buff_y += width_big;
        small_buff_y += width_small;
        small_buff_vu += width_small/2;  
     }
     //gome_dump_yuv(nv21_test_buff,width_small*height_small*3/2,"/sdcard/DCIM/image_nv21_small.yuv");
     //gome_dump_yuv(nv21_test_buff_big,width_big*height_big*3/2,"/sdcard/DCIM/image_nv21_big.yuv");
     //gome_dump_yuv(big_buff,width_big*height_big*3/2,"/sdcard/DCIM/image_nv21_merge.yuv");
}

//将jpeg合并到nv21图像中
void gome_merge_jpeg2yuv_for_nv21(const char *jpegPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big)
{
        int k = width_small;
        int j = height_small;
        //buff1为jpeg分配内存
        mjpeg_buff = (unsigned char*)malloc(width_small * height_small * 2);
        if(mjpeg_buff ==  NULL) {
                ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_nv21() step1 mjpeg_buff malloc err\n");
                return;
        }else{
                memset(mjpeg_buff, 0, width_small * height_small * 2);
        }
        //buff2为jpeg转码后的yuv422数据分配内存
        yuyv_buff = (unsigned char*)malloc(width_small * height_small * 2);
	if(yuyv_buff ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_nv21() step2 yuyv_buff malloc err\n");
	}else{
		memset(yuyv_buff, 0, WIDTH * HIGHT * 2);
	}
        //为yuv422转换为nv21数据分配内存
        yuv420_buff = (unsigned char*)malloc(width_small * height_small * 1.5);
	if(yuv420_buff ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_nv21() step3 yuv420_buff malloc yuv420_buff err\n");
	}else{
		memset(yuv420_buff, 0, WIDTH * HIGHT * 1.5);
	}

        FILE *pfile = fopen(jpegPath,"rb");
        ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_nv21() 111 pfile:%p \n",pfile);
        fseek(pfile,0,SEEK_END); /* 定位到文件末尾 */
        int len= ftell(pfile);
        fseek(pfile,0,SEEK_SET); /* 定位到文件开头 */
        size_t result = fread(mjpeg_buff,1,len,pfile); //读取mjpeg_buff
        jpeg_decode(&yuyv_buff, mjpeg_buff, &k, &j);   //将jpeg转码成yuyv422
        YUV422ToNv21(yuyv_buff,yuv420_buff,width_small,height_small); //将yuyv422转码成nv21  
        fclose(pfile); 
        ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_nv21() 222 mjpeg_buff:%p len:%d  \n",mjpeg_buff,len);
        merge_two_yuv_for_nv21(camera_buff,width_big,height_big,yuv420_buff,width_small,height_small);
}
//两个yv12图片合成
void merge_two_yuv_for_yv12(unsigned char *big_buff,int width_big,int height_big,unsigned char * small_buff,int width_small,int height_small)
{
    ALOGD("libGomeMultiScan merge_two_yuv_for_yv12 big_buff:%p,width_big:%d height_big:%d small_buff:%p width_small:%d height_small:%d \n",big_buff,width_big,height_big,small_buff,width_small,height_small);
    if(width_big<width_small || height_big < height_small)
    {
        ALOGD("libGomeMultiScan merge_two_yuv_for_yv12 error width_big<width_small || height_big < height_small \n");
        return;
    }
    int small_vu_distance = width_small*height_small;
    int large_vu_distance = width_big*height_big;
    //for test
    //gome_dump_yuv(big_buff,width_big*height_big*3/2,"/sdcard/DCIM/image_nv21_big_raw.yuv");
    //return;
    int offset_horizontal = 0; //基于小图居中横向偏移量
    int offset_vertical = 0;   //基于小图居中纵向偏移量
    getQrcodeOffset(&offset_horizontal,&offset_vertical);
    int y_offset = width_big*offset_vertical+offset_horizontal;//基于小图居中后y偏移量
    int v_offset = (width_big/2)*offset_vertical/2 +offset_horizontal/2; //基于小图居中后v偏移量
    int u_offset = (width_big/2)*offset_vertical/2 +offset_horizontal/2; //基于小图居中后u偏移量

    int y_center_offset = ((height_big - height_small)/2)*width_big + (width_big-width_small)/2;
    int v_center_offset = ((height_big - height_small)/4)*width_big/2 + (width_big-width_small)/4;
    int u_center_offset = ((height_big - height_small)/4)*width_big/2 + (width_big-width_small)/4;
    unsigned char * big_buff_y =big_buff + y_center_offset+y_offset;
    unsigned char * big_buff_v = big_buff+large_vu_distance + v_center_offset+v_offset;
    unsigned char * big_buff_u = big_buff+large_vu_distance*5/4 + u_center_offset+u_offset;
    unsigned char * small_buff_y = small_buff;
    unsigned char * small_buff_v = small_buff + small_vu_distance;
    unsigned char * small_buff_u = small_buff + small_vu_distance*5/4;
    ALOGD("libGomeMultiScan ##merge_two_yuv_for_yv12  large_vu_distance:%d,small_vu_distance:%d,offset_horizontal:%d,offset_vertical:%d \n",large_vu_distance,small_vu_distance,offset_horizontal,offset_vertical);
    for(int i=0;i<height_small;i++)
    {
        memcpy(big_buff_y,small_buff_y,width_small);
        if(i%2 == 0){ //每两行取1/2行
			memcpy(big_buff_v,small_buff_v,width_small/2);
			memcpy(big_buff_u,small_buff_u,width_small/2);
			big_buff_v += width_big/2;
			big_buff_u += width_big/2;
			small_buff_v += width_small/2;
			small_buff_u += width_small/2;
        }
        big_buff_y += width_big;
        small_buff_y += width_small;
     }

     //gome_dump_yuv(nv21_test_buff,width_small*height_small*3/2,"/sdcard/DCIM/image_nv21_small.yuv");
     //gome_dump_yuv(nv21_test_buff_big,width_big*height_big*3/2,"/sdcard/DCIM/image_nv21_big.yuv");
     //gome_dump_yuv(big_buff,width_big*height_big*3/2,"/sdcard/DCIM/image_yv12_merge.yuv");
}
//将jpeg合并到yv12图像中
void gome_merge_jpeg2yuv_for_yv12(const char *jpegPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big)
{
        int k = width_small;
        int j = height_small;
        //buff1为jpeg分配内存
        mjpeg_buff = (unsigned char*)malloc(width_small * height_small * 2);
        if(mjpeg_buff ==  NULL) {
                ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_yv12() step1 mjpeg_buff malloc err\n");
        }else{
                memset(mjpeg_buff, 0, width_small * height_small * 2);
        }
        //buff2为jpeg转码后的yuv422数据分配内存
        yuyv_buff = (unsigned char*)malloc(width_small * height_small * 2);
	if(yuyv_buff ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_yv12() step2 yuyv_buff malloc err\n");
	}else{
		memset(yuyv_buff, 0, WIDTH * HIGHT * 2);
	}
        //为yuv422转换为nv21数据分配内存
        yuv420_buff = (unsigned char*)malloc(width_small * height_small * 1.5);
	if(yuv420_buff ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_yv12() step3 yuv420_buff malloc yuv420_buff err\n");
	}else{
		memset(yuv420_buff, 0, WIDTH * HIGHT * 1.5);
	}

        FILE *pfile = fopen(jpegPath,"rb");
        ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_yv12() 111 pfile:%p \n",pfile);
        fseek(pfile,0,SEEK_END); /* 定位到文件末尾 */
        int len= ftell(pfile);
        fseek(pfile,0,SEEK_SET); /* 定位到文件开头 */
        size_t result = fread(mjpeg_buff,1,len,pfile); //读取mjpeg_buff
        jpeg_decode(&yuyv_buff, mjpeg_buff, &k, &j);   //将jpeg转码成yuyv422
        YUV422ToYv12(yuyv_buff,yuv420_buff,width_small,height_small); //将yuyv422转码成yv12
        fclose(pfile);
        ALOGD("libGomeMultiScan gome_merge_jpeg2yuv_for_yv12() 222 mjpeg_buff:%p len:%d  \n",mjpeg_buff,len);
        merge_two_yuv_for_yv12(camera_buff,width_big,height_big,yuv420_buff,width_small,height_small);
}
int get_camera_buff()
{
	yuv420_buff_big = (unsigned char*)malloc(WIDTH_BIG * HIGHT_BIG * 1.5);
        if(yuv420_buff_big ==  NULL){
                perror("get_camera_buff() libGomeMultiScan malloc yuv420_buff_big err\n");
        }else{
		memset(yuv420_buff_big, 0, WIDTH_BIG * HIGHT_BIG * 1.5);
	}       
        //////step2
        ALOGD("libGomeMultiScan get_camera_buff() 111");
        char stryuvpath[80]={"/sdcard/DCIM/camera.yuv"};
        FILE *pyuvfile;
        if((pyuvfile = fopen(stryuvpath,"rb")) == NULL){
	     ALOGD("libGomeMultiScan open /sdcard/DCIM/camera.yuv error ");
	     return 0;
	} 
	ALOGD("libGomeMultiScan get_camera_buff() 222");
	fseek(pyuvfile,0,SEEK_END); /* 定位到文件末尾 */
	ALOGD("libGomeMultiScan get_camera_buff() 333");
	int lenyuv= ftell(pyuvfile);
	ALOGD("libGomeMultiScan get_camera_buff() 444");
	fseek(pyuvfile,0,SEEK_SET); /* 定位到文件开头 */
	ALOGD("libGomeMultiScan get_camera_buff() 555");
	size_t result = fread(yuv420_buff_big,1,lenyuv,pyuvfile);
	fclose(pyuvfile);
	ALOGD("libGomeMultiScan get_camera_buff() 666 lenyuv:%d result:%d\n",lenyuv,result);
	return 1;
}
//将nv21合并到yv12图像中
void gome_merge_nv21_to_yv12(const char *yuvImgPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big)
{
	//test getprop app     ro.product.model
	char scanApp[PROPERTY_VALUE_MAX];
	char model[PROPERTY_VALUE_MAX];
	property_get("persist.sys.multiscan_app",scanApp,"null");
	property_get("ro.product.model",model,"null");
    FILE *pfile = fopen(yuvImgPath,"rb");
    ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12()** 111 pfile:%p scanApp:%s yuvImgPath:%s,yuvImgPath len:%d (NULL != pfile):%d\n",pfile,scanApp,yuvImgPath,strlen(yuvImgPath),(NULL != pfile));
    if(NULL == pfile){
    	ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12()** 112 !! no fclose err cannot find image file:%s",yuvImgPath);
    	//fclose(pfile);
    	return;
    }
	int k = width_small;
	int j = height_small;
	//为从sdcard读取的nv21图片分配内存,防止同一个变量被两个方法调用
	yuv420_buff = (unsigned char*)malloc(width_small * height_small * 1.5);
	if(yuv420_buff ==  NULL){
		printf("libGomeMultiScan gome_merge_nv21_to_yv12() 113 yuv420_buff malloc yuv420_buff err\n");
	}else{
		memset(yuv420_buff, 0, width_small * height_small * 1.5);
	}
	//step1 为nv21转换为yv12数据分配内存
	yuv420_buff_for_yv12 = (unsigned char*)malloc(width_small * height_small * 1.5);
	if(yuv420_buff_for_yv12 ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 114 malloc yuv420_buff_for_yv12 err\n");
	}else{
		memset(yuv420_buff_for_yv12, 0, width_small * height_small * 1.5);
	}
	//step2 读取文件并合成
	fseek(pfile,0,SEEK_END); /* 定位到文件末尾 */
	int len= ftell(pfile);
	fseek(pfile,0,SEEK_SET); /* 定位到文件开头 */
	size_t result = fread(yuv420_buff,1,len,pfile); //读取nv21文件

	//gome_dump_yuv(yuv420_buff_for_yv12,width_small*height_small*3/2,"out/qrd_image_yuv420.yuv");
	Nv21ToYv12(yuv420_buff,yuv420_buff_for_yv12,width_small,height_small); //将nv21转码成yv12
	fclose(pfile);
	ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 222 yuv420_buff_for_yv12:%p len:%d  test 11.27\n",yuv420_buff_for_yv12,len);
	merge_two_yuv_for_yv12(camera_buff,width_big,height_big,yuv420_buff_for_yv12,width_small,height_small);

	//step3 释放分配的内存
	if(yuv420_buff_for_yv12 != NULL){
		ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 331 yuv420_buff_for_yv12 != NULL call free()");
		free(yuv420_buff_for_yv12);
	}else{
		ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 332 yuv420_buff_for_yv12 == NULL");
	}
	if(yuv420_buff != NULL){
		ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 441 yuv420_buff != NULL call free()");
		free(yuv420_buff);
	}else{
		ALOGD("libGomeMultiScan gome_merge_nv21_to_yv12() 442 yuv420_buff == NULL");
	}
}
//将nv21合并到nv21图像中
void gome_merge_nv21_to_nv21(const char *yuvImgPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big)
{
	FILE *pfile = fopen(yuvImgPath,"rb");

	ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21() 111 pfile:%p NULL != pfile:%d\n",pfile,(NULL != pfile));
	if(NULL == pfile){
    	ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21()** 112 !! no fclose err cannot find image file:%s",yuvImgPath);
    	//fclose(pfile);
    	return;
	}
	int k = width_small;
	int j = height_small;

	//step1为yuv422转换为nv21数据分配内存
	yuv420_buff_for_nv21 = (unsigned char*)malloc(width_small * height_small * 1.5);
	if(yuv420_buff_for_nv21 ==  NULL){
		ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21() 113  malloc yuv420_buff_for_nv21 err\n");
	}else{
		memset(yuv420_buff_for_nv21, 0, width_small * height_small * 1.5);
	}
	//step2 读取文件并合成
	fseek(pfile,0,SEEK_END); /* 定位到文件末尾 */
	int len= ftell(pfile);
	fseek(pfile,0,SEEK_SET); /* 定位到文件开头 */
	size_t result = fread(yuv420_buff_for_nv21,1,len,pfile); //读取yuv420_buff_for_nv21

	//gome_dump_yuv(yuv420_buff_for_nv21,width_small*height_small*3/2,"/sdcard/DCIM/qrd_image_yuv420.yuv");
	//YUV422ToYv12(yuyv_buff,yuv420_buff_for_nv21,width_small,height_small); //将yuyv422转码成yv12
	fclose(pfile);
	ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21() 222 yuv420_buff_for_nv21:%p len:%d  \n",yuv420_buff_for_nv21,len);
	merge_two_yuv_for_nv21(camera_buff,width_big,height_big,yuv420_buff_for_nv21,width_small,height_small);

	//step3 释放分配的内存
	if(yuv420_buff_for_nv21 != NULL){
		ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21() 331 yuv420_buff_for_nv21 != NULL call free()");
		free(yuv420_buff_for_nv21);
	}else{
		ALOGD("libGomeMultiScan gome_merge_nv21_to_nv21() 332 yuv420_buff_for_nv21 == NULL");
	}

}
void relaseMemory(){
	if(yuv420_buff != NULL){
		ALOGD("libGomeMultiScan 11 yuv420_buff not null call free()");
		free(yuv420_buff);
	}else{
		ALOGD("libGomeMultiScan 222 yuv420_buff is null");
	}
}
void gome_testYuv()
{
	ALOGD("libGomeMultiScan testyuv.c gome_testYuv() 000 \n");
        if(get_camera_buff() == 1){
        	gome_merge_jpeg2yuv_for_nv21("/sdcard/DCIM/1.jpg",320,240,yuv420_buff_big,WIDTH_BIG,HIGHT_BIG);
        }
}
