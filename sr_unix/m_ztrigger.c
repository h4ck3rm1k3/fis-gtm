/****************************************************************
 *								*
 *	Copyright 2010 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"
#include "compiler.h"
#include "mdq.h"
#include "opcode.h"
#include "indir_enum.h"
#include "toktyp.h"
#include "advancewindow.h"
#include "cmd.h"

GBLREF char 	window_token;
GBLREF mident 	window_ident;
GBLREF char	director_token;

int m_ztrigger(void)
{
#	ifdef GTM_TRIGGER
	oprtype		tmparg;
	triple		*ref;

	error_def(ERR_GBLEXPECTED);

	switch (window_token)
	{
		case TK_CIRCUMFLEX:
			if (!gvn())
				return FALSE;
			ref = newtriple(OC_ZTRIGGER);
			break;
		case TK_ATSIGN:
			if (!indirection(&tmparg))
				return FALSE;
			ref = maketriple(OC_COMMARG);
			ref->operand[0] = tmparg;
			ref->operand[1] = put_ilit((mint)indir_ztrigger);
			ins_triple(ref);
			return TRUE;
		default:
			stx_error(ERR_GBLEXPECTED);
			return FALSE;
	}
	return TRUE;
#	else
	return FALSE;
#	endif
}
