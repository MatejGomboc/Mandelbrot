using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
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
                if (this.BackgroundImage == null) this.BackgroundImage = new Bitmap(this.ClientRectangle.Width, this.ClientRectangle.Height);

                Bitmap b = new Bitmap(this.BackgroundImage);
                Random r = new Random(Guid.NewGuid().GetHashCode());

                for (int n = 0; n < 10000; n++)
                {
                    b.SetPixel(x, y, Color.FromArgb(r.Next(0, 254), r.Next(0, 254), r.Next(0, 254), r.Next(0, 254)));

                    if (x < b.Width-1)
                    {
                        x++;
                    }
                    else
                    {
                        x = 0;
                        if (y < b.Height-1)
                        {
                            y++;
                        }
                        else
                        {
                            y = 0;
                        }
                    }
                }

                this.BackgroundImage = (Image)b;
            }
        }
    }
}
