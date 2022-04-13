/*
*********************************************************************************************************
*                                                    LINUX-KERNEL
*                                        AllWinner Linux Platform Develop Kits
*                                                   Kernel Module
*
*                                    (c) Copyright 2006-2011, kevin.z China
*                                             All Rights Reserved
*
* File    : super_i.h
* By      : kevin.z
* Version : v1.0
* Date    : 2011-5-30 17:21
* Descript:
* Update  : date                auther      ver     notes
*********************************************************************************************************
*/
#ifndef __SUPER_I_H__
#define __SUPER_I_H__

#include "../../pm_config.h"
#include "../../pm_types.h"
#include "../../pm.h"

#include "super_cfg.h"
#include "common.h"
#include "super_clock.h"
#include "super_power.h"
#include "super_twi.h"
#include "super_cpus.h"

//------------------------------------------------------------------------------
//return value defines
//------------------------------------------------------------------------------
#define	OK		(0)
#define	FAIL	(-1)
#define TRUE	(1)
#define	FALSE	(0)

// #define NULL	(0)

#define readb(addr)		(*((volatile unsigned char  *)(addr)))
#define readw(addr)		(*((volatile unsigned short *)(addr)))
#define readl(addr)		(*((volatile unsigned long  *)(addr)))
#define writeb(v, addr)	(*((volatile unsigned char  *)(addr)) = (unsigned char)(v))
#define writew(v, addr)	(*((volatile unsigned short *)(addr)) = (unsigned short)(v))
#define writel(v, addr)	(*((volatile unsigned long  *)(addr)) = (unsigned long)(v))

typedef signed char		  __s8;
typedef unsigned char		  __u8;
typedef short int		  __s16;
typedef unsigned short		  __u16;
typedef int			  __s32;
typedef unsigned int		  __u32;


#define RESUMEX_MAGIC             "eGON.BT0"
#define STAMP_VALUE               0x5F0A6C39
#define RESUME_FILE_HEAD_VERSION  "1100"     // X.X.XX
#define RESUME_VERSION            "1100"     // X.X.XX
#define PLATFORM                  "1633"

#define NF_ALIGN_SIZE			1024
#define RESUMEX_ALIGN_SIZE  		NF_ALIGN_SIZE
#define RESUME_PUB_HEAD_VERSION         "1100"    // X.X.XX
#define RESUMEX_FILE_HEAD_VERSION       "1230"    // X.X.XX
#define RESUMEX_VERSION                 "1230"    // X.X.XX
#define EGON_VERSION                    "1100"    // X.X.XX

/******************************************************************************/
/*                              file head of Resume                             */
/******************************************************************************/
typedef struct _Resume_file_head
{
	__u32  jump_instruction;   // one intruction jumping to real code
	__u8   magic[8];           // ="eGON.BT0" or "eGON.BT1",  not C-style string.
	__u32  check_sum;          // generated by PC
	__u32  length;             // generated by PC
	__u32  pub_head_size;      // the size of resume_file_head_t
	__u8   pub_head_vsn[4];    // the version of resume_file_head_t
	__u8   file_head_vsn[4];   // the version of resume0_file_head_t or resume1_file_head_t
	__u8   Resume_vsn[4];      // Resume version
	__u8   eGON_vsn[4];        // eGON version
	__u8   platform[8];        // platform information
}resume_file_head_t;


extern const resume_file_head_t  resume_head;

extern struct aw_pm_info  pm_info;
extern struct aw_mem_para mem_para_info;

extern int resume1_c_part(void);
extern void set_pll( void );

#endif  //__SUPER_I_H__

