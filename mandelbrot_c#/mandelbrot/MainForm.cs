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
        private const int maxIterations = 50;

        public MainForm()
        {
            InitializeComponent();

            MainForm_Resize(this, null);
        }

        private int getValue(float x0, float y0)
        {
            // x0 - scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
            // y0 - scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))

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

        public static Color Rainbow(float progress)
        {
            float div = (Math.Abs(progress % 1) * 6);
            int ascending = (int)((progress % 1) * 255);
            int descending = 255 - ascending;

            switch ((int)div)
            {
                case 0:
                    return Color.FromArgb(255, 255, ascending, 0); // RED -> YELLOW
                case 1:
                    return Color.FromArgb(255, descending, 255, 0); // YELLOW -> GREEEN
                case 2:
                    return Color.FromArgb(255, 0, 255, ascending); // GREEN -> CYAN
                case 3:
                    return Color.FromArgb(255, 0, descending, 255); // CYAN -> BLUE
                case 4:
                    return Color.FromArgb(255, ascending, 0, 255); // BLUE -> MAGENTA
                case 5:
                    return Color.FromArgb(255, 255, 0, descending); // MAGENTA -> RED
                default:
                    return Color.Black;
            }
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

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
                    mandelbrot_values[x, y] = getValue(-2.5f + 3.5f * ((float)x / (float)b.Width), -1.0f + 2.0f * ((float)y / (float)b.Height));
                }
            }

            // Copy the ARGB values into the array.
            Marshal.Copy(bmpData.Scan0, argbValues, 0, argbValues.Length);

            for (int x = 0; x < b.Width; x++)
            {
                for (int y = 0; y < b.Height; y++)
                {
                    Color color = Rainbow((float)mandelbrot_values[x, y] / (float)maxIterations);

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
