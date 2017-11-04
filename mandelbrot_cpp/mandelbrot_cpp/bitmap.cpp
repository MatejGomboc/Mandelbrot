#include <windows.h>
#include <fstream>
#include <stdexcept>

#include "bitmap.h"


void save_to_bmp_file(const std::string file_name, const int width, const int height, const std::vector<uint8_t>& pixel_data)
{
	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biSizeImage = width * height * 3;
	bmih.biWidth = width;
	bmih.biHeight = height;
	bmih.biBitCount = 24;
	bmih.biCompression = BI_RGB;
	bmih.biPlanes = 1;
	bmih.biClrImportant = 0;
	bmih.biClrUsed = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;

	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 'B' + ('M' << 8);
	bmfh.bfSize = bmih.biSizeImage + bmih.biSize + sizeof(BITMAPFILEHEADER);
	bmfh.bfOffBits = bmih.biSize + sizeof(BITMAPFILEHEADER);
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;

	const unsigned long padding_size = ((width * 3) % 4 > 0) ? (4 - (width * 3) % 4) : 0;

	std::ofstream file;
	file.open(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!file.is_open()) std::runtime_error("Cannot create BMP file.");

	file.write(reinterpret_cast<const char*>(&bmfh), sizeof(BITMAPFILEHEADER));
	file.write(reinterpret_cast<const char*>(&bmih), sizeof(BITMAPINFOHEADER));

	for (unsigned long y = 0; y < height; y++)
	{
		for (unsigned long x = 0; x < width; x++)
		{
			file.write(reinterpret_cast<const char*>(&pixel_data[y * width * 4 + x * 4 + 2]), 1);
			file.write(reinterpret_cast<const char*>(&pixel_data[y * width * 4 + x * 4 + 1]), 1);
			file.write(reinterpret_cast<const char*>(&pixel_data[y * width * 4 + x * 4 + 0]), 1);
		}

		const uint8_t pad = 0;
		if (padding_size > 0) file.write(reinterpret_cast<const char*>(&pad), padding_size);
	}

	file.close();
}
