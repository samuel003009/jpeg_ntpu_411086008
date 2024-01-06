#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
float PI = 3.14159265359;
#define N 8 // matrix size

// 定義 BMP 標頭的結構
typedef struct Bmpheader 
{
	char identifier[2];             // BMP 檔案標識，0x0000
	unsigned int filesize;          // 檔案大小，0x0002
	unsigned short reserved;        // 保留字段，0x0006
	unsigned short reserved2;
	unsigned int bitmap_dataoffset; // 位圖數據的偏移，0x000A
	unsigned int bitmap_headersize; // 位圖標頭的大小，0x000E
	unsigned int width;             // 影像寬度，0x0012
	unsigned int height;            // 影像高度，0x0016
	unsigned short planes;          // 圖像的位平面數，0x001A
	unsigned short bits_perpixel;   // 每像素位元數，0x001C
	unsigned int compression;       // 圖像壓縮方式，0x001E
	unsigned int bitmap_datasize;   // 位圖數據的大小，0x0022
	unsigned int hresolution;       // 水平分辨率，0x0026
	unsigned int vresolution;       // 垂直分辨率，0x002A
	unsigned int usedcolors;        // 使用的顏色索引數，0x002E
	unsigned int importantcolors;   // 重要的顏色索引數，0x0032
	unsigned int palette;           // 調色板，0x0036
} Bitmap;

// 定義 RGB 結構
typedef struct RGB 
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} ImgRGB;

//function 0
void readheader(FILE* fp, Bitmap *x);
ImgRGB** malloc_2D(int row, int col);
void InputData(FILE* fp, ImgRGB **array, int H, int W, int skip);
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip,FILE* out_R,FILE* out_G,FILE* out_B);

//function 1
void rgb2ycbcr(ImgRGB **array, int H, int W);
double C(int x);
void dct(double matrix[N][N], double result[N][N]); 
void img_dct(ImgRGB **array,int H,int W,FILE* out_Qt_Y,FILE* out_Qt_Cb,FILE* out_Qt_Cr,FILE* out_qF_Y,FILE* out_qF_Cb,FILE* out_qF_Cr,FILE* out_eF_Y,FILE* out_eF_Cb,FILE* out_eF_Cr);
void quantize(double dct_result[N][N], int quantizationTable[N][N],short quantize_result[N][N]);

int main(int argc, char **argv, ImgRGB **array) 
{
	int method;
	char inputbmp[100];
	char output_R[100];
	char output_G[100];
	char output_B[100];
	char output_size[100]; 
	char Qt_Y[100];
	char Qt_Cb[100];
	char Qt_Cr[100];
	char qF_Y[100];
	char qF_Cb[100];
	char qF_Cr[100];
	char eF_Y[100];
	char eF_Cb[100];
	char eF_Cr[100];
	int i, j, x, y ,a ,b;
	double matrix[N][N];
	FILE* fp_in;
	FILE* fp_R;
	FILE* fp_G;
	FILE* fp_B;
	FILE* fp_dim;
	FILE* fp_Qt_Y;
	FILE* fp_Qt_Cb;
	FILE* fp_Qt_Cr;
	FILE* fp_qF_Y;
	FILE* fp_qF_Cb;
	FILE* fp_qF_Cr;
	FILE* fp_eF_Y;
	FILE* fp_eF_Cb;
	FILE* fp_eF_Cr;

	method=atoi(argv[1]);
	strcpy(inputbmp,argv[2]);

	// 開啟輸入檔案
	fp_in = fopen(inputbmp, "rb");
	if (fp_in) printf("Open %s as input file!\n", inputbmp);
	else printf("Fail to open %s as input file!\n", inputbmp);

	if(method == 0)
	{
		strcpy(output_R,argv[3]);
		strcpy(output_G,argv[4]);
		strcpy(output_B,argv[5]);
		strcpy(output_size,argv[6]);
		fp_R=fopen(output_R,"w");
		fp_G=fopen(output_G,"w");
		fp_B=fopen(output_B,"w");
		fp_dim=fopen(output_size,"w");		
	}
	
	if(method == 1)
	{
		strcpy(Qt_Y,argv[3]);
		strcpy(Qt_Cb,argv[4]);
		strcpy(Qt_Cr,argv[5]);
		strcpy(output_size,argv[6]);
		strcpy(qF_Y,argv[7]);
		strcpy(qF_Cb,argv[8]);
		strcpy(qF_Cr,argv[9]);
		strcpy(eF_Y,argv[10]);
		strcpy(eF_Cb,argv[11]);
		strcpy(eF_Cr,argv[12]);
		fp_Qt_Y=fopen(Qt_Y,"w");
		fp_Qt_Cb=fopen(Qt_Cb,"w");
		fp_Qt_Cr=fopen(Qt_Cr,"w");
		fp_dim=fopen(output_size,"w");
		fp_qF_Y=fopen(qF_Y,"wb");
		fp_qF_Cb=fopen(qF_Cb,"wb");
		fp_qF_Cr=fopen(qF_Cr,"wb");
		fp_eF_Y=fopen(eF_Y,"wb");
		fp_eF_Cb=fopen(eF_Cb,"wb");
		fp_eF_Cr=fopen(eF_Cr,"wb");
	}

		// 讀取 BMP 標頭
	Bitmap bmpheader;
	readheader(fp_in, &bmpheader);

	// 影像寬度和高度
	int H = bmpheader.height;
	int W = bmpheader.width;

	fprintf(fp_dim,"%d %d",H,W);

	// 跳過每行結尾的填充（padding）
	int skip = (4 - (bmpheader.width * 3) % 4);
	if (skip == 4) skip = 0;
	char skip_buf[3] = { 0, 0, 0 };

	// 8x8 區塊，如果不是8的倍數，則位圖被填充，即多一個區塊
	int block_H = H / 8;
	if (H % 8 != 0) block_H++;
	int block_W = W / 8;
	if (W % 8 != 0) block_W++;

	printf("Image size: Width: %d x Height=%d pixels, total %d bytes\n", W, H, W*H * 3);
	//printf("Image line skip=%d bytes\n", skip);
	printf("\nLoading BMP data: RRGGBB RRGGBB ... (BGR format in BMP file)\n");
	printf("%d %d\n",H,W);

	// 分配二維記憶體空間，用於儲存 RGB 數據
	ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
	InputData(fp_in, Data_RGB, bmpheader.height, bmpheader.width, skip);
	fclose(fp_in);

	printf("Data size loaded: %d bytes\n", H*W * 3);

	if(method == 0)
	{
		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_in, bmpheader, skip,fp_R,fp_G,fp_B);
		printf("\n"); // DEBUG
		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_R);
		fclose(fp_G);
		fclose(fp_B);
		fclose(fp_dim);

		printf("Job done!\n");
		return 0;
	}
	if(method == 1)
	{
		// 對輸入資料進行rgb2ycbcr
		rgb2ycbcr(Data_RGB, H, W);
		img_dct(Data_RGB, H, W,fp_Qt_Y,fp_Qt_Cb,fp_Qt_Cr,fp_qF_Y,fp_qF_Cb,fp_qF_Cr,fp_eF_Y,fp_eF_Cb,fp_eF_Cr);

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_Qt_Y);
		fclose(fp_Qt_Cb);
		fclose(fp_Qt_Cr);
		fclose(fp_dim);
		fclose(fp_qF_Y);
		fclose(fp_qF_Cb);
		fclose(fp_qF_Cr);

		printf("Job done!\n");
		return 0;
	}

}

// 讀取 BMP 標頭的函數
void readheader(FILE* fp, Bitmap *x) 
{
	fread(&x->identifier, sizeof(x->identifier), 1, fp);
	printf("%c %c\n",x->identifier[0],x->identifier[1]);
	fread(&x->filesize, sizeof(x->filesize), 1, fp);
	printf("%d\n",x->filesize);
	fread(&x->reserved, sizeof(x->reserved), 1, fp);
	printf("%d\n",x->reserved);
	fread(&x->reserved2, sizeof(x->reserved2), 1, fp);
	printf("%d\n",x->reserved2);
	fread(&x->bitmap_dataoffset, sizeof(x->bitmap_dataoffset), 1, fp);
	printf("%d\n",x->bitmap_dataoffset);
	fread(&x->bitmap_headersize, sizeof(x->bitmap_headersize), 1, fp);
	printf("%d\n",x->bitmap_headersize);
	fread(&x->width, sizeof(x->width), 1, fp);
	printf("%d\n",x->width);
	fread(&x->height, sizeof(x->height), 1, fp);
	printf("%d\n",x->height);
	fread(&x->planes, sizeof(x->planes), 1, fp);
	printf("%d\n",x->planes);
	fread(&x->bits_perpixel, sizeof(x->bits_perpixel), 1, fp);
	printf("%d\n",x->bits_perpixel);
	fread(&x->compression, sizeof(x->compression), 1, fp);
	printf("%d\n",x->compression);
	fread(&x->bitmap_datasize, sizeof(x->bitmap_datasize), 1, fp);
	printf("%d\n",x->bitmap_datasize);
	fread(&x->hresolution, sizeof(x->hresolution), 1, fp);
	printf("%d\n",x->hresolution);
	fread(&x->vresolution, sizeof(x->vresolution), 1, fp);
	printf("%d\n",x->vresolution);
	fread(&x->usedcolors, sizeof(x->usedcolors), 1, fp);
	printf("%d\n",x->usedcolors);
	fread(&x->importantcolors, sizeof(x->importantcolors), 1, fp);
	printf("%d\n",x->importantcolors);
}

// 分配二維記憶體空間的函數
ImgRGB** malloc_2D(int row, int col) 
{
	ImgRGB **Array, *Data;
	int i;
	Array = (ImgRGB**)malloc(row * sizeof(ImgRGB *));
	Data = (ImgRGB*)malloc(row * col * sizeof(ImgRGB));
	for (i = 0; i < row; i++, Data += col) 
	{
		Array[i] = Data;
	}
	return Array;
}

// 將資料輸入到 RGB 結構中的函數
void InputData(FILE* fp, ImgRGB **array, int H, int W, int skip) 
{
	int temp;
	char skip_buf[3];
	int i, j;
	for (i = 0; i < H; i++) 
	{
		for (j = 0; j < W; j++)
		{
			temp = fgetc(fp);
			array[i][j].B = temp;
			temp = fgetc(fp);
			array[i][j].G = temp;
			temp = fgetc(fp);
			array[i][j].R = temp;
			if (i < 9 && j < 8) printf("%02x%02x%02x ", array[i][j].R, array[i][j].G, array[i][j].B); // DEBUG
		}
		if (skip != 0) fread(skip_buf, skip, 1, fp);
		if (i < 9) printf("\n"); // DEBUG
	}
	printf("\n"); // DEBUG
}

// 輸出 BMP 標頭和數據的函數
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip,FILE* out_R,FILE* out_G,FILE* out_B) 
{
	char skip_buf[3] = { 0, 0, 0 };
	int x, y;
	fwrite(&bmpheader.identifier, sizeof(short), 1, outfile);
	fwrite(&bmpheader.filesize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.reserved, sizeof(short), 1, outfile);
	fwrite(&bmpheader.reserved2, sizeof(short), 1, outfile);
	fwrite(&bmpheader.bitmap_dataoffset, sizeof(int), 1, outfile);
	fwrite(&bmpheader.bitmap_headersize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.width, sizeof(int), 1, outfile);
	fwrite(&bmpheader.height, sizeof(int), 1, outfile);
	fwrite(&bmpheader.planes, sizeof(short), 1, outfile);
	fwrite(&bmpheader.bits_perpixel, sizeof(short), 1, outfile);
	fwrite(&bmpheader.compression, sizeof(int), 1, outfile);
	fwrite(&bmpheader.bitmap_datasize, sizeof(int), 1, outfile);
	fwrite(&bmpheader.hresolution, sizeof(int), 1, outfile);
	fwrite(&bmpheader.vresolution, sizeof(int), 1, outfile);
	fwrite(&bmpheader.usedcolors, sizeof(int), 1, outfile);
	fwrite(&bmpheader.importantcolors, sizeof(int), 1, outfile);

	for (x = 0; x<bmpheader.height; x++)
	{
		for (y = 0; y<bmpheader.width; y++)
		{
			if (y < 0 && x < 0) 
			{ // 讓 x<300 和 y<400 的像素為零
				RGB[x][y].B = 0;
				RGB[x][y].G = 0;
				RGB[x][y].R = 0;
			}

			fwrite(&RGB[x][y].B, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].G, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].R, sizeof(char), 1, outfile);

			fprintf(out_R,"%d ",RGB[x][y].R);
			fprintf(out_G,"%d ",RGB[x][y].G);
			fprintf(out_B,"%d ",RGB[x][y].B);

			if (x < 9 && y < 8) printf("%02x%02x%02x ", RGB[x][y].R, RGB[x][y].G, RGB[x][y].B); // DEBUG
		}
		if (skip != 0) { fwrite(skip_buf, skip, 1, outfile); }
		if (x < 9) printf("\n"); // DEBUG
		fprintf(out_R,"\n");
		fprintf(out_G,"\n");
		fprintf(out_B,"\n");
	}
}

// 對輸入資料進行 RGB 到 YCbCr 的轉換
void rgb2ycbcr(ImgRGB **array, int H, int W) 
{
    for (int i = 0; i < H; i++) 
    {
        for (int j = 0; j < W; j++) 
        {
            float R = (float)array[i][j].R;
            float G = (float)array[i][j].G;
            float B = (float)array[i][j].B;

            // 轉換 RGB 到 YCbCr
            float Y = 0.299 * R + 0.587 * G + 0.114 * B;
            float Cb = -0.1687 * R - 0.3313 * G + 0.5 * B + 128;
            float Cr = 0.5 * R - 0.4187 * G - 0.0813 * B + 128;

            // 量化 Y、Cb、Cr 值
            array[i][j].R = (unsigned char)Y;
            array[i][j].G = (unsigned char)Cb;
            array[i][j].B = (unsigned char)Cr;
        }
    }
}

// function to calculate the DCT coefficient
double C(int x) 
{
    if (x == 0)
        return sqrt(1.0 / N);
    else
        return sqrt(2.0 / N);
}

// function to perform 2D DCT on an 8x8 matrix
void dct(double matrix[N][N], double result[N][N]) 
{
    int i, j, k, l;

    // loop through the rows and columns of the result matrix
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {

            // initialize the sum to zero
            double sum = 0;

            // loop through the rows and columns of the input matrix
            for (k = 0; k < N; k++) {
                for (l = 0; l < N; l++) {

                    // multiply the input matrix element with the cosine factors and add to the sum
                    sum += matrix[k][l] * cos((2 * k + 1) * i * PI / (2 * N)) * cos((2 * l + 1) * j * PI / (2 * N));
                }
            }

            // multiply the sum with the DCT coefficient and store in the result matrix
            result[i][j] = C(i) * C(j) * sum;
        }
    }
}

// function to perform 2D DCT on an 8x8 matrix
void img_dct(ImgRGB **array,int H,int W,FILE* out_Qt_Y,FILE* out_Qt_Cb,FILE* out_Qt_Cr,FILE* out_qF_Y,FILE* out_qF_Cb,FILE* out_qF_Cr,FILE* out_eF_Y,FILE* out_eF_Cb,FILE* out_eF_Cr) 
{
    int i, j, a, b, c, d;
	double matrix_R[N][N];
	double result_R[N][N];
	double matrix_G[N][N];
	double result_G[N][N];
	double matrix_B[N][N];
	double result_B[N][N];
	double temp_Y[N][N];
	double temp_Cb[N][N];
	double temp_Cr[N][N];
	double error_Y[N][N];
	double error_Cb[N][N];
	double error_Cr[N][N];
	short quantize_result_Y[N][N];
	short quantize_result_Cb[N][N];
	short quantize_result_Cr[N][N];

	int quantizationTable_Y[N][N] = 
	{
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	};
	int quantizationTable_CbCr[N][N] = 
	{
		{17, 18, 24, 47, 99, 99, 99, 99},
		{18, 21, 26, 66, 99, 99, 99, 99},
		{24, 26, 56, 99, 99, 99, 99, 99},
		{47, 66, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99}
	};


	for(i = 0;i < N;i++)
	{
		for(j = 0;j < N;j++)
		{
			fprintf(out_Qt_Y,"%d ",quantizationTable_Y[i][j]);
			fprintf(out_Qt_Cb,"%d ",quantizationTable_CbCr[i][j]);
			fprintf(out_Qt_Cr,"%d ",quantizationTable_CbCr[i][j]);	
		}
		fprintf(out_Qt_Y,"\n");
		fprintf(out_Qt_Cb,"\n");
		fprintf(out_Qt_Cr,"\n");
	}
    for (i = 0; i < H; i+=8) 
	{
        for (j = 0; j < W; j+=8) 
		{
			for (a = 0; a < N; a++)
			{
				for (b = 0; b < N; b++)
				{
					if (((i+a) < H) && ((j+b) < W))
					{
						matrix_R[a][b] = (double) array[i+a][j+b].R;
						matrix_G[a][b] = (double) array[i+a][j+b].G;
						matrix_B[a][b] = (double) array[i+a][j+b].B;
					}
					else
					{
						matrix_R[a][b] = (double) 0;
						matrix_G[a][b] = (double) 0;
						matrix_B[a][b] = (double) 0;
					}
				}
			}

			dct (matrix_R, result_R);
			dct (matrix_G, result_G);
			dct (matrix_B, result_B);
			
			for(c=0;c<N;c++)
			{
				for(d=0;d<N;d++)
				{
					temp_Y[c][d]=result_R[c][d];
					temp_Cb[c][d]=result_G[c][d];
					temp_Cr[c][d]=result_B[c][d];
				}
			}
			
			quantize(result_R, quantizationTable_Y,quantize_result_Y);
            quantize(result_G, quantizationTable_CbCr,quantize_result_Cb);
            quantize(result_B, quantizationTable_CbCr,quantize_result_Cr);

			for(c=0;c<N;c++)
			{
				for(d=0;d<N;d++)
				{
					error_Y[c][d]=temp_Y[c][d]-quantize_result_Y[c][d]*quantizationTable_Y[c][d];
					error_Cb[c][d]=temp_Cb[c][d]-quantize_result_Cb[c][d]*quantizationTable_CbCr[c][d];
					error_Cr[c][d]=temp_Cr[c][d]-quantize_result_Cr[c][d]*quantizationTable_CbCr[c][d];
				}
			}

			for (a = 0; a < N; a++)
			{
				for (b = 0; b < N; b++)
				{
					fwrite(&(quantize_result_Y[a][b]), sizeof(short), 1, out_qF_Y);
					fwrite(&(quantize_result_Cb[a][b]), sizeof(short), 1, out_qF_Cb);
					fwrite(&(quantize_result_Cr[a][b]), sizeof(short), 1, out_qF_Cr);
				}
			}

			for (a = 0; a < N; a++)
			{
				for (b = 0; b < N; b++)
				{
					fwrite(&(error_Y[a][b]), sizeof(double), 1, out_eF_Y);
					fwrite(&(error_Cb[a][b]), sizeof(double), 1, out_eF_Cb);
					fwrite(&(error_Cr[a][b]), sizeof(double), 1, out_eF_Cr);
				}
			}
		}
	}

}

// function to quantize the DCT coefficients
void quantize(double dct_result[N][N], int quantizationTable[N][N],short quantize_result[N][N]) 
{
    for (int i = 0; i < N; i++) 
    {
        for (int j = 0; j < N; j++) 
        {
            quantize_result[i][j] = round(dct_result[i][j] / quantizationTable[i][j]);
        }
    }
}