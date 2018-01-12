#pragma once

#include <vector>

#pragma pack(1)	//1字节对齐

typedef struct _GIM_FILE_FLAGS{
	unsigned char Signature[4];
	unsigned char GIMVersion[4];
	unsigned char Format[4];
	unsigned char reserved[4];
} GIM_FILE_FLAGS;

typedef struct _GIM_FILE_EOF_ADDR_PART{
	unsigned short flags;		//0x02
	unsigned short reserved;	//null
	unsigned long EOFAddr;		//End of File addr (このパートの先頭からのファイル終端のオフセット)
	unsigned long size;			//part size (実質16bytes[0x10]固定)
	unsigned long _const;		//0x10?
} GIM_FILE_EOF_ADDR_PART;

typedef struct _GIM_FILE_HEADER{
	GIM_FILE_FLAGS gff;
	GIM_FILE_EOF_ADDR_PART gfeof;
} GIM_FILE_HEADER;

typedef struct _GIM_FILE_FILEINFO_ADDR_PART{
	unsigned short flags;		//0x03
	unsigned short reserved;	//null
	unsigned long FileInfoAddr;	//fileinfo addr (このパートの先頭からのfileinfo data partのオフセット)
	unsigned long size;			//part size (実質16bytes[0x10]固定)
	unsigned long _const;		//0x10?
} GIM_FILE_FILEINFO_ADDR_PART;

typedef struct _GIM_PALETTE_DATA{
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
} GIM_PALETTE_DATA;

typedef struct _GIM_FILE_PALETTE_DATA_PART{
	unsigned short flags;		//0x05
	unsigned short reserved1;	//null
	unsigned long size0;		//(+0x08と同じ値？)
	unsigned long size1;		//+0x08 [04]	part size
	unsigned long _const1;		//0x10?
	unsigned short data_offset;	//data offset?
	unsigned short reserved2;	//null?
	unsigned short palette_format;	//0x00:RGBA5650 0x01:RGBA5551 0x02:RGBA4444 0x03:RGBA8888
	unsigned short reserved3;	//null?
	unsigned short palette_colors;	//0x10:16 colors 0x100:256 colors
	unsigned char unknow1[14];	//null?
	unsigned long _const2;		//0x30?
	unsigned long _const3;		//0x40?
	unsigned long size2;		//part size -16[0x10]
	unsigned long reserved4;	//null?
	unsigned short _const4;		//0x02?
	unsigned short _const5;		//0x01?
	unsigned short _const6;		//0x03?
	unsigned short _const7;		//0x01?
	unsigned long _const8;		//0x40?
	unsigned char unknow2[12];	//null?
	GIM_PALETTE_DATA palette_data[1];	//+0x50 [xx]	palette data(16色なら16x4=64B、256色なら256x4=1024B)
} GIM_FILE_PALETTE_DATA_PART;
/*
GIM用のパレットのデータはR,G,B,Aの4バイト×色数と並んでいる。
BMP用のパレットのデータはB,G,R,Aの順なのでR,Bの値の入れ替えが必要。
*/

typedef struct _GIM_FILE_IMAGE_DATA_PART{
	unsigned short flags;		//0x04
	unsigned short reserved1;	//null
	unsigned long size0;		//(+0x08と同じ値？)
	unsigned long size1;		//+0x08 [04]	part size
	unsigned long _const1;		//0x10?
	unsigned short data_offset;	//data offset?
	unsigned short reserved2;	//null?
	unsigned short image_format;	//image_format
								//0x00:rgba5650
								//0x01:rgba5551
								//0x02:rgba4444
								//0x03:rgba8888
								//0x04:index4
								//0x05:index8
								//0x06:index16
								//0x07:index32

	unsigned short pixel_order; //+0x16 [02]	pixel_order
								//0x00:normal
								//0x01:faster (PSP高速ピクセル格納形式 [16x8ブロック再配置])
	unsigned short visible_width;	//+0x18 [02]	visible width(XMBアイコンなどに使われる可視領域サイズ)
	unsigned short visible_height;	//+0x1A [02]	visible height( 同 )
	unsigned short color_depth;		//+0x1C [02]	color depth
									//index4:0x04
									//index8:0x08
									//index16:0x10
									//index32:0x20
									//rgba****:0x20
	unsigned short _const2;		//0x10?
	unsigned short _const3;		//0x08?
	unsigned short _const4;		//0x02?
	unsigned long reserved3;	//null?
	unsigned long _const5;		//0x30?
	unsigned long _const6;		//0x40?
	unsigned long size2;		//part size -16[0x10]
	unsigned long reserved4;	//null?
	unsigned short _const7;		//0x01?
	unsigned short _const8;		//0x01?
	unsigned short _const9;		//0x03?
	unsigned short _const10;	//0x01?
	unsigned long _const11;		//0x40?
	unsigned char unknow1[12];	//null?
	unsigned char image_data[1];	//+0x50 [xx]
} GIM_FILE_IMAGE_DATA_PART;

typedef struct _GIM_IMAGE_BLOCK_HEADER{
	GIM_FILE_FILEINFO_ADDR_PART file_info;
	GIM_FILE_PALETTE_DATA_PART palette_data;
} GIM_IMAGE_BLOCK_HEADER;

typedef struct _GIM_FILEINFO_DATA_PART{
	unsigned short flags;		//0xFF
	unsigned short reserved;	//null
	unsigned long size0;		//(+0x08と同じ値？)
	unsigned long size1;		//+0x08 [04]	part size
	unsigned long _const;		//0x10?
	unsigned char info[1];		//(自由に記述可能) GimConvというソフトで変換されていることが多いようで、
								//変換元ファイル名(0x00)PCユーザー名？(0x00)日時(0x00)GimConv 1.xx
								//と書かれていることが多い。
} GIM_FILEINFO_DATA_PART;

typedef struct _GIM_COMMON_HEADER{
	unsigned short flags;		//0x05
	unsigned short reserved1;	//null
	unsigned long size0;		//(+0x08と同じ値？)
	unsigned long size1;		//+0x08 [04]	part size
	unsigned long _const1;		//0x10?
} GIM_COMMON_HEADER;

typedef struct _GIM_IMAGE{
	GIM_FILE_FILEINFO_ADDR_PART *file_info;
	GIM_FILE_PALETTE_DATA_PART *palette;
	GIM_FILE_IMAGE_DATA_PART *image;
} GIM_IMAGE;

#pragma pack()

//1图象的描绘方式，BMP的图象数据排列是从左往右从下向上的，而GIM则是左上到右下角描绘。


#include <pshpack2.h>

typedef struct tagBITMAPFILEINFO {
	BITMAPFILEHEADER bmiFileHeader;
	BITMAPINFOHEADER bmiHeader; // 位图信息头
} BITMAPFILEINFO;

#include <poppack.h>


#define GIM_FILE_EOF_ADDR_PART_FLAG		0x02
#define GIM_FILEINFO_ADDR_PART_FLAG		0x03
#define GIM_IMAGE_DATA_PART_FLAG		0x04
#define GIM_PALETTE_DATA_PART_FLAG		0x05
#define GIM_FILEINFO_DATA_PART_FLAG		0xff


class CGim
{
public:
	CGim(unsigned char *gim_data, unsigned long gim_size);
	virtual ~CGim(void);

	static unsigned long BGRA2RGBA(unsigned long crColor);

	unsigned long GetImageCount()
	{
		return m_vecimages.size();
	};

	GIM_IMAGE& GetImage(unsigned long index)
	{
		return m_vecimages[index];
	};

protected:
	unsigned char *m_gim_data;
	unsigned long m_gim_size;

	void gim();

	std::vector<GIM_IMAGE> m_vecimages;
};

