#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
float PI = 3.14159265359;
#define N 8 // matrix size

// 定義 BMP 標頭的結構,54
typedef struct Bmpheader 
{
	char identifier[2];             // BMP 檔案標識,2
	unsigned int filesize;          // 檔案大小,4
	unsigned short reserved;        // 保留字段,2
	unsigned short reserved2;		//2
	unsigned int bitmap_dataoffset; // 位圖數據的偏移,4
	unsigned int bitmap_headersize; // 位圖標頭的大小,4
	unsigned int width;             // 影像寬度,4
	unsigned int height;            // 影像高度,4
	unsigned short planes;          // 圖像的位平面數,2
	unsigned short bits_perpixel;   // 每像素位元數,2
	unsigned int compression;       // 圖像壓縮方式,4
	unsigned int bitmap_datasize;   // 位圖數據的大小,4
	unsigned int hresolution;       // 水平分辨率,4
	unsigned int vresolution;       // 垂直分辨率,4
	unsigned int usedcolors;        // 使用的顏色索引數,4
	unsigned int importantcolors;   // 重要的顏色索引數,4
} Bitmap;

// 定義 RGB 結構
typedef struct RGB 
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} ImgRGB;

typedef struct RGB_1
{
	unsigned char R_1;
	unsigned char G_1;
	unsigned char B_1;
}ImgRGB_1;

ImgRGB_1** malloc_2D_1(int row, int col);
void InputData(FILE* fp, ImgRGB_1 **array, int H, int W);

//function 0
void readheader(FILE* fp, Bitmap *x);
ImgRGB** malloc_2D(int row, int col);
void InputData_0(FILE* fp_R, FILE* fp_G, FILE* fp_B, ImgRGB **array, int H, int W);
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip);

//function 1
void InputData_1(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr,FILE* fp_eF_Y,FILE* fp_eF_Cb,FILE* fp_eF_Cr, ImgRGB **array, int H, int W);
void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W);
void ycbcr2rgb(ImgRGB **array, int H, int W); 
void idct(short matrix[N][N], float result[N][N]);
void inverse_quantization(short matrix[N][N], int inverse_quantization_table[N][N]);

//function 2
void inverse_zigzag(short zigzag_result[N * N],short dct_result[N][N]);
void runLengthDecode(short *encoded, short zigzag_result[N * N]);
void InputData_2(ImgRGB **array, int H, int W,FILE* fp_inverserle);

int main(int argc, char **argv) 
{
	int method;
	char inputbmp[100];
	char original[100];
	char output_R[100];
	char output_G[100];
	char output_B[100]; 
	char output_Qt_Y[100];
	char output_Qt_Cb[100];
	char output_Qt_Cr[100];
	char output_qF_Y[100];
	char output_qF_Cb[100];
	char output_qF_Cr[100];
	char output_eF_Y[100];
	char output_eF_Cb[100];
	char output_eF_Cr[100];
	char output_size[100]; 
	int mode;
	char rle_ascii[100];
	int i, j, x, y ,a ,b;
	float matrix[N][N];
	float block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
	FILE *fp_out;

	float energy_noise_R=0.0;
    float energy_noise_G=0.0;
    float energy_noise_B=0.0;
    float energy_original_R=0.0;
    float energy_original_G=0.0;
    float energy_original_B=0.0;

	method=atoi(argv[1]);
	strcpy(inputbmp,argv[2]);

	//function 0
	if(method == 0)
	{
		strcpy(output_R,argv[3]);
		strcpy(output_G,argv[4]);
		strcpy(output_B,argv[5]);
		strcpy(output_size,argv[6]);
	}

	//function 1a
	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") != 0)
	{
		strcpy(original,argv[3]);
		strcpy(output_Qt_Y,argv[4]);
		strcpy(output_Qt_Cb,argv[5]);
		strcpy(output_Qt_Cr,argv[6]);
		strcpy(output_size,argv[7]);
		strcpy(output_qF_Y,argv[8]);
		strcpy(output_qF_Cb,argv[9]);
		strcpy(output_qF_Cr,argv[10]);
	}

	//function 1b
	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") == 0)
	{
		strcpy(output_Qt_Y,argv[3]);
		strcpy(output_Qt_Cb,argv[4]);
		strcpy(output_Qt_Cr,argv[5]);
		strcpy(output_size,argv[6]);
		strcpy(output_qF_Y,argv[7]);
		strcpy(output_qF_Cb,argv[8]);
		strcpy(output_qF_Cr,argv[9]);
		strcpy(output_eF_Y,argv[10]);
		strcpy(output_eF_Cb,argv[11]);
		strcpy(output_eF_Cr,argv[12]);
	}
	
	if(method == 2)
	{
		mode=atoi(argv[3]);
		strcpy(rle_ascii,argv[4]);
	}
	// 開啟輸出檔案
	fp_out = fopen(inputbmp, "wb");
	if (fp_out) printf("Open %s as output file!\n", inputbmp);
	else printf("Fail to open %s as output file!\n", inputbmp);

	Bitmap bmpheader;

	int H,W;
	if(method == 0 || method == 1)
	{
		FILE* fp_size=fopen(output_size,"r");
		fscanf(fp_size, "%d", &bmpheader.height);		//讀取bmp的height跟width
		fscanf(fp_size, "%d", &bmpheader.width);
		fclose(fp_size);
	}
	else
	{
		FILE* fp_rle_ascii=fopen(rle_ascii,"r");
		fscanf(fp_rle_ascii, "%d", &bmpheader.height);		//讀取bmp的height跟width
		fscanf(fp_rle_ascii, "%d", &bmpheader.width);
		fclose(fp_rle_ascii);
	}
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

	// 設定BMP的標頭檔
	bmpheader.identifier[0] = 'B';
	bmpheader.identifier[1] = 'M';
	bmpheader.filesize = 54 + H * W * 3; //54個標頭檔跟數據大小
	bmpheader.reserved = 0;
	bmpheader.reserved2 = 0;
	bmpheader.bitmap_dataoffset = 54;
	bmpheader.bitmap_headersize = 40;
	bmpheader.width = W;
	bmpheader.height = H;
	bmpheader.planes = 1;
	bmpheader.bits_perpixel = 24; 
	bmpheader.compression = 0;    
	bmpheader.bitmap_datasize = 3296040;
	bmpheader.hresolution = 2835; // 每米像素
	bmpheader.vresolution = 2835; // 每米像素
	bmpheader.usedcolors = 0;     
	bmpheader.importantcolors = 0; 

	printf("Image size: Width: %d x Height=%d pixels, total %d bytes\n", W, H, W*H * 3);
	printf("Image line skip=%d bytes\n", skip);
	printf("# of 8x8 blocks: W=%d x H=%d blocks, total %d blocks\n", block_W, block_H, block_W*block_H);

	printf("Loading BMP data: RRGGBB RRGGBB ... (BGR format in BMP file)\n");

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
		

		printf("Job done!\n");
		return 0;
	}

	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") == 0)
	{
		FILE* fp_qt_Y=fopen(output_Qt_Y,"r");
		FILE* fp_qt_Cb=fopen(output_Qt_Cb,"r");
		FILE* fp_qt_Cr=fopen(output_Qt_Cr,"r");
		FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
		FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
		FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");
		FILE* fp_eF_Y=fopen(output_eF_Y,"rb");
		FILE* fp_eF_Cb=fopen(output_eF_Cb,"rb");
		FILE* fp_eF_Cr=fopen(output_eF_Cr,"rb");


		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
		InputData_1(fp_qF_Y,fp_qF_Cb,fp_qF_Cr,fp_eF_Y,fp_eF_Cb,fp_eF_Cr, Data_RGB, H, W);

		printf("Data size loaded: %d bytes\n", H*W * 3);

		
		// 對輸入資料進行ycbcr2rgb;
		ycbcr2rgb(Data_RGB, H, W);

		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);

		printf("\nOutput decompressed image to: %s\n\n", inputbmp);

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_out);
		fclose(fp_qt_Y);
		fclose(fp_qt_Cb);
		fclose(fp_qt_Cr);
		fclose(fp_qF_Y);
		fclose(fp_qF_Cb);
		fclose(fp_qF_Cr);
		fclose(fp_eF_Y);
		fclose(fp_eF_Cb);
		fclose(fp_eF_Cr);

		printf("Job done!\n");
		return 0;
	}

	if(method == 1 && strcmp(inputbmp,"ResKimberlyNCat.bmp") != 0)
	{
		FILE* fp_original=fopen(original,"r");
		FILE* fp_qt_Y=fopen(output_Qt_Y,"r");
		FILE* fp_qt_Cb=fopen(output_Qt_Cb,"r");
		FILE* fp_qt_Cr=fopen(output_Qt_Cr,"r");
		FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
		FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
		FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");


		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
		ImgRGB_1 **Data_RGB_1 = malloc_2D_1(bmpheader.height, bmpheader.width);
		InputData_1_q(fp_qF_Y,fp_qF_Cb,fp_qF_Cr, Data_RGB, H, W);
		InputData(fp_original,Data_RGB_1,H,W);
		for (int c = 0; c < H; c++)
        {
            for (int d = 0; d < W; d++)
            {
				energy_original_R+=(Data_RGB_1[c][d].R_1)*(Data_RGB_1[c][d].R_1);
				energy_original_G+=(Data_RGB_1[c][d].G_1)*(Data_RGB_1[c][d].G_1);
				energy_original_B+=(Data_RGB_1[c][d].B_1)*(Data_RGB_1[c][d].B_1);
			}
		}

		printf("Data size loaded: %d bytes\n", H*W * 3);
		printf("\nOutput decompressed image to: %s\n\n", inputbmp);

		for (int c = 0; c < H; c++)
        {
            for (int d = 0; d < W; d++)
            {
				energy_original_R+=(Data_RGB_1[c][d].R_1)*(Data_RGB_1[c][d].R_1);
				energy_original_G+=(Data_RGB_1[c][d].G_1)*(Data_RGB_1[c][d].G_1);
				energy_original_B+=(Data_RGB_1[c][d].B_1)*(Data_RGB_1[c][d].B_1);
			}
		}
		// 對輸入資料進行ycbcr2rgb
		ycbcr2rgb(Data_RGB, H, W);

		for (int c = 0; c < H; c++)
        {
            for (int d = 0; d < W; d++)
            {
				energy_noise_R+=(Data_RGB[c][d].R-Data_RGB_1[c][d].R_1)*(Data_RGB[c][d].R-Data_RGB_1[c][d].R_1);
				energy_noise_G+=(Data_RGB[c][d].G-Data_RGB_1[c][d].G_1)*(Data_RGB[c][d].G-Data_RGB_1[c][d].G_1);
				energy_noise_B+=(Data_RGB[c][d].B-Data_RGB_1[c][d].B_1)*(Data_RGB[c][d].B-Data_RGB_1[c][d].B_1);
			}
		}
		

		printf("SQNR(R):%f\n",10*log10(energy_original_R/energy_noise_R));
		printf("SQNR(G):%f\n",10*log10(energy_original_G/energy_noise_G));
		printf("SQNR(B):%f\n",10*log10(energy_original_B/energy_noise_B));
		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);

		

		// 釋放記憶體並關閉檔案
		free(Data_RGB[0]);
		free(Data_RGB);
		fclose(fp_out);
		fclose(fp_qt_Y);
		fclose(fp_qt_Cb);
		fclose(fp_qt_Cr);
		fclose(fp_qF_Y);
		fclose(fp_qF_Cb);
		fclose(fp_qF_Cr);

		printf("Job done!\n");
		return 0;
	}

	if(method == 2)
	{
		FILE* fp_rle_ascii=fopen(rle_ascii,"r");
		// 分配二維記憶體空間，用於儲存 RGB 數據
		ImgRGB **Data_RGB = malloc_2D(bmpheader.height, bmpheader.width);
		InputData_2(Data_RGB, H, W,fp_rle_ascii);

		printf("Data size loaded: %d bytes\n\n", H*W * 3);
		printf("\nOutput decompressed image to: %s\n", inputbmp);
		
		// 對輸入資料進行ycbcr2rgb
		ycbcr2rgb(Data_RGB, H, W);

		// 輸出 BMP 檔案
		output_bmp(Data_RGB, fp_out, bmpheader, skip);

		

		fclose(fp_rle_ascii);
		fclose(fp_out);
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

ImgRGB_1** malloc_2D_1(int row, int col)
{
	ImgRGB_1 **Array, *Data;
	int i;
	Array = (ImgRGB_1**)malloc(row * sizeof(ImgRGB_1 *));
	Data = (ImgRGB_1*)malloc(row * col * sizeof(ImgRGB_1));
	for (i = 0; i < row; i++, Data += col) 
	{
		Array[i] = Data;
	}
	return Array;
}

void InputData(FILE* fp, ImgRGB_1 **array, int H, int W) 
{
	int temp;
	char skip_buf[3];
	int i, j;
	for (i = 0; i < H; i++) 
	{
		for (j = 0; j < W; j++)
		{
			temp = fgetc(fp);
			array[i][j].B_1 = temp;
			temp = fgetc(fp);
			array[i][j].G_1 = temp;
			temp = fgetc(fp);
			array[i][j].R_1 = temp;
			//if (i < 9 && j < 8) printf("%02x%02x%02x ", array[i][j].R, array[i][j].G, array[i][j].B); // DEBUG
		}
		//if (skip != 0) fread(skip_buf, skip, 1, fp);
		//if (i < 9) printf("\n"); // DEBUG
	}
	//printf("\n"); // DEBUG
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
	float error_Y[8][8],error_Cb[8][8],error_Cr[8][8];
    float idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
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
					fread(&block_Y[m][n], sizeof(short), 1, fp_qF_Y);		//讀取結果跟量化誤差
					fread(&block_Cb[m][n], sizeof(short), 1, fp_qF_Cb);
					fread(&block_Cr[m][n], sizeof(short), 1, fp_qF_Cr);
					fread(&error_Y[m][n], sizeof(float), 1, fp_eF_Y);
					fread(&error_Cb[m][n], sizeof(float), 1, fp_eF_Cb);
					fread(&error_Cr[m][n], sizeof(float), 1, fp_eF_Cr);					
				}
			}

            // 進行逆量化
            inverse_quantization(block_Y,inverse_quantizationTable_Y);
			inverse_quantization(block_Cb,inverse_quantizationTable_CbCr);
			inverse_quantization(block_Cr,inverse_quantizationTable_CbCr);

			//加上量化誤差
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
                }
            }
        }
    }
}

void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
    float idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
    int i, j, m, n;
	//Y量化table
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
	//CbCr量化table
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
					fread(&block_Y[m][n], sizeof(short), 1, fp_qF_Y);		//讀取結果
					fread(&block_Cb[m][n], sizeof(short), 1, fp_qF_Cb);
					fread(&block_Cr[m][n], sizeof(short), 1, fp_qF_Cr);					
				}
			}

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
                }
            }
        }
    }
}

void InputData_2(ImgRGB **array, int H, int W,FILE* fp_inverserle) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
	short block_Y_1[8][8], block_Cb_1[8][8], block_Cr_1[8][8];
	float error_Y[8][8],error_Cb[8][8],error_Cr[8][8];
    float idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
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
	short result_zigzag_Y[64];
	short result_zigzag_Cb[64];
	short result_zigzag_Cr[64]; 
	int result_dpcm_Y;
	int result_dpcm_Cb;
	int result_dpcm_Cr;
	short previousDC_Y = 0, previousDC_Cb = 0, previousDC_Cr = 0;
	int DC_Y;
	int DC_Cb;
	int DC_Cr;
	int freq;
	int count;
	int sum=0;
	short result_rle_Y[128]={0};
	short result_rle_Cb[128]={0};
	short result_rle_Cr[128]={0};
	int is_print;
	int printcon=0;
	int row;
	int column;

	

	fscanf(fp_inverserle,"%d %d",&row,&column);
    for (i = 0; i < H; i += 8) 
	{
        for (j = 0; j < W; j += 8) 
		{
			sum=0;
			fscanf(fp_inverserle,"%d",&DC_Y);
			for(m=0;m<N*N*2;m+=2)
			{
				fscanf(fp_inverserle,"%d",&freq);
				fscanf(fp_inverserle,"%d",&count);
				result_rle_Y[m]=freq;
				result_rle_Y[m+1]=count;
				sum+=count;
				if(sum>=63)
				{
					break;
				}
			}

			sum=0;
			fscanf(fp_inverserle,"%d",&DC_Cb);
			for(m=0;m<N*N*2;m+=2)
			{
				fscanf(fp_inverserle,"%d",&freq);
				fscanf(fp_inverserle,"%d",&count);
				result_rle_Cb[m]=freq;
				result_rle_Cb[m+1]=count;
				sum+=count;
				if(sum>=63)
				{
					break;
				}
			}

			sum=0;
			fscanf(fp_inverserle,"%d",&DC_Cr);
			for(m=0;m<N*N*2;m+=2)
			{
				fscanf(fp_inverserle,"%d",&freq);
				fscanf(fp_inverserle,"%d",&count);
				result_rle_Cr[m]=freq;
				result_rle_Cr[m+1]=count;
				sum+=count;
				if(sum>=63)
				{
					break;
				}
			}

			runLengthDecode(result_rle_Y,result_zigzag_Y);
			runLengthDecode(result_rle_Cb,result_zigzag_Cb);
			runLengthDecode(result_rle_Cr,result_zigzag_Cr);
			
			result_zigzag_Y[0]=DC_Y;
			result_zigzag_Cb[0]=DC_Cb;
			result_zigzag_Cr[0]=DC_Cr;

			

			// printf("\n");
			// if(i==0 && j==0)
			// {
			// 	for(m=0;m<N;m++)
			// 	{
			// 		for(n=0;n<N;n++)
			// 		{
			// 			printf("%5d",result_rle_Y[m*N+n]);
			// 		}
			// 		printf("\n");
			// 	}
			// }

			// for(int a=0;a<64;a++)
			// {
			// 	fscanf(fp_inversezigzag,"%d",&result_zigzag_Y[a]);
			// 	fscanf(fp_inversezigzag,"%d",&result_zigzag_Cb[a]);
			// 	fscanf(fp_inversezigzag,"%d",&result_zigzag_Cr[a]);
			// }
				// fscanf(fp_inversedpcm,"%d",&result_dpcm_Y);
				// fscanf(fp_inversedpcm,"%d",&result_dpcm_Cb);
				// fscanf(fp_inversedpcm,"%d",&result_dpcm_Cr);

			

            // if (i == 0 && j == 0) 
			// {
            //     // 第一個塊，直接使用DPCM值
            //     block_Y[0][0] = result_dpcm_Y;
            //     block_Cb[0][0] = result_dpcm_Cb;
            //     block_Cr[0][0] = result_dpcm_Cr;
            // } 
			// else 
			// {
            //     // 後續塊，加上前一塊的DC分量
            //     block_Y[0][0] = result_dpcm_Y + previousDC_Y;
            //     block_Cb[0][0] = result_dpcm_Cb + previousDC_Cb;
            //     block_Cr[0][0] = result_dpcm_Cr + previousDC_Cr;
            // }

            // // 更新前一塊的DC分量
            // previousDC_Y = block_Y[0][0];
            // previousDC_Cb = block_Cb[0][0];
            // previousDC_Cr = block_Cr[0][0];

			

			inverse_zigzag(result_zigzag_Y,block_Y);
			inverse_zigzag(result_zigzag_Cb,block_Cb);
			inverse_zigzag(result_zigzag_Cr,block_Cr);

			//DEBUG
			// is_print=0;
			// for(m=0;m<N;m++)
			// {
			// 	for(n=0;n<N;n++)
			// 	{
			// 		if(block_Y_1[m][n]!=block_Y[m][n])	
			// 		{
			// 			//printf("i=%d j=%d\n",i,j);
			// 			is_print=1;
			// 			printcon++;
			// 			break;
			// 		}
			// 	}
			// 	if(is_print==1)
			// 	{
			// 		break;
			// 	}
			// }

			// if(is_print==1&&printcon==1)
			// {
			// 	printf("i=%d j=%d\n",i,j);
			// 	for(m=0;m<N;m++)
			// 	{
			// 		for(n=0;n<N;n++)
			// 		{
			// 			printf("%5d",block_Y_1[m][n]);
			// 		}
			// 		printf("\n");
			// 	}
			

			
			
			// 	for(m=0;m<N;m++)
			// 	{
			// 		for(n=0;n<N;n++)
			// 		{
			// 			printf("%5d",block_Y[m][n]);
			// 		}
			// 		printf("\n");
			// 	}
			// }
								
				//}
			//}
			// block_Y[0][0]=result_dpcm_Y;
			// block_Cb[0][0]=result_dpcm_Cb;
			// block_Cr[0][0]=result_dpcm_Cr;
			
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
			//if (x < 9 && y < 8) printf("%02x%02x%02x ", RGB[x][y].R, RGB[x][y].G, RGB[x][y].B); // DEBUG
		}
		if (skip != 0) { fwrite(skip_buf, skip, 1, outfile); }
		//if (x < 9) printf("\n"); // DEBUG
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
            float R = Y - 0.00000122 * Cb + 1.402 * Cr;
            float G = Y - 0.34413568 * Cb - 0.71413616 * Cr;
            float B = Y + 1.77200007 * Cb + 0.00000041* Cr;

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

//計算DCT係數
float C(int x) 
{
    if (x == 0)
        return sqrt(1.0 / N);
    else
        return sqrt(2.0 / N);
}

// inverse DCT
void idct(short matrix[N][N], float result[N][N]) 
{
    int i, j, k, l;

    // loop through the rows and columns of the result matrix
    for (i = 0; i < N; i++) 
	{
        for (j = 0; j < N; j++) 
		{

            // initialize the sum to zero
            float sum = 0;

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

// inverse quantize
void inverse_quantization(short matrix[N][N], int inverse_quantization_table[N][N]) 
{
    int i, j;

    for (i = 0; i < N; i++) 
    {
        for (j = 0; j < N; j++) 
        {
            matrix[i][j] = matrix[i][j] * inverse_quantization_table[i][j];
        }
    }
}

void inverse_zigzag(short zigzag_result[N * N],short dct_result[N][N])
{
	int zigzag[N][N] = {
    {0, 1, 5, 6, 14, 15, 27, 28},
    {2, 4, 7, 13, 16, 26, 29, 42},
    {3, 8, 12, 17, 25, 30, 41, 43},
    {9, 11, 18, 24, 31, 40, 44, 53},
    {10, 19, 23, 32, 39, 45, 52, 54},
    {20, 22, 33, 38, 46, 51, 55, 60},
    {21, 34, 37, 47, 50, 56, 59, 61},
    {35, 36, 48, 49, 57, 58, 62, 63},
	};
	for(int i=0;i<N;i++)
	{
		for(int j=0;j<N;j++)
		{
			dct_result[i][j]=(short)zigzag_result[zigzag[i][j]];
			//dct_result[i][j]=0;
		}
	}
}

void runLengthDecode(short *encoded, short zigzag_result[N * N]) 
{
	int sum=1;
    for (int i = 0;sum<=63; i+=2) 
	{
        // Read the number
        int number = encoded[i];

        // Read the count (number of repetitions)
        int count = 0;
        
        count = encoded[i+1];
        
		// Print the number 'count' times
        for (int j = 0; j < count; j++) 
		{
            // Assuming zigzag_result is a 1D array representing a 2D array of size N*N
            zigzag_result[sum+j] = number; 
        }
		sum+=count;
    }
}
