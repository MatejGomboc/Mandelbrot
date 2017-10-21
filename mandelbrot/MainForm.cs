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
using System.Numerics;
using System.Threading;


namespace mandelbrot
{
    public partial class MainForm : Form
    {
        private Thread workerThread = null;
        private delegate void ThreadSafeDelegate();

        private int x = 0;
        private int y = 0;

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (workerThread != null)
            {
                workerThread.Abort();
                workerThread.Join();
                workerThread = null;
            }
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            if (workerThread == null)
            {
                workerThread = new Thread(new ThreadStart(threadProcess));
                workerThread.Start();
                while (!workerThread.IsAlive) ;
            }
        }


        private void threadProcess()
        {
            while (true)
            {
                try
                {
                    render();
                    Thread.Sleep(1); // To not saturate a core
                }
                catch (ThreadAbortException)
                {
                    break;
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    break;
                }
            }
        }


        private void render()
        {
            if (this.InvokeRequired)
            {
                ThreadSafeDelegate d = new ThreadSafeDelegate(render);
                this.Invoke(d);
            }
            else
            {
                if (this.BackgroundImage == null) this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height, PixelFormat.Format32bppArgb);

                Random r = new Random(Guid.NewGuid().GetHashCode());

                Bitmap b = new Bitmap(this.BackgroundImage);

                // Lock the bitmap's bits.
                BitmapData bmpData = b.LockBits(new Rectangle(0, 0, b.Width, b.Height), ImageLockMode.ReadWrite, b.PixelFormat);

                byte[] argbValues = new byte[Math.Abs(bmpData.Stride) * b.Height];

                // Copy the ARGB values into the array.
                System.Runtime.InteropServices.Marshal.Copy(bmpData.Scan0, argbValues, 0, argbValues.Length);

                for (int i = 0; i < b.Width * b.Height; i++)
                {
                    argbValues[i*4+0] = (byte)r.Next(254);
                    argbValues[i*4+1] = (byte)r.Next(254);
                    argbValues[i*4+2] = (byte)r.Next(254);
                    argbValues[i*4+3] = (byte)r.Next(254);
                }

                // Copy the ARGB values back to the bitmap
                System.Runtime.InteropServices.Marshal.Copy(argbValues, 0, bmpData.Scan0, argbValues.Length);

                // Unlock the bits.
                b.UnlockBits(bmpData);

                this.BackgroundImage = (Image)b;
            }
        }
    }
}
