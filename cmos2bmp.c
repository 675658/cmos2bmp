#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "libbmp.h"

typedef int (*read_pixels_fun)(FILE *fp_in,int x,int y,uint8_t rgb_out[3]);
typedef int (*interpolation_fun)(bmp_img *img,uint16_t h,uint16_t v);

int read_rgb888(FILE *fp_in,int x,int y,uint8_t rgb_out[3])
{
    if(fread(&rgb_out,1,3,fp_in)!=3){
        return -1;
    }
    return 0;
}

int read_rgb565(FILE *fp_in,int x,int y,uint8_t rgb_out[3])
{
    uint16_t pixel;
    if(fread(&pixel,1,2,fp_in)!=2){
        return -1;
    }
    rgb_out[0] = (pixel&0xf800u)>>8u;
    rgb_out[1] = (pixel&0x07e0u)>>3u;
    rgb_out[2] = (pixel&0x001fu)<<3u;
    return 0;
}

int read_raw_8(FILE *fp_in,int x,int y,uint8_t rgb_out[3])
{
    uint8_t byte;
    if(fread(&byte,1,1,fp_in)!=1){
        return -1;
    }
    rgb_out[0] = byte;
    rgb_out[1] = byte;
    rgb_out[2] = byte;
    return 0;
}

int read_raw_10(FILE *fp_in,int x,int y,uint8_t rgb_out[3])
{
    uint8_t byte;
    if(fread(&byte,1,1,fp_in)!=1){
        return -1;
    }
    rgb_out[0] = byte;
    rgb_out[1] = byte;
    rgb_out[2] = byte;
    if((x+1)%4==0)
    {
        if(fread(&byte,1,1,fp_in)!=1){
            return -1;
        }
    }
    return 0;
}

int cmos_raw_interpolation_get_average(bmp_img *img,uint16_t h,uint16_t v,int x,int y,uint8_t value_out[3][4])
{
    uint32_t sum[3][4]={0}; //- | + x
    uint8_t count[4]={0};
    if(x-1>=0)
    {
        sum[0][0]+=img->img_pixels[y][x-1].red;
        sum[1][0]+=img->img_pixels[y][x-1].green;
        sum[2][0]+=img->img_pixels[y][x-1].blue;
        count[0]++;
        sum[0][2]+=img->img_pixels[y][x-1].red;
        sum[1][2]+=img->img_pixels[y][x-1].green;
        sum[2][2]+=img->img_pixels[y][x-1].blue;
        count[2]++;
        if(y-1>=0)
        {
            sum[0][3]+=img->img_pixels[y-1][x-1].red;
            sum[1][3]+=img->img_pixels[y-1][x-1].green;
            sum[2][3]+=img->img_pixels[y-1][x-1].blue;
            count[3]++;
        }
        if(y+1<v)
        {
            sum[0][3]+=img->img_pixels[y+1][x-1].red;
            sum[1][3]+=img->img_pixels[y+1][x-1].green;
            sum[2][3]+=img->img_pixels[y+1][x-1].blue;
            count[3]++;
        }
    }
    if(x+1<h)
    {
        sum[0][0]+=img->img_pixels[y][x+1].red;
        sum[1][0]+=img->img_pixels[y][x+1].green;
        sum[2][0]+=img->img_pixels[y][x+1].blue;
        count[0]++;
        sum[0][2]+=img->img_pixels[y][x+1].red;
        sum[1][2]+=img->img_pixels[y][x+1].green;
        sum[2][2]+=img->img_pixels[y][x+1].blue;
        count[2]++;
        if(y-1>=0)
        {
            sum[0][3]+=img->img_pixels[y-1][x+1].red;
            sum[1][3]+=img->img_pixels[y-1][x+1].green;
            sum[2][3]+=img->img_pixels[y-1][x+1].blue;
            count[3]++;
        }
        if(y+1<v)
        {
            sum[0][3]+=img->img_pixels[y+1][x+1].red;
            sum[1][3]+=img->img_pixels[y+1][x+1].green;
            sum[2][3]+=img->img_pixels[y+1][x+1].blue;
            count[3]++;
        }
    }
    if(y-1>=0)
    {
        sum[0][1]+=img->img_pixels[y-1][x].red;
        sum[1][1]+=img->img_pixels[y-1][x].green;
        sum[2][1]+=img->img_pixels[y-1][x].blue;
        count[1]++;
        sum[0][2]+=img->img_pixels[y-1][x].red;
        sum[1][2]+=img->img_pixels[y-1][x].green;
        sum[2][2]+=img->img_pixels[y-1][x].blue;
        count[2]++;
    }
    if(y+1<v)
    {
        sum[0][1]+=img->img_pixels[y+1][x].red;
        sum[1][1]+=img->img_pixels[y+1][x].green;
        sum[2][1]+=img->img_pixels[y+1][x].blue;
        count[1]++;
        sum[0][2]+=img->img_pixels[y+1][x].red;
        sum[1][2]+=img->img_pixels[y+1][x].green;
        sum[2][2]+=img->img_pixels[y+1][x].blue;
        count[2]++;
    }
    value_out[0][0]=sum[0][0]/count[0];
    value_out[1][0]=sum[1][0]/count[0];
    value_out[2][0]=sum[2][0]/count[0];
    value_out[0][1]=sum[0][1]/count[1];
    value_out[1][1]=sum[1][1]/count[1];
    value_out[2][1]=sum[2][1]/count[1];
    value_out[0][2]=sum[0][2]/count[2];
    value_out[1][2]=sum[1][2]/count[2];
    value_out[2][2]=sum[2][2]/count[2];
    value_out[0][3]=sum[0][3]/count[3];
    value_out[1][3]=sum[1][3]/count[3];
    value_out[2][3]=sum[2][3]/count[3];
    return 0;
}

int cmos_raw_interpolat(bmp_img *img,uint16_t h,uint16_t v,uint8_t sw_x,uint8_t sw_y)
{
    int x,y;
    uint8_t average[3][4];
    for(y=0;y<v;y++)
    {
        for(x=0;x<h;x++)
        {
            cmos_raw_interpolation_get_average(img,h,v,x,y,average);
            if(((uint32_t)y&1u)==sw_y)
            {
                if(((uint32_t)x&1u)==sw_x) //G
                {
                    
                    img->img_pixels[y][x].red = average[0][0];
                    img->img_pixels[y][x].blue = average[2][1];
                }
                else                    //R
                {
                    img->img_pixels[y][x].green = average[1][2];
                    img->img_pixels[y][x].blue = average[2][3];
                }
            }
            else
            {
                if(((uint32_t)x&1u)==sw_x) //B
                {
                    img->img_pixels[y][x].green = average[1][2];
                    img->img_pixels[y][x].red = average[0][3];
                }
                else                    //G
                {
                    img->img_pixels[y][x].red = average[0][1];
                    img->img_pixels[y][x].blue = average[2][0];
                }
            }
        }
    }
    return 0;
}

int cmos_raw_interpolat_rggb(bmp_img *img,uint16_t h,uint16_t v)
{
    return cmos_raw_interpolat(img,h,v,1,0);
}
int cmos_raw_interpolat_grbg(bmp_img *img,uint16_t h,uint16_t v)
{
    return cmos_raw_interpolat(img,h,v,0,0);
}
int cmos_raw_interpolat_bggr(bmp_img *img,uint16_t h,uint16_t v)
{
    return cmos_raw_interpolat(img,h,v,0,1);
}
int cmos_raw_interpolat_gbrg(bmp_img *img,uint16_t h,uint16_t v)
{
    return cmos_raw_interpolat(img,h,v,1,1);
}

int convert_to_bmp(const char *in_file,const char *out_file,uint16_t h,uint16_t v,uint16_t h_b,uint16_t v_b,read_pixels_fun read_pixels,interpolation_fun interpolat)
{
    FILE *fp_in;
    bmp_img img;
    uint8_t pixel[3];
    int x,y;
    fp_in=fopen64(in_file,"rb");
    if(fp_in==NULL)
    {
        perror("Err: ");
        return -1;
    }
    bmp_img_init_df(&img, h, v);
    for(y=0;y<v;y++)
    {
        for(x=0;x<h;x++)
        {
            if(read_pixels(fp_in,x,y,pixel)!=0){
                (void)fclose(fp_in);
                printf("Err: Unexpected EOF\n");
                return -2;
            }
            bmp_pixel_init(&img.img_pixels[y][x],pixel[0],pixel[1],pixel[2]);

        }
        while(x<h+h_b){
            if(read_pixels(fp_in,x,y,pixel)!=0){
                (void)fclose(fp_in);
                printf("Err: Unexpected EOF\n");
                return -3;
            }
            x++;
        }
    }
    if(interpolat!=NULL)
    {
        interpolat(&img,h,v);
    }
    bmp_img_write(&img,out_file);
    bmp_img_free(&img);
    (void)fclose(fp_in);
    return 0;
}

typedef struct{
    char *type_name;
    read_pixels_fun read_pixels;
    interpolation_fun interpolat;
} cmos2bmp_convert_type;

cmos2bmp_convert_type convert_table[]=
{
    {"rgb565",read_rgb565,NULL},
    {"rgb888",read_rgb888,NULL},
    {"raw8",read_raw_8,NULL},
    {"raw8_rggb",read_raw_8,cmos_raw_interpolat_rggb},
    {"raw8_grbg",read_raw_8,cmos_raw_interpolat_grbg},
    {"raw8_bggr",read_raw_8,cmos_raw_interpolat_bggr},
    {"raw8_gbrg",read_raw_8,cmos_raw_interpolat_gbrg},
    {"raw10",read_raw_10,NULL},
    {"raw10_rggb",read_raw_10,cmos_raw_interpolat_rggb},
    {"raw10_grbg",read_raw_10,cmos_raw_interpolat_grbg},
    {"raw10_bggr",read_raw_10,cmos_raw_interpolat_bggr},
    {"raw10_gbrg",read_raw_10,cmos_raw_interpolat_gbrg},
};
int main(int argc,char **argv)
{
    char *in_file=NULL;
    char *out_file="out.bmp";
    char *type="raw8";
    uint16_t h=0;
    uint16_t v=0;
    uint16_t h_b=0;
    uint16_t v_b=0;

    for(int i=0;i<argc;i++)
    {
        if(strcmp(argv[i],"-i")==0 && i+1<argc)
        {
            in_file=argv[++i];
        }
        if(strcmp(argv[i],"-o")==0 && i+1<argc)
        {
            out_file=argv[++i];
        }
        if(strcmp(argv[i],"-t")==0 && i+1<argc)
        {
            type=argv[++i];
        }
        if(strcmp(argv[i],"-h")==0 && i+1<argc)
        {
            (void)sscanf(argv[++i],"%hu",&h);
        }
        if(strcmp(argv[i],"-v")==0 && i+1<argc)
        {
            (void)sscanf(argv[++i],"%hu",&v);
        }
        if(strcmp(argv[i],"-hb")==0 && i+1<argc)
        {
            (void)sscanf(argv[++i],"%hu",&h_b);
        }
        if(strcmp(argv[i],"-vb")==0 && i+1<argc)
        {
            (void)sscanf(argv[++i],"%hu",&v_b);
        }
        if(strcmp(argv[i],"--help")==0)
        {
            goto FLAG_PRINT_HELP_AND_RETURN;
        }
    }
    if(in_file==NULL)
    {
        printf("Err: Plase prase input file\n");
        goto FLAG_PRINT_HELP_AND_RETURN;
    }
    if(h==0||v==0)
    {
        printf("Err: Plase prase image hor_size and ver_size\n");
        goto FLAG_PRINT_HELP_AND_RETURN;
    }
    for(int i=0;i<sizeof(convert_table)/sizeof(cmos2bmp_convert_type);i++)
    {
        if(strcmp(type,convert_table[i].type_name)==0)
        {
            convert_to_bmp(in_file,out_file,h,v,h_b,v_b,convert_table[i].read_pixels,convert_table[i].interpolat);
            return 0;
        }
    }
    
FLAG_PRINT_HELP_AND_RETURN:
    printf("cmos2bmp -i input_file -o output_file -h hor_size -v ver_size [-t type] [-hb hor_blank]\n"
           "    type:\n");
    for(int i=0;i<sizeof(convert_table)/sizeof(cmos2bmp_convert_type);i++)
    {
        printf("        %s\n",convert_table[i].type_name);
    }
    return 0;
}