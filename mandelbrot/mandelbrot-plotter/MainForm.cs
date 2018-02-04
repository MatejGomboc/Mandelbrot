using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace mandelbrot_plotter
{
    public partial class MainForm : Form
    {
        private double min_x = -2.0;
        private double max_x = 1.0;
        private double min_y = -1.0;
        private double max_y = 1.0;
        private uint max_iterations = 40;

        public MainForm()
        {
            InitializeComponent();

            if (!MandelbrotDLL.init_opencl()) throw new Exception(Marshal.PtrToStringAnsi(MandelbrotDLL.get_status_message(), MandelbrotDLL.get_status_message_len()));
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            this.BackgroundImage = null;
            this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

            Bitmap b = new Bitmap(this.BackgroundImage);

            // Lock the bitmap's bits.
            BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

            if (!MandelbrotDLL.get_image_opencl(bmpData.Scan0, (uint)b.Width, (uint)b.Height, min_x, max_x, min_y, max_y, max_iterations))
                throw new Exception(Marshal.PtrToStringAnsi(MandelbrotDLL.get_status_message(), MandelbrotDLL.get_status_message_len()));

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

                if (!MandelbrotDLL.get_image_opencl(bmpData.Scan0, (uint)b.Width, (uint)b.Height, min_x, max_x, min_y, max_y, max_iterations))
                    throw new Exception(Marshal.PtrToStringAnsi(MandelbrotDLL.get_status_message(), MandelbrotDLL.get_status_message_len()));

                // Unlock the bits.
                b.UnlockBits(bmpData);

                this.BackgroundImage = (Image)b;
            }
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            MandelbrotDLL.release_opencl();
        }

        private void MainForm_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Down:
                case Keys.Up:
                case Keys.Left:
                case Keys.Right:
                case Keys.Add:
                case Keys.Subtract:
                    e.IsInputKey = true;
                    break;
                default:
                    break;
            }
        }

        private void MainForm_KeyDown(object sender, KeyEventArgs e)
        {
            double dist_x = max_x - min_x;
            double dist_y = max_y - min_y;

            switch (e.KeyCode)
            {
                case Keys.Down:
                    max_y += dist_y * 0.1;
                    min_y += dist_y * 0.1;
                    this.OnResize(null);
                    break;
                case Keys.Up:
                    max_y -= dist_y * 0.1;
                    min_y -= dist_y * 0.1;
                    this.OnResize(null);
                    break;
                case Keys.Left:
                    max_x -= dist_x * 0.1;
                    min_x -= dist_x * 0.1;
                    this.OnResize(null);
                    break;
                case Keys.Right:
                    max_x += dist_x * 0.1;
                    min_x += dist_x * 0.1;
                    this.OnResize(null);
                    break;
                case Keys.Add:
                    max_y -= dist_y * 0.1;
                    min_y += dist_y * 0.1;
                    max_x -= dist_x * 0.1;
                    min_x += dist_x * 0.1;
                    max_iterations += 10;
                    this.OnResize(null);
                    break;
                case Keys.Subtract:
                    max_y += dist_y * 0.1;
                    min_y -= dist_y * 0.1;
                    max_x += dist_x * 0.1;
                    min_x -= dist_x * 0.1;
                    if (max_iterations > 40) max_iterations -= 10;
                    this.OnResize(null);
                    break;
                default:
                    break;
            }
        }
    }
}
