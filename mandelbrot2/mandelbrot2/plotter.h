/* Copyright (C) 2019 Matej Gomboc https://github.com/MatejGomboc

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see http://www.gnu.org/licenses/. */


#ifndef PLOTTER_H
#define PLOTTER_H

#include <CL/opencl.h>
#include <vector>
#include <string>

class Plotter
{
private:
	std::vector<cl_device_id> m_ocl_devices;
	static std::string ocl_error_string(cl_int error_code);
public:
	Plotter();
	~Plotter();
	std::vector<std::string> get_device_names() const;
	bool select_device(int dev_indx, std::string& error_message);
	bool get_image(std::vector<char>& pixel_data, int width, int height, std::string& error_message);
};

#endif
