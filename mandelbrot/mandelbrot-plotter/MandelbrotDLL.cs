using System;
using System.Runtime.InteropServices;

namespace mandelbrot_plotter
{
    static class MandelbrotDLL
    {
        [DllImport("mandelbrot-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int get_status_message_len();

        [DllImport("mandelbrot-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern IntPtr get_status_message();

        [DllImport("mandelbrot-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool init_opencl();

        [DllImport("mandelbrot-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void release_opencl();

        [DllImport("mandelbrot-dll.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern bool get_image_opencl(IntPtr pixel_data,
            uint width, uint height, double x_min, double x_max,
            double y_min, double y_max, uint max_iterations);
    }
}
