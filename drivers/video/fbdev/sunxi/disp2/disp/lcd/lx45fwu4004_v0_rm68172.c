#include "panels.h"
//#include "../../../../../../power/supply/axp/axp20x/axp20x-gpio.h"
extern int axp20x_gpio_set_value(int gpio, int value);
#define rm68172_spi_scl_1	sunxi_lcd_gpio_set_value(0,1,1)
#define rm68172_spi_scl_0	sunxi_lcd_gpio_set_value(0,1,0)
#define rm68172_spi_sdi_1	sunxi_lcd_gpio_set_value(0,2,1)
#define rm68172_spi_sdi_0	sunxi_lcd_gpio_set_value(0,2,0)
#define rm68172_spi_cs_1  	//sunxi_lcd_gpio_set_value(0,3,1)
#define rm68172_spi_cs_0  	//sunxi_lcd_gpio_set_value(0,3,0)

#define rm68172_spi_reset_1  	//sunxi_lcd_gpio_set_value(0,0,1)
#define rm68172_spi_reset_0  	//sunxi_lcd_gpio_set_value(0,0,0)

#define DBG_INFO(format, args...) (printk("[LX45FWU4004 LCD INFO] LINE:%04d-->%s:"format, __LINE__, __func__, ##args))
#define DBG_ERR(format, args...) (printk("[LX45FWU4004 LCD ERR] LINE:%04d-->%s:"format, __LINE__, __func__, ##args))

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(struct panel_extend_para * info)
{
	u32 i = 0, j=0;
	u32 items;
	u8 lcd_gamma_tbl[][2] =
	{
		//{input value, corrected value}
		{0, 0},
		{15, 15},
		{30, 30},
		{45, 45},
		{60, 60},
		{75, 75},
		{90, 90},
		{105, 105},
		{120, 120},
		{135, 135},
		{150, 150},
		{165, 165},
		{180, 180},
		{195, 195},
		{210, 210},
		{225, 225},
		{240, 240},
		{255, 255},
	};

	u32 lcd_cmap_tbl[2][3][4] = {
	{
		{LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
		{LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
		{LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
		},
		{
		{LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
		{LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
		{LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
		},
	};

	items = sizeof(lcd_gamma_tbl)/2;
	for(i=0; i<items-1; i++) {
		u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

		for(j=0; j<num; j++) {
			u32 value = 0;

			value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
			info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
		}
	}
	info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

	memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
	DBG_INFO("\n");
	LCD_OPEN_FUNC(sel, LCD_power_on, 50);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 120);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 50);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	DBG_INFO("\n");
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 50);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	5);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 0);   //close lcd power, and delay 500ms
//	printk("turn off lcd power!!!\n");
	return 0;
}

static void LCD_power_on(u32 sel)
{
	DBG_INFO("\n");
	sunxi_lcd_power_enable(sel, 0); //config lcd_power pin to open lcd power0
	sunxi_lcd_pin_cfg(sel, 1);
	rm68172_spi_reset_0;
	sunxi_lcd_delay_ms(30);
}

static void LCD_power_off(u32 sel)
{
	DBG_INFO("\n");
	sunxi_lcd_pin_cfg(sel, 0);
	//sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
}

static void LCD_bl_open(u32 sel)
{
	DBG_INFO("\n");
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
	//axp_gpio_set_value(3, 1);
    axp20x_gpio_set_value(3, 1);
}

static void LCD_bl_close(u32 sel)
{
	DBG_INFO("\n");
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
	//axp_gpio_set_value(3, 0);
    axp20x_gpio_set_value(3, 0);
}
static void SPI_writedat(u32 value)
{
	u32 i;

	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(1); // 1
	rm68172_spi_cs_0;
	rm68172_spi_sdi_1;
	rm68172_spi_scl_0;
	//sunxi_lcd_delay_us(10);
	sunxi_lcd_delay_us(1); //1
	//rm68172_spi_scl_1;
	//sunxi_lcd_delay_us(10);
	//for(i=0;i<8;i++)
	for(i=0;i<16;i++)
	{
		sunxi_lcd_delay_us(10);
		rm68172_spi_scl_0;
		if(value & 0x8000)
			rm68172_spi_sdi_1;
		else
			rm68172_spi_sdi_0;
		//rm68172_spi_scl_0;
		//sunxi_lcd_delay_us(10);
		sunxi_lcd_delay_us(3); //
		rm68172_spi_scl_1;
		value <<= 1;
	}
	//sunxi_lcd_delay_us(10);
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(5); // 5
}

static void SPI_writecom(u32 value)
{
	u32 i;
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(1);  // 1
	rm68172_spi_cs_0;
	rm68172_spi_sdi_1;
	rm68172_spi_scl_0;
	//sunxi_lcd_delay_us(10);
	sunxi_lcd_delay_us(1);  //1
	//rm68172_spi_scl_1; 
	//sunxi_lcd_delay_us(10);
	//for(i=0;i<8;i++)
	for(i=0;i<16;i++)
	{
		sunxi_lcd_delay_us(10);
		rm68172_spi_scl_0;
		if(value & 0x8000)
			rm68172_spi_sdi_1;
		else
			rm68172_spi_sdi_0;
		//rm68172_spi_scl_0; 
		//sunxi_lcd_delay_us(10);
		sunxi_lcd_delay_us(3); // 3
		rm68172_spi_scl_1;
		value <<= 1;
		
	}
	//sunxi_lcd_delay_us(10);
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(5); // 5
}

/*
static void SPI_writedat(u32 value)
{
	u32 i;
	rm68172_spi_cs_0;
	rm68172_spi_sdi_1;
	rm68172_spi_scl_0;
	sunxi_lcd_delay_us(10);
	rm68172_spi_scl_1;
	//sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		//rm68172_spi_scl_0;
		if(value & 0x80)
			rm68172_spi_sdi_1;
		else
			rm68172_spi_sdi_0;
		value <<= 1;
		rm68172_spi_scl_0;
		sunxi_lcd_delay_us(10);
		rm68172_spi_scl_1;
	}
	sunxi_lcd_delay_us(10);
	rm68172_spi_cs_1;
}
static void SPI_writecom(u32 value)
{
	u32 i;
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(1);
	rm68172_spi_cs_0;
	rm68172_spi_sdi_1;
	rm68172_spi_scl_0;
	//sunxi_lcd_delay_us(10);
	sunxi_lcd_delay_us(1);
	//rm68172_spi_scl_1; 
	//sunxi_lcd_delay_us(10);
	//for(i=0;i<8;i++)
	for(i=0;i<16;i++)
	{
		sunxi_lcd_delay_us(10);
		rm68172_spi_scl_0;
		if(value & 0x8000)
			rm68172_spi_sdi_1;
		else
			rm68172_spi_sdi_0;
		//rm68172_spi_scl_0; 
		//sunxi_lcd_delay_us(10);
		sunxi_lcd_delay_us(3);
		rm68172_spi_scl_1;
		value <<= 1;
		
	}
	//sunxi_lcd_delay_us(10);
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(5);
}
*/
/*
static void SPI_writecom(u32 value)
{
	u32 i;
	rm68172_spi_cs_1;
	sunxi_lcd_delay_us(1);
	rm68172_spi_cs_0;
	rm68172_spi_sdi_1;
	rm68172_spi_scl_0;
	//sunxi_lcd_delay_us(10);
	sunxi_lcd_delay_us(1);
	//rm68172_spi_scl_1; 
	//sunxi_lcd_delay_us(10);
	//for(i=0;i<8;i++)

	for(i=0;i<16;i++)
	{
		sunxi_lcd_delay_us(10);
		//rm68172_spi_scl_0;
		if(value & 0x8000)
			rm68172_spi_sdi_1;
		else
			rm68172_spi_sdi_0;
		
		rm68172_spi_scl_0; 
		sunxi_lcd_delay_us(10);
		rm68172_spi_scl_1;
		value <<= 1;
		
	}
	sunxi_lcd_delay_us(10);
	rm68172_spi_cs_1;
}*/

#define write_spicom SPI_writecom
#define	write_spidat SPI_writedat

static void LCD_panel_init(u32 sel)
{
	static unsigned int vcom = 0x4023;
	printk("####rm68172 %s,vcom is %4x\n", __func__, vcom);
	//rm68172_spi_reset_1;
	DBG_INFO("\n");
	//sunxi_lcd_delay_ms(10);
	rm68172_spi_reset_0;
	sunxi_lcd_delay_ms(10);
	rm68172_spi_reset_1;
	sunxi_lcd_delay_ms(10);
	DBG_INFO("reset finish\n");
#if 0
	/**************************************************
	IC Name: RM68172
	Panel Maker/Size: IVO498
	Panel Product No.: 
	Version: C050SWY8-1
	Date: V0_140930
	**************************************************/
	SPI_writecom(0x2001); SPI_writecom(0x0000);SPI_writedat(0x4000);

	SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
	SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
	SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
	SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
	SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4002);
	                  
	SPI_writecom(0x20F6);SPI_writecom(0x0000); SPI_writedat(0x4060);
	SPI_writecom(0x20F6);SPI_writecom(0x0001); SPI_writedat(0x4040);
	          
	SPI_writecom(0x20FE);SPI_writecom(0x0000); SPI_writedat(0x4001);
	SPI_writecom(0x20FE);SPI_writecom(0x0001); SPI_writedat(0x4080);
	SPI_writecom(0x20FE);SPI_writecom(0x0002); SPI_writedat(0x4009);
	//SPI_writecom(0x20FE);SPI_writecom(0x0003); SPI_writedat(0x4009);
		SPI_writecom(0x20FE);SPI_writecom(0x0003); SPI_writedat(0x4018);
			SPI_writecom(0x20FE);SPI_writecom(0x0004); SPI_writedat(0x40a0);
	  
	//sunxi_lcd_delay_ms(200);   
	           
	SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
	SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
	SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
	SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
	SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4001);
	          
	SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x400A);
	           
	SPI_writecom(0x20B1);SPI_writecom(0x0000); SPI_writedat(0x400A);
	             
	SPI_writecom(0x20B5);SPI_writecom(0x0000); SPI_writedat(0x4008);
	                 
	SPI_writecom(0x20B6);SPI_writecom(0x0000); SPI_writedat(0x4054);//54
	                  
	SPI_writecom(0x20B7);SPI_writecom(0x0000); SPI_writedat(0x4044);//44
	                    
	SPI_writecom(0x20B8);SPI_writecom(0x0000); SPI_writedat(0x4024);
	                  
	SPI_writecom(0x20B9);SPI_writecom(0x0000); SPI_writedat(0x4034);
	                      
	SPI_writecom(0x20BA);SPI_writecom(0x0000); SPI_writedat(0x4014);
	            
	SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20BC);SPI_writecom(0x0001); SPI_writedat(0x4098);//A8
	SPI_writecom(0x20BC);SPI_writecom(0x0002); SPI_writedat(0x4013);
	               
	SPI_writecom(0x20BD);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20BD);SPI_writecom(0x0001); SPI_writedat(0x4090);
	SPI_writecom(0x20BD);SPI_writecom(0x0002); SPI_writedat(0x401A);
	                  
	SPI_writecom(0x20BE);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20BE);SPI_writecom(0x0001); SPI_writedat(0x401A);//20
	               
	SPI_writecom(0x20D1);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D1);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D1);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D1);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D1);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D1);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D1);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D1);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D1);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D1);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D1);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D1);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D1);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D1);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D1);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D1);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D1);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D1);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D1);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D1);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D1);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D1);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D1);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D1);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D1);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D1);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D1);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D1);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D1);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D1);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D1);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D1);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D1);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D1);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D1);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D1);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D1);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                       
	SPI_writecom(0x20D2);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D2);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D2);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D2);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D2);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D2);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D2);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D2);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D2);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D2);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D2);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D2);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D2);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D2);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D2);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D2);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D2);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D2);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D2);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D2);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D2);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D2);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D2);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D2);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D2);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D2);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D2);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D2);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D2);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D2);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D2);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D2);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D2);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D2);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D2);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D2);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D2);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                       
	SPI_writecom(0x20D3);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D3);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D3);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D3);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D3);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D3);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D3);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D3);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D3);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D3);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D3);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D3);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D3);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D3);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D3);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D3);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D3);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D3);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D3);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D3);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D3);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D3);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D3);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D3);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D3);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D3);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D3);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D3);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D3);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D3);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D3);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D3);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D3);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D3);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D3);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D3);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D3);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                       
	SPI_writecom(0x20D4);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D4);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D4);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D4);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D4);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D4);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D4);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D4);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D4);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D4);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D4);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D4);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D4);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D4);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D4);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D4);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D4);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D4);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D4);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D4);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D4);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D4);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D4);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D4);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D4);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D4);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D4);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D4);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D4);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D4);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D4);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D4);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D4);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D4);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D4);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D4);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D4);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                       
	SPI_writecom(0x20D5);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D5);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D5);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D5);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D5);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D5);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D5);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D5);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D5);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D5);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D5);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D5);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D5);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D5);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D5);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D5);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D5);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D5);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D5);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D5);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D5);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D5);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D5);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D5);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D5);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D5);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D5);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D5);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D5);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D5);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D5);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D5);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D5);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D5);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D5);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D5);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D5);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                 
	SPI_writecom(0x20D6);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0003); SPI_writedat(0x4017);
	SPI_writecom(0x20D6);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0005); SPI_writedat(0x403E);
	SPI_writecom(0x20D6);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0007); SPI_writedat(0x405E);
	SPI_writecom(0x20D6);SPI_writecom(0x0008); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x0009); SPI_writedat(0x407B);
	SPI_writecom(0x20D6);SPI_writecom(0x000A); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x000B); SPI_writedat(0x40A9);
	SPI_writecom(0x20D6);SPI_writecom(0x000C); SPI_writedat(0x4000);
	SPI_writecom(0x20D6);SPI_writecom(0x000D); SPI_writedat(0x40CE);
	SPI_writecom(0x20D6);SPI_writecom(0x000E); SPI_writedat(0x4001);
	SPI_writecom(0x20D6);SPI_writecom(0x000F); SPI_writedat(0x400A);
	SPI_writecom(0x20D6);SPI_writecom(0x0010); SPI_writedat(0x4001);
	SPI_writecom(0x20D6);SPI_writecom(0x0011); SPI_writedat(0x4037);
	SPI_writecom(0x20D6);SPI_writecom(0x0012); SPI_writedat(0x4001);
	SPI_writecom(0x20D6);SPI_writecom(0x0013); SPI_writedat(0x407C);
	SPI_writecom(0x20D6);SPI_writecom(0x0014); SPI_writedat(0x4001);
	SPI_writecom(0x20D6);SPI_writecom(0x0015); SPI_writedat(0x40B0);
	SPI_writecom(0x20D6);SPI_writecom(0x0016); SPI_writedat(0x4001);
	SPI_writecom(0x20D6);SPI_writecom(0x0017); SPI_writedat(0x40FF);
	SPI_writecom(0x20D6);SPI_writecom(0x0018); SPI_writedat(0x4002);
	SPI_writecom(0x20D6);SPI_writecom(0x0019); SPI_writedat(0x403D);
	SPI_writecom(0x20D6);SPI_writecom(0x001A); SPI_writedat(0x4002);
	SPI_writecom(0x20D6);SPI_writecom(0x001B); SPI_writedat(0x403F);
	SPI_writecom(0x20D6);SPI_writecom(0x001C); SPI_writedat(0x4002);
	SPI_writecom(0x20D6);SPI_writecom(0x001D); SPI_writedat(0x407C);
	SPI_writecom(0x20D6);SPI_writecom(0x001E); SPI_writedat(0x4002);
	SPI_writecom(0x20D6);SPI_writecom(0x001F); SPI_writedat(0x40C4);
	SPI_writecom(0x20D6);SPI_writecom(0x0020); SPI_writedat(0x4002);
	SPI_writecom(0x20D6);SPI_writecom(0x0021); SPI_writedat(0x40F6);
	SPI_writecom(0x20D6);SPI_writecom(0x0022); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0023); SPI_writedat(0x403A);
	SPI_writecom(0x20D6);SPI_writecom(0x0024); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0025); SPI_writedat(0x4068);
	SPI_writecom(0x20D6);SPI_writecom(0x0026); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0027); SPI_writedat(0x40A0);
	SPI_writecom(0x20D6);SPI_writecom(0x0028); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0029); SPI_writedat(0x40BF);
	SPI_writecom(0x20D6);SPI_writecom(0x002A); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x002B); SPI_writedat(0x40E0);
	SPI_writecom(0x20D6);SPI_writecom(0x002C); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x002D); SPI_writedat(0x40EC);
	SPI_writecom(0x20D6);SPI_writecom(0x002E); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x002F); SPI_writedat(0x40F5);
	SPI_writecom(0x20D6);SPI_writecom(0x0030); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0031); SPI_writedat(0x40FA);
	SPI_writecom(0x20D6);SPI_writecom(0x0032); SPI_writedat(0x4003);
	SPI_writecom(0x20D6);SPI_writecom(0x0033); SPI_writedat(0x40FF);
	                       
	//sunxi_lcd_delay_ms(200);      
	                       
	SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
	SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
	SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
	SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
	SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4003);
	                       
	SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x4005);
	SPI_writecom(0x20B0);SPI_writecom(0x0001); SPI_writedat(0x4017);
	SPI_writecom(0x20B0);SPI_writecom(0x0002); SPI_writedat(0x40F9);
	SPI_writecom(0x20B0);SPI_writecom(0x0003); SPI_writedat(0x4053);
	SPI_writecom(0x20B0);SPI_writecom(0x0004); SPI_writedat(0x4053);
	SPI_writecom(0x20B0);SPI_writecom(0x0005); SPI_writedat(0x4000);
	SPI_writecom(0x20B0);SPI_writecom(0x0006); SPI_writedat(0x4030);
	                       
	SPI_writecom(0x20B1);SPI_writecom(0x0000); SPI_writedat(0x4005);
	SPI_writecom(0x20B1);SPI_writecom(0x0001); SPI_writedat(0x4017);
	SPI_writecom(0x20B1);SPI_writecom(0x0002); SPI_writedat(0x40FB);
	SPI_writecom(0x20B1);SPI_writecom(0x0003); SPI_writedat(0x4055);
	SPI_writecom(0x20B1);SPI_writecom(0x0004); SPI_writedat(0x4053);
	SPI_writecom(0x20B1);SPI_writecom(0x0005); SPI_writedat(0x4000);
	SPI_writecom(0x20B1);SPI_writecom(0x0006); SPI_writedat(0x4030);
	                       
	SPI_writecom(0x20B2);SPI_writecom(0x0000); SPI_writedat(0x40FB);
	SPI_writecom(0x20B2);SPI_writecom(0x0001); SPI_writedat(0x40FC);
	SPI_writecom(0x20B2);SPI_writecom(0x0002); SPI_writedat(0x40FD);
	SPI_writecom(0x20B2);SPI_writecom(0x0003); SPI_writedat(0x40FE);
	SPI_writecom(0x20B2);SPI_writecom(0x0004); SPI_writedat(0x40F0);
	SPI_writecom(0x20B2);SPI_writecom(0x0005); SPI_writedat(0x4053);
	SPI_writecom(0x20B2);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20B2);SPI_writecom(0x0007); SPI_writedat(0x40C5);
	SPI_writecom(0x20B2);SPI_writecom(0x0008); SPI_writedat(0x4008);
	                       
	SPI_writecom(0x20B3);SPI_writecom(0x0000); SPI_writedat(0x405B);
	SPI_writecom(0x20B3);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20B3);SPI_writecom(0x0002); SPI_writedat(0x40FB);
	SPI_writecom(0x20B3);SPI_writecom(0x0003); SPI_writedat(0x405A);
	SPI_writecom(0x20B3);SPI_writecom(0x0004); SPI_writedat(0x405A);
	SPI_writecom(0x20B3);SPI_writecom(0x0005); SPI_writedat(0x400C);
	                       
	SPI_writecom(0x20B4);SPI_writecom(0x0000); SPI_writedat(0x40FF);
	SPI_writecom(0x20B4);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20B4);SPI_writecom(0x0002); SPI_writedat(0x4001);
	SPI_writecom(0x20B4);SPI_writecom(0x0003); SPI_writedat(0x4002);
	SPI_writecom(0x20B4);SPI_writecom(0x0004); SPI_writedat(0x40C0);
	SPI_writecom(0x20B4);SPI_writecom(0x0005); SPI_writedat(0x4040);
	SPI_writecom(0x20B4);SPI_writecom(0x0006); SPI_writedat(0x4005);
	SPI_writecom(0x20B4);SPI_writecom(0x0007); SPI_writedat(0x4008);
	SPI_writecom(0x20B4);SPI_writecom(0x0008); SPI_writedat(0x4053);
	                       
	SPI_writecom(0x20B5);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20B5);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20B5);SPI_writecom(0x0002); SPI_writedat(0x40FF);
	SPI_writecom(0x20B5);SPI_writecom(0x0003); SPI_writedat(0x4083);
	SPI_writecom(0x20B5);SPI_writecom(0x0004); SPI_writedat(0x405F);
	SPI_writecom(0x20B5);SPI_writecom(0x0005); SPI_writedat(0x405E);
	SPI_writecom(0x20B5);SPI_writecom(0x0006); SPI_writedat(0x4050);
	SPI_writecom(0x20B5);SPI_writecom(0x0007); SPI_writedat(0x4050);
	SPI_writecom(0x20B5);SPI_writecom(0x0008); SPI_writedat(0x4033);
	SPI_writecom(0x20B5);SPI_writecom(0x0009); SPI_writedat(0x4033);
	SPI_writecom(0x20B5);SPI_writecom(0x000A); SPI_writedat(0x4055);
	                       
	SPI_writecom(0x20B6);SPI_writecom(0x0000); SPI_writedat(0x40BC);
	SPI_writecom(0x20B6);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20B6);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20B6);SPI_writecom(0x0003); SPI_writedat(0x4000);
	SPI_writecom(0x20B6);SPI_writecom(0x0004); SPI_writedat(0x402A);
	SPI_writecom(0x20B6);SPI_writecom(0x0005); SPI_writedat(0x4080);
	SPI_writecom(0x20B6);SPI_writecom(0x0006); SPI_writedat(0x4000);
	                       
	SPI_writecom(0x20B7);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0001); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0002); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0003); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0004); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0005); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0006); SPI_writedat(0x4000);
	SPI_writecom(0x20B7);SPI_writecom(0x0007); SPI_writedat(0x4000);
	                       
	SPI_writecom(0x20B8);SPI_writecom(0x0000); SPI_writedat(0x4011);
	SPI_writecom(0x20B8);SPI_writecom(0x0001); SPI_writedat(0x4060);
	SPI_writecom(0x20B8);SPI_writecom(0x0002); SPI_writedat(0x4000);
	                       
	SPI_writecom(0x20B9);SPI_writecom(0x0000); SPI_writedat(0x4090);
	                       
	SPI_writecom(0x20BA);SPI_writecom(0x0000); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x0001); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x0002); SPI_writedat(0x4008);
	SPI_writecom(0x20BA);SPI_writecom(0x0003); SPI_writedat(0x40AC);
	SPI_writecom(0x20BA);SPI_writecom(0x0004); SPI_writedat(0x40E2);
	SPI_writecom(0x20BA);SPI_writecom(0x0005); SPI_writedat(0x4064);
	SPI_writecom(0x20BA);SPI_writecom(0x0006); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x0007); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x0008); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x0009); SPI_writedat(0x4044);
	SPI_writecom(0x20BA);SPI_writecom(0x000A); SPI_writedat(0x4047);
	SPI_writecom(0x20BA);SPI_writecom(0x000B); SPI_writedat(0x403F);
	SPI_writecom(0x20BA);SPI_writecom(0x000C); SPI_writedat(0x40DB);
	SPI_writecom(0x20BA);SPI_writecom(0x000D); SPI_writedat(0x4091);
	SPI_writecom(0x20BA);SPI_writecom(0x000E); SPI_writedat(0x4054);
	SPI_writecom(0x20BA);SPI_writecom(0x000F); SPI_writedat(0x4044);
	                       
	SPI_writecom(0x20BB);SPI_writecom(0x0000); SPI_writedat(0x4044);
	SPI_writecom(0x20BB);SPI_writecom(0x0001); SPI_writedat(0x4043);
	SPI_writecom(0x20BB);SPI_writecom(0x0002); SPI_writedat(0x4079);
	SPI_writecom(0x20BB);SPI_writecom(0x0003); SPI_writedat(0x40FD);
	SPI_writecom(0x20BB);SPI_writecom(0x0004); SPI_writedat(0x40B5);
	SPI_writecom(0x20BB);SPI_writecom(0x0005); SPI_writedat(0x4014);
	SPI_writecom(0x20BB);SPI_writecom(0x0006); SPI_writedat(0x4044);
	SPI_writecom(0x20BB);SPI_writecom(0x0007); SPI_writedat(0x4044);
	SPI_writecom(0x20BB);SPI_writecom(0x0008); SPI_writedat(0x4044);
	SPI_writecom(0x20BB);SPI_writecom(0x0009); SPI_writedat(0x4044);
	SPI_writecom(0x20BB);SPI_writecom(0x000A); SPI_writedat(0x4040);
	SPI_writecom(0x20BB);SPI_writecom(0x000B); SPI_writedat(0x404A);
	SPI_writecom(0x20BB);SPI_writecom(0x000C); SPI_writedat(0x40CE);
	SPI_writecom(0x20BB);SPI_writecom(0x000D); SPI_writedat(0x4086);
	SPI_writecom(0x20BB);SPI_writecom(0x000E); SPI_writedat(0x4024);
	SPI_writecom(0x20BB);SPI_writecom(0x000F); SPI_writedat(0x4044);
	                       
	
	SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x40E0);
	SPI_writecom(0x20BC);SPI_writecom(0x0001); SPI_writedat(0x401F);
	SPI_writecom(0x20BC);SPI_writecom(0x0002); SPI_writedat(0x40F8);
	SPI_writecom(0x20BC);SPI_writecom(0x0003); SPI_writedat(0x4007);
	                       
	SPI_writecom(0x20BD);SPI_writecom(0x0000); SPI_writedat(0x40E0);
	SPI_writecom(0x20BD);SPI_writecom(0x0001); SPI_writedat(0x401F);
	SPI_writecom(0x20BD);SPI_writecom(0x0002); SPI_writedat(0x40F8);
	SPI_writecom(0x20BD);SPI_writecom(0x0003); SPI_writedat(0x4007);
	                       
	//sunxi_lcd_delay_ms(200);      
	                       
	SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
	SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
	SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
	SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
	SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4000);
	                       
	SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x4000);
	SPI_writecom(0x20B0);SPI_writecom(0x0001); SPI_writedat(0x4010);
	                       
	//SPI_writecom(0x20B4);SPI_writecom(0x0000); SPI_writedat(0x4010);
	                       
	SPI_writecom(0x20B5);SPI_writecom(0x0000); SPI_writedat(0x406B);
	                       
	SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x4000);
	                       
	SPI_writecom(0x2035);SPI_writecom(0x0000); SPI_writedat(0x4000);

	SPI_writecom(0x2036);SPI_writecom(0x0000); SPI_writedat(0x4003);

	SPI_writecom(0x2011);SPI_writecom(0x0000); SPI_writedat(0x4000);
	                   
	sunxi_lcd_delay_ms(120);   
	                    
	SPI_writecom(0x2029);SPI_writecom(0x0000); SPI_writedat(0x4000);

	sunxi_lcd_delay_ms(20);

#endif
#if 0
/**************************************************
2015/12/3
IC Name: RM68172GA1
Panel Maker/Size: BOE495
Panel Product No.: BF050WVQ-100
Version: CODE
Date: V1.0_141222
**************************************************/

SPI_writecom(0x20F0); SPI_writecom(0x0000);SPI_writedat(0x4055);
SPI_writecom(0x20F0); SPI_writecom(0x0001);SPI_writedat(0x40AA);
SPI_writecom(0x20F0); SPI_writecom(0x0002);SPI_writedat(0x4052);
SPI_writecom(0x20F0); SPI_writecom(0x0003);SPI_writedat(0x4008);
SPI_writecom(0x20F0); SPI_writecom(0x0004);SPI_writedat(0x4002);

SPI_writecom(0x20F6); SPI_writecom(0x0000);SPI_writedat(0x4060);
SPI_writecom(0x20F6); SPI_writecom(0x0001);SPI_writedat(0x4040);

SPI_writecom(0x20FE); SPI_writecom(0x0000);SPI_writedat(0x4001);
SPI_writecom(0x20FE); SPI_writecom(0x0001);SPI_writedat(0x4080);
SPI_writecom(0x20FE); SPI_writecom(0x0002);SPI_writedat(0x4009);
SPI_writecom(0x20FE); SPI_writecom(0x0003);SPI_writedat(0x4009);

sunxi_lcd_delay_ms(200);

SPI_writecom(0x20F0); SPI_writecom(0x0000);SPI_writedat(0x4055);
SPI_writecom(0x20F0); SPI_writecom(0x0001);SPI_writedat(0x40AA);
SPI_writecom(0x20F0); SPI_writecom(0x0002);SPI_writedat(0x4052);
SPI_writecom(0x20F0); SPI_writecom(0x0003);SPI_writedat(0x4008);
SPI_writecom(0x20F0); SPI_writecom(0x0004);SPI_writedat(0x4001);

SPI_writecom(0x20B0); SPI_writecom(0x0000);SPI_writedat(0x400D);

SPI_writecom(0x20B1); SPI_writecom(0x0000);SPI_writedat(0x400D);

SPI_writecom(0x20B2); SPI_writecom(0x0000);SPI_writedat(0x4000);

SPI_writecom(0x20B5); SPI_writecom(0x0000);SPI_writedat(0x4008);

SPI_writecom(0x20B6); SPI_writecom(0x0000);SPI_writedat(0x4054);

SPI_writecom(0x20B7); SPI_writecom(0x0000);SPI_writedat(0x4044);

SPI_writecom(0x20B8); SPI_writecom(0x0000);SPI_writedat(0x4024);

SPI_writecom(0x20B9); SPI_writecom(0x0000);SPI_writedat(0x4034);

SPI_writecom(0x20BA); SPI_writecom(0x0000);SPI_writedat(0x4024);

SPI_writecom(0x20BC); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20BC); SPI_writecom(0x0001);SPI_writedat(0x4058);
SPI_writecom(0x20BC); SPI_writecom(0x0002);SPI_writedat(0x4000);

SPI_writecom(0x20BD); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20BD); SPI_writecom(0x0001);SPI_writedat(0x4058);
SPI_writecom(0x20BD); SPI_writecom(0x0002);SPI_writedat(0x4000);

SPI_writecom(0x20BE); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20BE); SPI_writecom(0x0001);SPI_writedat(0x403a);

SPI_writecom(0x20D1); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D1); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D1); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D1);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D1);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D1);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D1); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D1);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D1);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D1);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D1);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D1);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D1);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D1);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D1);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D1);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D1);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D1);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D1);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D1);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D1);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D1);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D1);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D1);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D1);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D1);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D1);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D1);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D1);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D1);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D1);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D1);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D1);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D1);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D1);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

SPI_writecom(0x20D2); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D2); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D2); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D2);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D2);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D2);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D2); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D2);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D2);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D2);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D2);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D2);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D2);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D2);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D2);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D2);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D2);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D2);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D2);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D2);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D2);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D2);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D2);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D2);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D2);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D2);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D2);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D2);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D2);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D2);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D2);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D2);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D2);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D2);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D2);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

SPI_writecom(0x20D3); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D3); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D3); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D3);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D3);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D3);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D3); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D3);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D3);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D3);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D3);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D3);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D3);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D3);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D3);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D3);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D3);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D3);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D3);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D3);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D3);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D3);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D3);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D3);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D3);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D3);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D3);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D3);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D3);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D3);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D3);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D3);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D3);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D3);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D3);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

SPI_writecom(0x20D4); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D4); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D4); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D4);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D4);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D4);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D4); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D4);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D4);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D4);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D4);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D4);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D4);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D4);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D4);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D4);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D4);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D4);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D4);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D4);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D4);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D4);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D4);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D4);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D4);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D4);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D4);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D4);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D4);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D4);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D4);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D4);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D4);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D4);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D4);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

SPI_writecom(0x20D5); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D5); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D5); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D5);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D5);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D5);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D5); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D5);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D5);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D5);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D5);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D5);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D5);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D5);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D5);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D5);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D5);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D5);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D5);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D5);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D5);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D5);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D5);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D5);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D5);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D5);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D5);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D5);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D5);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D5);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D5);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D5);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D5);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D5);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D5);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

SPI_writecom(0x20D6); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x0003); SPI_writedat(0x4051);
SPI_writecom(0x20D6); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x0005); SPI_writedat(0x407E);
SPI_writecom(0x20D6); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x0007); SPI_writedat(0x409A);
SPI_writecom(0x20D6);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20D6);  SPI_writecom(0x0009);SPI_writedat(0x40B0);
SPI_writecom(0x20D6);  SPI_writecom(0x000A);SPI_writedat(0x4000);
SPI_writecom(0x20D6); SPI_writecom(0x000B); SPI_writedat(0x40D2);
SPI_writecom(0x20D6);  SPI_writecom(0x000C);SPI_writedat(0x4000);
SPI_writecom(0x20D6);  SPI_writecom(0x000D);SPI_writedat(0x40EE);
SPI_writecom(0x20D6);  SPI_writecom(0x000E);SPI_writedat(0x4001);
SPI_writecom(0x20D6);  SPI_writecom(0x000F);SPI_writedat(0x401A);
SPI_writecom(0x20D6);  SPI_writecom(0x0010);SPI_writedat(0x4001);
SPI_writecom(0x20D6);  SPI_writecom(0x0011);SPI_writedat(0x403C);
SPI_writecom(0x20D6);  SPI_writecom(0x0012);SPI_writedat(0x4001);
SPI_writecom(0x20D6);  SPI_writecom(0x0013);SPI_writedat(0x4071);
SPI_writecom(0x20D6);  SPI_writecom(0x0014);SPI_writedat(0x4001);
SPI_writecom(0x20D6);  SPI_writecom(0x0015);SPI_writedat(0x409C);
SPI_writecom(0x20D6);  SPI_writecom(0x0016);SPI_writedat(0x4001);
SPI_writecom(0x20D6);  SPI_writecom(0x0017);SPI_writedat(0x40DF);
SPI_writecom(0x20D6);  SPI_writecom(0x0018);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x0019);SPI_writedat(0x4016);
SPI_writecom(0x20D6);  SPI_writecom(0x001A);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x001B);SPI_writedat(0x4018);
SPI_writecom(0x20D6);  SPI_writecom(0x001C);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x001D);SPI_writedat(0x404B);
SPI_writecom(0x20D6);  SPI_writecom(0x001E);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x001F);SPI_writedat(0x4080);
SPI_writecom(0x20D6);  SPI_writecom(0x0020);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x0021);SPI_writedat(0x40A1);
SPI_writecom(0x20D6);  SPI_writecom(0x0022);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x0023);SPI_writedat(0x40CD);
SPI_writecom(0x20D6);  SPI_writecom(0x0024);SPI_writedat(0x4002);
SPI_writecom(0x20D6);  SPI_writecom(0x0025);SPI_writedat(0x40EB);
SPI_writecom(0x20D6);  SPI_writecom(0x0026);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x0027);SPI_writedat(0x4011);
SPI_writecom(0x20D6);  SPI_writecom(0x0028);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x0029);SPI_writedat(0x4027);
SPI_writecom(0x20D6);  SPI_writecom(0x002A);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x002B);SPI_writedat(0x4042);
SPI_writecom(0x20D6);  SPI_writecom(0x002C);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x002D);SPI_writedat(0x404F);
SPI_writecom(0x20D6);  SPI_writecom(0x002E);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x002F);SPI_writedat(0x405A);
SPI_writecom(0x20D6);  SPI_writecom(0x0030);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x0031);SPI_writedat(0x4062);
SPI_writecom(0x20D6);  SPI_writecom(0x0032);SPI_writedat(0x4003);
SPI_writecom(0x20D6);  SPI_writecom(0x0033);SPI_writedat(0x40FF);

sunxi_lcd_delay_ms(200);

SPI_writecom(0x20F0); SPI_writecom(0x0000);SPI_writedat(0x4055);
SPI_writecom(0x20F0); SPI_writecom(0x0001);SPI_writedat(0x40AA);
SPI_writecom(0x20F0); SPI_writecom(0x0002);SPI_writedat(0x4052);
SPI_writecom(0x20F0); SPI_writecom(0x0003);SPI_writedat(0x4008);
SPI_writecom(0x20F0); SPI_writecom(0x0004);SPI_writedat(0x4003);

SPI_writecom(0x20B0); SPI_writecom(0x0000);SPI_writedat(0x4003);
SPI_writecom(0x20B0); SPI_writecom(0x0001);SPI_writedat(0x4015);
SPI_writecom(0x20B0); SPI_writecom(0x0002);SPI_writedat(0x40FA);
SPI_writecom(0x20B0); SPI_writecom(0x0003);SPI_writedat(0x4000);
SPI_writecom(0x20B0); SPI_writecom(0x0004);SPI_writedat(0x4000);
SPI_writecom(0x20B0); SPI_writecom(0x0005);SPI_writedat(0x4000);
SPI_writecom(0x20B0); SPI_writecom(0x0006);SPI_writedat(0x4000);

SPI_writecom(0x20B2); SPI_writecom(0x0000); SPI_writedat(0x40FB);
SPI_writecom(0x20B2); SPI_writecom(0x0001); SPI_writedat(0x40FC);
SPI_writecom(0x20B2); SPI_writecom(0x0002); SPI_writedat(0x40FD);
SPI_writecom(0x20B2); SPI_writecom(0x0003); SPI_writedat(0x40FE);
SPI_writecom(0x20B2); SPI_writecom(0x0004); SPI_writedat(0x40F0);
SPI_writecom(0x20B2); SPI_writecom(0x0005); SPI_writedat(0x4020);
SPI_writecom(0x20B2); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20B2); SPI_writecom(0x0007); SPI_writedat(0x4083);
SPI_writecom(0x20B2); SPI_writecom(0x0008);SPI_writedat(0x4004);

SPI_writecom(0x20B3); SPI_writecom(0x0000);SPI_writedat(0x405B);
SPI_writecom(0x20B3); SPI_writecom(0x0001);SPI_writedat(0x4000);
SPI_writecom(0x20B3); SPI_writecom(0x0002);SPI_writedat(0x40FB);
SPI_writecom(0x20B3); SPI_writecom(0x0003);SPI_writedat(0x4021);
SPI_writecom(0x20B3); SPI_writecom(0x0004);SPI_writedat(0x4023);
SPI_writecom(0x20B3); SPI_writecom(0x0005);SPI_writedat(0x400C);

SPI_writecom(0x20B4); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0002);SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0003); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0005); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0007); SPI_writedat(0x4000);
SPI_writecom(0x20B4); SPI_writecom(0x0008);SPI_writedat(0x4000);

SPI_writecom(0x20B5); SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0002);SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0003); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0005); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20B5); SPI_writecom(0x0007); SPI_writedat(0x4000);
SPI_writecom(0x20B5);  SPI_writecom(0x0008);SPI_writedat(0x4000);
SPI_writecom(0x20B5);  SPI_writecom(0x0009);SPI_writedat(0x4000);
SPI_writecom(0x20B5);  SPI_writecom(0x000A);SPI_writedat(0x4044);

SPI_writecom(0x20B6); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0001);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0002);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0003);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0004);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0005);SPI_writedat(0x4000);
SPI_writecom(0x20B6); SPI_writecom(0x0006);SPI_writedat(0x4000);

SPI_writecom(0x20B7); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20B7); SPI_writecom(0x0001);SPI_writedat(0x4000);
SPI_writecom(0x20B7); SPI_writecom(0x0002);SPI_writedat(0x4020);
SPI_writecom(0x20B7); SPI_writecom(0x0003);SPI_writedat(0x4020);
SPI_writecom(0x20B7); SPI_writecom(0x0004);SPI_writedat(0x4020);
SPI_writecom(0x20B7); SPI_writecom(0x0005);SPI_writedat(0x4020);
SPI_writecom(0x20B7); SPI_writecom(0x0006);SPI_writedat(0x4000);
SPI_writecom(0x20B7); SPI_writecom(0x0007);SPI_writedat(0x4000);

SPI_writecom(0x20B8); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20B8); SPI_writecom(0x0001);SPI_writedat(0x4000);
SPI_writecom(0x20B8); SPI_writecom(0x0002);SPI_writedat(0x4000);

SPI_writecom(0x20B9); SPI_writecom(0x0000);SPI_writedat(0x4082);

SPI_writecom(0x20BA); SPI_writecom(0x0000); SPI_writedat(0x4045);
SPI_writecom(0x20BA); SPI_writecom(0x0001); SPI_writedat(0x408A);
SPI_writecom(0x20BA); SPI_writecom(0x0002);SPI_writedat(0x4004);
SPI_writecom(0x20BA); SPI_writecom(0x0003); SPI_writedat(0x405F);
SPI_writecom(0x20BA); SPI_writecom(0x0004); SPI_writedat(0x40FF);
SPI_writecom(0x20BA); SPI_writecom(0x0005); SPI_writedat(0x40FF);
SPI_writecom(0x20BA); SPI_writecom(0x0006); SPI_writedat(0x40FF);
SPI_writecom(0x20BA); SPI_writecom(0x0007); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);  SPI_writecom(0x0008);SPI_writedat(0x40FF);
SPI_writecom(0x20BA);  SPI_writecom(0x0009);SPI_writedat(0x40FF);
SPI_writecom(0x20BA);  SPI_writecom(0x000A);SPI_writedat(0x40FF);
SPI_writecom(0x20BA); SPI_writecom(0x000B); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);  SPI_writecom(0x000C);SPI_writedat(0x40F5);
SPI_writecom(0x20BA);  SPI_writecom(0x000D);SPI_writedat(0x4041);
SPI_writecom(0x20BA);  SPI_writecom(0x000E);SPI_writedat(0x40B9);
SPI_writecom(0x20BA);  SPI_writecom(0x000F);SPI_writedat(0x4054);

SPI_writecom(0x20BB); SPI_writecom(0x0000); SPI_writedat(0x4054);
SPI_writecom(0x20BB); SPI_writecom(0x0001); SPI_writedat(0x40B9);
SPI_writecom(0x20BB); SPI_writecom(0x0002);SPI_writedat(0x4015);
SPI_writecom(0x20BB); SPI_writecom(0x0003); SPI_writedat(0x404F);
SPI_writecom(0x20BB); SPI_writecom(0x0004); SPI_writedat(0x40FF);
SPI_writecom(0x20BB); SPI_writecom(0x0005); SPI_writedat(0x40FF);
SPI_writecom(0x20BB); SPI_writecom(0x0006); SPI_writedat(0x40FF);
SPI_writecom(0x20BB); SPI_writecom(0x0007); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);  SPI_writecom(0x0008);SPI_writedat(0x40FF);
SPI_writecom(0x20BB);  SPI_writecom(0x0009);SPI_writedat(0x40FF);
SPI_writecom(0x20BB);  SPI_writecom(0x000A);SPI_writedat(0x40FF);
SPI_writecom(0x20BB); SPI_writecom(0x000B); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);  SPI_writecom(0x000C);SPI_writedat(0x40F4);
SPI_writecom(0x20BB);  SPI_writecom(0x000D);SPI_writedat(0x4050);
SPI_writecom(0x20BB);  SPI_writecom(0x000E);SPI_writedat(0x408A);
SPI_writecom(0x20BB);  SPI_writecom(0x000F);SPI_writedat(0x4045);

SPI_writecom(0x20BC); SPI_writecom(0x0000);SPI_writedat(0x40C7);
SPI_writecom(0x20BC); SPI_writecom(0x0001);SPI_writedat(0x40FF);
SPI_writecom(0x20BC); SPI_writecom(0x0002);SPI_writedat(0x40FF);
SPI_writecom(0x20BC); SPI_writecom(0x0003);SPI_writedat(0x40E3);

SPI_writecom(0x20BD); SPI_writecom(0x0000);SPI_writedat(0x40C7);
SPI_writecom(0x20BD); SPI_writecom(0x0001);SPI_writedat(0x40FF);
SPI_writecom(0x20BD); SPI_writecom(0x0002);SPI_writedat(0x40FF);
SPI_writecom(0x20BD); SPI_writecom(0x0003);SPI_writedat(0x40E3);

SPI_writecom(0x20C0); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20C0); SPI_writecom(0x0001);SPI_writedat(0x4001);
SPI_writecom(0x20C0); SPI_writecom(0x0002);SPI_writedat(0x40FA);
SPI_writecom(0x20C0); SPI_writecom(0x0003);SPI_writedat(0x4000);

sunxi_lcd_delay_ms(200);

SPI_writecom(0x20F0); SPI_writecom(0x0000);SPI_writedat(0x4055);
SPI_writecom(0x20F0); SPI_writecom(0x0001);SPI_writedat(0x40AA);
SPI_writecom(0x20F0); SPI_writecom(0x0002);SPI_writedat(0x4052);
SPI_writecom(0x20F0); SPI_writecom(0x0003);SPI_writedat(0x4008);
SPI_writecom(0x20F0); SPI_writecom(0x0004);SPI_writedat(0x4000);

SPI_writecom(0x20B0); SPI_writecom(0x0000);SPI_writedat(0x4000);
SPI_writecom(0x20B0); SPI_writecom(0x0001);SPI_writedat(0x4010);

SPI_writecom(0x20B1); SPI_writecom(0x0000);SPI_writedat(0x40FC);

SPI_writecom(0x20BA); SPI_writecom(0x0000);SPI_writedat(0x4001);

//SPI_writecom(0x20B4); SPI_writecom(0x0000);SPI_writedat(0x4010);

SPI_writecom(0x20B5); SPI_writecom(0x0000);SPI_writedat(0x406B);

SPI_writecom(0x20BC); SPI_writecom(0x0000);SPI_writedat(0x4000);

SPI_writecom(0x2036); SPI_writecom(0x0000);SPI_writedat(0x4003);

SPI_writecom(0x2011);SPI_writecom(0x0000);SPI_writedat(0x4000);

sunxi_lcd_delay_ms(200);

SPI_writecom(0x2029);SPI_writecom(0x0000);SPI_writedat(0x4000);

sunxi_lcd_delay_ms(200);
#endif
#if 1
/**************************************************
IC Name: RM68172
Panel Maker/Size: AUO445
Panel Product No.: H445VAN01
Version: V2
Date: 140925
**************************************************/

SPI_writecom(0x2001); SPI_writecom(0x0000);SPI_writedat(0x4000);

sunxi_lcd_delay_ms(120);

SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4002);

SPI_writecom(0x20F6);SPI_writecom(0x0000); SPI_writedat(0x4060);
SPI_writecom(0x20F6);SPI_writecom(0x0001); SPI_writedat(0x4040);

SPI_writecom(0x20FE);SPI_writecom(0x0000); SPI_writedat(0x4001);
SPI_writecom(0x20FE);SPI_writecom(0x0001); SPI_writedat(0x4080);
SPI_writecom(0x20FE);SPI_writecom(0x0002); SPI_writedat(0x4009);
SPI_writecom(0x20FE);SPI_writecom(0x0003); SPI_writedat(0x4009);

SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4001);

SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x400D);

SPI_writecom(0x20B1);SPI_writecom(0x0000); SPI_writedat(0x400D);

SPI_writecom(0x20B2);SPI_writecom(0x0000); SPI_writedat(0x4000);

SPI_writecom(0x20B6);SPI_writecom(0x0000); SPI_writedat(0x4034);

SPI_writecom(0x20B7);SPI_writecom(0x0000); SPI_writedat(0x4044);

SPI_writecom(0x20B8);SPI_writecom(0x0000); SPI_writedat(0x4024);

SPI_writecom(0x20B9);SPI_writecom(0x0000); SPI_writedat(0x4034);

SPI_writecom(0x20BA);SPI_writecom(0x0000); SPI_writedat(0x4024);

SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20BC);SPI_writecom(0x0001); SPI_writedat(0x4098);
SPI_writecom(0x20BC);SPI_writecom(0x0002); SPI_writedat(0x4000);

SPI_writecom(0x20BD);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20BD);SPI_writecom(0x0001); SPI_writedat(0x4098);
SPI_writecom(0x20BD);SPI_writecom(0x0002); SPI_writedat(0x4000);

SPI_writecom(0x20BE);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20BE);SPI_writecom(0x0001); SPI_writedat(0x4060);

SPI_writecom(0x20D1);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D1);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D1);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D1);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D1);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D1);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D1);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D1);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D1);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D1);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D1);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D1);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D1);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D1);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D1);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D1);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D1);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D1);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D1);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D1);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D1);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D1);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D1);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D1);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D1);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D1);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D1);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D1);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D1);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D1);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D1);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D1);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D1);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D1);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D1);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D1);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D1);SPI_writecom(0x0033); SPI_writedat(0x40FF);

SPI_writecom(0x20D2);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D2);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D2);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D2);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D2);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D2);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D2);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D2);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D2);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D2);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D2);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D2);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D2);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D2);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D2);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D2);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D2);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D2);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D2);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D2);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D2);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D2);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D2);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D2);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D2);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D2);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D2);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D2);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D2);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D2);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D2);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D2);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D2);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D2);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D2);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D2);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D2);SPI_writecom(0x0033); SPI_writedat(0x40FF);


SPI_writecom(0x20D3);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D3);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D3);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D3);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D3);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D3);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D3);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D3);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D3);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D3);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D3);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D3);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D3);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D3);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D3);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D3);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D3);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D3);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D3);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D3);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D3);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D3);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D3);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D3);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D3);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D3);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D3);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D3);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D3);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D3);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D3);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D3);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D3);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D3);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D3);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D3);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D3);SPI_writecom(0x0033); SPI_writedat(0x40FF);

SPI_writecom(0x20D4);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D4);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D4);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D4);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D4);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D4);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D4);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D4);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D4);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D4);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D4);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D4);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D4);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D4);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D4);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D4);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D4);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D4);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D4);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D4);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D4);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D4);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D4);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D4);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D4);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D4);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D4);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D4);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D4);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D4);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D4);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D4);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D4);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D4);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D4);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D4);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D4);SPI_writecom(0x0033); SPI_writedat(0x40FF);

SPI_writecom(0x20D5);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D5);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D5);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D5);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D5);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D5);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D5);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D5);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D5);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D5);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D5);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D5);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D5);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D5);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D5);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D5);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D5);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D5);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D5);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D5);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D5);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D5);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D5);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D5);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D5);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D5);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D5);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D5);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D5);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D5);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D5);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D5);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D5);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D5);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D5);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D5);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D5);SPI_writecom(0x0033); SPI_writedat(0x40FF);

SPI_writecom(0x20D6);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0003); SPI_writedat(0x4011);
SPI_writecom(0x20D6);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0005); SPI_writedat(0x402E);
SPI_writecom(0x20D6);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0007); SPI_writedat(0x4048);
SPI_writecom(0x20D6);SPI_writecom(0x0008); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0009); SPI_writedat(0x405C);
SPI_writecom(0x20D6);SPI_writecom(0x000A); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x000B); SPI_writedat(0x407D);
SPI_writecom(0x20D6);SPI_writecom(0x000C); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x000D); SPI_writedat(0x4099);
SPI_writecom(0x20D6);SPI_writecom(0x000E); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x000F); SPI_writedat(0x40C9);
SPI_writecom(0x20D6);SPI_writecom(0x0010); SPI_writedat(0x4000);
SPI_writecom(0x20D6);SPI_writecom(0x0011); SPI_writedat(0x40F0);
SPI_writecom(0x20D6);SPI_writecom(0x0012); SPI_writedat(0x4001);
SPI_writecom(0x20D6);SPI_writecom(0x0013); SPI_writedat(0x402F);
SPI_writecom(0x20D6);SPI_writecom(0x0014); SPI_writedat(0x4001);
SPI_writecom(0x20D6);SPI_writecom(0x0015); SPI_writedat(0x4061);
SPI_writecom(0x20D6);SPI_writecom(0x0016); SPI_writedat(0x4001);
SPI_writecom(0x20D6);SPI_writecom(0x0017); SPI_writedat(0x40B2);
SPI_writecom(0x20D6);SPI_writecom(0x0018); SPI_writedat(0x4001);
SPI_writecom(0x20D6);SPI_writecom(0x0019); SPI_writedat(0x40F4);
SPI_writecom(0x20D6);SPI_writecom(0x001A); SPI_writedat(0x4001);
SPI_writecom(0x20D6);SPI_writecom(0x001B); SPI_writedat(0x40F6);
SPI_writecom(0x20D6);SPI_writecom(0x001C); SPI_writedat(0x4002);
SPI_writecom(0x20D6);SPI_writecom(0x001D); SPI_writedat(0x4033);
SPI_writecom(0x20D6);SPI_writecom(0x001E); SPI_writedat(0x4002);
SPI_writecom(0x20D6);SPI_writecom(0x001F); SPI_writedat(0x4075);
SPI_writecom(0x20D6);SPI_writecom(0x0020); SPI_writedat(0x4002);
SPI_writecom(0x20D6);SPI_writecom(0x0021); SPI_writedat(0x409E);
SPI_writecom(0x20D6);SPI_writecom(0x0022); SPI_writedat(0x4002);
SPI_writecom(0x20D6);SPI_writecom(0x0023); SPI_writedat(0x40D3);
SPI_writecom(0x20D6);SPI_writecom(0x0024); SPI_writedat(0x4002);
SPI_writecom(0x20D6);SPI_writecom(0x0025); SPI_writedat(0x40F5);
SPI_writecom(0x20D6);SPI_writecom(0x0026); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x0027); SPI_writedat(0x4022);
SPI_writecom(0x20D6);SPI_writecom(0x0028); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x0029); SPI_writedat(0x403F);
SPI_writecom(0x20D6);SPI_writecom(0x002A); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x002B); SPI_writedat(0x405F);
SPI_writecom(0x20D6);SPI_writecom(0x002C); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x002D); SPI_writedat(0x406F);
SPI_writecom(0x20D6);SPI_writecom(0x002E); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x002F); SPI_writedat(0x4080);
SPI_writecom(0x20D6);SPI_writecom(0x0030); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x0031); SPI_writedat(0x408A);
SPI_writecom(0x20D6);SPI_writecom(0x0032); SPI_writedat(0x4003);
SPI_writecom(0x20D6);SPI_writecom(0x0033); SPI_writedat(0x40FF);

sunxi_lcd_delay_ms(10);

SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4003);

SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x4003);
SPI_writecom(0x20B0);SPI_writecom(0x0001); SPI_writedat(0x4017);
SPI_writecom(0x20B0);SPI_writecom(0x0002); SPI_writedat(0x40FD);
SPI_writecom(0x20B0);SPI_writecom(0x0003); SPI_writedat(0x4055);
SPI_writecom(0x20B0);SPI_writecom(0x0004); SPI_writedat(0x4064);
SPI_writecom(0x20B0);SPI_writecom(0x0005); SPI_writedat(0x4000);
SPI_writecom(0x20B0);SPI_writecom(0x0006); SPI_writedat(0x4030);

SPI_writecom(0x20B2);SPI_writecom(0x0000); SPI_writedat(0x40FF);
SPI_writecom(0x20B2);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B2);SPI_writecom(0x0002); SPI_writedat(0x4001);
SPI_writecom(0x20B2);SPI_writecom(0x0003); SPI_writedat(0x4002);
SPI_writecom(0x20B2);SPI_writecom(0x0004); SPI_writedat(0x4010);
SPI_writecom(0x20B2);SPI_writecom(0x0005); SPI_writedat(0x4064);
SPI_writecom(0x20B2);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20B2);SPI_writecom(0x0007); SPI_writedat(0x4083);
SPI_writecom(0x20B2);SPI_writecom(0x0008); SPI_writedat(0x4004);

SPI_writecom(0x20B3);SPI_writecom(0x0000); SPI_writedat(0x405B);
SPI_writecom(0x20B3);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B3);SPI_writecom(0x0002); SPI_writedat(0x40FF);
SPI_writecom(0x20B3);SPI_writecom(0x0003); SPI_writedat(0x4057);
SPI_writecom(0x20B3);SPI_writecom(0x0004); SPI_writedat(0x405D);
SPI_writecom(0x20B3);SPI_writecom(0x0005); SPI_writedat(0x4003);

SPI_writecom(0x20B4);SPI_writecom(0x0000); SPI_writedat(0x4001);
SPI_writecom(0x20B4);SPI_writecom(0x0001); SPI_writedat(0x4002);
SPI_writecom(0x20B4);SPI_writecom(0x0002); SPI_writedat(0x4003);
SPI_writecom(0x20B4);SPI_writecom(0x0003); SPI_writedat(0x4004);
SPI_writecom(0x20B4);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20B4);SPI_writecom(0x0005); SPI_writedat(0x4040);
SPI_writecom(0x20B4);SPI_writecom(0x0006); SPI_writedat(0x4003);
SPI_writecom(0x20B4);SPI_writecom(0x0007); SPI_writedat(0x4004);
SPI_writecom(0x20B4);SPI_writecom(0x0008); SPI_writedat(0x4046);
SPI_writecom(0x20B4);SPI_writecom(0x0009); SPI_writedat(0x4062);
SPI_writecom(0x20B4);SPI_writecom(0x000A); SPI_writedat(0x4000);

SPI_writecom(0x20B5);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20B5);SPI_writecom(0x0001); SPI_writedat(0x4044);
SPI_writecom(0x20B5);SPI_writecom(0x0002); SPI_writedat(0x4001);
SPI_writecom(0x20B5);SPI_writecom(0x0003); SPI_writedat(0x4000);
SPI_writecom(0x20B5);SPI_writecom(0x0004); SPI_writedat(0x4058);
SPI_writecom(0x20B5);SPI_writecom(0x0005); SPI_writedat(0x405D);
SPI_writecom(0x20B5);SPI_writecom(0x0006); SPI_writedat(0x405D);
SPI_writecom(0x20B5);SPI_writecom(0x0007); SPI_writedat(0x405D);
SPI_writecom(0x20B5);SPI_writecom(0x0008); SPI_writedat(0x4033);
SPI_writecom(0x20B5);SPI_writecom(0x0009); SPI_writedat(0x4033);
SPI_writecom(0x20B5);SPI_writecom(0x000A); SPI_writedat(0x4055);

SPI_writecom(0x20B6);SPI_writecom(0x0000); SPI_writedat(0x40BC);
SPI_writecom(0x20B6);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B6);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20B6);SPI_writecom(0x0003); SPI_writedat(0x4000);
SPI_writecom(0x20B6);SPI_writecom(0x0004); SPI_writedat(0x402A);
SPI_writecom(0x20B6);SPI_writecom(0x0005); SPI_writedat(0x4000);
SPI_writecom(0x20B6);SPI_writecom(0x0006); SPI_writedat(0x4000);

SPI_writecom(0x20B7);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0001); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0002); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0003); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0004); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0005); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0006); SPI_writedat(0x4000);
SPI_writecom(0x20B7);SPI_writecom(0x0007); SPI_writedat(0x4000);

SPI_writecom(0x20B8);SPI_writecom(0x0000); SPI_writedat(0x4011);
SPI_writecom(0x20B8);SPI_writecom(0x0001); SPI_writedat(0x4060);
SPI_writecom(0x20B8);SPI_writecom(0x0002); SPI_writedat(0x4000);

SPI_writecom(0x20B9);SPI_writecom(0x0000); SPI_writedat(0x4082);

SPI_writecom(0x20BA);SPI_writecom(0x0000); SPI_writedat(0x400F);
SPI_writecom(0x20BA);SPI_writecom(0x0001); SPI_writedat(0x40A8);
SPI_writecom(0x20BA);SPI_writecom(0x0002); SPI_writedat(0x40EC);
SPI_writecom(0x20BA);SPI_writecom(0x0003); SPI_writedat(0x402F);
SPI_writecom(0x20BA);SPI_writecom(0x0004); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x0005); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x0006); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x0007); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x0008); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x0009); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x000A); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x000B); SPI_writedat(0x40FF);
SPI_writecom(0x20BA);SPI_writecom(0x000C); SPI_writedat(0x40F3);
SPI_writecom(0x20BA);SPI_writecom(0x000D); SPI_writedat(0x40DF);
SPI_writecom(0x20BA);SPI_writecom(0x000E); SPI_writedat(0x409B);
SPI_writecom(0x20BA);SPI_writecom(0x000F); SPI_writedat(0x40F1);

SPI_writecom(0x20BB);SPI_writecom(0x0000); SPI_writedat(0x403F);
SPI_writecom(0x20BB);SPI_writecom(0x0001); SPI_writedat(0x40B9);
SPI_writecom(0x20BB);SPI_writecom(0x0002); SPI_writedat(0x40DF);
SPI_writecom(0x20BB);SPI_writecom(0x0003); SPI_writedat(0x401F);
SPI_writecom(0x20BB);SPI_writecom(0x0004); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x0005); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x0006); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x0007); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x0008); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x0009); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x000A); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x000B); SPI_writedat(0x40FF);
SPI_writecom(0x20BB);SPI_writecom(0x000C); SPI_writedat(0x40F0);
SPI_writecom(0x20BB);SPI_writecom(0x000D); SPI_writedat(0x40EC);
SPI_writecom(0x20BB);SPI_writecom(0x000E); SPI_writedat(0x408A);
SPI_writecom(0x20BB);SPI_writecom(0x000F); SPI_writedat(0x40F2);

SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x4041);
SPI_writecom(0x20BC);SPI_writecom(0x0001); SPI_writedat(0x40FF);
SPI_writecom(0x20BC);SPI_writecom(0x0002); SPI_writedat(0x40FF);
SPI_writecom(0x20BC);SPI_writecom(0x0003); SPI_writedat(0x4082);

SPI_writecom(0x20BD);SPI_writecom(0x0000); SPI_writedat(0x4041);
SPI_writecom(0x20BD);SPI_writecom(0x0001); SPI_writedat(0x40FF);
SPI_writecom(0x20BD);SPI_writecom(0x0002); SPI_writedat(0x40FF);
SPI_writecom(0x20BD);SPI_writecom(0x0003); SPI_writedat(0x4082);

sunxi_lcd_delay_ms(10);

SPI_writecom(0x20F0);SPI_writecom(0x0000); SPI_writedat(0x4055);
SPI_writecom(0x20F0);SPI_writecom(0x0001); SPI_writedat(0x40AA);
SPI_writecom(0x20F0);SPI_writecom(0x0002); SPI_writedat(0x4052);
SPI_writecom(0x20F0);SPI_writecom(0x0003); SPI_writedat(0x4008);
SPI_writecom(0x20F0);SPI_writecom(0x0004); SPI_writedat(0x4000);

SPI_writecom(0x20B0);SPI_writecom(0x0000); SPI_writedat(0x4000);
SPI_writecom(0x20B0);SPI_writecom(0x0001); SPI_writedat(0x4010);

//rotation 180

//SPI_writecom(0x20B1);SPI_writecom(0x0000); SPI_writedat(0x40FC);//???Ú^????
//SPI_writecom(0x20B1);SPI_writecom(0x0001); SPI_writedat(0x4006);//???Ú^????
//


SPI_writecom(0x20B5);SPI_writecom(0x0000); SPI_writedat(0x406B);

SPI_writecom(0x20BC);SPI_writecom(0x0000); SPI_writedat(0x4000);

SPI_writecom(0x2035);SPI_writecom(0x0000); SPI_writedat(0x4000);

SPI_writecom(0x2036);SPI_writecom(0x0000); SPI_writedat(0x4003);

SPI_writecom(0x2011);SPI_writecom(0x0000); SPI_writedat(0x4000);

sunxi_lcd_delay_ms(200);

SPI_writecom(0x2029);SPI_writecom(0x0000); SPI_writedat(0x4000);

sunxi_lcd_delay_ms(200);
#endif
	DBG_INFO("\n");

}

static void LCD_panel_exit(u32 sel)
{
	DBG_INFO("\n");

	SPI_writecom(0x2028); SPI_writecom(0x0000);	SPI_writedat(0x4000);

	sunxi_lcd_delay_ms(120);	// Display off
	SPI_writecom(0x2010); SPI_writecom(0x0000);	SPI_writedat(0x4000);

	sunxi_lcd_delay_ms(120);	// sleep in
	DBG_INFO(" display off \n");

	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel rm68172_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "lx45fwu4004_v0_rm68172",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
