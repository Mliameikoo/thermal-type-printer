using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Management;
using System.IO.Ports;
using Microsoft.VisualBasic;
using Microsoft.VisualBasic.CompilerServices;

namespace TypeWriterHostApp
{
    public partial class Form1 : Form
    {
        private string str_vid = "0483";
        private string str_pid = "5740";

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            string[] available_serial_port;
            available_serial_port = System.IO.Ports.SerialPort.GetPortNames();

            ManagementObjectCollection.ManagementObjectEnumerator enumerator = null;
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("root\\CIMV2", "SELECT * FROM WIN32_PnPEntity");
            string comName = "";
            try
            {
                enumerator = searcher.Get().GetEnumerator();
                while (enumerator.MoveNext())
                {
                    ManagementObject current = (ManagementObject)enumerator.Current;
                    if (Strings.InStr(Conversions.ToString(current["Caption"]), "(COM", CompareMethod.Binary) <= 0)
                    {
                        continue;
                    }
                    /*foreach (var property in current.Properties)
                    {
                        richTextBox1.AppendText(property.Name + ":" + property.Value + "\r\n");//列出清单
                    }*/
                    if (current["DeviceID"].ToString().Substring(0, 5).Equals(@"USB\V"))//排除系统的COM1
                    {
                        if (current["DeviceID"].ToString().Substring(0, 21).Equals("USB\\VID_" + str_vid + @"&PID_" + str_pid))
                        {
                            comName = current["Name"].ToString().Substring(10, 4);
                            richTextBox1.AppendText(comName + "\r\n");
                            break;
                        }
                    }
                }
            }
            finally
            {
                if (enumerator != null)
                {
                    enumerator.Dispose();
                }
            }


        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
