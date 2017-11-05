using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Threading;
using System.Runtime.InteropServices;


namespace mandelbrot
{
    public partial class MainForm : Form
    {
        private const int maxIterations = 100;

        public MainForm()
        {
            InitializeComponent();

            MainForm_Resize(this, null);
        }

        private int getValue(float x_start, float y_start, float x_min, float x_max, float y_min, float y_max)
        {
            // Mandelbrot X scale (-2.5, 1)
            // Mandelbrot Y scale (-1, 1)

            float x0 = x_start * (x_max - x_min) + x_min;
            float y0 = y_start * (y_max - y_min) + y_min;

            float x = 0.0f;
            float y = 0.0f;
            int iteration = 0;
            float temp = 0.0f;

            while ((x * x + y * y < 2.0f * 2.0f) && (iteration < maxIterations))
            {
                temp = x * x - y * y + x0;
                y = 2.0f * x * y + y0;
                x = temp;
                iteration = iteration + 1;
            }

            return iteration;
        }

        public static Color Rainbow(double progress)
        {
            const double freq1 = 2 * Math.PI;
            const double freq2 = 2 * Math.PI;
            const double freq3 = 2 * Math.PI;

            const double phase1 = 0.0;
            const double phase2 = Math.PI / 2;
            const double phase3 = Math.PI;

            const double center = 127.0;
            const double width = 127.0;

            return Color.FromArgb(
                (int)(Math.Sin(freq1 * progress - phase1) * width + center),
                (int)(Math.Sin(freq2 * progress - phase2) * width + center),
                (int)(Math.Sin(freq3 * progress - phase3) * width + center),
                254);
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            this.BackgroundImage = new Bitmap(2048, 1024, PixelFormat.Format32bppArgb);

            Bitmap b = new Bitmap(this.BackgroundImage);

            // Lock the bitmap's bits.
            BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

            byte[] argbValues = new byte[Math.Abs(bmpData.Stride) * b.Height];
            int[,] mandelbrot_values = new int[b.Width, b.Height];

            // Calculate Mandelbrot values
            for (int x = 0; x < b.Width; x++)
            {
                for (int y = 0; y < b.Height; y++)
                {
                    mandelbrot_values[x, y] = getValue((float)x / (float)b.Width, (float)y / (float)b.Height, -2.0f, 1.0f, -1.0f, 1.0f);
                }
            }

            //int min = mandelbrot_values.Cast<int>().Min();
            //int max = mandelbrot_values.Cast<int>().Max();

            // Copy the ARGB values into the array.
            Marshal.Copy(bmpData.Scan0, argbValues, 0, argbValues.Length);

            for (int x = 0; x < b.Width; x++)
            {
                for (int y = 0; y < b.Height; y++)
                {
                    Color color = Rainbow((double)mandelbrot_values[x, y] / (double)maxIterations);

                    argbValues[(y * b.Width + x) * 4 + 0] = color.A;
                    argbValues[(y * b.Width + x) * 4 + 1] = color.R;
                    argbValues[(y * b.Width + x) * 4 + 2] = color.G;
                    argbValues[(y * b.Width + x) * 4 + 3] = color.B;
                }
            }

            // Copy the ARGB values back to the bitmap
            Marshal.Copy(argbValues, 0, bmpData.Scan0, argbValues.Length);

            // Unlock the bits.
            b.UnlockBits(bmpData);

            this.BackgroundImage = (Image)b;
        }
    }
}
