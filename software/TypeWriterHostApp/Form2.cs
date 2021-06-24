using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace TypeWriterHostApp
{
    public partial class Form2 : Form
    {
        printer_logic printerClass = new printer_logic();

        public Form2()
        {
            InitializeComponent();
        }

        private void Form2_Load(object sender, EventArgs e)
        {

        }
        private void Form2_Paint(object sender, PaintEventArgs e)
        {
            Graphics g = e.Graphics;//创建画板
            g.Clear(Color.White);
            Pen curPen = new Pen(Brushes.Black, 1);


            /*g.DrawLine(curPen, 20, 20, 120, 20); //划线 ; 水平坐标形同 10,0,10,30;  y坐标不同

            g.DrawPie(curPen, 0, 0, width, height, 0, 360);*/

            /*// 绘制方格
            for (int i = 0; i < char_width + 1; i++)
            {
                g.DrawLine(curPen, i * space, 0, i * space, char_height * space);
            }
            for (int i = 0; i < char_height + 1; i++)
            {
                g.DrawLine(curPen, 0, i * space, char_width * space, i * space);
            }*/

            

            Bitmap map;
            map = printerClass.GetStrImage(richTextBox1.Text[0].ToString(), new Font(richTextBox1.SelectionFont.FontFamily, richTextBox1.SelectionFont.Size, richTextBox1.SelectionFont.Style));
            byte[] bit_result = new byte[((map.Height + 7) / 8) * map.Width];
            bit_result = printerClass.GetCodeTabFromBitmap_ColRowMode(map, map.Width, map.Height);

            textBox1.Text = map.Width + "," + map.Height + "," + bit_result.Length;

            // 绘制方格
            int space = 20;
            int char_width = map.Width;
            int char_height = map.Height;
            int pie_width = space, pie_height = space;
            for (int i = 0; i < char_width + 1; i++)
            {
                g.DrawLine(curPen, i * space, 0, i * space, char_height * space);
            }
            for (int i = 0; i < char_height + 1; i++)
            {
                g.DrawLine(curPen, 0, i * space, char_width * space, i * space);
            }
            for (int i = 0; i < char_width; i++)
            {
                for (int j = 0; j < char_height; j++)
                {
                    if(map.GetPixel(i,j).ToArgb() == Color.Black.ToArgb())
                    {
                        g.DrawPie(curPen, i*space, j*space, pie_width, pie_height, 0, 360);
                    }
                }
            }



        }




        public Bitmap Create(int[] arry)
        {
            //获得数组中最大值
            int max = 0;
            for (int i = 0; i < arry.Length; i++)
            {
                if (arry[i] > max)
                    max = arry[i];
            }
            Bitmap bitmap = new Bitmap(arry.Length, max + 10);
            Graphics g = Graphics.FromImage(bitmap);//创建Graphics对象
            g.Clear(Color.Blue);
            Pen curPen = new Pen(Brushes.Black, 1);

            // g.DrawLine(curPen, 10, 0, 10, 30); //划线 ; 水平坐标形同 10,0,10,30;  y坐标不同

            for (int i = 0; i < arry.Length; i++)
            {
                g.DrawLine(curPen, i, arry[i], i, 0); //划线 ; 水平坐标形同 10,0,10,30;  y坐标不同
            }

            return bitmap;
        }

        private void button1_Click(object sender, EventArgs e)
        {

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
    }
}
