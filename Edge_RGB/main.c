//
//  main.c
//  Edge_RGB
//
//  Created by 장지선 on 2/8/19.
//  Copyright © 2019 Geesun. All rights reserved.
//

#include "image_prep.h"


void fileInfo(BITMAPFILEHEADER *bh, BITMAPINFOHEADER *bi){
    printf("<BITMAP FILE HEADER>\n");
    printf("bfSize: %d\n",bh->bfSize);
    printf("bfOffBits: %d\n",bh->bfOffBits);
    
    printf("<BITMAP INFO HEADER>\n");
    printf("biSize: %d\n",bi->biSize);
    printf("biWidth: %d\n",bi->biWidth);
    printf("biHeight: %d\n",bi->biHeight);
    printf("biPlanes: %d\n",bi->biPlanes);
    printf("biBitCount: %d\n",bi->biBitCount);
    printf("biSizeImage: %d\n",bi->biSizeImage);
    printf("biXpixels: %d\n",bi->biXPelsPerMeter);
    printf("biYpixels: %d\n",bi->biYPelsPerMeter);
    printf("biClrUsed: %d\n",bi->biClrUsed);
    printf("biClrImportant: %d\n",bi->biClrImportant);
    
}

int main(int argc, char* argv[])
{
    FILE *fpBmp;                    // 비트맵 파일 포인터
    FILE *fpRedBmp;                    // 텍스트 파일 포인터
    FILE *fpGreenBmp;                    // 텍스트 파일 포인터
    FILE *fpBlueBmp;                    // 텍스트 파일 포인터
    BITMAPFILEHEADER fileHeader;    // 비트맵 파일 헤더 구조체 변수
    BITMAPINFOHEADER infoHeader;    // 비트맵 정보 헤더 구조체 변수
    
    unsigned char *image;    // 픽셀 데이터 포인터
    int size;                // 픽셀 데이터 크기
    int width, height;       // 비트맵 이미지의 가로, 세로 크기
    int padding;             // 픽셀 데이터의 가로 크기가 4의 배수가 아닐 때 남는 공간의 크기
    char* Rfilename;
    char* Gfilename;
    char* Bfilename;
    
    Rfilename = (char*)malloc(sizeof(char)*11);
    Rfilename[0] = '\0';
    
    Gfilename = (char*)malloc(sizeof(char)*11);
    Gfilename[0] = '\0';
    
    Bfilename = (char*)malloc(sizeof(char)*11);
    Bfilename[0] = '\0';
    
    
    
    if(argc!=2)printf("<USAGE> RGB_split file.bmp\n");
    // 각 픽셀을 표현할 ASCII 문자. 인덱스가 높을 수록 밝아지는 것을 표현
    char ascii[] = { '#', '#', '@', '%', '=', '+', '*', ':', '-', '.', ' ' };   // 11개
    
    
    fpBmp = fopen(argv[1], "rb");    // 비트맵 파일을 바이너리 모드로 열기
    
    if (fpBmp == NULL)    // 파일 열기에 실패하면
    {
        printf("file open error\n");
        return 0;         // 프로그램 종료
    }
    // 비트맵 파일 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    
    if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpBmp) < 1)
    {
        printf("file header error\n");
        fclose(fpBmp);
        return 0;
    }
   
    // 매직 넘버가 MB가 맞는지 확인(2바이트 크기의 BM을 리틀 엔디언으로 읽었으므로 MB가 됨)
    // 매직 넘버가 맞지 않으면 프로그램 종료
    if (fileHeader.bfType != 'MB')
    {
        printf("magic error\n");
        fclose(fpBmp);
        return 0;
    }
    
    // 비트맵 정보 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpBmp) < 1)
    {
        printf("info header error\n");
        fclose(fpBmp);
        return 0;
    }
    
    // 24비트 비트맵이 아니면 프로그램 종료
    if (infoHeader.biBitCount != 24)
    {
        printf("info bitcount error\n");
        fclose(fpBmp);
        return 0;
    }
  
    size = infoHeader.biSizeImage;    // 픽셀 데이터 크기
    width = infoHeader.biWidth;       // 비트맵 이미지의 가로 크기
    height = infoHeader.biHeight;     // 비트맵 이미지의 세로 크기
    
    // 이미지의 가로 크기에 픽셀 크기를 곱하여 가로 한 줄의 크기를 구하고 4로 나머지를 구함
    // 그리고 4에서 나머지를 빼주면 남는 공간을 구할 수 있음.
    // 만약 남는 공간이 0이라면 최종 결과가 4가 되므로 여기서 다시 4로 나머지를 구함
    padding = (PIXEL_ALIGN - ((width * PIXEL_COLOR_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;
    fileInfo(&fileHeader,&infoHeader);
    printf("padding is %d\n",padding);
    
    
    if (size == 0)    // 픽셀 데이터 크기가 0이라면
    {
        // 이미지의 가로 크기 * 픽셀 크기에 남는 공간을 더해주면 완전한 가로 한 줄 크기가 나옴
        // 여기에 이미지의 세로 크기를 곱해주면 픽셀 데이터의 크기를 구할 수 있음
        size = (width + padding) * height* PIXEL_COLOR_SIZE;
        printf("New computed size is %d\n",size);
        infoHeader.biSizeImage = size;
    }
    
    printf("computed size is %d\n", size);
    image = malloc(size);    // 픽셀 데이터의 크기만큼 동적 메모리 할당
    
    // 파일 포인터를 픽셀 데이터의 시작 위치로 이동
    fseek(fpBmp, fileHeader.bfOffBits, SEEK_SET);
    
    // 파일에서 픽셀 데이터 크기만큼 읽음. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(image, size, 1, fpBmp) < 1)
    {
        printf("read error\n");
        fclose(fpBmp);
        return 0;
    }
    
    fclose(fpBmp);    // 비트맵 파일 닫기
   
    printf("*file load success*\n");
    
    
    strcat(Rfilename, "Red_");
    strcat(Rfilename, &argv[1][7]);
    strcat(Gfilename, "Green_");
    strcat(Gfilename, &argv[1][7]);
    strcat(Bfilename, "Blue_");
    strcat(Bfilename, &argv[1][7]);
    
    
    char* pad_buffer;
    pad_buffer = (char*)malloc(sizeof(char)*padding);
    for(int i=0; i<padding;i++){
        pad_buffer[i] = 0x00; 
    }

    
    fpRedBmp = fopen(Rfilename, "w");    // 결과 출력용 텍스트 파일 열기
    if (fpRedBmp == NULL)    // 파일 열기에 실패하면
    {
        printf("red write fail\n");
        free(image);      // 픽셀 데이터를 저장한 동적 메모리 해제
        return 0;         // 프로그램 종료
    }
    
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpRedBmp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpRedBmp);

    for(int i=0; i<height ; i++){
        
        for(int j=0; j<width; j++){
            RGBTRIPLE buffer;
            buffer.rgbtRed = 0;
            buffer.rgbtGreen = 0;
            buffer.rgbtBlue = 0;
            
            int index = 0;
            index = i*(width*PIXEL_COLOR_SIZE+ padding) + j*PIXEL_COLOR_SIZE;
            
            buffer.rgbtBlue = image[index+2];
            buffer.rgbtGreen = image[index+2];
            buffer.rgbtRed = image[index+2];
            
            fwrite(&buffer, sizeof(RGBTRIPLE), 1, fpRedBmp);
        }

            fwrite(&pad_buffer, sizeof(char)*padding, 1, fpRedBmp); //padding을 안넣어서 이 사단이 났다
        
    }
    
    fclose(fpRedBmp);
    
    

    fpGreenBmp = fopen(Gfilename, "w");    // 결과 출력용 텍스트 파일 열기
    if (fpGreenBmp == NULL)    // 파일 열기에 실패하면
    {
        printf("green write fail\n");
        free(image);      // 픽셀 데이터를 저장한 동적 메모리 해제
        return 0;         // 프로그램 종료
    }
    
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpGreenBmp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpGreenBmp);

    for(int i=0; i<height ; i++){
        
        for(int j=0; j<width; j++){
            RGBTRIPLE buffer;
            buffer.rgbtRed = 0;
            buffer.rgbtGreen = 0;
            buffer.rgbtBlue = 0;
            
            int index = 0;
            index = i*(width*PIXEL_COLOR_SIZE+ padding) + j*PIXEL_COLOR_SIZE;
            
            buffer.rgbtBlue = image[index+1];
            buffer.rgbtGreen = image[index+1];
            buffer.rgbtRed = image[index+1];
            
            fwrite(&buffer, sizeof(RGBTRIPLE), 1, fpGreenBmp);
        }

            fwrite(&pad_buffer, sizeof(char)*padding, 1, fpGreenBmp); //padding을 안넣어서 이 사단이 났다
        
    }
    
    fclose(fpGreenBmp);


    fpBlueBmp = fopen(Bfilename, "w");    // 결과 출력용 텍스트 파일 열기
    if (fpBlueBmp == NULL)    // 파일 열기에 실패하면
    {
        printf("blue write fail\n");
        free(image);      // 픽셀 데이터를 저장한 동적 메모리 해제
        return 0;         // 프로그램 종료
    }
    
    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpBlueBmp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpBlueBmp);

    for(int i=0; i<height ; i++){
        
        for(int j=0; j<width; j++){
            RGBTRIPLE buffer;
            buffer.rgbtRed = 0;
            buffer.rgbtGreen = 0;
            buffer.rgbtBlue = 0;
            
            int index = 0;
            index = i*(width*PIXEL_COLOR_SIZE+ padding) + j*PIXEL_COLOR_SIZE;
            
            buffer.rgbtBlue = image[index];
            buffer.rgbtGreen = image[index];
            buffer.rgbtRed = image[index];
            
            fwrite(&buffer, sizeof(RGBTRIPLE), 1, fpBlueBmp);
        }

            fwrite(&pad_buffer, sizeof(char)*padding, 1, fpBlueBmp); //padding을 안넣어서 이 사단이 났다
        
    }
    
    fclose(fpBlueBmp);

    free(image);
    
    
   
    
    return 0;
}

