/*
 * oled.c
 *
 *  Created on: 2021年7月22日
 *      Author: USER
 */
#include "oled.h"
#include "oledfont.h"
#include "type.h"
// the storage format is as follow
//[0]0 1 2 3 ... 127
//[1]0 1 2 3 ... 127
//[2]0 1 2 3 ... 127
//[3]0 1 2 3 ... 127
//[4]0 1 2 3 ... 127
//[5]0 1 2 3 ... 127
//[6]0 1 2 3 ... 127
//[7]0 1 2 3 ... 127
/**********************************************
//IIC Start
**********************************************/
void IIC_Start()
{

    OLED_SCLK_Set();
    OLED_SDIN_Set();
    OLED_SDIN_Clr();
    OLED_SCLK_Clr();
}

/**********************************************
//IIC Stop
**********************************************/
void IIC_Stop()
{
OLED_SCLK_Set() ;
//  OLED_SCLK_Clr();
    OLED_SDIN_Clr();
    OLED_SDIN_Set();

}

void IIC_Wait_Ack()
{
    OLED_SCLK_Set() ;
    OLED_SCLK_Clr();
}
/**********************************************
// IIC Write byte
**********************************************/
void Write_IIC_Byte(unsigned char IIC_Byte)
{
    unsigned char i;
    unsigned char m,da;
    da=IIC_Byte;
    OLED_SCLK_Clr();
    for(i=0;i<8;i++)
    {
            m=da;
        //  OLED_SCLK_Clr();
        m=m&0x80;
        if(m==0x80)
        {OLED_SDIN_Set();}
        else OLED_SDIN_Clr();
            da=da<<1;
        OLED_SCLK_Set();
        OLED_SCLK_Clr();
        }


}
/**********************************************
// IIC Write Command
**********************************************/
void Write_IIC_Command(unsigned char IIC_Command)
{
   IIC_Start();
   Write_IIC_Byte(0x78);            //Slave address,SA0=0
    IIC_Wait_Ack();
   Write_IIC_Byte(0x00);            //write command
    IIC_Wait_Ack();
   Write_IIC_Byte(IIC_Command);
    IIC_Wait_Ack();
   IIC_Stop();
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
   IIC_Start();
   Write_IIC_Byte(0x78);            //D/C#=0; R/W#=0
    IIC_Wait_Ack();
   Write_IIC_Byte(0x40);            //write data
    IIC_Wait_Ack();
   Write_IIC_Byte(IIC_Data);
    IIC_Wait_Ack();
   IIC_Stop();
}
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
    if(cmd)
            {

   Write_IIC_Data(dat);

        }
    else {
   Write_IIC_Command(dat);

    }


}


/********************************************
// fill_Picture
********************************************/
void fill_picture(unsigned char fill_Data)
{
    unsigned char m,n;
    for(m=0;m<8;m++)
    {
        OLED_WR_Byte(0xb0+m,0);     //page0-page1
        OLED_WR_Byte(0x00,0);       //low column start address
        OLED_WR_Byte(0x10,0);       //high column start address
        for(n=0;n<128;n++)
            {
                OLED_WR_Byte(fill_Data,1);
            }
    }
}

void Delay_1ms(unsigned int Del_1ms)
{
    unsigned char j;
    while(Del_1ms--)
    {
        for(j=0;j<123;j++);
    }
}


void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
    OLED_WR_Byte((x&0x0f),OLED_CMD);
}

void OLED_Display_On(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);
    OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
    OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}

void OLED_Display_Off(void)
{
    OLED_WR_Byte(0X8D,OLED_CMD);
    OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
    OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}

void OLED_Clear(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);
        OLED_WR_Byte (0x00,OLED_CMD);
        OLED_WR_Byte (0x10,OLED_CMD);
        for(n=0;n<128;n++)OLED_WR_Byte(0,OLED_DATA);
    }
}
void OLED_On(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);
        OLED_WR_Byte (0x00,OLED_CMD);
        OLED_WR_Byte (0x10,OLED_CMD);
        for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA);
    }
}

//x:0~127
//y:0~63
//size:16/12
//void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size)
//{
//    unsigned char c=0,i=0;
//    c=chr-' ';
//    if(x>Max_Column-1){x=0;y=y+2;}
//    if(Char_Size ==16)
//        {
//        OLED_Set_Pos(x,y);
//        for(i=0;i<8;i++)
//        OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
//        OLED_Set_Pos(x,y+1);
//        for(i=0;i<8;i++)
//        OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
//        }
//        else {
//            OLED_Set_Pos(x,y);
//            for(i=0;i<6;i++)
//            OLED_WR_Byte(F6x8[c][i],OLED_DATA);
//        }
//}

u32 oled_pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}

//void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2)
//{
//    u8 t,temp;
//    u8 enshow=0;
//    for(t=0;t<len;t++)
//    {
//        temp=(num/oled_pow(10,len-t-1))%10;
//        if(enshow==0&&t<(len-1))
//        {
//            if(temp==0)
//            {
//                OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
//                continue;
//            }else enshow=1;
//
//        }
//        OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2);
//    }
//}
//void OLED_ShowVI(u8 x,u8 y,u32 num,u8 size2)
//{
//    OLED_ShowNum(x+size2*4-1,y,num%10,1,size2);
//    OLED_ShowNum(x+size2*3-1,y,num/10%10,1,size2);
//    OLED_ShowNum(x+size2*2-1,y,num/100%10,1,size2);
//    OLED_ShowString(x+size2*1-1,y,".",size2);
//    OLED_ShowNum(x-1,y,num/1000,1,size2);
//}
//
//void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size)
//{
//    unsigned char j=0;
//    while (chr[j]!='\0')
//    {       OLED_ShowChar(x,y,chr[j],Char_Size);
//            x+=8;
//        if(x>120){x=0;y+=2;}
//            j++;
//    }
//}
//void OLED_ShowCHinese(u8 x,u8 y,u8 no)
//{
//    u8 t,adder=0;
//    OLED_Set_Pos(x,y);
//    for(t=0;t<16;t++)
//        {
//                OLED_WR_Byte(LHR[2*no][t],OLED_DATA);
//                adder+=1;
//     }
//        OLED_Set_Pos(x,y+1);
//    for(t=0;t<16;t++)
//            {
//                OLED_WR_Byte(LHR[2*no+1][t],OLED_DATA);
//                adder+=1;
//      }
//}
//void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
//{
// unsigned int j=0;
// unsigned char x,y;
//
//  if(y1%8==0) y=y1/8;
//  else y=y1/8+1;
//    for(y=y0;y<y1;y++)
//    {
//        OLED_Set_Pos(x0,y);
//    for(x=x0;x<x1;x++)
//        {
//            OLED_WR_Byte(BMP[j++],OLED_DATA);
//        }
//    }
//}
void OLED_Init(void)
{
    P3DIR |= BIT5+BIT6;
    Delay_1ms(200);
    OLED_WR_Byte(0xAE,OLED_CMD);//--display off
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address
    OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
    OLED_WR_Byte(0x81,OLED_CMD); // contract control
    OLED_WR_Byte(0xFF,OLED_CMD);//--128
    OLED_WR_Byte(0xA1,OLED_CMD);//set segment remap
    OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty
    OLED_WR_Byte(0xC8,OLED_CMD);//Com scan direction
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
    OLED_WR_Byte(0x00,OLED_CMD);//

    OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
    OLED_WR_Byte(0x80,OLED_CMD);//

    OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
    OLED_WR_Byte(0x05,OLED_CMD);//

    OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
    OLED_WR_Byte(0xF1,OLED_CMD);//

    OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
    OLED_WR_Byte(0x12,OLED_CMD);//

    OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
    OLED_WR_Byte(0x30,OLED_CMD);//

    OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable
    OLED_WR_Byte(0x14,OLED_CMD);//

    OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel
}


/*设置起始地址
*Y轴是按8格递进的，y轴0~63，只能按8格的倍数显示，
*因为列行式只能按8个字节进行
*/
void LCD_Set_Pos(u8 x, u8 y)
{
    Write_IIC_Command(0xb0+(y>>3));
    Write_IIC_Command(((x&0xf0)>>4)|0x10);
    Write_IIC_Command((x&0x0f)|0x01);
}


//==============================================================
//函数名：LCD_P6x8Str(u8 x,u8 y,u8 *p)
//功能描述：写入一组标准ASCII字符串
//参数：显示的位置（x,y），y为页范围0～7，要显示的字符串
//返回：无
//==============================================================
void LCD_P6x8Str(u8 x,u8 y,u8 *ch,const u8 *F6x8)
{
    u8 c=0,i=0,j=0;

    while (*(ch+j)!='\0')
    {
        c =*(ch+j)-32;
        if(x>126)
        {
            x=0;
            y++;
        }
        LCD_Set_Pos(x,y);
        for(i=0;i<6;i++)
        {
            Write_IIC_Data(*(F6x8+c*6+i));
        }
        x+=6;
        j++;
    }
}
//==============================================================
//函数名：LCD_P8x16Str(u8 x,u8 y,u8 *p)
//功能描述：写入一组标准ASCII字符串
//参数：显示的位置（x,y），y为页范围0～63，要显示的字符串
//返回：无
//==============================================================
void LCD_P8x16Str(u8 x,u8 y,u8 *ch,const u8 *F8x16)
{
  u8 c=0,i=0,j=0;

  while (*(ch+j)!='\0')
  {
    c =*(ch+j)-32;
    if(x>120)
    {
        x=0;
        y++;
    }
    LCD_Set_Pos(x,y);
    for(i=0;i<8;i++)
    {
        Write_IIC_Data(*(F8x16+c*16+i));
    }
    LCD_Set_Pos(x,y+8);
    for(i=0;i<8;i++)
    {
        Write_IIC_Data(*(F8x16+c*16+i+8));
    }
    x+=8;
    j++;
  }
}
//输出汉字字符串
void LCD_P14x16Str(u8 x,u8 y,u8 ch[],const u8 *F14x16_Idx,const u8 *F14x16)
{
    u8 wm=0,ii = 0;
    u16 adder=1;

    while(ch[ii] != '\0')
    {
        wm = 0;
        adder = 1;
        while(*(F14x16_Idx+wm) > 127)
        {
            if(*(F14x16_Idx+wm) == ch[ii])
            {
                if(*(F14x16_Idx+wm+1) == ch[ii + 1])
                {
                    adder = wm * 14;
                    break;
                }
            }
            wm += 2;
        }
        if(x>118)
        {
            x=0;
            y++;
        }
        LCD_Set_Pos(x , y);
        if(adder != 1)// 显示汉字
        {
            LCD_Set_Pos(x , y);
            for(wm = 0;wm < 14;wm++)
            {
                Write_IIC_Data(*(F14x16+adder));
                adder += 1;
            }
            LCD_Set_Pos(x,y + 1);
            for(wm = 0;wm < 14;wm++)
            {
                Write_IIC_Data(*(F14x16+adder));
                adder += 1;
            }
        }
        else              //显示空白字符
        {
            ii += 1;
            LCD_Set_Pos(x,y);
            for(wm = 0;wm < 16;wm++)
            {
                Write_IIC_Data(0);
            }
            LCD_Set_Pos(x,y + 1);
            for(wm = 0;wm < 16;wm++)
            {
                Write_IIC_Data(0);
            }
        }
        x += 14;
        ii += 2;
    }
}
//输出汉字字符串
void LCD_P16x16Str(u8 x,u8 y,u8 *ch,const u8 *F16x16_Idx,const u8 *F16x16)
{
    u8 wm=0,ii = 0;
    u16 adder=1;

    while(*(ch+ii) != '\0')
    {
    wm = 0;
    adder = 1;
    while(*(F16x16_Idx+wm) > 127)
    {
        if(*(F16x16_Idx+wm) == *(ch+ii))
        {
            if(*(F16x16_Idx+wm + 1) == *(ch+ii + 1))
            {
                adder = wm * 16;
                break;
            }
        }
        wm += 2;
    }
    if(x>118){x=0;y++;}
    LCD_Set_Pos(x , y);
    if(adder != 1)// 显示汉字
    {
        LCD_Set_Pos(x , y);
        for(wm = 0;wm < 16;wm++)
        {
            Write_IIC_Data(*(F16x16+adder));
            adder += 1;
        }
        LCD_Set_Pos(x,y + 8);
        for(wm = 0;wm < 16;wm++)
        {
            Write_IIC_Data(*(F16x16+adder));
            adder += 1;
        }
    }
    else              //显示空白字符
    {
        ii += 1;
        LCD_Set_Pos(x,y);
        for(wm = 0;wm < 16;wm++)
        {
            Write_IIC_Data(0);
        }
        LCD_Set_Pos(x,y + 1);
        for(wm = 0;wm < 16;wm++)
        {
            Write_IIC_Data(0);
        }
    }
    x += 16;
    ii += 2;
    }
}


/*输出汉字和字符混合字符串
*Y轴是按8格递进的，y轴0~63，只能按8格的倍数显示，
*因为列行式只能按8个字节进行
*
*/

void OLED_Print(u8 x, u8 y, u8 *ch,u8 char_size, u8 ascii_size)
{
    u8 ch2[3];
    u8 ii=0;
    while(*(ch+ii) != '\0')
    {
        if(*(ch+ii) > 127)//大于127为中文，小于等于127为ASCII
        {
            ch2[0] = *(ch+ii);
            ch2[1] = *(ch+ii+1);
            ch2[2] = '\0';          //汉字为两个字节
            LCD_P16x16Str(x , y, ch2,F16x16_Idx,F16x16);    //显示汉字
            x += 16;
            ii += 2;
        }
        else
        {
            ch2[0] = *(ch+ii);
            ch2[1] = '\0';          //字母占一个字节
            if(TYPE8X16==ascii_size)
            {
                LCD_P8x16Str(x , y ,ch2,F8X16); //显示字母
                x += 8;
            }
            else if(TYPE6X8==ascii_size)
            {
                LCD_P6x8Str(x , y ,ch2,F6x8);   //显示字母
                x += 6;
            }

            ii+= 1;
        }
    }
}


//————————————————
//版权声明：本文为CSDN博主「会动的栗子」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/lhooer/article/details/116278562

