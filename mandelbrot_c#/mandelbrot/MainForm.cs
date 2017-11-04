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
        public MainForm()
        {
            InitializeComponent();

            if (this.BackgroundImage == null) this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

            Bitmap b = new Bitmap(this.BackgroundImage);

            // Lock the bitmap's bits.
            BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

            byte[] argbValues = new byte[Math.Abs(bmpData.Stride) * b.Height];

            // Copy the ARGB values into the array.
            Marshal.Copy(bmpData.Scan0, argbValues, 0, argbValues.Length);

            Color color = Color.FromArgb(0, 0, 0, 0);

            for (int x = 0; x < b.Width; x++)
            {
                for (int y = 0; y < b.Height; y++)
                {
                    color = getColor(-2.5f + 3.5f * ((float)x / (float)b.Width), -1.0f + 2.0f * ((float)y / (float)b.Height), 100);

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

        private Color getColor(float x0, float y0, int maxIterations)
        {
            // x0 - scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
            // y0 - scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))

            float x = 0.0f;
            float y = 0.0f;
            int iteration = 0;
            float temp = 0.0f;

            while ((x*x + y*y < 2*2) && (iteration < maxIterations))
            {
                temp = x*x - y*y + x0;
                y = 2*x*y + y0;
                x = temp;
                iteration = iteration + 1;
            }

            return Color.FromArgb(iteration * 254 / maxIterations, iteration * 254 / maxIterations, iteration * 254 / maxIterations, iteration * 254 / maxIterations);
        }
    }
}
