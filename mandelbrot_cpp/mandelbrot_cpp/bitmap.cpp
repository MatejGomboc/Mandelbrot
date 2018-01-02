#include <fstream>
#include <stdexcept>
#include <cstdint>

#include "bitmap.h"

typedef struct
{
	uint16_t bfType; // 2
	uint32_t bfSize; // 4
	uint16_t bfReserved1; // 2
	uint16_t bfReserved2; // 2
	uint32_t bfOffBits; // 4
} BitmapFileHeader; // 14

typedef struct
{
	uint32_t biSize; // 4
	int32_t biWidth; // 4
	int32_t biHeight; // 4
	uint16_t biPlanes; // 2
	uint16_t biBitCount; // 2
	uint32_t biCompression; // 4
	uint32_t biSizeImage; // 4
	int32_t biXPelsPerMeter; // 4
	int32_t biYPelsPerMeter; // 4
	uint32_t biClrUsed; // 4
	uint32_t biClrImportant; // 4
} BitmapInfoHeader; // 40

static void serialise_uint16(uint16_t value, char* buffer, int& indx)
{
	buffer[indx++] = (char)(value & 0x00FF);
	buffer[indx++] = (char)(((value & 0xFF00) >> 8) & 0x00FF);
}

static void serialise_uint32(uint32_t value, char* buffer, int& indx)
{
	buffer[indx++] = (char)(value & 0x000000FF);
	buffer[indx++] = (char)(((value & 0x0000FF00) >> 8) & 0x000000FF);
	buffer[indx++] = (char)(((value & 0x00FF0000) >> 16) & 0x000000FF);
	buffer[indx++] = (char)(((value & 0xFF000000) >> 24) & 0x000000FF);
}

void save_to_bmp_file(const std::string file_name, const unsigned width, const unsigned height, const std::vector<uint8_t>& pixel_data)
{
	const int padding_size = ((width * 3) % 4 > 0) ? (4 - (width * 3) % 4) : 0;

	BitmapFileHeader bmfh;
	bmfh.bfType = 'B' + ('M' << 8);
	bmfh.bfSize = height * (width * 3 + padding_size) + 54;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 54;

	BitmapInfoHeader bmih;
	bmih.biSize = 40;
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biPlanes = 1;
	bmih.biBitCount = 24;
	bmih.biCompression = 0; // BI_RGB
	bmih.biSizeImage = height * (width * 3 + padding_size);
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	std::ofstream file;
	file.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open()) std::runtime_error("Cannot create BMP file.");

	// BITMAPFILEHEADER
	char buff_bmfh[14];
	int indx_bmfh = 0;
	serialise_uint16(bmfh.bfType, buff_bmfh, indx_bmfh);
	serialise_uint32(bmfh.bfSize, buff_bmfh, indx_bmfh);
	serialise_uint16(bmfh.bfReserved1, buff_bmfh, indx_bmfh);
	serialise_uint16(bmfh.bfReserved2, buff_bmfh, indx_bmfh);
	serialise_uint32(bmfh.bfOffBits, buff_bmfh, indx_bmfh);
	file.write(buff_bmfh, 14);

	// BITMAPINFOHEADER
	char buff_bmih[40];
	int indx_bmih = 0;
	serialise_uint32(bmih.biSize, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biWidth, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biHeight, buff_bmih, indx_bmih);
	serialise_uint16(bmih.biPlanes, buff_bmih, indx_bmih);
	serialise_uint16(bmih.biBitCount, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biCompression, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biSizeImage, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biXPelsPerMeter, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biYPelsPerMeter, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biClrUsed, buff_bmih, indx_bmih);
	serialise_uint32(bmih.biClrImportant, buff_bmih, indx_bmih);
	file.write(buff_bmih, 40);

	// pixel data (RGBA -> BGR)
	for (unsigned y = 0; y < height; y++)
	{
		for (unsigned x = 0; x < width; x++)
		{
			file.write(reinterpret_cast<const char*>(&pixel_data.data()[y * width * 4 + x * 4 + 2]), 1);
			file.write(reinterpret_cast<const char*>(&pixel_data.data()[y * width * 4 + x * 4 + 1]), 1);
			file.write(reinterpret_cast<const char*>(&pixel_data.data()[y * width * 4 + x * 4 + 0]), 1);
		}

		const uint8_t pad = 0;
		if (padding_size > 0) file.write(reinterpret_cast<const char*>(&pad), padding_size);
	}

	file.close();
}
