using System;
using System.Windows.Forms;
using System.IO;

namespace mandelbrot_plotter
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            try
            {
                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);
                Application.Run(new MainForm());
            }
            catch(Exception ex)
            {
                using (StreamWriter writetext = new StreamWriter("error-log.txt"))
                {
                    writetext.Write(ex.Message);
                }

                MessageBox.Show("Error occurred in OpenCL. Error log written to error-log.txt.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);

                return;
            }
        }
    }
}
