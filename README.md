# 第五次作業：土製 JPEG 圖片壓縮
411086008 通訊三 章棋翔

## 流程說明
### 使用方法0:
將bmp檔案分別存入R、G、B、dim，四個檔案，bmp檔案可以由RGB三個分量所組成，所以在還原回來的時候，先用dim判斷bmp大小，在讀入R、G、B三個分量後，加在一起還原出原本的bmp檔案。

### 使用方法一:
RGB轉YCbCr:原本bmp由R、G、B三個分量組成，經過換算之後，可以得出bmp由Y、Cb、Cr三個分量所組成，Y為亮度訊息，Cb跟Cr是色度訊息，因為人的眼睛對亮度的差別感應比較明顯，以及Y、Cb、Cr相比R、G、B可以有更好的壓縮效果，所以採用Y、Cb、Cr三個分量。

DCT:把YCbCr資料每8x8矩陣做離散餘弦轉換(DCT)，可以把圖像分成好幾個不同頻率的成分，由頻率低到高從8x8矩陣的左上往右下遞增，低頻成分如大範圍的背景(變化緩慢)，高頻如細節的部份(變化快速)，因為人眼對高頻的部分感知不明顯，所以在壓縮的時候，通常會在低頻的部分有較高的比重。
(程式在DCT的過程中出現些許誤差)

量化:把數據以較低的精度儲存，會進行一些四捨五入的運算，所以會出現量化誤差，造成還原bmp檔案的不精準，需要再加上量化誤差後，方可精準還原bmp檔案。(壓縮)

### 使用方法二:
DPCM:用來儲存8x8矩陣的直流成分，在matrix[0][0]的位置，紀錄圖像的平均亮度。
zigzag:處理8*8矩陣的交流成分，依照右上到左下，左下到右上的順序依序存取數值。
runlengthencode:把8x8矩陣的zigzag數據一樣的放在一起，變換成(頻率:次數)的形式。(壓縮)

### 使用方法三:
Huffman coding:將資料頻率高的以較短的編碼表示，將資料頻率低的以較長的編碼表示，用以減少整體的位元數量。(壓縮)

bmp的標頭檔跟RGB結構設定，其中，標頭檔的大小為54
```
// 定義 BMP 標頭的結構,54byte
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
```

## 使用方式0
### encoder 0

使用函數
```
//function 0
void readheader(FILE* fp, Bitmap *x);
ImgRGB** malloc_2D(int row, int col);
void InputData(FILE* fp, ImgRGB **array, int H, int W, int skip);
void output_bmp(ImgRGB **RGB, Bitmap bmpheader,FILE* out_R,FILE* out_G,FILE* out_B);
```


開啟輸入的bmp檔案跟要寫入的RGB檔案
```
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
```

輸出RGB數據(R.txt,G.txt,B.txt)
```
// 輸出RGB數據
void output_bmp(ImgRGB **RGB, Bitmap bmpheader,FILE* out_R,FILE* out_G,FILE* out_B) 
{
	char skip_buf[3] = { 0, 0, 0 };
	int x, y;
	for (x = 0; x<bmpheader.height; x++)
	{
		for (y = 0; y<bmpheader.width; y++)
		{
			fprintf(out_R,"%d ",RGB[x][y].R);
			fprintf(out_G,"%d ",RGB[x][y].G);
			fprintf(out_B,"%d ",RGB[x][y].B);
		}
		fprintf(out_R,"\n");
		fprintf(out_G,"\n");
		fprintf(out_B,"\n");
	}
}
```
輸出圖片大小(dim.txt)

```
if(method == 0 || method == 1)
{
	fprintf(fp_dim,"%d %d",H,W);
}
else
{
	fprintf(fp_rle_ascii,"%d %d\n",H,W);
}
```
### decoder 0

使用函式

```
//function 0
void readheader(FILE* fp, Bitmap *x);
ImgRGB** malloc_2D(int row, int col);
void InputData_0(FILE* fp_R, FILE* fp_G, FILE* fp_B, ImgRGB **array, int H, int W);
void output_bmp(ImgRGB **RGB, FILE* outfile, Bitmap bmpheader, int skip);
```

讀入圖片大小
```
if(method == 0 || method == 1)
{
	FILE* fp_size=fopen(output_size,"r");
	fscanf(fp_size, "%d", &bmpheader.height);		//讀取bmp的height跟width
	fscanf(fp_size, "%d", &bmpheader.width);
	fclose(fp_size);
}
H=bmpheader.height;
W=bmpheader.width;
```

設定bmp檔案的標頭檔並將RGB檔案的資料寫入bmp檔案
```
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
```

## 使用方法1
### encoder 1

使用函式
```
//function 1
void rgb2ycbcr(ImgRGB **array, int H, int W);
double C(int x);
void dct(double matrix[N][N], double result[N][N]); 
void img_dct(ImgRGB **array,int H,int W,FILE* out_Qt_Y,FILE* out_Qt_Cb,FILE* out_Qt_Cr,FILE* out_qF_Y,FILE* out_qF_Cb,FILE* out_qF_Cr,FILE* out_eF_Y,FILE* out_eF_Cb,FILE* out_eF_Cr);
void quantize(double dct_result[N][N], int quantizationTable[N][N],short quantize_result[N][N]);
```

開啟要寫入的bmp檔案,Qt_Y.txt,Qt_Cb.txt,Qt_Cr.txt,dim.txt,qF_Y.raw,qF_Cb.raw,qF_Cr.raw等資料

```
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
```

把資料從RGB轉換成YCbCr
```
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

            array[i][j].R = (unsigned char)Y;
            array[i][j].G = (unsigned char)Cb;
            array[i][j].B = (unsigned char)Cr;
        }
    }
}
```

DCT的轉換函式
```
// 每8*8的matrix做2D DCT
void dct(double matrix[N][N], double result[N][N]) 
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
                    // multiply the input matrix element with the cosine factors and add to the sum
                    sum += matrix[k][l] * cos((2 * k + 1) * i * PI / (2 * N)) * cos((2 * l + 1) * j * PI / (2 * N));
                }
            }
            // multiply the sum with the DCT coefficient and store in the result matrix
            result[i][j] = C(i) * C(j) * sum;
        }
    }
}
```
量化函式
```
// quantize
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
```
在這個函式中做DCT跟量化
```
// DCT and quantization
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
			fprintf(out_Qt_Y,"%d ",quantizationTable_Y[i][j]);		//印出量化表
			fprintf(out_Qt_Cb,"%d ",quantizationTable_CbCr[i][j]);		//印出量化表
			fprintf(out_Qt_Cr,"%d ",quantizationTable_CbCr[i][j]);		//印出量化表	
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

			dct (matrix_R, result_R);	//DCT
			dct (matrix_G, result_G);
			dct (matrix_B, result_B);
			
			for(c=0;c<N;c++)
			{
				for(d=0;d<N;d++)
				{
					temp_Y[c][d]=result_R[c][d];		//暫存DCT結果
					temp_Cb[c][d]=result_G[c][d];
					temp_Cr[c][d]=result_B[c][d];
				}
			}
			
			quantize(result_R, quantizationTable_Y,quantize_result_Y);		//quantize
            quantize(result_G, quantizationTable_CbCr,quantize_result_Cb);
            quantize(result_B, quantizationTable_CbCr,quantize_result_Cr);

			for(c=0;c<N;c++)
			{
				for(d=0;d<N;d++)		//quantize error
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
					fwrite(&(quantize_result_Y[a][b]), sizeof(short), 1, out_qF_Y);		//印出量化結果
					fwrite(&(quantize_result_Cb[a][b]), sizeof(short), 1, out_qF_Cb);
					fwrite(&(quantize_result_Cr[a][b]), sizeof(short), 1, out_qF_Cr);
				}
			}

			for (a = 0; a < N; a++)
			{
				for (b = 0; b < N; b++)
				{
					fwrite(&(error_Y[a][b]), sizeof(double), 1, out_eF_Y);		//印出量化誤差
					fwrite(&(error_Cb[a][b]), sizeof(double), 1, out_eF_Cb);
					fwrite(&(error_Cr[a][b]), sizeof(double), 1, out_eF_Cr);
				}
			}
		}
	}

}
```
### decoder 1a

使用函式

```
//function 1
void InputData_1(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr,FILE* fp_eF_Y,FILE* fp_eF_Cb,FILE* fp_eF_Cr, ImgRGB **array, int H, int W);
void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W);
void ycbcr2rgb(ImgRGB **array, int H, int W); 
void idct(short matrix[N][N], double result[N][N]);
void inverse_quantization(short matrix[N][N], int inverse_quantization_table[N][N]);
```

把bmp檔案,Qt_Y.txt,Qt_Cb.txt,Qt_Cr.txt,dim.txt,qF_Y.raw,qF_Cb.raw,qF_Cr.raw等資料讀進來

```
FILE* fp_qt_Y=fopen(output_Qt_Y,"r");
FILE* fp_qt_Cb=fopen(output_Qt_Cb,"r");
FILE* fp_qt_Cr=fopen(output_Qt_Cr,"r");
FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");
```

逆DCT函式

```
//計算DCT係數
double C(int x) 
{
    if (x == 0)
        return sqrt(1.0 / N);
    else
        return sqrt(2.0 / N);
}

// inverse DCT
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
```

逆量化函式
```
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
```

做逆DCT跟逆量化
```
void InputData_1_q(FILE* fp_qF_Y,FILE* fp_qF_Cb,FILE* fp_qF_Cr, ImgRGB **array, int H, int W) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
    double idct_Y[8][8], idct_Cb[8][8], idct_Cr[8][8];
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
```

### decoder 1b

把Qt_Y.txt,Qt_Cb.txt,Qt_Cr.txt,dim.txt,qF_Y.raw, qF_Cb.raw,qF_Cr.raw,eF_Y.raw,eF_Cb.raw,eF_Cr.raw等資料讀進來

```
FILE* fp_qt_Y=fopen(output_Qt_Y,"r");
FILE* fp_qt_Cb=fopen(output_Qt_Cb,"r");
FILE* fp_qt_Cr=fopen(output_Qt_Cr,"r");
FILE* fp_qF_Y=fopen(output_qF_Y,"rb");
FILE* fp_qF_Cb=fopen(output_qF_Cb,"rb");
FILE* fp_qF_Cr=fopen(output_qF_Cr,"rb");
FILE* fp_eF_Y=fopen(output_eF_Y,"rb");
FILE* fp_eF_Cb=fopen(output_eF_Cb,"rb");
FILE* fp_eF_Cr=fopen(output_eF_Cr,"rb");
```

做逆DCT跟逆量化(沒有加量化誤差)
```
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
					fread(&block_Y[m][n], sizeof(short), 1, fp_qF_Y);		//讀取結果跟量化誤差
					fread(&block_Cb[m][n], sizeof(short), 1, fp_qF_Cb);
					fread(&block_Cr[m][n], sizeof(short), 1, fp_qF_Cr);
					fread(&error_Y[m][n], sizeof(double), 1, fp_eF_Y);
					fread(&error_Cb[m][n], sizeof(double), 1, fp_eF_Cb);
					fread(&error_Cr[m][n], sizeof(double), 1, fp_eF_Cr);					
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
```

## 使用方法2
### encoder 2

使用函式
```
//function 2
void dpcm(short dct_result[N][N], short dpcm_result[N][N], short* previousDC, FILE* fp_dpcm);
void zigzag(short dct_result[N][N],short zigzag_result[N*N]);
void runLengthEncode(short zigzag_result[N * N],FILE* fp_rle);
void img_rle(ImgRGB **array,int H,int W,FILE* out_rle);
```

把資料類型跟資料讀進來

```
strcpy(mode,argv[3]);
strcpy(rle_ascii,argv[4]);
fp_rle_ascii=fopen(rle_ascii,"w");
```
DPCM函式
```
void dpcm(short dct_result[N][N], short dpcm_result[N][N], short* previousDC, FILE* fp_dpcm) 
{
    // 處理第一個元素（DC分量）
    short currentDC = dct_result[0][0];
    dpcm_result[0][0] = currentDC - *previousDC;
    fprintf(fp_dpcm,"%d ", dpcm_result[0][0]);

    // 更新上一個DC分量
    *previousDC = currentDC;

    // 處理其餘元素（AC分量）
    for (int i = 1; i < N; i++) 
    {
        for (int j = 1; j < N; j++) 
        {
            dpcm_result[i][j] = dct_result[i][j];
        }
    }
}
```

zigzag函式
```
void zigzag(short dct_result[N][N],short zigzag_result[N * N])
{
	
	int min=dct_result[0][0];
	int zigzag[N][N] = 
    {
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
			zigzag_result[zigzag[i][j]]= (short) dct_result[i][j];
		}
	}
	
}
```

runlength函式
```
void runLengthEncode(short zigzag_result[N * N],FILE* fp_rle) 
{
    int length = N*N;
	//fprintf(fp_rle,"(%d,%d,%s)",row,column,ycbcr);
	fprintf(fp_rle,"%d ",zigzag_result[0]);
    for (int i = 1; i < length; i++) 
	{
        // Count occurrences of the current character
        unsigned char count = 1;
        while (i < length - 1 && zigzag_result[i] == zigzag_result[i + 1]) 
		{
            count++;
            i++;
        }

        // Print the character and its count
        fprintf(fp_rle,"%d %d ", zigzag_result[i], count);
    }
	fprintf(fp_rle,"\n");

}
```

做DCT,量化,zigzag,runlengthencode
```
void img_rle(ImgRGB **array,int H,int W,FILE* out_rle) 
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

	short dpcm_result_Y[N][N];
	short dpcm_result_Cb[N][N];
	short dpcm_result_Cr[N][N];

	short zigzag_result_Y[N * N];
	short zigzag_result_Cb[N * N];
	short zigzag_result_Cr[N * N];

    short rle_result_Y[N*N]={0};
    short rle_result_Cb[N*N]={0};
    short rle_result_Cr[N*N]={0};

	short previousDC = 0;

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

			

			

			// dpcm(quantize_result_Y, dpcm_result_Y,&previousDC,out_dpcm);
			// dpcm(quantize_result_Cb, dpcm_result_Cb,&previousDC,out_dpcm);
			// dpcm(quantize_result_Cr, dpcm_result_Cr,&previousDC,out_dpcm);

			zigzag(quantize_result_Y,zigzag_result_Y);
			zigzag(quantize_result_Cb,zigzag_result_Cb);
			zigzag(quantize_result_Cr,zigzag_result_Cr);


            runLengthEncode(zigzag_result_Y,out_rle);
			runLengthEncode(zigzag_result_Cb,out_rle);
			runLengthEncode(zigzag_result_Cr,out_rle);


		}
	}

}
```

### decoder 2
使用函式
```
//function 2
void inverse_zigzag(short zigzag_result[N * N],short dct_result[N][N]);
void runLengthDecode(short *encoded, short zigzag_result[N * N]);
void InputData_2(ImgRGB **array, int H, int W,FILE* fp_inverserle);
```

逆runlengthencode函式
```
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
```
逆zigzag函式
```
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
```

做逆runlengthencode,逆zigzag,逆量化,逆DCT
```
void InputData_2(ImgRGB **array, int H, int W,FILE* fp_inverserle) 
{
    short block_Y[8][8], block_Cb[8][8], block_Cr[8][8];
	short block_Y_1[8][8], block_Cb_1[8][8], block_Cr_1[8][8];
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

			is_print=0;
			for(m=0;m<N;m++)
			{
				for(n=0;n<N;n++)
				{
					if(block_Y_1[m][n]!=block_Y[m][n])	
					{
						//printf("i=%d j=%d\n",i,j);
						is_print=1;
						printcon++;
						break;
					}
				}
				if(is_print==1)
				{
					break;
				}
			}

			if(is_print==1&&printcon==1)
			{
				printf("i=%d j=%d\n",i,j);
				for(m=0;m<N;m++)
				{
					for(n=0;n<N;n++)
					{
						printf("%5d",block_Y_1[m][n]);
					}
					printf("\n");
				}
			

			
			
				for(m=0;m<N;m++)
				{
					for(n=0;n<N;n++)
					{
						printf("%5d",block_Y[m][n]);
					}
					printf("\n");
				}
			}
								
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

                    // YCbCr到RGB的轉換（如果有必要）
                    // ycbcrToRgb(&array[i + m][j + n]);
                }
            }

}
 
```



