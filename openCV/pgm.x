//read/write(?) a binary pgm
#include <stdio.h>
#include <stdlib.h>

unsigned char * pgm_read(const char *filename,int *width,int *height)
{
    FILE *f = fopen(filename,"rb");
    unsigned char *r = 0;
    if(f) {
        char id[3];
        int w,h,d;
        fscanf(f,"%2s %d %d %d\n",id,&w,&h,&d);
        if(id[0]=='P'&&id[1]=='5') {
            r = malloc(w*h);
            fread(r,w,h,f);
            if(width)*width=w;
            if(height)*height=h;
        }
        fclose(f);
    }
    return r;
}

void pgm_write(const char *filename,int width,int height,unsigned char *buf)
{
    FILE *f = fopen(filename,"wb");
    if(f) {
        fprintf(f,"P5\n%d %d\n255\n",width,height);
        fwrite(buf,width,height,f);
        fclose(f);   
    }
}



