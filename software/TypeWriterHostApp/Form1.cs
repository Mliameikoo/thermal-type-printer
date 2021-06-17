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

namespace TypeWriterHostApp
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            /*string[] available_serial_port;
            available_serial_port = System.IO.Ports.SerialPort.GetPortNames();*/

            //定义注册表顶级节点 其命名空间是using Microsoft.Win32;
            RegistryKey USBKey;
            //检索子项    
            USBKey = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Enum\USBSTOR", false);

            //检索所有子项USBSTOR下的字符串数组
            foreach (string sub1 in USBKey.GetSubKeyNames())
            {
                RegistryKey sub1key = USBKey.OpenSubKey(sub1, false);
                foreach (string sub2 in sub1key.GetSubKeyNames())
                {
                    try
                    {
                        //打开sub1key的子项
                        RegistryKey sub2key = sub1key.OpenSubKey(sub2, false);
                        //检索Service=disk(磁盘)值的子项 cdrom(光盘)
                        if (sub2key.GetValue("Service", "").Equals("disk"))
                        {
                            String Path = "USBSTOR" + "\\" + sub1 + "\\" + sub2;
                            String Name = (string)sub2key.GetValue("FriendlyName", "");
                            richTextBox1.AppendText("USB名称  " + Name + "\r\n");
                            richTextBox1.AppendText("UID标记  " + sub2 + "\r\n");
                            richTextBox1.AppendText("路径信息 " + Path + "\r\n\r\n");
                        }
                    }
                    catch (Exception msg) //异常处理
                    {
                        MessageBox.Show(msg.Message);
                    }
                }
            }

        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }
    }
}
