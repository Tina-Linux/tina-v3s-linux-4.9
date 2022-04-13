#include "default_panel.h"

#define t27p06_spi_scl_1	sunxi_lcd_gpio_set_value(0,2,1)
#define t27p06_spi_scl_0	sunxi_lcd_gpio_set_value(0,2,0)
#define t27p06_spi_sdi_1	sunxi_lcd_gpio_set_value(0,1,1)
#define t27p06_spi_sdi_0	sunxi_lcd_gpio_set_value(0,1,0)
#define t27p06_spi_cs_1  	sunxi_lcd_gpio_set_value(0,0,1)
#define t27p06_spi_cs_0  	sunxi_lcd_gpio_set_value(0,0,0)

#define t27p06_spi_reset_1  	sunxi_lcd_gpio_set_value(0,3,1)
#define t27p06_spi_reset_0  	sunxi_lcd_gpio_set_value(0,3,0)
static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

static void LCD_cfg_panel_info(struct panel_extend_para * info)
{
	printk(">>>>>>>>>>>>>>>>>>>>>>>>>>LCD_cfg_panel_info---kernel\n");
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
	LCD_OPEN_FUNC(sel, LCD_power_on, 30);   //open lcd power, and delay 50ms
	LCD_OPEN_FUNC(sel, LCD_panel_init, 50);   //open lcd power, than delay 200ms
	LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 100);     //open lcd controller, and delay 100ms
	LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

	return 0;
}

static s32 LCD_close_flow(u32 sel)
{
	LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
	LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
	LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
	LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

	return 0;
}

static void LCD_power_on(u32 sel)
{
	sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
	sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
	sunxi_lcd_pin_cfg(sel, 0);
	sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
}

static void LCD_bl_open(u32 sel)
{
	sunxi_lcd_pwm_enable(sel);
	sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
	sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
	sunxi_lcd_pwm_disable(sel);
}

static void write_spidat(u32 value)
{
	u32 i;
	t27p06_spi_cs_0;
	t27p06_spi_sdi_1;
	t27p06_spi_scl_0;
	sunxi_lcd_delay_us(10);
	t27p06_spi_scl_1;
	sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		t27p06_spi_scl_0;
		if(value & 0x80)
			t27p06_spi_sdi_1;
		else
			t27p06_spi_sdi_0;
		value <<= 1;
		sunxi_lcd_delay_us(10);
		t27p06_spi_scl_0;
		t27p06_spi_scl_1;
	}
	sunxi_lcd_delay_us(10);
	t27p06_spi_cs_1;
}

static void write_spicom(u32 value)
{
	u32 i;
	t27p06_spi_cs_0;
	t27p06_spi_sdi_0;
	t27p06_spi_scl_0;
	sunxi_lcd_delay_us(10);
	t27p06_spi_scl_1; 
	sunxi_lcd_delay_us(10);
	for(i=0;i<8;i++)
	{
		sunxi_lcd_delay_us(10);
		t27p06_spi_scl_0;
		if(value & 0x80)
			t27p06_spi_sdi_1;
		else
			t27p06_spi_sdi_0;
		
		t27p06_spi_scl_0; 
		sunxi_lcd_delay_us(10);
		t27p06_spi_scl_1;
		value <<= 1;
		
	}
	sunxi_lcd_delay_us(10);
	t27p06_spi_cs_1;
}


//#define write_spicom write_spicom
//#define	write_spidat write_spidat

static void LCD_panel_init(u32 sel)
{

        #if 0

///3.1.1.  Initial setting for BF050WVQ-100(480x854 IPS Column) Panel 
//Resolution: 480x854 
//External system porch setting: VS=4, VBP=5, VFP=6, HS=10, HBP=10, HFP=10 
//Frame rate: 60HZ  
//VCI=2.8V, IOVCC=1.8V 

write_spicom(0xBF); // Password 
write_spidat(0x91); // 
write_spidat(0x61); // 
write_spidat(0xF2); // 
 

write_spicom(0xB3); //VCOM 
write_spidat(0x00); // 
write_spidat(0x69); // 
 

write_spicom(0xB4); //VCOM_R 
write_spidat(0x00); // 
write_spidat(0x69); // 
 

write_spicom(0xB8); //VGMP, VGSP, VGMN, VGSN 
write_spidat(0x00); // 
write_spidat(0x9F); //VGMP[7:0] // 
write_spidat(0x01); //VGSP[7:0] // 
write_spidat(0x00); // 
write_spidat(0x9F); //VGMN[7:0] // 
write_spidat(0x01); //VGSN[7:0] // 
 

write_spicom(0xBA); //GIP output voltage level. 
write_spidat(0x34); //VGH_REG[6:0] 
write_spidat(0x23); //VGL_REG[5:0] 
write_spidat(0x00); // 

write_spicom(0xC3); //SET RGB CYC 
write_spidat(0x04); //Column 
 

write_spicom(0xC4); //SET TCON 
write_spidat(0x30); // 
write_spidat(0x6A); //854 LINE 
 

write_spicom(0xC7); //POWER CTRL 
write_spidat(0x00); //DCDCM[3:0] 
write_spidat(0x01); //AVDD_RT[1:0] 
write_spidat(0x32); // 
write_spidat(0x05); // 
write_spidat(0x65); // 
write_spidat(0x2C); // 
write_spidat(0x13); // 
write_spidat(0xA5); // 
write_spidat(0xA5); // 

#if 1
//Gamma 2.2  
write_spicom(0xC8); //Gamma 
write_spidat(0x60); // 
write_spidat(0x4E); // 
write_spidat(0x3F); // 
write_spidat(0x34); // 
write_spidat(0x33); // 
write_spidat(0x25); // 
write_spidat(0x2B); // 
write_spidat(0x16); // 
write_spidat(0x30); // 
write_spidat(0x2F); // 
write_spidat(0x30); // 
write_spidat(0x4E); // 

write_spidat(0x3C); // 
write_spidat(0x44); // 
write_spidat(0x36); // 
write_spidat(0x34); // 
write_spidat(0x2A); // 
write_spidat(0x23); // 
write_spidat(0x1F); // 
write_spidat(0x60); // 
write_spidat(0x4E); // 
write_spidat(0x3F); // 
write_spidat(0x34); // 
write_spidat(0x33); // 
write_spidat(0x25); // 
write_spidat(0x2B); // 
write_spidat(0x16); // 
write_spidat(0x30); // 
write_spidat(0x2F); // 
write_spidat(0x30); // 
write_spidat(0x4E); // 
write_spidat(0x3C); // 
write_spidat(0x44); // 
write_spidat(0x36); // 
write_spidat(0x34); // 
write_spidat(0x2A); // 
write_spidat(0x23); // 
write_spidat(0x1F); // 
 
#endif

write_spicom(0xD4); //CGOUTx_L GS=0 
write_spidat(0x1F); // 
write_spidat(0x1E); // 
write_spidat(0x05); // 
write_spidat(0x07); // 
write_spidat(0x01); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 

write_spicom(0xD5); //CGOUTx_R GS=0 
write_spidat(0x1F); // 
write_spidat(0x1E); // 
write_spidat(0x04); // 
write_spidat(0x06); // 
write_spidat(0x00); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 

write_spicom(0xD6); //CGOUTx_L GS=1 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x06); // 
write_spidat(0x04); // 
write_spidat(0x00); // 
write_spidat(0x1E); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 

write_spicom(0xD7); //CGOUTx_R GS=1 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x07); // 
write_spidat(0x05); // 
write_spidat(0x01); // 
write_spidat(0x1E); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
write_spidat(0x1F); // 
 

write_spicom(0xD8); //SETGIP1 
write_spidat(0x20); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x10); // 
write_spidat(0x03); // 
write_spidat(0x10); // 
write_spidat(0x01); // 
write_spidat(0x02); // 
write_spidat(0x00); // 
write_spidat(0x01); // 
write_spidat(0x02); // 
write_spidat(0x03); // 
write_spidat(0x7b); // 
 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x31); // 
write_spidat(0x04); // 
write_spidat(0x06); // 
write_spidat(0x7b); // 
write_spidat(0x08); // 
 

write_spicom(0xD9); // SETGIP2 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x88); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x06); // 
write_spidat(0x7b); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x3B); // 
write_spidat(0x2F); // 
write_spidat(0x1F); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x00); // 
write_spidat(0x03); // 
write_spidat(0x7b); // 
 

write_spicom(0x11); // SLPOUT 
 
mdelay(120); 

write_spicom(0x29); // DSPON 
mdelay(30); 


#endif

	return;
}

static void LCD_panel_exit(u32 sel)
{
	return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
	return 0;
}

struct __lcd_panel default_panel = {
	/* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
	.name = "default_lcd",
	.func = {
		.cfg_panel_info = LCD_cfg_panel_info,
		.cfg_open_flow = LCD_open_flow,
		.cfg_close_flow = LCD_close_flow,
		.lcd_user_defined_func = LCD_user_defined_func,
	},
};
