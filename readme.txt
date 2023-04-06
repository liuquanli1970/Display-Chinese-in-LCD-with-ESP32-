如果想在ESP32的SPIFFS中加载汉字库，需要做下面的准备工作


1、SPIFFS至少需要1M字节左右的空间
2、下载三个文件到SPIFFS： HZK16M、HZK24M、U2offset.bin(这三个文件位于显示库目录下 HZK子目录里)
  （可以先用乐鑫提供的spiffsgen.py程序生成SPIFFS映像文件，通过烧录工具flash_download_tool_3.9.4 烧录进SPIFFS，
    也可以通过Arduino的插件“ESP32 Sketch Data Upload”下载进SPIFFS）
3、下面几个函数就可以使用了：
1） void printStr16(String dstr, int x, int y, uint16_t c);   //显示十六点阵汉字和英文
	参数：	sdtr 被显示的字符串（可以中英文混合）
			x,y  显示的坐标
			c    显示的文字颜色
2）	void printStr24(String dstr, int x, int y, uint16_t c);   //显示二十四点阵汉字和英文
	参数：	sdtr 被显示的字符串（可以中英文混合）
			x,y  显示的坐标
			c    显示的文字颜色	
3）	void msgbox16(String msg, int boxtype = 1, int waittime = 1);//显示16点阵对话框
	参数：	msg  被显示的信息框内容（可以中英文混合），可以分为三行显示，‘/’字符用于分行
			boxtype  信息框类型 1：提示 2：信息 3：警告 4：错误 5：Tips 6：Info 7：Alert 8：Error
			waittime	信息框等待时间 单位ms    
4）	void msgbox24(String msg, int boxtype = 1, int waittime = 1);//显示24点阵对话框
	参数：	msg  被显示的信息框内容（可以中英文混合），可以分为两行显示，‘/’字符用于分行
			boxtype  信息框类型 1：提示 2：信息 3：警告 4：错误 5：Tips 6：Info 7：Alert 8：Error
			waittime	信息框等待时间 单位ms  
5） void setBoxTextColor(uint16_t fColor = TFT_WHITE); //设置对话框文字颜色

举例： 
	#include <HZK_ILI9341.h>

	TFTLCD LCD;

	void setup(){
		LCD.begin();
	}

	void loop{
		LCD.fillScreen(TFT_BLACK);
		LCD.printStr16("显示十六点阵信息框msgbox16.", 0, 16, TFT_YELLOW);
		delay(3000);
		LCD.msgbox16("中英文混合Chinese & English/只支持UTF-8编码/其他编码会 no good", 1, 3000);
		LCD.fillScreen(TFT_BLACK);
		LCD.setBoxTextColor(TFT_WHITE);
		LCD.printStr24("二十四点阵信息框msgbox24.", 0, 16, TFT_RED);
		delay(3000);
		LCD.setBoxTextColor(TFT_PINK);
		LCD.msgbox24("中英混合CHN&EN/支持UTF-8编码", 2, 3000);
	}