## Host Computer Communication Protocol 
> version 1.0
> date: 2021/06/15

#### 1. Format  
|frame_head|valid_length|command|valid_data|sum_check|frame_tail|
|:-:|:-:|:-:|:-:|:-:|:-:|
|0xF5|x1|x2|x3[...]|x4|0x5F|  
#### 2. Valid Length - x1  
The length of valid_data.  
#### 3. Command List - x2  
|command|x1|
|:-:|:-:|
|single_word_write|0x01|
|paragraph_write|0x02|  
#### 4. Valid Data - x3[...]  
Should limit as the APP_RX_DATA_SIZE.  
#### 5. Sum Check - x4  
One byte sum of (command + valid_data).