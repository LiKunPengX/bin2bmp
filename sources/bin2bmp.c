#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#pragma pack(1)

#define BMP_FILE_HEAD_SIZE 14
#define BMP_INFO_HEAD_SIZE 52 //40bytes info head + 12bytes 565/555 bit mask
#define IMAGE_LENGTH 800 
#define IMAGE_HEIGHT -480

struct type_bmp_file_head
{
	unsigned short bmp_file_mark; //32 4d "BM"
	unsigned int   bmp_file_size; 
	unsigned short _reserved1;     //0
	unsigned short _reserved2;     //0
	unsigned int   bmp_file_offset; //文件头+信息头+调色板 
}bmp_file_head;

struct type_bmp_info_head
{
	unsigned int   bmp_info_head_size;
	unsigned int   image_length;
	unsigned int   image_height;
	unsigned short image_dimention; //位图位面数 保持为1 
	unsigned short bits_per_pixle;  //每像素多少比特 
	unsigned int   image_compression; //图像压缩方式：BI_RGB  BI_RLE8  BI_RLE4  BI_BITFIELDS
	unsigned int   image_size;  //图像大小，字节数，压缩方式BI RGB时可设置为0 
	unsigned int   pixles_per_meter_x; //水平分辨率 
	unsigned int   pixles_per_meter_y; //垂直分辨率
	unsigned int   image_color_used;  //彩色表中的颜色数，0表示全部颜色 
	unsigned int   image_important_color; //重要颜色数量，为零表示都重要 
	unsigned int   red_mask;
	unsigned int   green_mask;
	unsigned int   blue_mask;
	
}bmp_info_head;

unsigned int cal_file_size(FILE* fp)
{
	unsigned int file_size;
	fseek(fp,0,SEEK_END);
	file_size=ftell(fp);
	rewind(fp);
	return file_size;
}

void gen_bmp_file_head(unsigned int bmp_filesize)
{
	bmp_file_head.bmp_file_mark = 'M';
	bmp_file_head.bmp_file_mark <<= 8;
	bmp_file_head.bmp_file_mark |= 'B';
	bmp_file_head._reserved1 = 0;
	bmp_file_head._reserved2 = 0;
	//bmp_file_head.bmp_file_size |= (bmp_filesize << 24);
	//bmp_file_head.bmp_file_size |= ((bmp_filesize << 8) & 0x00ff0000);
	//bmp_file_head.bmp_file_size |= ((bmp_filesize >> 8) & 0x0000ff00);
	//bmp_file_head.bmp_file_size |= (bmp_filesize >> 24);
	bmp_file_head.bmp_file_size = bmp_filesize;
	//bmp_file_head.bmp_file_offset |= ((BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE) << 24);
	//bmp_file_head.bmp_file_offset |= ((BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE) & 0x00ff0000);
	//bmp_file_head.bmp_file_offset |= ((BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE) & 0x0000ff00);
	//bmp_file_head.bmp_file_offset |= ((BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE) >> 24);	
	bmp_file_head.bmp_file_offset = (BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE);
}

void gen_bmp_info_head(unsigned int image_height,unsigned int image_length)
{
	bmp_info_head.bits_per_pixle = 16;
	//bmp_info_head.bits_per_pixle <<= 8;
	
	bmp_info_head.bmp_info_head_size = 0x00000028; //40 bytes little endian 
	bmp_info_head.image_color_used = 0;
	bmp_info_head.image_compression = 3; // BI_BITFILEDS 
	bmp_info_head.image_dimention = 1; //图像维度数 
	//bmp_info_head.image_dimention <<= 8;
	//bmp_info_head.image_height |= (image_height << 24);
	//bmp_info_head.image_height |= ((image_height << 8) & 0x00ff0000);
	//bmp_info_head.image_height |= ((image_height >> 8) & 0x0000ff00);
	//bmp_info_head.image_height |= (image_height >> 24);
	bmp_info_head.image_height = image_height; //图像高度 
	//bmp_info_head.image_length |= (image_length << 24);
	//bmp_info_head.image_length |= ((image_length << 8) & 0x00ff0000);
	//bmp_info_head.image_length |= ((image_length >> 8) & 0x0000ff00);
	//bmp_info_head.image_length |= (image_length >> 24);
	bmp_info_head.image_length = image_length; //图像长度 
	bmp_info_head.image_important_color = 0;
	bmp_info_head.image_size = 0; //图像大小，压缩方式为不压缩或BITFILEDS时可以为零 
	bmp_info_head.pixles_per_meter_x = 0; //水平分辨率  为零缺省 
	bmp_info_head.pixles_per_meter_y = 0; // 垂直分辨率 为零缺省 
	bmp_info_head.red_mask = 0x0000f800; //红色掩码，bin文件中的原始颜色数据与上此值为5bits红色数据 
	bmp_info_head.green_mask = 0x000007e0; //绿色掩码，bin文件中的原始颜色数据与上此值为6bits绿色数据 
	bmp_info_head.blue_mask = 0x00000001f; //蓝色掩码，bin文件中的原始颜色数据与上此值为5bits蓝色数据 
}

unsigned char* rgb_data_convert(FILE* rgb_bin_file_pointer,unsigned int bin_filesize)
{
	unsigned int cnt;
	unsigned int rgb_bin_filesize=0;
	unsigned char* bin_dat_buffer;
	unsigned short temp_color=0;
	unsigned char r=0,g=0,b=0;
	
	bin_dat_buffer = (unsigned char*)calloc(bin_filesize,1);
	fread(bin_dat_buffer,1,bin_filesize,rgb_bin_file_pointer);
	fclose(rgb_bin_file_pointer);
	for(cnt = 0; cnt < bin_filesize; cnt++)
	{
		if(!(cnt%2))
		{
			//rgb443 转 rgb565 
			temp_color=0;
			temp_color = *(bin_dat_buffer+cnt);
			temp_color <<= 8;
			temp_color |= *(bin_dat_buffer+cnt+1);
			r = (float)(((temp_color>>7)*16)*0.122);
			g = (float)((((temp_color>>3)&0x000f)*16)*0.247);
			b = (float)(((temp_color & 0x0007)*32)*0.122);
			temp_color=0;
			temp_color |= ((r<<11) & 0xf800);
			temp_color |= ((g<<5) & 0x07e0);
			temp_color |= (b & 0x1f);
			//交换高低字节，小端模式存储 
			*(bin_dat_buffer + cnt + 1) = (unsigned char)(temp_color >> 8);
			*(bin_dat_buffer + cnt) = (unsigned char)(temp_color & 0x00ff);
		}
	}
	return  bin_dat_buffer; 
}

void gen_bmp(FILE* rgb_bin_file_pointer,char* bmp_filename)
{
	unsigned int cnt;
	unsigned int path_len;
	unsigned char* bmp_dat_pool_start;
	unsigned char* stru_pointer;
	unsigned int rgb_bin_filesize=0;
	unsigned char* bmp_dat_buffer;
	unsigned char* bin_dat_buffer;
	
	FILE* bmp_fp;
	
	rgb_bin_filesize = cal_file_size(rgb_bin_file_pointer);
	bmp_dat_buffer = (unsigned char*)calloc((rgb_bin_filesize + BMP_INFO_HEAD_SIZE + BMP_FILE_HEAD_SIZE),1);
	if(bmp_dat_buffer == NULL)
	{
		printf("分配BMP内存空间失败！");
		return; 
	}
	bmp_dat_pool_start = bmp_dat_buffer;
	//生成属性信息 
	gen_bmp_file_head(rgb_bin_filesize + BMP_FILE_HEAD_SIZE + BMP_INFO_HEAD_SIZE);
	gen_bmp_info_head(IMAGE_HEIGHT,IMAGE_LENGTH);
	//拷贝BMP图像文件头 
	stru_pointer = (unsigned char*)&(bmp_file_head.bmp_file_mark);
	for(cnt=0; cnt < sizeof(bmp_file_head); cnt++)
	{
		*(bmp_dat_buffer++) =  *(stru_pointer++);
	}
	//拷贝BMP图像信息头 
	stru_pointer = (unsigned char*)&(bmp_info_head.bmp_info_head_size);
	for(cnt=0; cnt < sizeof(bmp_info_head); cnt++)
	{
		*(bmp_dat_buffer++) =  *(stru_pointer++);
	}
	bin_dat_buffer = rgb_data_convert(rgb_bin_file_pointer,rgb_bin_filesize);
	//bmp_dat_pool = bmp_dat_pool_start;
	for(cnt=0; cnt < rgb_bin_filesize; cnt++)
	{
		*(bmp_dat_buffer++) =  *(bin_dat_buffer++);
	}	
	
	path_len = strlen(bmp_filename);
	while(path_len)
	{
		if(*(bmp_filename + path_len) == '.')
		{
			strcpy((bmp_filename + path_len),".bmp");
			break;
		}
		else if((*(bmp_filename + path_len) == '\\') || (path_len < 3))
		{
			printf("输入文件类型错误，请重试！");
			getchar();
			return; 
		}
		path_len--;
	 } 
	
	bmp_fp = fopen(bmp_filename,"rb+");
	if(bmp_fp == NULL)
	{
		bmp_fp = fopen(bmp_filename,"w");
		fclose(bmp_fp);
		bmp_fp = fopen(bmp_filename,"rb+");
	}
	fwrite(bmp_dat_pool_start,(rgb_bin_filesize + BMP_INFO_HEAD_SIZE + BMP_FILE_HEAD_SIZE),1,bmp_fp);
	fclose(bmp_fp);
}

int main(int argc,char *argv[])
{
	FILE *bin_fp;
	printf("%s",argv[1]);
	bin_fp = fopen(argv[1],"rb+");
	if(bin_fp == NULL)
	{
		printf("file read failed！"); 
		return 1; 
	}
	
	gen_bmp(bin_fp,argv[1]);
	return 0;
}
