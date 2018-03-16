#include <fcntl.h> // for open
#include <unistd.h> // for close

#ifdef __cplusplus 
extern "C" { 
#endif
int gome_dump_yuv(void *addr, int length,char* name);
void gome_merge_jpeg2yuv_for_nv21(const char *jpegPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big);
void gome_merge_jpeg2yuv_for_yv12(const char *jpegPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big);
//将nv21合并到yv12图像中
void gome_merge_nv21_to_yv12(const char *yuvImgPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big);
//将nv21合并到nv21图像中
void gome_merge_nv21_to_nv21(const char *yuvImgPath,int width_small,int height_small,unsigned char * camera_buff,int width_big,int height_big);
void gome_testYuv();
#ifdef __cplusplus
}
#endif
