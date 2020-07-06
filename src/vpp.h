/*
  vpp.h [ vpp / VHDL PreProcessor ]
  Copyright (c) Takashige Sugie <takashige@users.sourceforge.net>, All rights reserved.
  Last update: <2007-12-02 20:21:51>
*/

/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef	__TAKA_VPP_H__
#define	__TAKA_VPP_H__

#include <stdio.h>
#include "taka/list.h"

#ifdef __cplusplus
extern "C" {
#endif

/* error codes */
#define	EVPPORIG		1	/* Can't open an original file */
#define	EVPPCONV		2	/* Can't open a converted file */
#define	EVPPNOENDIF		3	/* #ifdef is NOT closed by #endif */
#define	EVPPNOENDFOR		4	/* #for is NOT closed by #endfor */
#define	EVPPMESERR		16	/* #message error */
#define	EVPPMESCRIT		17	/* #message critical */
#define	EVPPMESALERT		18	/* #message alert */
#define	EVPPMESEMERG		19	/* #message emergency */
#define	EVPPMTORIG		32	/* Can't open an original file for module template */
#define	EVPPMTCONV		33	/* Can't open a converted file for module template */

/* message prefix */
#define	VPP_MES_DEBUG		"DEBUG"
#define	VPP_MES_INFO		"INFO"
#define	VPP_MES_NOTICE		"NOTICE"
#define	VPP_MES_WARN		"WARN"
#define	VPP_MES_ERR		"ERR"
#define	VPP_MES_CRIT		"CRIT"
#define	VPP_MES_ALERT		"ALERT"
#define	VPP_MES_EMERG		"EMERG"

/* default name */
#define	VPP_TEMPLATE_MODULE	"module.vhd"
#define	VPP_TEMPLATE_PACKAGE	"package.vhd"
#define	VPP_TEMPLATE_SIMULATION	"simulation.vhd"
#define	VPP_TEMPLATE_ENTITY	"entity"
#define	VPP_TEMPLATE_COMPONENT	"component"
#define	VPP_TEMPLATE_GENERIC	"generic"
#define	VPP_TEMPLATE_PORT	"port"
#define	VPP_TEMPLATE_IN		"in"
#define	VPP_TEMPLATE_OUT	"out"
#define	VPP_TEMPLATE_INOUT	"inout"

/* scans behavior flags */
#define VPP_BEHAVIOR_TEMPLATE	0x00000001

  int taka_tvpp_convert(const char *, const char *, tlist_t *, const char, FILE *);
  tlist_t * taka_tvpp_define_label(tlist_t *, const char *);
  int taka_tvpp_create_template(const char *, const char, FILE *);

#ifdef __cplusplus
}
#endif

#endif // __TAKA_VPP_H__
