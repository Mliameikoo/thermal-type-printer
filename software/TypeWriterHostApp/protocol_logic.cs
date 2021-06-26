using System.Linq;

namespace TypeWriterHostApp
{
    public enum SalveCommandEnumDef
    {
        __cmd_one_step_ack = 0x80,
        __cmd_temper_update = 0x81,
    };
    public enum MessageReceiveInfoEnumDef
    {
        __rx_state_idle = 0,
        __rx_state_head_ok,
        __rx_state_length_high8bit_ok,
        __rx_state_length_low8bit_ok,
        __rx_state_cmd_ok,
        __rx_state_data_ok,
        __rx_state_sum_check_ok /* finish */
    };
    public struct ProtocalFrameDef
    {
        public byte head;
        public int length;
        public byte cmd;
        public byte[] valid_data;
        public byte sum_check;
        public byte tail;
    };
    public struct MessageReceiveInfoTypeDef
    {
        public MessageReceiveInfoEnumDef state;
        public ProtocalFrameDef protocal;
    };

    class protocol_logic
    {
        public MessageReceiveInfoTypeDef recInfo;
        private ProtocalFrameDef protocal = new ProtocalFrameDef
        {
            head = 0xF5,
            tail = 0x5F
        };
        private int length = 0;
        private byte cmd = 0;

        /**
          * @brief  接收帧解析
          * @retval 返回true表示解析成功，否则返回false
          */
        public bool Rec_Protocol_Analysis(byte[] rxbuf)
        {
            int read_pos = 0;
            bool retval = false;

            while (read_pos < rxbuf.Length)
            {
                switch (recInfo.state)
                {
                    case MessageReceiveInfoEnumDef.__rx_state_idle:
                        if (rxbuf[read_pos] == protocal.head)
                        {
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_head_ok;
                            recInfo.protocal.valid_data = new byte[200];
                            recInfo.protocal.length = 0;
                        }
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_head_ok:
                        length = rxbuf[read_pos] * 256;
                        recInfo.state = MessageReceiveInfoEnumDef.__rx_state_length_high8bit_ok;
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_length_high8bit_ok:
                        length += rxbuf[read_pos];
                        recInfo.state = MessageReceiveInfoEnumDef.__rx_state_length_low8bit_ok;
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_length_low8bit_ok:
                        cmd = ((byte)rxbuf[read_pos]);
                        recInfo.state = MessageReceiveInfoEnumDef.__rx_state_cmd_ok;
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_cmd_ok:
                        if (recInfo.protocal.length < length)
                        {
                            recInfo.protocal.valid_data[recInfo.protocal.length ++] = (rxbuf[read_pos]);
                        }
                        else
                        {
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_data_ok;
                            continue;
                        }
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_data_ok:
                        byte sum_check = cmd;
                        for (int i = 0; i < recInfo.protocal.valid_data.Length; i++)
                        {
                            sum_check += ((byte)recInfo.protocal.valid_data[i]);
                        }
                        if (rxbuf[read_pos] == sum_check)
                        {
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_sum_check_ok;
                        }
                        else
                        {
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_idle;
                            continue;
                        }
                        break;

                    case MessageReceiveInfoEnumDef.__rx_state_sum_check_ok:
                        if (rxbuf[read_pos] == protocal.tail)
                        {
                            // finish
                            recInfo.protocal.cmd = cmd;
                            recInfo.protocal.length = recInfo.protocal.valid_data.Length;
                            retval = true;
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_idle;
                        }
                        else
                        {
                            recInfo.state = MessageReceiveInfoEnumDef.__rx_state_idle;
                            continue;
                        }
                        break;

                    default:
                        recInfo.state = MessageReceiveInfoEnumDef.__rx_state_idle;
                        break;
                }
                read_pos++;
            }
            return retval;
        }



    }
}
