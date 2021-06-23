## Host Computer Communication Protocol 
> version 1.0
> date: 2021/06/15

#### 1. Format  
|frame_head|valid_length_high8bits|valid_length_low8bits|command|valid_data|sum_check|frame_tail|
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
|0xF5|x1|x2|x3|x4[...]|x3|0x5F|  
#### 2. Valid Length High-8bits - x1  
The High-8bits of length of valid_data.  
#### 3. Valid Length Low-8bits - x2  
The Low-8bits of length of valid_data.  
#### 3. Command List - x3  
|command|x1|
|:-:|:-:|
|single_word_write|0x01|
|paragraph_write|0x02|  
|image_write|0x03|  
|special_order|0x04|  
#### 4. Valid Data - x4[...]  
Should limit as the APP_RX_DATA_SIZE.  
#### 5. Sum Check - x5  
One byte sum of (command + valid_data).