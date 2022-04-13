/*
 * SDIO Classes, Interface Types, Manufacturer IDs, etc.
 */

#ifndef LINUX_MMC_SDIO_IDS_H
#define LINUX_MMC_SDIO_IDS_H

/*
 * Standard SDIO Function Interfaces
 */

#define SDIO_CLASS_NONE		0x00	/* Not a SDIO standard interface */
#define SDIO_CLASS_UART		0x01	/* standard UART interface */
#define SDIO_CLASS_BT_A		0x02	/* Type-A BlueTooth std interface */
#define SDIO_CLASS_BT_B		0x03	/* Type-B BlueTooth std interface */
#define SDIO_CLASS_GPS		0x04	/* GPS standard interface */
#define SDIO_CLASS_CAMERA	0x05	/* Camera standard interface */
#define SDIO_CLASS_PHS		0x06	/* PHS standard interface */
#define SDIO_CLASS_WLAN		0x07	/* WLAN interface */
#define SDIO_CLASS_ATA		0x08	/* Embedded SDIO-ATA std interface */
#define SDIO_CLASS_BT_AMP	0x09	/* Type-A Bluetooth AMP interface */

/*
 * Vendors and devices.  Sort key: vendor first, device next.
 */
#define SDIO_VENDOR_ID_BROADCOM			0x02d0
#define SDIO_DEVICE_ID_BROADCOM_43143		0xa887
#define SDIO_DEVICE_ID_BROADCOM_43241		0x4324
#define SDIO_DEVICE_ID_BROADCOM_4329		0x4329
#define SDIO_DEVICE_ID_BROADCOM_4330		0x4330
#define SDIO_DEVICE_ID_BROADCOM_4334		0x4334
#define SDIO_DEVICE_ID_BROADCOM_43340		0xa94c
#define SDIO_DEVICE_ID_BROADCOM_43341		0xa94d
#define SDIO_DEVICE_ID_BROADCOM_4335_4339	0x4335
#define SDIO_DEVICE_ID_BROADCOM_4339		0x4339
#define SDIO_DEVICE_ID_BROADCOM_43362		0xa962
#define SDIO_DEVICE_ID_BROADCOM_43364		0xa9a4
#define SDIO_DEVICE_ID_BROADCOM_43430		0xa9a6
#define SDIO_DEVICE_ID_BROADCOM_4345		0x4345
#define SDIO_DEVICE_ID_BROADCOM_4354		0x4354
#define SDIO_DEVICE_ID_BROADCOM_4356		0x4356

#define SDIO_VENDOR_ID_INTEL			0x0089
#define SDIO_DEVICE_ID_INTEL_IWMC3200WIMAX	0x1402
#define SDIO_DEVICE_ID_INTEL_IWMC3200WIFI	0x1403
#define SDIO_DEVICE_ID_INTEL_IWMC3200TOP	0x1404
#define SDIO_DEVICE_ID_INTEL_IWMC3200GPS	0x1405
#define SDIO_DEVICE_ID_INTEL_IWMC3200BT		0x1406
#define SDIO_DEVICE_ID_INTEL_IWMC3200WIMAX_2G5	0x1407

#define SDIO_VENDOR_ID_MARVELL			0x02df
#define SDIO_DEVICE_ID_MARVELL_LIBERTAS		0x9103
#define SDIO_DEVICE_ID_MARVELL_8688WLAN		0x9104
#define SDIO_DEVICE_ID_MARVELL_8688BT		0x9105

#define SDIO_VENDOR_ID_SIANO			0x039a
#define SDIO_DEVICE_ID_SIANO_NOVA_B0		0x0201
#define SDIO_DEVICE_ID_SIANO_NICE		0x0202
#define SDIO_DEVICE_ID_SIANO_VEGA_A0		0x0300
#define SDIO_DEVICE_ID_SIANO_VENICE		0x0301
#define SDIO_DEVICE_ID_SIANO_NOVA_A0		0x1100
#define SDIO_DEVICE_ID_SIANO_STELLAR 		0x5347

#endif /* LINUX_MMC_SDIO_IDS_H */
