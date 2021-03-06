/****************************************************************
 *								*
 *	Copyright 2001, 2010 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"
#include "gdsroot.h"
#include "gtm_facility.h"
#include "fileinfo.h"
#include "gdsbt.h"
#include "gdsfhead.h"
#include "stringpool.h"
#include "op.h"
#include "gvcst_protos.h"	/* for gvcst_order prototype */
#include "gvsub2str.h"
#include "gvcmx.h"
#include "gvusr.h"

GBLREF gv_namehead	*gv_target;
GBLREF gv_key		*gv_currkey;
GBLREF gv_key		*gv_altkey;
GBLREF spdesc		stringpool;
GBLREF gd_region	*gv_cur_region;

/****************** SHOULD BE IN .H FILES !!!!!!!! ***********************/
#define MAX_SUBSC_LEN 255
#define MAX_NUM_SLEN 128
/*************************************************************************/

void op_gvnext(mval *v)
{
	register char		*c;
	boolean_t		found;
 	int4			n;
	enum db_acc_method 	acc_meth;
	DCL_THREADGBL_ACCESS;

	SETUP_THREADGBL_ACCESS;
	acc_meth = gv_cur_region->dyn.addr->acc_meth;
	/* if the lowest subscript is -1, then make it null */
	if ( 	(gv_currkey->end == gv_currkey->prev + 4) &&
		*(&gv_currkey->base[0] + gv_currkey->prev) == 0x40 &&
		*(&gv_currkey->base[0] + gv_currkey->prev + 1) == 0xEE &&
		*(&gv_currkey->base[0] + gv_currkey->prev + 2) == 0xFF)

	{
		*(&gv_currkey->base[0] + gv_currkey->prev) = 01;
		*(char *)(&gv_currkey->base[0] + gv_currkey->prev + 2) = 0;
		if (0 == gv_cur_region->std_null_coll)
		{
			*(char *)(&gv_currkey->base[0] + gv_currkey->prev + 1) = 0;
			gv_currkey->end -= 2;
		} else
		{
			*(char *)(&gv_currkey->base[0] + gv_currkey->prev + 1) = 1;
			*(char *)(&gv_currkey->base[0] + gv_currkey->prev + 3) = 0;
			gv_currkey->end --;
		}
	} else
	{
		if (!TREF(gv_last_subsc_null) || gv_cur_region->std_null_coll )
		{
			*(&gv_currkey->base[0] + gv_currkey->end - 1) = 1;
			*(&gv_currkey->base[0] + gv_currkey->end + 1) = 0;
			gv_currkey->end += 1;
		} else
			*(&gv_currkey->base[0] + gv_currkey->prev) = 01;
	}

	if (acc_meth == dba_bg || acc_meth == dba_mm)
	{
		if (gv_target->root)
			found = gvcst_order();
		else
			found = FALSE;		/* global does not exist */
	} else if (acc_meth == dba_cm)
		found = gvcmx_order();
	else
		found = gvusr_order();

	v->mvtype = 0; /* so stp_gcol (if invoked below) can free up space currently occupied by this to-be-overwritten mval */
	if (!found)
	{
		ENSURE_STP_FREE_SPACE(2);
		c = v->str.addr = (char *) stringpool.free;
		*c++ = '-';
		*c = '1';
	 	v->str.len = 2;
 		stringpool.free += 2;
	} else
	{
		gv_altkey->prev = gv_currkey->prev;
 		if (stringpool.top - stringpool.free < MAX_SUBSC_LEN)
		{
			if (*(&gv_altkey->base[0] + gv_altkey->prev) != 0xFF)
				n = MAX_NUM_SLEN;
			else
			{
				n = gv_altkey->top - gv_altkey->prev;
				assert (n > 0);
			}
			ENSURE_STP_FREE_SPACE(n);
		}
		v->str.addr = (char *) stringpool.free;
		c = (char *)(&gv_altkey->base[0] + gv_altkey->prev);
		stringpool.free = gvsub2str ((uchar_ptr_t)c,stringpool.free, FALSE);
		v->str.len = INTCAST(stringpool.free - (unsigned char *) v->str.addr);
		assert (v->str.addr < (char *) stringpool.top && v->str.addr >= (char *) stringpool.base);
		assert (v->str.addr + v->str.len <= (char *) stringpool.top &&
			v->str.addr + v->str.len >= (char *) stringpool.base);
	}
	v->mvtype = MV_STR; /* initialize mvtype now that mval has been otherwise completely set up */
	return;
}
