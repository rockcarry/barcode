+-------------------+
 barcode 条码生成库
+-------------------+

可以将字符串转换为 code128 标准的条码

接口说明
--------
int barcode_code128_bufsize(char *str);
根据字符串计算用来存放条码数据所需要的缓冲区大小

int barcode_str2code128(char *str, char *bufcode, int bufsize);
将字符串转换为条码数据
str - 为源字符串
bufcode + bufsize - 指定了用于存放转换结果的缓冲区首地址和长度
返回值为 0 代表转换成功，-1 表示失败，失败的原因是缓冲区不够大
转换后 bufcode 中存放的是一个字符串，字符串中 b 字符表示画黑色竖线，s 表示画白色竖线


编译运行
--------
ubuntu 或 msys2 环境下执行：
./build.sh
即可生成 barcode 测试程序

执行命令：
./barcode helloworld
即可生成 barcode.bmp 条码图片


chenk@apical.com.cn
2021-4-22
