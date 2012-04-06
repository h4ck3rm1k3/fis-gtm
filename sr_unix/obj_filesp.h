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
extern int x_we_have_included;
#ifndef OBJ_FILESP_INCLUDED2
#define OBJ_FILESP_INCLUDED2

typedef struct linkage_entry_struct
{
	struct linkage_entry_struct	*next;
	struct sym_table	*symbol;
} linkage_entry;

struct sym_table *define_symbol(unsigned char psect, mstr *name);
void comp_linkages(void);
void resolve_sym (void);
void output_relocation (void);
void output_symbol (void);
void buff_emit(void);
void buff_flush(void);

/* Currently JSB contains two instructions:
 * 	load %ret, -1
 * 	ret
 #	nop	# (hppa)
 */
#if defined(__ia64)
#  define JSB_ACTION_N_INS	4
#elif defined(__hpux)
#  define JSB_ACTION_N_INS	3
#elif defined(__x86_64__)
#  define JSB_ACTION_N_INS	8
#elif defined(__sparc)
#  define JSB_ACTION_N_INS	3
#elif defined(__mvs__) || defined(Linux390)
#  define JSB_ACTION_N_INS	4
#else
#  define JSB_ACTION_N_INS	2
#endif
#define JSB_MARKER		"GTM_CODE"
#define MIN_LINK_PSECT_SIZE	0

#endif
