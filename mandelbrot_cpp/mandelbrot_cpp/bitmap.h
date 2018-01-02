#ifndef BITMAP_H
#define BITMAP_H

#include <vector>
#include <cstdint>
#include <string>

extern void save_to_bmp_file(const std::string file_name, const unsigned width, const unsigned height, const std::vector<uint8_t>& pixel_data);

#endif
