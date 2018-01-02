using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;

namespace mandelbrot_plotter
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            MandelbrotDLL.init_opencl();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            this.BackgroundImage = null;
            this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

            Bitmap b = new Bitmap(this.BackgroundImage);

            // Lock the bitmap's bits.
            BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

            MandelbrotDLL.get_image_opencl(bmpData.Scan0, (uint)b.Width, (uint)b.Height, -2.0f, 1.0f, -1.0f, 1.0f, 40);

            // Unlock the bits.
            b.UnlockBits(bmpData);

            this.BackgroundImage = (Image)b;
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            if ((this.ClientRectangle.Width > 0) && (this.ClientRectangle.Height > 0))
            {
                this.BackgroundImage = null;
                this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

                Bitmap b = new Bitmap(this.BackgroundImage);

                // Lock the bitmap's bits.
                BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

                MandelbrotDLL.get_image_opencl(bmpData.Scan0, (uint)b.Width, (uint)b.Height, -2.0f, 1.0f, -1.0f, 1.0f, 40);

                // Unlock the bits.
                b.UnlockBits(bmpData);

                this.BackgroundImage = (Image)b;
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            MandelbrotDLL.release_opencl();
        }
    }
}
