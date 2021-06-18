using Microsoft.Graph;
using Microsoft.VisualBasic;
using Microsoft.VisualBasic.CompilerServices;
using System;
using System.Management;
using System.Windows.Forms;
using System.IO.Ports;


namespace TypeWriterHostApp
{
    public struct PrinterTypeDef
    {
        public string com;
        public bool is_com_open;
    };

    


    public partial class Form1 : Form
    {
        PrinterClass printerClass = new PrinterClass();

        public Form1()
        {
            
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            timer1.Interval = 1000;     // 毫秒为单位
            timer1.Enabled = true;      // 开启定时器
            
        }

        public void Uart_Send_Info(byte cmd, string buf)
        {
            var sendbuf = new byte[buf.Length + 5];
            sendbuf[0] = 0xF5;
            sendbuf[1] = ((byte)buf.Length);
            sendbuf[2] = cmd;
            for (int i = 0; i < buf.Length; i++)
            {
                sendbuf[i + 3] = (byte)buf[i];
            }
            byte sum_check = 0;
            for (int i = 2; i < sendbuf.Length - 1; i++)
            {
                sum_check += sendbuf[i];
            }
            sendbuf[sendbuf.Length - 2] = sum_check;
            sendbuf[sendbuf.Length - 1] = 0x5F;

            serialPort1.Write(sendbuf, 0, sendbuf.Length);
        }



        private void button1_Click(object sender, EventArgs e)
        {
            string[] available_serial_port;
            available_serial_port = SerialPort.GetPortNames();



            // Uart_Send_Info(0x01, "hello");

            Uart_Send_Info(0x01, richTextBox1.Text);
            richTextBox1.Clear();


        }

        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void richTextBox1_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            
            if (e.Control && e.KeyCode == Keys.Enter)
            {
                Uart_Send_Info(0x01, richTextBox1.Text);
                richTextBox1.Clear();
                e.Handled = true;
            }
            
        }





        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (!serialPort1.IsOpen)
            {
                if(printerClass.TargetCom_Find())
                {
                    serialPort1.PortName = printerClass.PrinterInfo.com;
                    serialPort1.BaudRate = 9600;
                    serialPort1.DataBits = 8;
                    serialPort1.StopBits = System.IO.Ports.StopBits.One;
                    serialPort1.Parity = System.IO.Ports.Parity.None;
                    try
                    {
                        serialPort1.Open();
                        textBox1.Text = ("open" + "\r\n");
                        printerClass.PrinterInfo.is_com_open = true;
                    }
                    catch
                    {
                        textBox1.Text = ("open error" + "\r\n");
                    }
                }
                else
                {
                    textBox1.Text = ("not find com" + "\r\n");
                }
            }
            else
            {
                textBox1.Text = ("is opening" + "\r\n");
            }
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }


    }

    public class PrinterClass
    {
        private string str_vid = "0483";
        private string str_pid = "5740";
        public PrinterTypeDef PrinterInfo;

        public bool TargetCom_Find()
        {
            bool retval = false;
            ManagementObjectCollection.ManagementObjectEnumerator enumerator = null;
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("root\\CIMV2", "SELECT * FROM WIN32_PnPEntity");
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
                            PrinterInfo.com = current["Name"].ToString().Substring(10, 4);
                            retval = true;
                            /*richTextBox1.AppendText(Printer.com + "\r\n");*/
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
            return retval;
        }



    }
}
