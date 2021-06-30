using Microsoft.VisualBasic;
using Microsoft.VisualBasic.CompilerServices;
using System;
using System.Drawing;
using System.Management;
using System.Windows.Forms;

namespace TypeWriterHostApp
{
    public class printer_logic
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

        public Size GetStrFontSize(Font font)
        {
            string hz = "1";
            var flags = TextFormatFlags.WordBreak | TextFormatFlags.NoPadding;
            Size size = System.Windows.Forms.TextRenderer.MeasureText(hz, font, new Size(0, 0), flags);
            return size;
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
            var flags = TextFormatFlags.WordBreak | TextFormatFlags.NoPadding;
            Size size = System.Windows.Forms.TextRenderer.MeasureText(hz, font, new Size(0, 0), flags);

            // 重设位图大小
            Bitmap bmp = new Bitmap(Convert.ToInt32(size.Width), Convert.ToInt32(size.Height));
            Graphics g = Graphics.FromImage(bmp);

            StringFormat format = new StringFormat
            {
                /*Alignment = StringAlignment.Center,
                LineAlignment = StringAlignment.Center*/
            };
            Rectangle rectangle = new Rectangle(0, 0, Convert.ToInt32(size.Width), Convert.ToInt32(size.Height));
            g.Clear(Color.Black); // 背景色黑色
            g.DrawString(hz, font, new SolidBrush(Color.White), rectangle, format);

            // 截取左右两边的多余部分
            int horizon_start_pos = 0;
            int horizon_end_pos = bmp.Width - 1;
            // 寻找出左右边界
            for (int i = 0; i < bmp.Width; i++)
            {
                bool find_valid_pixel = false;
                for (int j = 0; j < bmp.Height; j++)
                {
                    if (bmp.GetPixel(i, j).ToArgb() != Color.Black.ToArgb())
                    {
                        find_valid_pixel = true;
                        break;
                    }
                }
                if (find_valid_pixel)
                {
                    /*horizon_start_pos = i;*/
                    horizon_end_pos = i;
                }
            }
            for (int i = bmp.Width - 1; i >= 0; i--)
            {
                bool find_valid_pixel = false;
                for (int j = 0; j < bmp.Height; j++)
                {
                    if (bmp.GetPixel(i, j).ToArgb() != Color.Black.ToArgb())
                    {
                        find_valid_pixel = true;
                        break;
                    }
                }
                if (find_valid_pixel)
                {
                    /*horizon_end_pos = i;*/
                    horizon_start_pos = i;
                }
            }

            // 左右默认各留出一列空白
            int left_offset = 1, right_offset = 1;
            if (horizon_start_pos == 0)
            {
                left_offset = 0;
            }
            if (horizon_end_pos == bmp.Width - 1)
            {
                right_offset = 0;
            }
            Bitmap new_bmp = new Bitmap(horizon_end_pos - horizon_start_pos + 1 + left_offset + right_offset, bmp.Height);
            // 截取出对应区域
            Graphics g2 = Graphics.FromImage(new_bmp);
            Rectangle origReg = new Rectangle(horizon_start_pos - left_offset, 0, horizon_end_pos - horizon_start_pos + 1 + left_offset + right_offset, bmp.Height);
            Rectangle destReg = new Rectangle(0, 0, new_bmp.Width, new_bmp.Height);
            g2.DrawImage(bmp, destReg, origReg, GraphicsUnit.Pixel);

            return new_bmp;
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
