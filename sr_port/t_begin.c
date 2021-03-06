/****************************************************************
 *								*
 *	Copyright 2001, 2011 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"

#include "gdsroot.h"
#include "gdskill.h"
#include "gtm_facility.h"
#include "fileinfo.h"
#include "gdsbt.h"
#include "gdsfhead.h"
#include "gdsblk.h"
#include "gdscc.h"
#include "filestruct.h"
#include "jnl.h"
#include "buddy_list.h"		/* needed for tp.h */
#include "hashtab_int4.h"	/* needed for tp.h */
#include "tp.h"
#include "t_begin.h"
#include "have_crit.h"

GBLREF	short			crash_count;
GBLREF	trans_num		start_tn;
GBLREF	cw_set_element		cw_set[];
GBLREF	unsigned char		cw_set_depth, cw_map_depth;
GBLREF	unsigned int		t_tries;
GBLREF	uint4			t_err;
GBLREF	uint4			update_trans;
GBLREF	gv_namehead		*gv_target;
GBLREF	sgmnt_addrs		*cs_addrs;
GBLREF	uint4			dollar_tlevel;
GBLREF	jnl_format_buffer	*non_tp_jfb_ptr;
GBLREF	jnl_gbls_t		jgbl;
GBLREF	volatile int4		fast_lock_count;
GBLREF	sgm_info		*first_sgm_info;

void t_begin(uint4 err, uint4 upd_trans) 	/* err --> error code for current gvcst_routine */
{
	srch_blk_status	*s;
	trans_num	histtn;
	DCL_THREADGBL_ACCESS;

	SETUP_THREADGBL_ACCESS;
	assert(!dollar_tlevel); /* if in TP, the T_BEGIN_xxx_NONTP_OR_TP macro should have been used and we will not be here */
	/* any changes to the initialization in the two lines below might need a similar change in T_BEGIN_xxx_NONTP_OR_TP macros */
	assert(INTRPT_OK_TO_INTERRUPT == intrpt_ok_state);
	update_trans = upd_trans;
	t_err = err;

	/* If we use a clue then we must consider the oldest tn in the search history to be the start tn for this transaction */
        /* start_tn manipulation for TP taken care of in tp_hist */
	if (cs_addrs->critical)
		crash_count = cs_addrs->critical->crashcnt;
	start_tn = cs_addrs->ti->curr_tn;
	/* Note: If gv_target was NULL before the start of a transaction and the only operations done inside the transaction
	 * are trigger deletions causing bitmap free operations, we can reach here with gv_target being NULL.
	 */
	if ((NULL != gv_target) && gv_target->clue.end)
	{	/* Since we have a clue, determine if the clue history has lesser transaction numbers and if so use that
		 * as the start tn. Note that we need to take the MIN of all history level tns (see comment in tp_tend.c
		 * in valid_thru processing logic for why).
		 */
		for (s = &gv_target->hist.h[0]; s->blk_num; s++)
		{
			histtn = s->tn;
			if (start_tn > histtn)
				start_tn = histtn;
		}
	}
	cw_set_depth = 0;
	cw_map_depth = 0;
	/* since this is mainline code and we know fast_lock_count should be 0 at this point reset it just in case it is not.
	 * having fast_lock_count non-zero will defer the database flushing logic and other critical parts of the system.
	 * hence this periodic reset at the beginning of each transaction.
	 */
	assert(0 == fast_lock_count);
	fast_lock_count = 0;
	t_tries = 0;
	if (non_tp_jfb_ptr)
		non_tp_jfb_ptr->record_size = 0; /* re-initialize it to 0 since TOTAL_NONTPJNL_REC_SIZE macro uses it */
	assert(!TREF(donot_commit));
}
