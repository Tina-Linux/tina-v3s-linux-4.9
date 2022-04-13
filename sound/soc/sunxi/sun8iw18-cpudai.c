/*
 * sound\soc\sunxi\sun8iw18-cpudai.c
 * (C) Copyright 2014-2019
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * wolfgang huang <huangjinhui@allwinnertech.com>
 * yumingfeng <yumingfeng@allwinnertech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/dma/sunxi-dma.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include "sunxi-pcm.h"

#include "sunxi-daudio.h"

#if defined(CONFIG_ARCH_SUN8IW18)
#include "sun8iw18-codec.h"
#include "sun8iw18-sndcodec.h"
#endif

#ifdef CONFIG_SND_SUNXI_MAD
#include "sunxi-mad.h"
#endif

#define DRV_NAME "sunxi-internal-cpudai"

struct sunxi_cpudai_info {
	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;
#ifdef SUNXI_CODEC_MAP_TO_DAUDIO
#ifdef CONFIG_SND_SUNXI_MAD
	unsigned int mad_bind;
	struct sunxi_codec_info *sunxi_codec;
#endif
#endif
};

static int sunxi_cpudai_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	struct sunxi_cpudai_info *sunxi_cpudai = snd_soc_dai_get_drvdata(dai);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		snd_soc_dai_set_dma_data(dai, substream,
					&sunxi_cpudai->playback_dma_param);
	else {
#ifdef SUNXI_CODEC_MAP_TO_DAUDIO
		struct snd_soc_pcm_runtime *rtd = substream->private_data;
		struct sunxi_codec_info *sunxi_codec =
				snd_soc_dai_get_drvdata(rtd->codec_dai);
		int DRQSRC_TYPE_ID = 0;
		int DMA_RXFIFO_ADDR = 0;
#ifdef CONFIG_SND_SUNXI_MAD
		struct sunxi_sndcodec_priv *sndcodec_priv =
				snd_soc_card_get_drvdata(rtd->card);
		struct sunxi_mad_priv *mad_priv = &(sndcodec_priv->mad_priv);

		sunxi_cpudai->mad_bind = mad_priv->mad_bind;
		sunxi_cpudai->sunxi_codec = sunxi_codec;
#endif

		switch (sunxi_codec->i2s_port) {
		case SUNXI_CODEC_MAP_PORT_NULL:
		default:
			DRQSRC_TYPE_ID = DRQSRC_AUDIO_CODEC;
			DMA_RXFIFO_ADDR = sunxi_codec->digital_res.start +
						SUNXI_ADC_RXDATA;
			sunxi_codec->sunxi_daudio = NULL;
			break;
		case SUNXI_CODEC_MAP_PORT_I2S0:
			DRQSRC_TYPE_ID = DRQSRC_DAUDIO_0_RX;
			DMA_RXFIFO_ADDR = sunxi_codec->daudio[0].res.start +
						SUNXI_DAUDIO_RXFIFO;
			sunxi_codec->sunxi_daudio = &(sunxi_codec->daudio[0]);
			break;
		case SUNXI_CODEC_MAP_PORT_I2S1:
			DRQSRC_TYPE_ID = DRQSRC_DAUDIO_1_RX;
			DMA_RXFIFO_ADDR = sunxi_codec->daudio[1].res.start +
						SUNXI_DAUDIO_RXFIFO;
			sunxi_codec->sunxi_daudio = &(sunxi_codec->daudio[1]);
			break;
		case SUNXI_CODEC_MAP_PORT_I2S2:
			DRQSRC_TYPE_ID = DRQSRC_DAUDIO_2_RX;
			DMA_RXFIFO_ADDR = sunxi_codec->daudio[2].res.start +
						SUNXI_DAUDIO_RXFIFO;
			sunxi_codec->sunxi_daudio = &(sunxi_codec->daudio[2]);
			break;
		}

#ifdef CONFIG_SND_SUNXI_MAD
		if (mad_priv->mad_bind == 1)
			sunxi_sram_dma_config(&(sunxi_cpudai->capture_dma_param));
		else
#endif
		{
			sunxi_cpudai->capture_dma_param.dma_addr = DMA_RXFIFO_ADDR;
			sunxi_cpudai->capture_dma_param.dma_drq_type_num = DRQSRC_TYPE_ID;
		}
		sunxi_cpudai->capture_dma_param.src_maxburst = 8;
		sunxi_cpudai->capture_dma_param.dst_maxburst = 8;
#endif
		snd_soc_dai_set_dma_data(dai, substream,
				&sunxi_cpudai->capture_dma_param);
	}

	return 0;
}

static int sunxi_cpudai_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	return 0;
}

extern void sunxi_codec_mad_enter_standby(struct sunxi_codec_info *sunxi_codec);

static int sunxi_cpudai_suspend(struct snd_soc_dai *dai)
{
#ifdef SUNXI_CODEC_MAP_TO_DAUDIO
#ifdef CONFIG_SND_SUNXI_MAD
	struct sunxi_cpudai_info *sunxi_cpudai = snd_soc_dai_get_drvdata(dai);

	if (sunxi_cpudai->mad_bind == 1) {
		sunxi_codec_mad_enter_standby(sunxi_cpudai->sunxi_codec);
		sunxi_mad_suspend_external();
#ifdef CONFIG_SUNXI_AUDIO_DEBUG
		show_audio_all_reg(sunxi_cpudai->sunxi_codec);
#endif
		pr_warn("[%s] mad suspend succeed!\n", __func__);
		return 0;
	}
#endif
#endif
	return 0;
}

static int sunxi_cpudai_resume(struct snd_soc_dai *dai)
{
#ifdef SUNXI_CODEC_MAP_TO_DAUDIO
#ifdef CONFIG_SND_SUNXI_MAD
	struct sunxi_cpudai_info *sunxi_cpudai = snd_soc_dai_get_drvdata(dai);

	if (sunxi_cpudai->mad_bind == 1) {
		sunxi_mad_resume_external();
		pr_warn("mad resume succeed %s\n", __func__);
		return 0;
	}
#endif
#endif
	return 0;
}

static void sunxi_cpudai_shutdown(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
#ifdef SUNXI_CODEC_MAP_TO_DAUDIO
#ifdef CONFIG_SND_SUNXI_MAD
	struct sunxi_cpudai_info *sunxi_cpudai = snd_soc_dai_get_drvdata(dai);

	sunxi_cpudai->mad_bind = 0;
#endif
#endif
}

static struct snd_soc_dai_ops sunxi_cpudai_dai_ops = {
	.startup = sunxi_cpudai_startup,
	.hw_params = sunxi_cpudai_hw_params,
	.shutdown = sunxi_cpudai_shutdown,
};

static struct snd_soc_dai_driver sunxi_cpudai_dai = {
	.suspend = sunxi_cpudai_suspend,
	.resume = sunxi_cpudai_resume,
	.playback = {
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_192000 |
			SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE |
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.capture = {
		.channels_min = 1,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_8000_48000 |
			SNDRV_PCM_RATE_KNOT,
		.formats = SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S24_LE |
			SNDRV_PCM_FMTBIT_S32_LE,
	},
	.ops		= &sunxi_cpudai_dai_ops,

};

static const struct snd_soc_component_driver sunxi_asoc_cpudai_component = {
	.name = DRV_NAME,
};
static const struct of_device_id sunxi_asoc_cpudai_of_match[] = {
	{ .compatible = "allwinner,sunxi-internal-cpudai", },
	{},
};

static int  sunxi_asoc_cpudai_dev_probe(struct platform_device *pdev)
{
	struct resource res;
	struct sunxi_cpudai_info *sunxi_cpudai;
	struct device_node *np = pdev->dev.of_node;
	int ret;

	sunxi_cpudai = devm_kzalloc(&pdev->dev,
			sizeof(struct sunxi_cpudai_info), GFP_KERNEL);
	if (!sunxi_cpudai) {
		ret = -ENOMEM;
		goto err_node_put;
	}
	dev_set_drvdata(&pdev->dev, sunxi_cpudai);

	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		dev_err(&pdev->dev, "Can't parse device node resource\n");
		ret = -ENODEV;
		goto err_devm_kfree;
	}

	sunxi_cpudai->playback_dma_param.dma_addr = res.start+SUNXI_DAC_TXDATA;
	sunxi_cpudai->playback_dma_param.dma_drq_type_num = DRQDST_AUDIO_CODEC;
	sunxi_cpudai->playback_dma_param.dst_maxburst = 4;
	sunxi_cpudai->playback_dma_param.src_maxburst = 4;

	sunxi_cpudai->capture_dma_param.dma_addr = res.start+SUNXI_ADC_RXDATA;
	sunxi_cpudai->capture_dma_param.dma_drq_type_num = DRQSRC_AUDIO_CODEC;
	sunxi_cpudai->capture_dma_param.src_maxburst = 4;
	sunxi_cpudai->capture_dma_param.dst_maxburst = 4;

	ret = snd_soc_register_component(&pdev->dev, &sunxi_asoc_cpudai_component,
					&sunxi_cpudai_dai, 1);
	if (ret) {
		dev_err(&pdev->dev, "Could not register DAI: %d\n", ret);
		ret = -EBUSY;
		goto err_devm_kfree;
	}

	ret = asoc_dma_platform_register(&pdev->dev, 0);
	if (ret) {
		dev_err(&pdev->dev, "Could not register PCM: %d\n", ret);
		goto err_unregister_component;
	}

	return 0;

err_unregister_component:
	snd_soc_unregister_component(&pdev->dev);
err_devm_kfree:
	devm_kfree(&pdev->dev, sunxi_cpudai);
err_node_put:
	of_node_put(np);
	return ret;
}

static int __exit sunxi_asoc_cpudai_dev_remove(struct platform_device *pdev)
{
	struct sunxi_cpudai_info *sunxi_cpudai = dev_get_drvdata(&pdev->dev);

	asoc_dma_platform_unregister(&pdev->dev);
	snd_soc_unregister_component(&pdev->dev);
	devm_kfree(&pdev->dev, sunxi_cpudai);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sunxi_asoc_cpudai_driver = {
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.of_match_table = sunxi_asoc_cpudai_of_match,
	},
	.probe = sunxi_asoc_cpudai_dev_probe,
	.remove = __exit_p(sunxi_asoc_cpudai_dev_remove),
};

module_platform_driver(sunxi_asoc_cpudai_driver);

MODULE_AUTHOR("wolfgang huang <huangjinhui@allwinnertech.com>");
MODULE_DESCRIPTION("SUNXI Internal cpudai ASoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
