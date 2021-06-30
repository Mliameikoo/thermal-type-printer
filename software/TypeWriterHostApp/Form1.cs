﻿using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;

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
        public bool use_cache_buf;              // 是否不使用richTextBox内容而使用临时缓存内容进行发送
        public byte[] cache_buf;                    // 临时缓存内容
        public byte cmd;

        public int total_send_nums;               // 本轮总共需要发送的数目
        public int now_send_nums;               // 本轮发送中当前已发送的数目
        public bool send_enable_license;    // 是否启动发送
        public bool send_ack_flag;              // 发送后是否收到应答标志
        public int ack_param;                   // 应答校验内容
        public int text_read_pos;               // 本轮发送中当前读取的位置
    };

    public partial class Form1 : Form
    {
        printer_logic printerClass = new printer_logic();
        protocol_logic protocol_Logic = new protocol_logic();
        public MessageSendInfoTypeDef sendInfo;

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

            sendInfo.use_cache_buf = false; // 默认使用richTextBox内容进行发送

            Thread Thread_SendInfoSnoop_Handle = new Thread(new ThreadStart(Thread_SendInfoSnoop));
            Thread_SendInfoSnoop_Handle.Start();

            ToolStripMenuItem_2.Checked = true; // 默认是image_text写入模式

            printerClass.PrinterInfo.mode = PrinterOperatingModeEnumDef.__ImageTextMode;

            richTextBox1.LanguageOption = RichTextBoxLanguageOptions.UIFonts;

            /*Thread Thread_ProgressBar_Handle = new Thread(new ThreadStart(Thread_ProgressBar));
            Thread_ProgressBar_Handle.Start();*/
        }

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
            catch (Exception ex)
            {
                serialPort1.Close();
                serialPort1.Dispose();
                richTextBox1.AppendText("\r\n" + ex.GetType().ToString() + "\r\n");
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
            if (!sendInfo.use_cache_buf)
            {
                sendInfo.total_send_nums = richTextBox1.Text.Length;    // 本轮总共要发送的字符个数
            }
            else
            {
                sendInfo.total_send_nums = 1; // 发送一轮即可
            }
            sendInfo.now_send_nums = 0;                             // 已发送的字符个数
            sendInfo.send_enable_license = true;                    // 开启发送
        }
        private void uartSend_Cancel()
        {
            progressBar1.Value = 0;                                 // 清空进度条
            sendInfo.use_cache_buf = false;                     // 置位
            sendInfo.text_read_pos = sendInfo.total_send_nums = 0;
            sendInfo.send_enable_license = false;            // 关闭发送
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
                    this.Invoke(new EventHandler(delegate
                    {
                        // button1.Enabled = false; // 锁定按键
                        button6.Enabled = false; // 锁定按键
                        button7.Enabled = false; // 锁定按键
                        button1.Text = "取消打印";
                        richTextBox1.Enabled = false; // 锁定写入框
                    }));

                    if (sendInfo.send_ack_flag == true)
                    {
                        sendInfo.send_ack_flag = false;
                        Bitmap map;
                        if (sendInfo.use_cache_buf)
                        {
                            Size size = printerClass.GetStrFontSize(new Font(richTextBox1.Font.FontFamily, richTextBox1.Font.Size, richTextBox1.Font.Style));
                            byte[] temporary_buf;

                            switch (sendInfo.cmd)
                            {
                                case 0x04:
                                    // 发送换行符
                                    temporary_buf = new byte[2];
                                    temporary_buf[0] = ((byte)size.Width);
                                    temporary_buf[1] = ((byte)size.Height);
                                    Uart_Send_Raw_Info(sendInfo.cmd, temporary_buf);
                                    break;

                                case 0x05:
                                    // 发送缩放值
                                    temporary_buf = new byte[2];
                                    temporary_buf[0] = ((byte)0); // 正值
                                    temporary_buf[1] = ((byte)(toolStripTextBox1.Text[0] - '0'));
                                    Uart_Send_Raw_Info(sendInfo.cmd, temporary_buf);
                                    break;

                                case 0x06:
                                    // 发送偏移值
                                    int offset = Convert.ToInt32(toolStripMenuItem1.Text);
                                    temporary_buf = new byte[2];
                                    temporary_buf[0] = ((byte)((offset < 0) ? 1 : 0)); // 正负值：1为负，0为非负
                                    temporary_buf[1] = ((byte)System.Math.Abs(offset)); // 取值范围-250~250
                                    Uart_Send_Raw_Info(sendInfo.cmd, temporary_buf);
                                    break;
                            }

                            sendInfo.text_read_pos++;

                            this.Invoke(new EventHandler(delegate
                            {
                                textBox3.Clear();
                                textBox3.Text = (size.Width.ToString() + "," + size.Height.ToString());
                            }));
                        }
                        else
                        {
                            this.Invoke(new EventHandler(delegate
                            {
                                richTextBox1.Select(sendInfo.text_read_pos, 1); // 单独对每个字符进行字体判断
                                map = printerClass.GetStrImage(richTextBox1.Text[sendInfo.text_read_pos++].ToString(), new Font(richTextBox1.SelectionFont.FontFamily, richTextBox1.SelectionFont.Size, richTextBox1.SelectionFont.Style));
                                byte[] bit_result = new byte[((map.Height + 7) / 8) * map.Width];
                                bit_result = printerClass.GetCodeTabFromBitmap_ColRowMode(map, map.Width, map.Height);
                                char text_read = richTextBox1.Text[sendInfo.text_read_pos - 1];
                                if (text_read == '\n')
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
                                    valid_buf[2] = ((byte)(bit_result.Length >> 8)); // first high-8-bits
                                    valid_buf[3] = ((byte)(bit_result.Length & 0xFF)); // second low-8-bits
                                    bit_result.CopyTo(valid_buf, 4);
                                    Uart_Send_Raw_Info(0x03, valid_buf);
                                }
                                textBox3.Clear();
                                textBox3.Text = (map.Width.ToString() + "," + map.Height.ToString() + "," + bit_result.Length.ToString());
                            }));
                        }
                        // 发送数目自增
                        sendInfo.now_send_nums++;
                        this.Invoke(new EventHandler(delegate
                        {
                            // 填充进度条
                            progressBar1.Value = sendInfo.now_send_nums * 100 / sendInfo.total_send_nums;
                        }));
                        // 结束判断
                        if (sendInfo.text_read_pos >= sendInfo.total_send_nums)
                        {
                            sendInfo.send_enable_license = false; // 发送完毕
                            sendInfo.use_cache_buf = false;       // 置位
                        }
                    }
                    if (sendInfo.send_enable_license == false)
                    {
                        this.Invoke(new EventHandler(delegate
                        {
                            // button1.Enabled = true; // 释放按键
                            button6.Enabled = true; // 释放按键
                            button7.Enabled = true; // 释放按键
                            button1.Text = "开始打印";
                            richTextBox1.Enabled = true; // 释放写入框
                        }));
                    }
                }
                else
                {
                    System.Threading.Thread.Sleep(10);
                }
            }
        }

        private void timer2_Tick(object sender, EventArgs e)
        {

        }

        private void SerialPort1_DataReceived(object sender, System.IO.Ports.SerialDataReceivedEventArgs e)
        {
            try
            {
                //因为要访问UI资源，所以需要使用invoke方式同步ui
                this.Invoke((EventHandler)(delegate
                {
                    int rx_length = serialPort1.BytesToRead;
                    byte[] rxbuf = new byte[rx_length];
                    if (rx_length == 0)
                    {
                        return;
                    }
                    serialPort1.Read(rxbuf, 0, rx_length);

                    bool retval = protocol_Logic.Rec_Protocol_Analysis(rxbuf);
                    if (retval)
                    {
                        switch (protocol_Logic.recInfo.protocal.cmd)
                        {
                            case (byte)SalveCommandEnumDef.__cmd_one_step_ack:
                                sendInfo.send_ack_flag = true;
                                break;

                            case (byte)SalveCommandEnumDef.__cmd_temper_update:
                                int temp_raw_val = protocol_Logic.recInfo.protocal.valid_data[0] * 256 + protocol_Logic.recInfo.protocal.valid_data[1];
                                // 计算出温度电阻的阻值，单位kΩ
                                double temp_res = 51 * temp_raw_val / (4096 - temp_raw_val);
                                double real_temp = -22.08 * System.Math.Log(temp_res) + 103.22;
                                toolStripLabel1.Text = "温度：" + real_temp.ToString("f0");
                                toolStripLabel1.ToolTipText = "上次更新时间：" + DateTime.Now.ToLongTimeString().ToString();

                                break;
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
                MessageBox.Show(ex.Message, "端口异常", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


        private void richTextBox1_TextChanged(object sender, EventArgs e)
        {
            /*if (printerClass.PrinterInfo.mode == PrinterOperatingModeEnumDef.__SignalWordMode)
            {
                byte[] signal_byte_buf = new byte[1];
                signal_byte_buf[0] = ((byte)richTextBox1.Text[richTextBox1.Text.Length - 1]);
                Uart_Send_Raw_Info(0x01, signal_byte_buf);
            }*/
        }

        private void richTextBox1_KeyDown(object sender, System.Windows.Forms.KeyEventArgs e)
        {
            if (e.Control && e.KeyCode == Keys.Enter)
            {
                if (printerClass.PrinterInfo.mode == PrinterOperatingModeEnumDef.__ImageTextMode)
                {
                    if (richTextBox1.Text.Length == 0)
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

        private void button2_Click(object sender, EventArgs e)
        {
            FontDialog font_choose = new FontDialog();
            font_choose.Font = richTextBox1.Font;
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

        private void button3_Click(object sender, EventArgs e)
        {
            // 加粗按钮-Bold
            if (!sendInfo.send_enable_license)
            {
                Font oldFont, newFont;
                int start_pos = richTextBox1.SelectionStart;
                int selection_length = richTextBox1.SelectionLength;
                for (int i = start_pos; i < start_pos + selection_length; i++)
                {
                    richTextBox1.Select(i, 1);
                    oldFont = richTextBox1.SelectionFont;
                    if (oldFont.Bold)
                    {
                        newFont = new Font(oldFont, oldFont.Style ^ FontStyle.Bold);
                    }
                    else
                    {
                        newFont = new Font(oldFont, oldFont.Style | FontStyle.Bold);
                    }
                    richTextBox1.SelectionFont = newFont;
                }
                richTextBox1.SelectionLength = 0;
                richTextBox1.Focus();
            }
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // 倾斜按钮-Italic
            if (!sendInfo.send_enable_license)
            {
                Font oldFont, newFont;
                int start_pos = richTextBox1.SelectionStart;
                int selection_length = richTextBox1.SelectionLength;
                for (int i = start_pos; i < start_pos + selection_length; i++)
                {
                    richTextBox1.Select(i, 1);
                    oldFont = richTextBox1.SelectionFont;
                    if (oldFont.Italic)
                    {
                        newFont = new Font(oldFont, oldFont.Style ^ FontStyle.Italic);
                    }
                    else
                    {
                        newFont = new Font(oldFont, oldFont.Style | FontStyle.Italic);
                    }
                    richTextBox1.SelectionFont = newFont;
                }
                richTextBox1.SelectionLength = 0;
                richTextBox1.Focus();
            }
        }

        private void button5_Click(object sender, EventArgs e)
        {
            // 下划线按钮-Underline
            if (!sendInfo.send_enable_license)
            {
                Font oldFont, newFont;
                int start_pos = richTextBox1.SelectionStart;
                int selection_length = richTextBox1.SelectionLength;
                for (int i = start_pos; i < start_pos + selection_length; i++)
                {
                    richTextBox1.Select(i, 1);
                    oldFont = richTextBox1.SelectionFont;
                    if (oldFont.Underline)
                    {
                        newFont = new Font(oldFont, oldFont.Style ^ FontStyle.Underline);
                    }
                    else
                    {
                        newFont = new Font(oldFont, oldFont.Style | FontStyle.Underline);
                    }
                    richTextBox1.SelectionFont = newFont;
                }
                richTextBox1.SelectionLength = 0;
                richTextBox1.Focus();
            }
        }

        private void button6_Click(object sender, EventArgs e)
        {
            // 清除内容按钮
            richTextBox1.Clear();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            // 快速换行按钮
            sendInfo.use_cache_buf = true;
            sendInfo.cmd = 0x04;
            uartSend_Get_Ready();
        }

        private void richTextBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (printerClass.PrinterInfo.mode == PrinterOperatingModeEnumDef.__SignalWordMode)
            {
                textBox3.Text = e.KeyChar.ToString() + "," + ((byte)e.KeyChar).ToString();
                if (!sendInfo.send_enable_license)
                {
                    byte[] signal_byte_buf = new byte[1];
                    signal_byte_buf[0] = ((byte)e.KeyChar);
                    Uart_Send_Raw_Info(0x01, signal_byte_buf);
                }
            }
        }

        private void ToolStripMenuItem1_1_Click(object sender, EventArgs e)
        {
            // 剪切
            if (richTextBox1.SelectedText.Length != 0)
            {
                Clipboard.SetDataObject(richTextBox1.SelectedText);
                richTextBox1.SelectedText = "";
            }
        }
        private void ToolStripMenuItem1_2_Click(object sender, EventArgs e)
        {
            // 复制
            if (richTextBox1.SelectedText.Length != 0)
            {
                Clipboard.SetDataObject(richTextBox1.SelectedText);
            }
        }
        private void ToolStripMenuItem1_3_Click(object sender, EventArgs e)
        {
            // 粘贴
            IDataObject iData = Clipboard.GetDataObject();
            if (iData.GetDataPresent(DataFormats.Text))
            {
                richTextBox1.SelectedText = (String)iData.GetData(DataFormats.Text);
            }
        }

        private void ToolStripMenuItem1_4_Click(object sender, EventArgs e)
        {
            // 字体
            FontDialog font_choose = new FontDialog();
            font_choose.Font = richTextBox1.SelectionFont;
            if (font_choose.ShowDialog() == DialogResult.OK)
            {
                System.Drawing.Font font = new System.Drawing.Font(font_choose.Font.FontFamily, font_choose.Font.Size, font_choose.Font.Style);
                richTextBox1.SelectionFont = font;
            }
        }

        private void contextMenuStrip1_Opening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            // 判断是否启用剪切、复制、粘贴、字体操作
            if (richTextBox1.SelectedText.Length == 0)
            {
                ToolStripMenuItem1_1.Enabled = false;
                ToolStripMenuItem1_2.Enabled = false;
                ToolStripMenuItem1_4.Enabled = false;
            }
            else
            {
                ToolStripMenuItem1_1.Enabled = true;
                ToolStripMenuItem1_2.Enabled = true;
                ToolStripMenuItem1_4.Enabled = true;
            }
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            // 更新偏移值按钮
            if (toolStripMenuItem1.Text.Length != 0)
            {
                int offset = Convert.ToInt32(toolStripMenuItem1.Text);
                if (System.Math.Abs(offset) < 250)
                {
                    if (!sendInfo.send_enable_license)
                    {
                        sendInfo.use_cache_buf = true;
                        sendInfo.cmd = 0x06;
                        uartSend_Get_Ready();
                        MessageBox.Show("设置成功", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    }
                    else
                    {
                        MessageBox.Show("发送正忙，请稍后重试", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }
                }
                else
                {
                    MessageBox.Show("偏移值过大，请设置在-250~250范围内", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
            else
            {
                MessageBox.Show("请输入有效值", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void toolStripMenuItem3_Click(object sender, EventArgs e)
        {
            // 更新缩放值按钮
            if (toolStripTextBox1.Text.Length != 0)
            {
                int scale = toolStripTextBox1.Text[0] - '0';
                if (scale == 1 || scale == 2)
                {
                    if (!sendInfo.send_enable_license)
                    {
                        sendInfo.use_cache_buf = true;
                        sendInfo.cmd = 0x05;
                        uartSend_Get_Ready();
                        MessageBox.Show("设置成功", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    }
                    else
                    {
                        MessageBox.Show("发送正忙，请稍后重试", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                    }
                }
                else
                {
                    MessageBox.Show("暂时只支持x1，x2倍比例放大", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            }
            else
            {
                MessageBox.Show("请输入有效值", "使用提示", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void ToolStripMenuItem_Options_4_Click(object sender, EventArgs e)
        {
            // help
            MessageBox.Show("Software Version: 1.0.0\r\n" +
                "Adapted Hardware Version: 1.0.0\r\n" +
                "Any Questions Please Contact \"jadechen_y@qq.com\"" +"\r\n\r\n"+
                "                                                                           —— To CC", "Help", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
