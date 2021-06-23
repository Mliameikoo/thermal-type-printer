using Microsoft.Graph;
using Microsoft.VisualBasic;
using Microsoft.VisualBasic.CompilerServices;
using System;
using System.Management;
using System.Windows.Forms;
using System.IO.Ports;
using System.Drawing;
using System.Threading;

namespace TypeWriterHostApp
{
    public enum PrinterOperatingModeEnumDef
    {
        __SignalWordMode = 0,
        __ImageTextMode = 1,
    };

    public struct PrinterTypeDef
    {
        public string com;
        public bool is_com_open;
        public PrinterOperatingModeEnumDef mode;
    };
    public struct MessageSendInfoTypeDef
    {
        public int total_send_nums;         // 本轮总共需要发送的数目
        public int now_send_nums;           // 本轮发送中当前已发送的数目
        public bool send_enable_license;    // 是否启动发送
        public bool send_ack_flag;          // 发送后是否收到应答标志
        public int text_read_pos;           // 本轮发送中当前读取的位置
    };

    public partial class Form1 : Form
    {
        PrinterClass printerClass = new PrinterClass();
        FontDialog font_choose = new FontDialog();
        MessageSendInfoTypeDef sendInfo;

        public Form1()
        {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            timer1.Interval = 1000;     // 毫秒为单位
            timer1.Enabled = true;      // 开启定时器

            /*timer2.Interval = 1;        // 毫秒为单位
            timer2.Enabled = true;      // 开启定时器*/

            serialPort1.WriteTimeout = 20; //毫秒

            toolTip1.AutoPopDelay = 10000;
            toolTip1.InitialDelay = 500;
            toolTip1.ReshowDelay = 500;
            toolTip1.ShowAlways = true;
            toolTip1.SetToolTip(button1, "快捷键:Ctrl+Enter");

            Thread Thread_SendInfoSnoop_Handle = new Thread(new ThreadStart(Thread_SendInfoSnoop));
            Thread_SendInfoSnoop_Handle.Start();

            ToolStripMenuItem_2.Checked = true; // 默认是image_text写入模式

            printerClass.PrinterInfo.mode = PrinterOperatingModeEnumDef.__ImageTextMode;

            /*Thread Thread_ProgressBar_Handle = new Thread(new ThreadStart(Thread_ProgressBar));
            Thread_ProgressBar_Handle.Start();*/
        }

        /*public void Uart_Send_Info(byte cmd, string buf)
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

            try
            {
                serialPort1.Write(sendbuf, 0, sendbuf.Length);
            }
            catch
            {
                MessageBox.Show("设备未连接，请连接设备", "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }*/

        public void Uart_Send_Raw_Info(byte cmd, byte[] buf)
        {
            var sendbuf = new byte[buf.Length + 6];
            sendbuf[0] = 0xF5;
            sendbuf[1] = ((byte)(buf.Length >> 8));
            sendbuf[2] = ((byte)(buf.Length & 0xFF));
            sendbuf[3] = cmd;
            for (int i = 0; i < buf.Length; i++)
            {
                sendbuf[i + 4] = (byte)buf[i];
            }
            byte sum_check = 0;
            for (int i = 3; i < sendbuf.Length - 1; i++)
            {
                sum_check += sendbuf[i];
            }
            sendbuf[sendbuf.Length - 2] = sum_check;
            sendbuf[sendbuf.Length - 1] = 0x5F;
            
            try
            {
                serialPort1.Write(sendbuf, 0, sendbuf.Length);
            }
            catch(Exception ex)
            {
                serialPort1.Close();
                serialPort1.Dispose();
                richTextBox1.AppendText("\r\n" + ex.GetType().ToString()+"\r\n");
                richTextBox1.AppendText(ex.ToString());
                if (ex.GetType().ToString().Equals("System.InvalidOperationException"))
                {
                    MessageBox.Show("设备未连接，请连接设备", "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
                else if (ex.GetType().ToString().Equals("System.IO.IOException"))
                {
                    
                    MessageBox.Show("设备连接超时，已重启", "异常提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
        }

        private void uartSend_Get_Ready()
        {
            progressBar1.Value = 0;                                 // 清空进度条

            sendInfo.send_ack_flag = true;                          // 为首次发送做准备
            sendInfo.text_read_pos = 0;                             // 重置字符串读写指针
            sendInfo.total_send_nums = richTextBox1.Text.Length;    // 本轮总共要发送的字符个数
            sendInfo.now_send_nums = 0;                             // 已发送的字符个数
            sendInfo.send_enable_license = true;                    // 开启发送
        }
        private void uartSend_Cancel()
        {
            progressBar1.Value = 0;                                 // 清空进度条
            sendInfo.send_enable_license = false;                   // 关闭发送
        }

        private void null_text_error_handle()
        {
            MessageBox.Show("请输入内容", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            if (!sendInfo.send_enable_license)
            {
                if (richTextBox1.Text.Length == 0)
                {
                    null_text_error_handle();
                    return;
                }
                uartSend_Get_Ready();
            }
            else
            {
                button1.Text = "开始打印";
                uartSend_Cancel();
            }
        }

        private void Thread_SendInfoSnoop()
        {
            while (true)
            {
                if (sendInfo.send_enable_license)
                {
                    // button1.Enabled = false; // 锁定按键
                    button1.Text = "取消打印";
                    richTextBox1.Enabled = false; // 锁定写入框

                    if (sendInfo.send_ack_flag == true)
                    {
                        sendInfo.send_ack_flag = false;
                        Font font = new Font(richTextBox1.Font.FontFamily, richTextBox1.Font.Size, richTextBox1.Font.Style);
                        Bitmap map = printerClass.GetStrImage(richTextBox1.Text[sendInfo.text_read_pos++].ToString(), font);
                        // Bitmap map = printerClass.ChangeStringToImage(richTextBox1.Text[sendInfo.text_read_pos++].ToString());

                        byte[] bit_result = new byte[((map.Height + 7) / 8) * map.Width];
                        bit_result = printerClass.GetCodeTabFromBitmap_ColRowMode(map, map.Width, map.Height);

                        if (richTextBox1.Text[sendInfo.text_read_pos - 1] == '\n')
                        {
                            // 如果当前即将发送换行符，改用特殊指令发送
                            byte[] temporary_buf = new byte[2];
                            temporary_buf[0] = ((byte)map.Width);
                            temporary_buf[1] = ((byte)map.Height);
                            Uart_Send_Raw_Info(0x04, temporary_buf);
                        }
                        else
                        {
                            byte[] valid_buf = new byte[bit_result.Length + 4];
                            valid_buf[0] = ((byte)map.Width);
                            valid_buf[1] = ((byte)map.Height);
                            valid_buf[2] = ((byte)(bit_result.Length >> 8)); // first high-8-bit
                            valid_buf[3] = ((byte)(bit_result.Length & 0xFF)); // second low-8-bit
                            bit_result.CopyTo(valid_buf, 4);
                            Uart_Send_Raw_Info(0x03, valid_buf);
                        }
                        textBox3.Clear();
                        textBox3.AppendText(map.Width.ToString() + "," + map.Height.ToString() + "," + bit_result.Length.ToString());
                        // 发送数目自增
                        sendInfo.now_send_nums++;
                        // 填充进度条
                        progressBar1.Value = sendInfo.now_send_nums * 100 / sendInfo.total_send_nums;
                        // 结束判断
                        if (sendInfo.text_read_pos == richTextBox1.Text.Length)
                        {
                            sendInfo.send_enable_license = false; // 发送完毕
                        }
                    }
                    if (sendInfo.send_enable_license == false)
                    {
                        // button1.Enabled = true; // 释放按键
                        button1.Text = "开始打印";
                        richTextBox1.Enabled = true; // 释放写入框
                        /*System.Threading.Thread.CurrentThread.Abort();*/
                    }
                }
                else
                {
                    System.Threading.Thread.Sleep(10);
                }
            }
        }

        /*private void Thread_ProgressBar()
        {
            while (true)
            {
                if (sendInfo.send_enable_license)
                {
                    progressBar1.Value = sendInfo.now_send_nums * 100 / sendInfo.total_send_nums;
                }
                System.Threading.Thread.Sleep(500);
            }
        }*/

        private void timer2_Tick(object sender, EventArgs e)
        {
            /*if (sendInfo.send_enable_license)
            {
                button1.Enabled = false; // 锁定按键
                richTextBox1.Enabled = false; // 锁定写入框

                if (sendInfo.send_ack_flag == true)
                {
                    sendInfo.send_ack_flag = false;

                    Font font = new Font(richTextBox1.Font.FontFamily, richTextBox1.Font.Size, richTextBox1.Font.Style);
                    Bitmap map = printerClass.GetStrImage(richTextBox1.Text[sendInfo.text_read_pos++].ToString(), font);
                    byte[] bit_result = new byte[((map.Height + 7) / 8) * map.Width];
                    bit_result = printerClass.GetCodeTabFromBitmap_ColRowMode(map, map.Width, map.Height);

                    if (richTextBox1.Text[sendInfo.text_read_pos - 1] == '\n')
                    {
                        // 如果当前即将发送换行符，改用特殊指令发送
                        byte[] temporary_buf = new byte[2];
                        temporary_buf[0] = ((byte)map.Width);
                        temporary_buf[1] = ((byte)map.Height);

                        Uart_Send_Raw_Info(0x04, temporary_buf);
                    }
                    else
                    {
                        byte[] valid_buf = new byte[bit_result.Length + 4];
                        valid_buf[0] = ((byte)map.Width);
                        valid_buf[1] = ((byte)map.Height);
                        valid_buf[2] = ((byte)(bit_result.Length >> 8)); // first high-8-bit
                        valid_buf[3] = ((byte)(bit_result.Length & 0xFF)); // second low-8-bit
                        bit_result.CopyTo(valid_buf, 4);

                        Uart_Send_Raw_Info(0x03, valid_buf);
                    }

                    textBox3.Clear();
                    textBox3.AppendText(map.Width.ToString() + "," + map.Height.ToString() + "," + bit_result.Length.ToString());

                    sendInfo.now_send_nums++;
                    progressBar1.Value = sendInfo.now_send_nums / sendInfo.total_send_nums * 100;

                    if (sendInfo.text_read_pos == richTextBox1.Text.Length)
                    {
                        sendInfo.send_enable_license = false; // 发送完毕
                    }
                }
                
                if(sendInfo.send_enable_license == false)
                {
                    button1.Enabled = true; // 释放按键
                    richTextBox1.Enabled = true; // 释放写入框
                }
            }*/
        }

        private void SerialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            try
            {
                //因为要访问UI资源，所以需要使用invoke方式同步ui
                this.Invoke((EventHandler)(delegate
                {
                    string rxbuf = serialPort1.ReadExisting();
                    if(rxbuf.Length == 1)
                    {
                        if(rxbuf[0] == '1')
                        {
                            sendInfo.send_ack_flag = true;
                        }
                    }
                }
                    )
                );
            }
            catch (Exception ex)
            {
                //响铃并显示异常给用户
                System.Media.SystemSounds.Beep.Play();
                MessageBox.Show(ex.Message);
            }
        }


        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {
            if(printerClass.PrinterInfo.mode == PrinterOperatingModeEnumDef.__SignalWordMode)
            {
                byte[] signal_byte_buf = new byte[1];
                signal_byte_buf[0] = ((byte)richTextBox1.Text[richTextBox1.Text.Length - 1]);
                Uart_Send_Raw_Info(0x01, signal_byte_buf);
            }
        }

        private void richTextBox1_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.Control && e.KeyCode == Keys.Enter)
            {
                if (printerClass.PrinterInfo.mode ==  PrinterOperatingModeEnumDef.__ImageTextMode)
                {
                    if(richTextBox1.Text.Length == 0)
                    {
                        null_text_error_handle();
                    }
                    else
                    {
                        uartSend_Get_Ready();
                    }
                    e.Handled = true;
                }
            }
        }
        private void textBox1_TextChanged(object sender, EventArgs e)
        {

        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            // 需要加入new serialport...
            if (!serialPort1.IsOpen)
            {
                if (printerClass.TargetCom_Find())
                {
                    serialPort1.PortName = printerClass.PrinterInfo.com;
                    serialPort1.BaudRate = 115200;
                    serialPort1.DataBits = 8;
                    serialPort1.StopBits = System.IO.Ports.StopBits.One;
                    serialPort1.Parity = System.IO.Ports.Parity.None;
                    try
                    {
                        serialPort1.Open();
                        textBox1.Text = ("发现设备" + "\r\n");
                        textBox2.Text = printerClass.PrinterInfo.com;
                        printerClass.PrinterInfo.is_com_open = true;
                    }
                    catch
                    {
                        textBox1.Text = ("建立连接失败" + "\r\n");
                    }
                }
                else
                {
                    textBox1.Text = ("未找到设备" + "\r\n");
                }
            }
            else
            {
                textBox1.Text = ("连接中" + "\r\n");
            }
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (font_choose.ShowDialog() == DialogResult.OK)
            {
                System.Drawing.Font font = new System.Drawing.Font(font_choose.Font.FontFamily, font_choose.Font.Size, font_choose.Font.Style);
                richTextBox1.Font = font;
            }
        }

        private void toolTip1_Popup(object sender, PopupEventArgs e)
        {
            /*toolTip1.SetToolTip(button1, "haha");*/
        }

        private void textBox2_TextChanged_1(object sender, EventArgs e)
        {

        }

        private void textBox3_TextChanged(object sender, EventArgs e)
        {

        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            System.Environment.Exit(0); // 关闭窗体时，结束所有线程
        }

        private void ToolStripMenuItem_1_Click(object sender, EventArgs e)
        {
            printerClass.PrinterInfo.mode = PrinterOperatingModeEnumDef.__SignalWordMode;
            ToolStripMenuItem_1.Checked = true;
            ToolStripMenuItem_2.Checked = false;
            richTextBox1.Clear();
            button1.Enabled = false;
        }

        private void ToolStripMenuItem_2_Click(object sender, EventArgs e)
        {
            printerClass.PrinterInfo.mode = PrinterOperatingModeEnumDef.__ImageTextMode;
            ToolStripMenuItem_1.Checked = false;
            ToolStripMenuItem_2.Checked = true;
            richTextBox1.Clear();
            button1.Enabled = true;
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            if (!sendInfo.send_enable_license)
            {
                byte[] byte_buf = new byte[3];
                byte_buf[0] = ((byte)richTextBox1.Text[richTextBox1.Text.Length - 1]);
                Uart_Send_Raw_Info(0x05, byte_buf);
            }
        }
    }

    public class PrinterClass
    {
        private string str_vid = "0483";
        private string str_pid = "5740";
        public PrinterTypeDef PrinterInfo;


        public System.Drawing.Bitmap ChangeStringToImage(string pic)
        {
            try
            {
                byte[] imageBytes = Convert.FromBase64String(pic);
                //读入MemoryStream对象
                System.IO.MemoryStream memoryStream = new System.IO.MemoryStream(imageBytes, 0, imageBytes.Length);
                memoryStream.Write(imageBytes, 0, imageBytes.Length);

                /*//转成图片
                System.Drawing.Image image = System.Drawing.Image.FromStream(memoryStream);*/

                

                System.Drawing.Bitmap bitmap = new System.Drawing.Bitmap(memoryStream);

                

                return bitmap;
            }
            catch (Exception)
            {
                System.Drawing.Bitmap bitmap = null;
                return bitmap;
            }
        }

        //获取字符串图片
        public Bitmap GetStrImage(string hz, Font font)
        {
            //Bitmap bmp = new Bitmap(1, 1);
            /*// 使用g.MeasureString()获得
            Graphics g = Graphics.FromImage(bmp);
            SizeF sizeF = g.MeasureString(hz, font);*/
            // 使用TextRenderer.MeasureText()获得

            /*SizeF sizeF = TextRenderer.MeasureText( hz, font, new Size(0, 0), TextFormatFlags.NoPadding);
            // 重设位图大小
            Bitmap bmp = new Bitmap(Convert.ToInt32(sizeF.Width), Convert.ToInt32(sizeF.Height));
            Graphics g = Graphics.FromImage(bmp);

            // 背景色黑色
            g.Clear(Color.Black);
            g.DrawString(hz, font, new SolidBrush(Color.White), new PointF(0, 0));
            return bmp;*/

            Size size = TextRenderer.MeasureText(hz, font, new Size(0, 0), TextFormatFlags.NoPadding);
            // 重设位图大小
            Bitmap bmp = new Bitmap(Convert.ToInt32(size.Width), Convert.ToInt32(size.Height));
            Graphics g = Graphics.FromImage(bmp);


            StringFormat format = new StringFormat
            {
                Alignment = StringAlignment.Near, 
                LineAlignment = StringAlignment.Near
            };
            Rectangle rectangle = new Rectangle(0, 0, Convert.ToInt32(size.Width), Convert.ToInt32(size.Height));
            g.Clear(Color.Black); // 背景色黑色
            g.DrawString(hz, font, new SolidBrush(Color.White), rectangle, format);

            return bmp;
        }

        //检查Bitmap的一行有没有有效数据
        public bool CheckBitmapRow(Bitmap src, int row)
        {
            for (int i = 0; i < src.Width; i++)
            {
                if (src.GetPixel(i, row).ToArgb() != Color.Black.ToArgb())
                {
                    return true;
                }
            }
            return false;
        }
        //检查Bitmap的一列有没有有效数据
        public bool CheckBitmapCol(Bitmap src, int col)
        {
            for (int i = 0; i < src.Height; i++)
            {
                if (src.GetPixel(col, i).ToArgb() != Color.Black.ToArgb())
                {
                    return true;
                }
            }
            return false;
        }
        //获取点阵字模从Bitmap 列行式顺向
        //modWidth-字模宽 modHeight-字模高
        public byte[] GetCodeTabFromBitmap_ColRowMode(Bitmap src, int modWidth, int modHeight)
        {
            byte[] result = new byte[((modHeight + 7) / 8) * modWidth];
            int colOffset = 0, rowOffset = 0;
            //这里做了一点取模的偏移
            if (src.Height > modHeight)
            {
                for (rowOffset = 0; rowOffset < src.Height; rowOffset++)
                {
                    if ((src.Height - rowOffset) <= modHeight)
                    {
                        break;
                    }
                    if (CheckBitmapRow(src, rowOffset))
                    {
                        break;
                    }
                }
            }
            if (src.Width > modWidth)
            {
                for (colOffset = 0; colOffset < src.Width; colOffset++)
                {
                    if ((src.Width - colOffset) <= modWidth)
                    {
                        break;
                    }
                    if (CheckBitmapCol(src, colOffset))
                    {
                        break;
                    }
                }
            }
            //取模
            for (int page = 0; page < (((modHeight + 7) / 8)); page++)
            {
                for (int col = 0; col < modWidth; col++)
                {
                    byte temp = 0;
                    for (int row = 0; row < 8; row++)
                    {
                        temp = (byte)(temp >> 1);
                        if ((col + colOffset < src.Width) && (row + rowOffset + page * 8 < src.Height))
                        {
                            if (src.GetPixel(col + colOffset, row + rowOffset + page * 8).ToArgb() != Color.Black.ToArgb())
                            {
                                temp |= 0x80;
                            }
                        }
                    }
                    result[col + page * modWidth] = temp;
                }
            }
            return result;
        }

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
                        if (current["DeviceID"].ToString().Substring(4, 17).Equals("VID_" + str_vid + @"&PID_" + str_pid))
                        {
                            string com = current["Name"].ToString();
                            com = com.Replace(" ", ""); // 删除空格
                            com = com.Replace("USB串行设备", "");
                            com = com.Replace("(", ""); // 删除括号
                            com = com.Replace(")", ""); // 删除括号

                            PrinterInfo.com = com;
                            retval = true;
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
