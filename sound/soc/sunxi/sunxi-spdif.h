/*
 * sound\soc\sunxi\spdif\sunxi-spdif.h
 * (C) Copyright 2010-2016
 * AllWinner Technology Co., Ltd. <www.allwinnertech.com>
 * wolfgang huang <huangjinhui@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#ifndef	__SUNXI_SPDIF_H_
#define	__SUNXI_SPDIF_H_

#if defined(CONFIG_ARCH_SUN8IW10) || defined(CONFIG_ARCH_SUN50IW6) || \
	defined(CONFIG_ARCH_SUN50IW8) || defined(CONFIG_ARCH_SUN50IW9) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define SPDIF_LOOPBACK_DEBUG
#else
#undef SPDIF_LOOPBACK_DEBUG
#endif

#if defined(CONFIG_ARCH_SUN8IW18)
#define SPDIF_PLL_AUDIO_X4
#else
#undef SPDIF_PLL_AUDIO_X4
#endif

#undef SPDIF_PINCTRL_STATE_DEFAULT_B

/* SPDIF register definition */
#define	SUNXI_SPDIF_CTL		0x00
#define	SUNXI_SPDIF_TXCFG	0x04
#define	SUNXI_SPDIF_RXCFG	0x08
#if	defined(CONFIG_ARCH_SUN9IW1) || \
	defined(CONFIG_ARCH_SUN8IW6) || \
	defined(CONFIG_ARCH_SUN8IW7) || \
	defined(CONFIG_ARCH_SUN50I) || \
	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN8IW11) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define SUNXI_SPDIF_TXFIFO (0x20)
#else
#define SUNXI_SPDIF_TXFIFO (0x0C)
#endif
#define	SUNXI_SPDIF_RXFIFO	0x10
#define	SUNXI_SPDIF_FIFO_CTL	0x14
#define	SUNXI_SPDIF_FIFO_STA	0x18
#define	SUNXI_SPDIF_INT		0x1C
#if	defined(CONFIG_ARCH_SUN9IW1) || \
	defined(CONFIG_ARCH_SUN8IW6) || \
	defined(CONFIG_ARCH_SUN8IW7) || \
	defined(CONFIG_ARCH_SUN50I) || \
	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN8IW11) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define SUNXI_SPDIF_INT_STA (0x0C)
#else
#define SUNXI_SPDIF_INT_STA (0x20)
#endif
#define	SUNXI_SPDIF_TXCNT	0x24
#define	SUNXI_SPDIF_RXCNT	0x28
#define	SUNXI_SPDIF_TXCH_STA0	0x2C
#define	SUNXI_SPDIF_TXCH_STA1	0x30
#define	SUNXI_SPDIF_RXCH_STA0	0x34
#define	SUNXI_SPDIF_RXCH_STA1	0x38

/* SUNXI_SPDIF_CTL register */
#define	CTL_RESET		0
#define	CTL_GEN_EN		1
#if	defined(CONFIG_ARCH_SUN8IW1)
#define	CTL_MCLKOUTEN		2
#elif	defined(CONFIG_ARCH_SUN50IW6) || defined(CONFIG_ARCH_SUN8IW17)
#define CTL_MCLKOOUTEN		3
#endif
#if	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN50IW6) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN50IW9) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define	CTL_LOOP_EN		2
#endif
#if	defined(CONFIG_ARCH_SUN8IW1) || \
	defined(CONFIG_ARCH_SUN8IW6) || \
	defined(CONFIG_ARCH_SUN8IW7) || \
	defined(CONFIG_ARCH_SUN50I) || \
	defined(CONFIG_ARCH_SUN8IW11)
#define	CTL_MCLKDIV		4
#elif defined(CONFIG_ARCH_SUN50IW6) || \
      defined(CONFIG_ARCH_SUN8IW17)
#define	CTL_MCLKDIV		5
#endif

/* SUNXI_SPDIF_TXCFG register */
#define	TXCFG_TXEN		0
/* Chan status generated form TX_CHSTA */
#define	TXCFG_CHAN_STA_EN	1
#define	TXCFG_SAMPLE_BIT	2
#define	TXCFG_CLK_DIV_RATIO	4
#define	TXCFG_DATA_TYPE		16
/* Only valid in PCM mode */
#define	TXCFG_ASS		17
#define	TXCFG_SINGLE_MOD	31

/* SUNXI_SPDIF_RXCFG register */
#define	RXCFG_RXEN		0
#define	RXCFG_CHSR_CP		1
#define	RXCFG_CHST_SRC		3
#define	RXCFG_LOCK_FLAG		4

/* SUNXI_SPDIF_FIFO_CTL register */
#define	FIFO_CTL_RXOM		0
#define	FIFO_CTL_TXIM		2
#if	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN50IW6) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define	FIFO_CTL_RXTL		4
#define	FIFO_CTL_TXTL		12
#define	FIFO_CTL_FRX		29
#define	FIFO_CTL_FTX		30
#else
#define	FIFO_CTL_RXTL		3
#define	FIFO_CTL_TXTL		8
#define	FIFO_CTL_FRX		16
#define	FIFO_CTL_FTX		17
#endif
#if	defined(CONFIG_ARCH_SUN9IW1) || \
	defined(CONFIG_ARCH_SUN8IW6) || \
	defined(CONFIG_ARCH_SUN8IW7) || \
	defined(CONFIG_ARCH_SUN50I) || \
	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN8IW11) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define	FIFO_CTL_HUBEN		31
#else
#define	FIFO_CTL_SRC		31
#endif
#if	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN50IW6) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define	CTL_TXTL_MASK		255
#define	CTL_TXTL_DEFAULT	0x40
#define	CTL_RXTL_MASK		127
#define	CTL_RXTL_DEFAULT	0x20
#else
#define	CTL_TXTL_MASK		31
#define	CTL_TXTL_DEFAULT	16
#define	CTL_RXTL_MASK		31
#define	CTL_RXTL_DEFAULT	15
#endif

/* SUNXI_SPDIF_FIFO_STA register */
#define	FIFO_STA_RXA_CNT	0
#if	defined(CONFIG_ARCH_SUN8IW10) || \
	defined(CONFIG_ARCH_SUN50IW6) || \
	defined(CONFIG_ARCH_SUN50IW8) || \
	defined(CONFIG_ARCH_SUN8IW17) || \
	defined(CONFIG_ARCH_SUN8IW18)
#define	FIFO_STA_RXA		15
#define	FIFO_STA_TXA_CNT	16
#define	FIFO_STA_TXE		31
#else
#define	FIFO_STA_RXA		6
#define	FIFO_STA_TXA_CNT	8
#define	FIFO_STA_TXE		14
#endif

/* SUNXI_SPDIF_INT register */
#define	INT_RXAIEN		0
#define	INT_RXOIEN		1
#define	INT_RXDRQEN		2
#define	INT_TXEIEN		4
#define	INT_TXOIEN		5
#define	INT_TXUIEN		6
#define	INT_TXDRQEN		7
#define	INT_RXPAREN		16
#define	INT_RXUNLOCKEN		17
#define	INT_RXLOCKEN		18

/* SUNXI_SPDIF_INT_STA  */
#define	INT_STA_RXA		0
#define	INT_STA_RXO		1
#define	INT_STA_TXE		4
#define	INT_STA_TXO		5
#define	INT_STA_TXU		6
#define	INT_STA_RXPAR		16
#define	INT_STA_RXUNLOCK	17
#define	INT_STA_RXLOCK		18

/* SUNXI_SPDIF_TXCH_STA0 register */
#define	TXCHSTA0_PRO		0
#define	TXCHSTA0_AUDIO		1
#define	TXCHSTA0_CP		2
#define	TXCHSTA0_EMPHASIS	3
#define	TXCHSTA0_MODE		6
#define	TXCHSTA0_CATACOD	8
#define	TXCHSTA0_SRCNUM		16
#define	TXCHSTA0_CHNUM		20
#define	TXCHSTA0_SAMFREQ	24
#define	TXCHSTA0_CLK		28

/* SUNXI_SPDIF_TXCH_STA1 register */
#define	TXCHSTA1_MAXWORDLEN	0
#define	TXCHSTA1_SAMWORDLEN	1
#define	TXCHSTA1_ORISAMFREQ	4
#define	TXCHSTA1_CGMSA		8

/* SUNXI_SPDIF_RXCH_STA0 register */
#define	RXCHSTA0_PRO		0
#define	RXCHSTA0_AUDIO		1
#define	RXCHSTA0_CP		2
#define	RXCHSTA0_EMPHASIS	3
#define	RXCHSTA0_MODE		6
#define	RXCHSTA0_CATACOD	8
#define	RXCHSTA0_SRCNUM		16
#define	RXCHSTA0_CHNUM		20
#define	RXCHSTA0_SAMFREQ	24
#define	RXCHSTA0_CLK		28

/* SUNXI_SPDIF_RXCH_STA1 register */
#define	RXCHSTA1_MAXWORDLEN	0
#define	RXCHSTA1_SAMWORDLEN	1
#define	RXCHSTA1_ORISAMFREQ	4
#define	RXCHSTA1_CGMSA		8
#endif	/* __SUNXI_SPDIF_H_ */
