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

#include "compiler.h"
#include "svnames.h"
#include "opcode.h"
#include "toktyp.h"
#include "nametabtyp.h"
#include "funsvn.h"
#include "advancewindow.h"
#include "mdq.h"
#include "stringpool.h"
#include "namelook.h"
#include "fullbool.h"
#include "show_source_line.h"

GBLREF	bool		devctlexp;
GBLREF	char		director_token;
GBLREF	triple		*curtchain;
GBLREF	boolean_t	run_time;
GBLREF	mident		window_ident;
GBLREF	mval		window_mval;
GBLREF	char		window_token;

error_def(ERR_BOOLSIDEFFECT);
error_def(ERR_EXPR);
error_def(ERR_FCNSVNEXPECTED);
error_def(ERR_FNOTONSYS);
error_def(ERR_INVFCN);
error_def(ERR_INVSVN);
error_def(ERR_RPARENMISSING);
error_def(ERR_VAREXPECTED);

LITREF	toktabtype	tokentable[];
LITREF	mval		literal_null;

#ifndef UNICODE_SUPPORTED
#define f_char f_zchar
#endif

/* note that svn_index array provides indexes into this array for each letter of the
 * alphabet so changes here should be reflected there.
 */
LITDEF nametabent svn_names[] =
{
         { 1, "D" }, { 6, "DEVICE" }
	,{ 2, "EC" }, { 5, "ECODE" }
	,{ 2, "ES" }, { 6, "ESTACK" }
	,{ 2, "ET" }, { 5, "ETRAP" }
	,{ 1, "H" }, { 7, "HOROLOG" }
	,{ 1, "I" }, { 2, "IO" }
	,{ 1, "J" }, { 3, "JOB" }
	,{ 1, "K" }, { 3, "KEY" }
	,{ 1, "P" }, { 8, "PRINCIPA*" }
	,{ 1, "Q" }, { 4, "QUIT" }
	,{ 1, "R" }, { 8, "REFERENC*" }
	,{ 1, "S" }, { 7, "STORAGE" }
	,{ 2, "ST" }, { 5, "STACK" }
	,{ 2, "SY" }, { 6, "SYSTEM" }
	,{ 1, "T" }, { 4, "TEST" }
	,{ 2, "TL"}, { 6, "TLEVEL"}
	,{ 2, "TR"}, { 8, "TRESTART"}
	,{ 1, "X" }
	,{ 1, "Y" }
	,{ 2, "ZA" }
	,{ 3, "ZAL*"}
	,{ 2, "ZB" }
	,{ 2, "ZC" }
	,{ 3, "ZCH" }, { 6, "ZCHSET" }
	,{ 3, "ZCM*" }
	,{ 3, "ZCO*" }
	,{ 3, "ZCS*" }
	,{ 3, "ZDA*" }
	,{ 2, "ZD*" }
	,{ 2, "ZE" }
	,{ 3, "ZED*" }
	,{ 3, "ZEO*" }
	,{ 3, "ZER*" }
	,{ 2, "ZG*" }
	,{ 4, "ZINI*"}
	,{ 4, "ZINT*"}
	,{ 3, "ZIO" }
	,{ 2, "ZJ" }, { 4, "ZJOB" }
	,{ 2, "ZL*" }
	,{ 8, "ZMAXTPTI*" }
	,{ 3, "ZMO*" }
	,{ 5, "ZPATN" }, {8, "ZPATNUME*" }
	,{ 4, "ZPOS*" }
	,{ 5, "ZPROC*" }
	,{ 5, "ZPROM*" }
	,{ 2, "ZQ*" }
	,{ 3, "ZRE*" }
	,{ 3, "ZRO*" }
	,{ 3, "ZSO*" }
	,{ 2, "ZS" }, { 4, "ZSTA*" }
	,{ 5, "ZSTEP"}
	,{ 3, "ZSY*"}
	,{ 4, "ZTCO*"}
	,{ 4, "ZTDA*"}
	,{ 3, "ZTE" }, { 4, "ZTEX*"}
	,{ 4, "ZTLE*"}
	,{ 4, "ZTNA*"}
	,{ 4, "ZTOL*"}
	,{ 4, "ZTRI*"}
	,{ 4, "ZTSL*"}
	,{ 4, "ZTUP*"}
	,{ 4, "ZTVA*"}
	,{ 4, "ZTWO*"}
	,{ 2, "ZT*" }
	,{ 3, "ZUS*" }
	,{ 2, "ZV*" }
	,{ 4, "ZYER*" }
};

/* Indexes into svn_names array for each letter of the alphabet */
LITDEF unsigned char svn_index[27] = {
	 0,  0,  0,  0,  2,  8,  8,  8, 10,	/* a b c d e f g h i */
	12, 14 ,16, 16, 16, 16, 16, 18, 20,	/* j k l m n o p q r */
	22, 28, 34 ,34, 34, 34, 35, 36, 89	/* s t u v w x y z ~ */
};

/* These entries correspond to the entries in the svn_names array */
LITDEF svn_data_type svn_data[] =
{
	{ SV_DEVICE, FALSE, ALL_SYS }, { SV_DEVICE, FALSE, ALL_SYS }
	,{ SV_ECODE, TRUE, ALL_SYS }, { SV_ECODE, TRUE, ALL_SYS }
	,{ SV_ESTACK, FALSE, ALL_SYS }, { SV_ESTACK, FALSE, ALL_SYS }
	,{ SV_ETRAP, TRUE, ALL_SYS }, { SV_ETRAP, TRUE, ALL_SYS }
	,{ SV_HOROLOG, FALSE, ALL_SYS }, { SV_HOROLOG, FALSE, ALL_SYS }
	,{ SV_IO, FALSE, ALL_SYS }, { SV_IO, FALSE, ALL_SYS }
	,{ SV_JOB, FALSE, ALL_SYS }, { SV_JOB, FALSE, ALL_SYS }
	,{ SV_KEY, FALSE, ALL_SYS }, { SV_KEY, FALSE, ALL_SYS }
	,{ SV_PRINCIPAL, FALSE, ALL_SYS }, { SV_PRINCIPAL, FALSE, ALL_SYS }
	,{ SV_QUIT, FALSE, ALL_SYS }, { SV_QUIT, FALSE, ALL_SYS }
	,{ SV_REFERENCE, FALSE, ALL_SYS }, { SV_REFERENCE, FALSE, ALL_SYS }
	,{ SV_STORAGE, FALSE, ALL_SYS }, { SV_STORAGE, FALSE, ALL_SYS }
	,{ SV_STACK, FALSE, ALL_SYS }, { SV_STACK, FALSE, ALL_SYS }
	,{ SV_SYSTEM, FALSE, ALL_SYS }, { SV_SYSTEM, FALSE, ALL_SYS }
	,{ SV_TEST, FALSE, ALL_SYS }, { SV_TEST, FALSE, ALL_SYS }
	,{ SV_TLEVEL, FALSE, ALL_SYS }, { SV_TLEVEL, FALSE, ALL_SYS }
	,{ SV_TRESTART, FALSE, ALL_SYS }, { SV_TRESTART, FALSE, ALL_SYS }
	,{ SV_X, TRUE, ALL_SYS }
	,{ SV_Y, TRUE, ALL_SYS }
	,{ SV_ZA, FALSE, ALL_SYS }
	,{ SV_ZALLOCSTOR, FALSE, ALL_SYS }
	,{ SV_ZB, FALSE, ALL_SYS }
	,{ SV_ZC, FALSE, ALL_SYS }
	,{ SV_ZCHSET, FALSE, ALL_SYS }, { SV_ZCHSET, FALSE, ALL_SYS }
	,{ SV_ZCMDLINE, FALSE, ALL_SYS }
	,{ SV_ZCOMPILE, TRUE, ALL_SYS }
	,{ SV_ZCSTATUS, FALSE, ALL_SYS}
	,{ SV_ZDATE_FORM, TRUE, ALL_SYS }
	,{ SV_ZDIR, TRUE, ALL_SYS }
	,{ SV_ZERROR, TRUE, ALL_SYS }
	,{ SV_ZEDITOR, FALSE, ALL_SYS }
	,{ SV_ZEOF, FALSE, ALL_SYS }
	,{ SV_ZERROR, TRUE, ALL_SYS }
	,{ SV_ZGBLDIR, TRUE, ALL_SYS }
	,{ SV_ZININTERRUPT, FALSE, ALL_SYS}
	,{ SV_ZINTERRUPT, TRUE, ALL_SYS}
	,{ SV_ZIO, FALSE, ALL_SYS }
	,{ SV_ZJOB, FALSE, ALL_SYS }, { SV_ZJOB, FALSE, ALL_SYS }
	,{ SV_ZLEVEL, FALSE, ALL_SYS }
	,{ SV_ZMAXTPTIME, TRUE, ALL_SYS }
	,{ SV_ZMODE, FALSE, ALL_SYS }
	,{ SV_ZPATNUMERIC, FALSE, ALL_SYS }, { SV_ZPATNUMERIC, FALSE, ALL_SYS }
	,{ SV_ZPOS, FALSE, ALL_SYS }
	,{ SV_ZPROC, FALSE, ALL_SYS }
	,{ SV_PROMPT, TRUE, ALL_SYS }
	,{ SV_ZQUIT, TRUE, ALL_SYS }
	,{ SV_ZREALSTOR, FALSE, ALL_SYS }
	,{ SV_ZROUTINES, TRUE, ALL_SYS }
	,{ SV_ZSOURCE, TRUE, ALL_SYS }
	,{ SV_ZSTATUS, TRUE, ALL_SYS }, { SV_ZSTATUS, TRUE, ALL_SYS }
	,{ SV_ZSTEP, TRUE, ALL_SYS }
	,{ SV_ZSYSTEM, FALSE, ALL_SYS }
	,{ SV_ZTCODE, FALSE, TRIGGER_OS }
	,{ SV_ZTDATA, FALSE, TRIGGER_OS }
	,{ SV_ZTEXIT, TRUE, ALL_SYS }, { SV_ZTEXIT, TRUE, ALL_SYS }
	,{ SV_ZTLEVEL, FALSE, TRIGGER_OS}
	,{ SV_ZTNAME, FALSE, TRIGGER_OS }
	,{ SV_ZTOLDVAL, FALSE, TRIGGER_OS }
	,{ SV_ZTRIGGEROP, FALSE, TRIGGER_OS}
	,{ SV_ZTSLATE, TRUE, TRIGGER_OS}
	,{ SV_ZTUPDATE, FALSE, TRIGGER_OS }
	,{ SV_ZTVALUE, TRUE, TRIGGER_OS }
	,{ SV_ZTWORMHOLE, TRUE, TRIGGER_OS }
	,{ SV_ZTRAP, TRUE, ALL_SYS }
	,{ SV_ZUSEDSTOR, FALSE, ALL_SYS }
	,{ SV_ZVERSION, FALSE, ALL_SYS }
	,{ SV_ZYERROR, TRUE, ALL_SYS }
};

/* note that fun_index array provides indexes into this array for each letter of the
 * alphabet so changes here should be reflected there.
 * "*" is used below only after 8 characters.
 */
LITDEF nametabent fun_names[] =
{
	 {1, "A"}, {5, "ASCII"}
	,{1, "C"}, {4, "CHAR"}
	,{1, "D"}, {4, "DATA"}
	,{1, "E"}, {7, "EXTRACT"}
	,{1, "F"}, {4, "FIND"}
	,{2, "FN"}, {7, "FNUMBER"}
	,{1, "G"}, {3, "GET"}
	,{1, "I"}, {4, "INCR"}, {8, "INCREMEN*"}
	,{1, "J"}, {7, "JUSTIFY"}
	,{1, "L"}, {6, "LENGTH"}
	,{1, "N"}
	,{2, "NA"}, {4, "NAME"}
	,{4, "NEXT"}
	,{1, "O"}, {5, "ORDER"}
	,{1, "P"}, {5, "PIECE"}
	,{2, "QL"}, {7, "QLENGTH"}
	,{2, "QS"}, {8, "QSUBSCRI*"}
	,{1, "Q"}, {5, "QUERY"}
	,{1, "R"}, {6, "RANDOM"}
	,{2, "RE"}, {7, "REVERSE"}
	,{1, "S"}, {6, "SELECT"}
	,{2, "ST"}, {5, "STACK"}
	,{1, "T"}, {4, "TEXT"}
	,{2, "TR"}, {8, "TRANSLAT*"}
	,{1, "V*"}
	,{2, "ZA"}, {6, "ZASCII"}
	,{3, "ZAH"}, {8, "ZAHANDLE"}
	,{7, "ZBITAND"}
	,{8, "ZBITCOUN*"}
	,{8, "ZBITFIND"}
	,{7, "ZBITGET"}
	,{7, "ZBITLEN"}
	,{7, "ZBITNOT"}
	,{6, "ZBITOR"}
	,{7, "ZBITSET"}
	,{7, "ZBITSTR"}
	,{7, "ZBITXOR"}
	,{2, "ZC"}, {5, "ZCALL"}
	,{3, "ZCH"}, {5, "ZCHAR"}
	,{3, "ZCO"}, {8, "ZCONVERT"}
	,{2, "ZD"}
	,{5, "ZDATA"}
	,{5, "ZDATE"}
	,{2, "ZE"}, {8, "ZEXTRACT"}
	,{2, "ZF"}, {5, "ZFIND"}
	,{5, "ZFILE"}, {8, "ZFILEATT*"}
	,{7, "ZGETDVI"}
	,{7, "ZGETJPI"}
	,{7, "ZGETLKI"}
	,{7, "ZGETSYI"}
	,{5, "ZINCR"}, {8, "ZINCREME*"}
	,{2, "ZJ"}, {8, "ZJUSTIFY"}
	,{8, "ZJOBEXAM"}
	,{2, "ZL"}, {7, "ZLENGTH"}
	,{5, "ZLKID"}
	,{2, "ZM"}, {8, "ZMESSAGE"}
	,{2, "ZP"}, {8, "ZPREVIOU*"}
	,{6, "ZPARSE"}
	,{3, "ZPI"}, {6, "ZPIECE"}
	,{4, "ZPID"}
	,{5, "ZPRIV"}, {8, "ZPRIVILE*"}
	,{2, "ZQ"}, {8, "ZQGBLMOD"}
	,{7, "ZSEARCH"}
	,{7, "ZSETPRV"}
	,{8, "ZSIGPROC"}
	,{4, "ZSUB"}, {7, "ZSUBSTR"}
	,{3, "ZTR"}, {8, "ZTRANSLA*"}
	,{4, "ZTRI"}, {8, "ZTRIGGER"}
	,{7, "ZTRNLNM"}
	,{2, "ZW"}, {6, "ZWIDTH"}
};

/* Index into fun_names array where entries that start with each letter of the alphabet begin. */
LITDEF unsigned char fun_index[27] =
{
	 0,  2,  2,  4,  6,  8, 12, 14, 14,	/* a b c d e f g h i */
	17, 19, 19, 21, 21, 25, 27, 29, 35,	/* j k l m n o p q r */
	39, 43, 47, 47, 48, 48, 48, 48, 113	/* s t u v w x y z ~ */
};

/* Each entry corresponds to an entry in fun_names */
LITDEF fun_data_type fun_data[] =
{
	 { OC_FNASCII, ALL_SYS }, { OC_FNASCII, ALL_SYS }
        ,{ OC_FNCHAR, ALL_SYS }, { OC_FNCHAR, ALL_SYS }
	,{ OC_FNDATA, ALL_SYS }, { OC_FNDATA, ALL_SYS }
	,{ OC_FNEXTRACT, ALL_SYS }, { OC_FNEXTRACT, ALL_SYS }
	,{ OC_FNFIND, ALL_SYS }, { OC_FNFIND, ALL_SYS }
	,{ OC_FNFNUMBER, ALL_SYS }, { OC_FNFNUMBER, ALL_SYS }
	,{ OC_FNGET, ALL_SYS }, { OC_FNGET, ALL_SYS }
	,{ OC_FNINCR, ALL_SYS }, { OC_FNINCR, ALL_SYS }, { OC_FNINCR, ALL_SYS }
	,{ OC_FNJ2, ALL_SYS }, { OC_FNJ2, ALL_SYS }
	,{ OC_FNLENGTH, ALL_SYS }, { OC_FNLENGTH, ALL_SYS }
	,{ OC_FNNEXT, ALL_SYS }
	,{ OC_FNNAME, ALL_SYS }, { OC_FNNAME, ALL_SYS }
	,{ OC_FNNEXT, ALL_SYS }
	,{ OC_FNORDER, ALL_SYS }, {OC_FNORDER, ALL_SYS }
	,{ OC_FNPIECE, ALL_SYS }, { OC_FNPIECE, ALL_SYS }
	,{ OC_FNQLENGTH, ALL_SYS }, { OC_FNQLENGTH, ALL_SYS }
	,{ OC_FNQSUBSCR, ALL_SYS }, { OC_FNQSUBSCR, ALL_SYS }
	,{ OC_FNQUERY, ALL_SYS }, { OC_FNQUERY, ALL_SYS }
	,{ OC_FNRANDOM, ALL_SYS }, { OC_FNRANDOM, ALL_SYS }
	,{ OC_FNREVERSE, ALL_SYS }, { OC_FNREVERSE, ALL_SYS }
	,{ OC_PASSTHRU, ALL_SYS }, { OC_PASSTHRU, ALL_SYS }
	,{ OC_FNSTACK1, ALL_SYS }, { OC_FNSTACK1, ALL_SYS }
	,{ OC_FNTEXT, ALL_SYS }, { OC_FNTEXT, ALL_SYS }
	,{ OC_FNTRANSLATE, ALL_SYS }, { OC_FNTRANSLATE, ALL_SYS }
	,{ OC_FNVIEW, ALL_SYS }
	,{ OC_FNZASCII, ALL_SYS }, { OC_FNZASCII, ALL_SYS }
	,{ OC_FNZAHANDLE, ALL_SYS }, { OC_FNZAHANDLE, ALL_SYS }
	,{ OC_FNZBITAND, ALL_SYS }
	,{ OC_FNZBITCOUN, ALL_SYS }
	,{ OC_FNZBITFIND, ALL_SYS }
	,{ OC_FNZBITGET, ALL_SYS }
	,{ OC_FNZBITLEN, ALL_SYS }
	,{ OC_FNZBITNOT, ALL_SYS }
	,{ OC_FNZBITOR, ALL_SYS }
	,{ OC_FNZBITSET, ALL_SYS }
	,{ OC_FNZBITSTR, ALL_SYS }
	,{ OC_FNZBITXOR, ALL_SYS }
#	ifdef __sun
	,{ OC_FNZCALL,UNIX_OS}, { OC_FNZCALL,UNIX_OS}
#	else
	,{ OC_FNZCALL, VMS_OS }, { OC_FNZCALL, VMS_OS }
#	endif
	,{ OC_FNZCHAR, ALL_SYS }, { OC_FNZCHAR, ALL_SYS }
	,{ OC_FNZCONVERT2, UNIX_OS }, { OC_FNZCONVERT2, UNIX_OS }
	,{ OC_FNZDATE, ALL_SYS }
	,{ OC_FNZDATA, ALL_SYS }
	,{ OC_FNZDATE, ALL_SYS }
	,{ OC_FNZEXTRACT, ALL_SYS }, { OC_FNZEXTRACT, ALL_SYS }
	,{ OC_FNZFIND, ALL_SYS }, { OC_FNZFIND, ALL_SYS }
	,{ OC_FNZFILE, VMS_OS }, { OC_FNZFILE, VMS_OS }
	,{ OC_FNZGETDVI, VMS_OS }
	,{ OC_FNZGETJPI, ALL_SYS }
	,{ OC_FNZGETLKI, VMS_OS }
	,{ OC_FNZGETSYI, VMS_OS }
	,{ OC_FNINCR, ALL_SYS }, { OC_FNINCR, ALL_SYS }
	,{ OC_FNZJ2, ALL_SYS }, { OC_FNZJ2, ALL_SYS }
	,{ OC_FNZJOBEXAM, ALL_SYS }
	,{ OC_FNZLENGTH, ALL_SYS }, { OC_FNZLENGTH, ALL_SYS }
	,{ OC_FNZLKID, VMS_OS}
	,{ OC_FNZM, ALL_SYS }, { OC_FNZM, ALL_SYS }
	,{ OC_FNZPREVIOUS, ALL_SYS }, { OC_FNZPREVIOUS, ALL_SYS }
	,{ OC_FNZPARSE, ALL_SYS }
	,{ OC_FNZPIECE, ALL_SYS }, { OC_FNZPIECE, ALL_SYS }
	,{ OC_FNZPID, VMS_OS }
	,{ OC_FNZPRIV, VMS_OS }, { OC_FNZPRIV, VMS_OS }
	,{ OC_FNZQGBLMOD, ALL_SYS }, { OC_FNZQGBLMOD, ALL_SYS }
	,{ OC_FNZSEA, ALL_SYS }
	,{ OC_FNZSETPRV, VMS_OS }
	,{ OC_FNZSIGPROC, ALL_SYS }
	,{ OC_FNZSUBSTR, ALL_SYS }, { OC_FNZSUBSTR, ALL_SYS }
	,{ OC_FNZTRANSLATE, ALL_SYS }, { OC_FNZTRANSLATE, ALL_SYS }
	,{ OC_FNZTRIGGER, TRIGGER_OS }, { OC_FNZTRIGGER, TRIGGER_OS }
	,{ OC_FNZTRNLNM, ALL_SYS }
	,{ OC_FNZWIDTH, ALL_SYS }, { OC_FNZWIDTH, ALL_SYS }
};

/* Each entry corresponds to an entry in fun_names */
GBLDEF int (*fun_parse[])(oprtype *, opctype) =		/* contains addresses so can't be a LITDEF */
{
	f_ascii, f_ascii,
	f_char, f_char,
	f_data, f_data,
	f_extract, f_extract,
	f_find, f_find,
	f_fnumber, f_fnumber,
	f_get, f_get,
	f_incr, f_incr, f_incr,
	f_justify, f_justify,
	f_length, f_length,
	f_next,
	f_name, f_name,
	f_next,
	f_order, f_order,
	f_piece, f_piece,
	f_qlength, f_qlength,
	f_qsubscript, f_qsubscript,
	f_query, f_query,
	f_mint, f_mint,
	f_reverse, f_reverse,
	f_select, f_select,
	f_stack, f_stack,
	f_text, f_text,
	f_translate, f_translate,
	f_view,
	f_ascii, f_ascii,
	f_zahandle, f_zahandle,
	f_two_mval,
	f_one_mval,
	f_fnzbitfind,
	f_fnzbitget,
	f_one_mval,
	f_one_mval,
	f_two_mval,
	f_fnzbitset,
	f_fnzbitstr,
	f_two_mval,
	f_zcall, f_zcall,
	f_zchar, f_zchar,
	f_zconvert, f_zconvert,
	f_zdate,
	f_data,				/* $ZDATA reuses parser for $DATA since only runtime execution differs */
	f_zdate,
	f_extract, f_extract,
	f_find, f_find,
	f_two_mstrs, f_two_mstrs,
	f_two_mstrs,
	f_mint_mstr,
	f_two_mstrs,
	f_zgetsyi,
	f_incr, f_incr,
	f_justify, f_justify,
	f_zjobexam,
	f_length, f_length,
	f_mint,
	f_mint, f_mint,
	f_zprevious, f_zprevious,
	f_zparse,
	f_piece, f_piece,
	f_mint,
	f_mstr, f_mstr,
	f_zqgblmod, f_zqgblmod,
	f_zsearch,
	f_mstr,
	f_zsigproc,
	f_extract, f_extract,		/* $ZSUBSTR */
	f_translate, f_translate,
	f_ztrigger, f_ztrigger,
	f_ztrnlnm,
	f_zwidth, f_zwidth
};

int expritem(oprtype *a)
{
	int 		i, index, sv_opcode;
	triple 		*oldchain, *ref, tmpchain, *triptr;
	boolean_t	parse_warn, save_shift;
	oprtype 	x1;
	char		source_line_buff[MAX_SRCLINE + SIZEOF(ARROW)];
	unsigned char 	type;
	DCL_THREADGBL_ACCESS;

	SETUP_THREADGBL_ACCESS;
	assert(svn_index[26] == (SIZEOF(svn_names)/SIZEOF(nametabent)));
	assert(SIZEOF(svn_names)/SIZEOF(nametabent) == SIZEOF(svn_data)/SIZEOF(svn_data_type)); /* are all SVNs covered? */
	assert(fun_index[26] == (SIZEOF(fun_names)/SIZEOF(nametabent)));
	assert(SIZEOF(fun_names)/SIZEOF(nametabent) == SIZEOF(fun_data)/SIZEOF(fun_data_type)); /* are all functions covered? */
	if (i = tokentable[window_token].uo_type)	/* Note assignment */
	{
		type = tokentable[window_token].opr_type;
		advancewindow();
		if ((OC_NEG == i) && ((TK_NUMLIT == window_token) || (TK_INTLIT == window_token)))
		{
			assert(MV_IS_NUMERIC(&window_mval));
			if (window_mval.mvtype & MV_INT)
				window_mval.m[1] = -window_mval.m[1];
			else
				window_mval.sgn = 1;
			if (TK_NUMLIT == window_token)
				n2s(&window_mval);
		} else
		{
			if (!expratom(&x1))
				return FALSE;
			coerce(&x1,type);
			ref = newtriple((opctype) i);
			ref->operand[0] = x1;
			*a = put_tref(ref);
			return TRUE;
		}
	}
	switch(i = window_token)	/* Note assignment */
	{
		case TK_INTLIT:
			n2s(&window_mval);
		case TK_NUMLIT:
		case TK_STRLIT:
			*a = put_lit(&window_mval);
			advancewindow();
			return TRUE;
		case TK_LPAREN:
			advancewindow();
			if (eval_expr(a) && TK_RPAREN == window_token)
			{
				advancewindow();
				return TRUE;
			}
			stx_error(ERR_RPARENMISSING);
			return FALSE;
		case TK_DOLLAR:
			if ((TK_DOLLAR == director_token) || (TK_AMPERSAND == director_token))
			{
				if ((-TRUE == TREF(shift_side_effects)) && !run_time && (FULL_BOOL_WARN == TREF(gtm_fullbool)))
				{	/* warnings requested by by gtm_fullbool and enabled by -TRUE from eval_expr */
					show_source_line(source_line_buff, SIZEOF(source_line_buff), TRUE);
					dec_err(VARLSTCNT(1) ERR_BOOLSIDEFFECT);
				}
				advancewindow();
				dqinit(&tmpchain, exorder);	/* a new chain in case we need to juggle things */
				oldchain = setcurtchain(&tmpchain);
				save_shift = TREF(shift_side_effects);
				if (GTM_BOOL != TREF(gtm_fullbool))	/* if no short circuit, only the outermost should juggle */
					TREF(shift_side_effects) = FALSE;
				TREF(temp_subs) = TRUE;
				if ((TK_DOLLAR == window_token) ? !exfunc(a, FALSE) : !extern_func(a))
				{
					setcurtchain(oldchain);
					return FALSE;
				}
				TREF(shift_side_effects) = save_shift;
				if (!save_shift || (GTM_BOOL == TREF(gtm_fullbool)))
				{	/* put it on the end of the main chain as there's no reason to play with the ordering */
					setcurtchain(oldchain);
					triptr = curtchain->exorder.bl;
					dqadd(triptr, &tmpchain, exorder);	/* this is a violation of info hiding */
				} else /* put on the shift chain to get side effects in boolean expression */
				{	/* add the chain after "expr_start" which may be much before "curtchain" */
					ref = newtriple(OC_GVSAVTARG);
					dqadd(TREF(expr_start), &tmpchain, exorder);	/* this is a violation of info hiding */
					TREF(expr_start) = tmpchain.exorder.bl;
					setcurtchain(oldchain);
					triptr = newtriple(OC_GVRECTARG);
					triptr->operand[0] = put_tref(ref);
				}
			} else
			{
				advancewindow();
				if (TK_IDENT != window_token)
				{
					stx_error(ERR_FCNSVNEXPECTED);
					return FALSE;
				}
				parse_warn = FALSE;
				if (TK_LPAREN == director_token)
				{
					index = namelook(fun_index, fun_names, window_ident.addr, window_ident.len);
					if (index < 0)
					{
						STX_ERROR_WARN(ERR_INVFCN);	/* sets "parse_warn" to TRUE */
					} else
					{
						assert(SIZEOF(fun_names) / SIZEOF(fun_data_type) > index);
						if (!VALID_FUN(index))
						{
							STX_ERROR_WARN(ERR_FNOTONSYS);	/* sets "parse_warn" to TRUE */
						} else if (OC_FNINCR == fun_data[index].opcode)
						{	/* $INCR is used. This can operate on undefined local variables
							 * and make them defined. If used in a SET where the left and right
							 * side of the = operator use this variable (as a subscript on the left
							 * and as input to the $INCR function on the right), we want an UNDEF
							 * error to show up which means we need to set "temp_subs" to TRUE.
							 */
							TREF(temp_subs) = TRUE;
							if ((-TRUE == TREF(shift_side_effects)) && !run_time
								&& (FULL_BOOL_WARN == TREF(gtm_fullbool))) /* warnings requested */
							{	/* by gtm_fullbool and enabled by -TRUE from eval_expr  */
								show_source_line(source_line_buff, SIZEOF(source_line_buff), TRUE);
								dec_err(VARLSTCNT(1) ERR_BOOLSIDEFFECT);
							}
						}
					}
					advancewindow();
					advancewindow();
					if (!parse_warn)
					{
						assert(OPCODE_COUNT > fun_data[index].opcode);
						if (!(boolean_t)((*fun_parse[index])(a, fun_data[index].opcode)))
							return FALSE;
					} else
					{
						*a = put_lit((mval *)&literal_null);
						/* Parse the remaining arguments until the corresponding RIGHT-PAREN/SPACE/EOL
						   is reached */
						if (!parse_until_rparen_or_space())
							return FALSE;
					}
					if (TK_RPAREN != window_token)
					{
						stx_error(ERR_RPARENMISSING);
						return FALSE;
					}
					advancewindow();
				} else
				{
					index = namelook(svn_index, svn_names, window_ident.addr, window_ident.len);
					if (0 > index)
					{
						STX_ERROR_WARN(ERR_INVSVN);	/* sets "parse_warn" to TRUE */
					} else
					{
						assert(SIZEOF(svn_names) / SIZEOF(svn_data_type) > index);
						if (!VALID_SVN(index))
						{
							STX_ERROR_WARN(ERR_FNOTONSYS);	/* sets "parse_warn" to TRUE */
						}
					}
					advancewindow();
					if (!parse_warn)
					{
						sv_opcode = svn_data[index].opcode;
						assert(SV_NUM_SV > sv_opcode);
						if (SV_TEST == sv_opcode)
							*a = put_tref(newtriple(OC_GETTRUTH));
						else
						{
							if (sv_opcode == SV_X || sv_opcode == SV_Y)
								devctlexp = TRUE;
							ref = newtriple(OC_SVGET);
							ref->operand[0] = put_ilit(sv_opcode);
							*a = put_tref(ref);
						}
					} else
						*a = put_lit((mval *)&literal_null);
				}
			}
			return TRUE;
		case TK_COLON:
			stx_error(ERR_EXPR);
			return FALSE;
	}
	return FALSE;
}
