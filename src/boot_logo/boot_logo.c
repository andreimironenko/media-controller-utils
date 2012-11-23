#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define VPDMA_LIST_ATTR                 0x4810D008
#define VPDMA_LIST_STAT_SYNC            0x4810D00C
#define VPDMA_TOTAL_LIST_NO             8

#ifdef TI814X_BUILD
#define TI814x_HDMI_MUX_ADDR            0x481C52C8
#define CM_HDVPSS_HDVPSS_CLK_CTRL       0x48180820
#define VPS_MODULE_CLK                  CM_HDVPSS_HDVPSS_CLK_CTRL
#define STOP_CLKSRC_MUX()               ti814x_stop_clksrc_mux()
#endif

#ifdef TI816X_BUILD
#define CM_ACTIVE_HDDSS_CLKCTRL         0x48180424
#define VPS_MODULE_CLK                  CM_ACTIVE_HDDSS_CLKCTRL
#define STOP_CLKSRC_MUX()
#endif

void vpdma_stop_list(void)
{
	u32 list_stat_value,list_attr_value;
	u32 reg_base;
	int vpdma_list_no = -1;
	int i;
	
	/* check which list number for vpdma is busy */
	reg_base = (u32)ioremap(VPDMA_LIST_STAT_SYNC, 0x10);
	list_stat_value = __raw_readl(reg_base);
	iounmap((u32 *)VPDMA_LIST_STAT_SYNC);
	for (i = 0; i < VPDMA_TOTAL_LIST_NO; i++)
	{	
		if ( (list_stat_value && (0x10000 << i)) == 1)
		{	
			vpdma_list_no = i;
			break;
		}
	}
	if (vpdma_list_no != -1)
	{
		/* stop the vpdma list with the stop bit for the required list number */
		reg_base = (u32)ioremap(VPDMA_LIST_ATTR, 0x10);
		list_attr_value = __raw_readl(reg_base);
		printk(KERN_INFO "Stopping the logo\n");
		list_attr_value = list_attr_value | (vpdma_list_no<<24)  /* list number */
				 | (1<<20)  /* stop bit for the list */;
		__raw_writel(list_attr_value, reg_base);
		iounmap((u32 *)VPDMA_LIST_ATTR);
	}
}

#ifdef TI814X_BUILD
/* This mux is for configuring the pixel clock to Venc through HDMI or PLL */
void ti814x_stop_clksrc_mux(void)
{
	u32 hdmi_mux_value;
	u32 reg_base;
	reg_base = (u32)ioremap(TI814x_HDMI_MUX_ADDR, 0x10);
	hdmi_mux_value = __raw_readl(reg_base);
	hdmi_mux_value &= 0xFFFFFFFE;
	__raw_writel(hdmi_mux_value, reg_base);
	iounmap((u32 *)TI814x_HDMI_MUX_ADDR);
}
#endif

static int boot_logo_init(void) 
{
	u32 vps_clk_value;
	u32 reg_base;
	
	/* check whether the vps is enabled or not in dm814x */
	reg_base = (u32)ioremap(VPS_MODULE_CLK, 0x10);
	vps_clk_value = __raw_readl(reg_base);	
	iounmap((u32 *)VPS_MODULE_CLK);

	if (vps_clk_value == 2)
	{
		vpdma_stop_list();
		STOP_CLKSRC_MUX();
	}
	
	return 0;
}

void boot_logo_exit(void) 
{
	printk(KERN_INFO "Stopping the logo module\n");
}

MODULE_LICENSE("GPL");
module_init(boot_logo_init);
module_exit(boot_logo_exit);
