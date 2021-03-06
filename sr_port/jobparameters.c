/****************************************************************
 *								*
 *	Copyright 2001, 2007 Fidelity Information Services, Inc	*
 *								*
 *	This source code contains the intellectual property	*
 *	of its copyright holder(s), and is made available	*
 *	under a license.  If you do not know the terms of	*
 *	the license, please stop and do not read further.	*
 *								*
 ****************************************************************/

#include "mdef.h"

#include "compiler.h"
/*
#include "opcode.h"
*/

#include "toktyp.h"
#include "job.h"
#include "advancewindow.h"


GBLREF char window_token;


int jobparameters (oprtype *c)
{
	char		*parptr;
	/* This is a workaround for the moment.  The former maximum size of the
		parameter string was completely incapable of handling the variety
		of string parameters that may be passed.  To further compound things,
		no checks exist to enforce upper limits on parameter string lengths.
		This new maximum was reached by calculating the maximum length of
		the job parameter string (presuming each possible qualifier is represented
		once and only once) and tacking on a safety net.  This came to
		10 255 byte strings (plus length byte), 1 longword, and 17 single byte
		identifiers for each of the job keywords.  The maximum of 3000 leaves
		a little room for expansion in the future
	*/
	char		parastr[3000];
	error_def	(ERR_RPARENMISSING);

	parptr = parastr;
	if (window_token != TK_LPAREN)
	{
		if (window_token != TK_COLON)
		{
			if (!one_job_param (&parptr))
				return FALSE;
		}
	} else
	{
		advancewindow ();
		for (;;)
		{
			if (!one_job_param (&parptr))
				return FALSE;
			if (window_token == TK_COLON)
				advancewindow ();
			else if (window_token == TK_RPAREN) {
				advancewindow ();
				break;
			}
			else {
				stx_error (ERR_RPARENMISSING);
				return FALSE;
			}
		}
	}
	*parptr++ = jp_eol;
	*c = put_str (parastr,(mstr_len_t)(parptr - parastr));
	return TRUE;
}
