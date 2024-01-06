#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
float PI = 3.14159265359;
#define N 8 // matrix size

/* 定義 BMP 標頭的結構 */
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

/* 定義 RGB 結構 */
typedef struct RGB 
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} ImgRGB;

void readheader(FILE* fp, Bitmap *x);
ImgRGB** malloc_2D(int row, int col);
void InputData_0(FILE* fp_R, FILE* fp_G, FILE* fp_B, ImgRGB **array, int H, int W);
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip);

void InputData_1(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr,FILE* fp_eF_Y,FILE* fp_eF_Cb,FILE* fp_eF_Cr, ImgRGB **array, int H, int W);
void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W);
void ycbcr2rgb(ImgRGB **array, int H, int W); 
void idct(short matrix[N][N], double result[N][N]);
void img_idct(ImgRGB **array,int H,int W,FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr);
void inverse_quantization(short matrix[N][N], int inverse_quantization_table[N][N]);

int main(int argc, char **argv) 
{
	int method;
	char inputbmp[100];
	char original[100];
	char output_R[100];
	char output_G[100];
	char output_B[100]; 
	char output_qF_Y[100];
	char output_qF_Cb[100];
	char output_qF_Cr[100];
	char output_eF_Y[100];
	char output_eF_Cb[100];
	char output_eF_Cr[100];
	char output_size[100]; 
	int i, j, x, y ,a ,b;
	double matrix[N][N];
	double block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
	FILE *fp_out;

	method=atoi(argv[1]);
	strcpy(inputbmp,argv[2]);
	if(method == 0)
	{
		strcpy(output_R,argv[3]);
		strcpy(output_G,argv[4]);
		strcpy(output_B,argv[5]);
		strcpy(output_size,argv[6]);
	}

	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") == 0)
	{
		strcpy(output_R,argv[3]);
		strcpy(output_G,argv[4]);
		strcpy(output_B,argv[5]);
		strcpy(output_size,argv[6]);
		strcpy(output_qF_Y,argv[7]);
		strcpy(output_qF_Cb,argv[8]);
		strcpy(output_qF_Cr,argv[9]);
		strcpy(output_eF_Y,argv[10]);
		strcpy(output_eF_Cb,argv[11]);
		strcpy(output_eF_Cr,argv[12]);
	}
	
	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") != 0)
	{
		strcpy(original,argv[3]);
		strcpy(output_R,argv[4]);
		strcpy(output_G,argv[5]);
		strcpy(output_B,argv[6]);
		strcpy(output_size,argv[7]);
		strcpy(output_qF_Y,argv[8]);
		strcpy(output_qF_Cb,argv[9]);
		strcpy(output_qF_Cr,argv[10]);
	}
	// 開啟輸出檔案
	fp_out = fopen(inputbmp, "wb");
	if (fp_out) printf("Open %s as output file!\n", inputbmp);
	else printf("Fail to open %s as output file!\n", inputbmp);

	Bitmap bmpheader;

	int H,W;
	FILE* fp_size=fopen(output_size,"r");
	fscanf(fp_size, "%d", &bmpheader.height);
	fscanf(fp_size, "%d", &bmpheader.width);
	H=bmpheader.height;
	W=bmpheader.width;

	// 跳過每行結尾的填充（padding）
	int skip = (4 - (bmpheader.width * 3) % 4);
	if (skip == 4) skip = 0;
	char skip_buf[3] = { 0, 0, 0 };

	// 8x8 區塊，如果不是8的倍數，則位圖被填充，即多一個區塊
	int block_H = H / 8;
	if (H % 8 != 0) block_H++;
	int block_W = W / 8;
	if (W % 8 != 0) block_W++;

	// 设置 BMP 文件头字段
	bmpheader.identifier[0] = 'B';
	bmpheader.identifier[1] = 'M';
	bmpheader.filesize = 54 + H * W * 3; // 54 字节的头部 + 像素数据
	bmpheader.reserved = 0;
	bmpheader.reserved2 = 0;
	bmpheader.bitmap_dataoffset = 54;
	bmpheader.bitmap_headersize = 40;
	bmpheader.width = W;
	bmpheader.height = H;
	bmpheader.planes = 1;
	bmpheader.bits_perpixel = 24; // 24位彩色图
	bmpheader.compression = 0;    // 不压缩
	bmpheader.bitmap_datasize = 3296040;
	bmpheader.hresolution = 2835; // 像素每米（可根据需要调整）
	bmpheader.vresolution = 2835; // 像素每米（可根据需要调整）
	bmpheader.usedcolors = 0;     // 使用的颜色数（0 表示所有颜色）
	bmpheader.importantcolors = 0; // 重要的颜色数（0 表示所有颜色）

	printf("Image size: Width: %d x Height=%d pixels, total %d bytes\n", W, H, W*H * 3);
	printf("Image line skip=%d bytes\n", skip);
	printf("# of 8x8 blocks: W=%d x H=%d blocks, total %d blocks\n", block_W, block_H, block_W*block_H);

	printf("\nLoading BMP data: RRGGBB RRGGBB ... (BGR format in BMP file)\n");

	if(method == 0)
	{
		// 打開RGB文件
		FILE *fp_R = fopen(output_R, "r");
		FILE *fp_G = fopen(output_G, "r");
		FILE *fp_B = fopen(output_B, "r");
		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(H,W);
		InputData_0(fp_R, fp_G, fp_B, Data_RGB, H, W);
		printf("Data size loaded: %d bytes\n", H*W * 3);
		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);
		printf("\n"); // DEBUG

		printf("Output decompressed image to: %s\n\n", inputbmp);

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_out);
		fclose(fp_R);
		fclose(fp_G);
		fclose(fp_B);
		fclose(fp_size);

		printf("Job done!\n");
		return 0;
	}

	if(method == 1 && strcmp(inputbmp,"ResKimberly.bmp") == 0)
	{
		FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
		FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
		FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");
		FILE* fp_eF_Y=fopen(output_eF_Y,"rb");
		FILE* fp_eF_Cb=fopen(output_eF_Cb,"rb");
		FILE* fp_eF_Cr=fopen(output_eF_Cr,"rb");


		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
		InputData_1(fp_qF_Y,fp_qF_Cb,fp_qF_Cr,fp_eF_Y,fp_eF_Cb,fp_eF_Cr, Data_RGB, H, W);
		//fclose(fp_in);

		printf("Data size loaded: %d bytes\n", H*W * 3);

		
		// 對輸入資料進行ycbcr2rgb
		//img_idct(Data_RGB, H, W,fp_qF_Y,fp_qF_Cb,fp_qF_Cr);
		ycbcr2rgb(Data_RGB, H, W);

		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);
		printf("\n"); // DEBUG

		printf("Output decompressed image to: %s\n\n", inputbmp);

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_out);
		fclose(fp_qF_Y);
		fclose(fp_qF_Cb);
		fclose(fp_qF_Cr);
		fclose(fp_eF_Y);
		fclose(fp_eF_Cb);
		fclose(fp_eF_Cr);

		printf("Job done!\n");
		return 0;
	}

	if(method == 1 && strcmp(inputbmp,"ResKimberly.bmp") != 0)
	{
		FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
		FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
		FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");


		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
		InputData_1_q(fp_qF_Y,fp_qF_Cb,fp_qF_Cr, Data_RGB, H, W);
		//fclose(fp_in);

		printf("Data size loaded: %d bytes\n", H*W * 3);

		
		// 對輸入資料進行ycbcr2rgb
		//img_idct(Data_RGB, H, W,fp_qF_Y,fp_qF_Cb,fp_qF_Cr);
		ycbcr2rgb(Data_RGB, H, W);

		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);
		printf("\n"); // DEBUG

		printf("Output decompressed image to: %s\n\n", inputbmp);

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_out);
		fclose(fp_qF_Y);
		fclose(fp_qF_Cb);
		fclose(fp_qF_Cr);

		printf("Job done!\n");
		return 0;
	}

	
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
void InputData_0(FILE* fp_R, FILE* fp_G, FILE* fp_B, ImgRGB **array, int H, int W)  
{
	int R, G, B;
	int i, j;
	for (i = 0; i < H; i++) 
	{
		for (j = 0; j < W; j++)
		{
			fscanf(fp_R, "%d", &R);
            fscanf(fp_G, "%d", &G);
            fscanf(fp_B, "%d", &B);

            array[i][j].R = R;
            array[i][j].G = G;
            array[i][j].B = B;
		}
	}
}

// 將資料輸入到 RGB 結構中的函數
void InputData_1(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr,FILE* fp_eF_Y,FILE* fp_eF_Cb,FILE* fp_eF_Cr, ImgRGB **array, int H, int W) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
	double error_Y[8][8],error_Cb[8][8],error_Cr[8][8];
    double idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
    int i, j, m, n;
	int inverse_quantizationTable_Y[N][N] = 
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
	int inverse_quantizationTable_CbCr[N][N] = 
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


    for (i = 0; i < H; i += 8) 
	{
        for (j = 0; j < W; j += 8) 
		{
			for(m=0;m<N;m++)
			{
				for(n=0;n<N;n++)
				{
					fread(&block_Y[m][n], sizeof(short), 1, fp_qF_Y);
					fread(&block_Cb[m][n], sizeof(short), 1, fp_qF_Cb);
					fread(&block_Cr[m][n], sizeof(short), 1, fp_qF_Cr);
					fread(&error_Y[m][n], sizeof(double), 1, fp_eF_Y);
					fread(&error_Cb[m][n], sizeof(double), 1, fp_eF_Cb);
					fread(&error_Cr[m][n], sizeof(double), 1, fp_eF_Cr);					
				}
			}
            // // 讀取每個8x8的壓縮塊
            // fread(block_Y, sizeof(double), 64, fp_qF_Y);
            // fread(block_Cb, sizeof(double), 64, fp_qF_Cb);
            // fread(block_Cr, sizeof(double), 64, fp_qF_Cr);

            // 進行逆量化
            inverse_quantization(block_Y,inverse_quantizationTable_Y);
			inverse_quantization(block_Cb,inverse_quantizationTable_CbCr);
			inverse_quantization(block_Cr,inverse_quantizationTable_CbCr);

			for (m = 0; m < 8 && (i + m) < H; m++) 
			{
                for (n = 0; n < 8 && (j + n) < W; n++) 
				{
					block_Y[m][n]=block_Y[m][n]+error_Y[m][n];
					block_Cb[m][n]=block_Cb[m][n]+error_Cb[m][n];
					block_Cr[m][n]=block_Cr[m][n]+error_Cr[m][n];
				}
			}

            // 進行逆DCT
            idct(block_Y, idct_Y);
            idct(block_Cb, idct_Cb);
            idct(block_Cr, idct_Cr);

            // 將轉換後的數據填充到array中
            for (m = 0; m < 8 && (i + m) < H; m++) 
			{
                for (n = 0; n < 8 && (j + n) < W; n++) 
				{
                    array[i + m][j + n].R = (unsigned char) idct_Y[m][n];
                    array[i + m][j + n].G = (unsigned char) idct_Cb[m][n];
                    array[i + m][j + n].B = (unsigned char) idct_Cr[m][n];

                    // YCbCr到RGB的轉換（如果有必要）
                    // ycbcrToRgb(&array[i + m][j + n]);
                }
            }
        }
    }
}

void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
    double idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
    int i, j, m, n;
	int inverse_quantizationTable_Y[N][N] = 
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
	int inverse_quantizationTable_CbCr[N][N] = 
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


    for (i = 0; i < H; i += 8) 
	{
        for (j = 0; j < W; j += 8) 
		{
			for(m=0;m<N;m++)
			{
				for(n=0;n<N;n++)
				{
					fread(&block_Y[m][n], sizeof(short), 1, fp_qF_Y);
					fread(&block_Cb[m][n], sizeof(short), 1, fp_qF_Cb);
					fread(&block_Cr[m][n], sizeof(short), 1, fp_qF_Cr);					
				}
			}
            // // 讀取每個8x8的壓縮塊
            // fread(block_Y, sizeof(double), 64, fp_qF_Y);
            // fread(block_Cb, sizeof(double), 64, fp_qF_Cb);
            // fread(block_Cr, sizeof(double), 64, fp_qF_Cr);

            // 進行逆量化
            inverse_quantization(block_Y,inverse_quantizationTable_Y);
			inverse_quantization(block_Cb,inverse_quantizationTable_CbCr);
			inverse_quantization(block_Cr,inverse_quantizationTable_CbCr);


            // 進行逆DCT
            idct(block_Y, idct_Y);
            idct(block_Cb, idct_Cb);
            idct(block_Cr, idct_Cr);

            // 將轉換後的數據填充到array中
            for (m = 0; m < 8 && (i + m) < H; m++) 
			{
                for (n = 0; n < 8 && (j + n) < W; n++) 
				{
                    array[i + m][j + n].R = (unsigned char) idct_Y[m][n];
                    array[i + m][j + n].G = (unsigned char) idct_Cb[m][n];
                    array[i + m][j + n].B = (unsigned char) idct_Cr[m][n];

                    // YCbCr到RGB的轉換（如果有必要）
                    // ycbcrToRgb(&array[i + m][j + n]);
                }
            }
        }
    }
}

// 輸出 BMP 標頭和數據的函數
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip) 
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
			fwrite(&RGB[x][y].B, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].G, sizeof(char), 1, outfile);
			fwrite(&RGB[x][y].R, sizeof(char), 1, outfile);
			if (x < 9 && y < 8) printf("%02x%02x%02x ", RGB[x][y].R, RGB[x][y].G, RGB[x][y].B); // DEBUG
		}
		if (skip != 0) { fwrite(skip_buf, skip, 1, outfile); }
		if (x < 9) printf("\n"); // DEBUG
	}
}

// YCbCr 到 RGB 的轉換函數
void ycbcr2rgb(ImgRGB **array, int H, int W) 
{
    for (int i = 0; i < H; i++) 
    {
        for (int j = 0; j < W; j++) 
        {
            float Y = array[i][j].R;
            float Cb = array[i][j].G - 128;
            float Cr = array[i][j].B - 128;

            // 轉換 YCbCr 到 RGB
            float R = Y + 1.402 * Cr;
            float G = Y - 0.344136 * Cb - 0.714136 * Cr;
            float B = Y + 1.772 * Cb;

            // 限制 RGB 值的範圍在 0 到 255 之間
            R = fmax(0, fmin(255, R));
            G = fmax(0, fmin(255, G));
            B = fmax(0, fmin(255, B));

            // 將計算得到的 RGB 值寫回數組
            array[i][j].R = (unsigned char)R;
            array[i][j].G = (unsigned char)G;
            array[i][j].B = (unsigned char)B;
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

void idct(short matrix[N][N], double result[N][N]) 
{
    int i, j, k, l;

    // loop through the rows and columns of the result matrix
    for (i = 0; i < N; i++) 
	{
        for (j = 0; j < N; j++) 
		{

            // initialize the sum to zero
            double sum = 0;

            // loop through the rows and columns of the input matrix
            for (k = 0; k < N; k++) 
			{
                for (l = 0; l < N; l++) 
				{

                    // multiply the input matrix element with the cosine factors and the IDCT coefficient and add to the sum
                    sum += C(k) * C(l) * matrix[k][l] * cos((2 * i + 1) * k * PI / (2 * N)) * cos((2 * j + 1) * l * PI / (2 * N));
                }
            }

            // store the sum in the result matrix
            result[i][j] = sum;
        }
    }
}


void inverse_quantization(short matrix[N][N], int inverse_quantization_table[N][N]) 
{
    int i, j;

    // 逆量化
    for (i = 0; i < N; i++) 
    {
        for (j = 0; j < N; j++) 
        {
            matrix[i][j] = matrix[i][j] * inverse_quantization_table[i][j];
        }
    }
}