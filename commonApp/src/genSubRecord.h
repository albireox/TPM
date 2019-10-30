#include <vxWorks.h>
#include <semLib.h>
#include "ellLib.h"
#include "fast_lock.h"
#include "link.h"
#include "tsDefs.h"

#ifndef INCgenSubLFLGH
#define INCgenSubLFLGH
typedef enum {
	genSubLFLG_IGNORE,
	genSubLFLG_READ
}genSubLFLG;
#endif /*INCgenSubLFLGH*/

#ifndef INCgenSubEFLGH
#define INCgenSubEFLGH
typedef enum {
	genSubEFLG_NEVER,
	genSubEFLG_ON_CHANGE,
	genSubEFLG_ALWAYS
}genSubEFLG;
#endif /*INCgenSubEFLGH*/
#ifndef INCgenSubH
#define INCgenSubH
typedef struct genSubRecord {
	char		name[29]; /*Record Name*/
	char		desc[29]; /*Descriptor*/
	char		asg[29]; /*Access Security Group*/
	unsigned short	scan;	/*Scan Mechanism*/
	unsigned short	pini;	/*Process at iocInit*/
	short		phas;	/*Scan Phase*/
	short		evnt;	/*Event Number*/
	short		tse;	/*Time Stamp Event*/
	DBLINK		tsel;	/*Time Stamp Link*/
	unsigned short	dtyp;	/*Device Type*/
	short		disv;	/*Disable Value*/
	short		disa;	/*Disable*/
	DBLINK		sdis;	/*Scanning Disable*/
	FAST_LOCK	mlok;	/*Monitor fastlock*/
	ELLLIST		mlis;	/*Monitor List*/
	unsigned char	disp;	/*Disable putField*/
	unsigned char	proc;	/*Force Processing*/
	unsigned short	stat;	/*Alarm Status*/
	unsigned short	sevr;	/*Alarm Severity*/
	unsigned short	nsta;	/*New Alarm Status*/
	unsigned short	nsev;	/*New Alarm Severity*/
	unsigned short	acks;	/*Alarm Ack Severity*/
	unsigned short	ackt;	/*Alarm Ack Transient*/
	unsigned short	diss;	/*Disable Alarm Sevrty*/
	unsigned char	lcnt;	/*Lock Count*/
	unsigned char	pact;	/*Record active*/
	unsigned char	putf;	/*dbPutField process*/
	unsigned char	rpro;	/*Reprocess */
	void		*asp;	/*Access Security Pvt*/
	void		*ppn;	/*addr of PUTNOTIFY*/
	void		*ppnn;	/*next record PUTNOTIFY*/
	void		*spvt;	/*Scan Private*/
	void		*rset;	/*Address of RSET*/
	struct dset	*dset;	/*DSET address*/
	void		*dpvt;	/*Device Private*/
	void		*rdes;	/*Address of dbRecordType*/
	void		*lset;	/*Lock Set*/
	unsigned short	prio;	/*Scheduling Priority*/
	unsigned char	tpro;	/*Trace Processing*/
	char bkpt;	/*Break Point*/
	unsigned char	udf;	/*Undefined*/
	TS_STAMP	time;	/*Time*/
	DBLINK		flnk;	/*Forward Process Link*/
	double		vers;	/*Version Number*/
	long		val;	/*Subr. return value*/
	long		oval;	/*Old return value*/
	long		sadr;	/*Subroutine Address*/
	long		osad;	/*Old Subr. Address*/
	unsigned short	lflg;	/*Link Flag*/
	unsigned short	eflg;	/*Event Flag*/
	DBLINK		subl;	/*Subroutine Input Link*/
	char		inam[40]; /*Init Routine Name*/
	char		snam[40]; /*Process Subr. Name*/
	char		onam[40]; /*Old Subroutine Name*/
	short		styp;	/*Subr symbol type*/
	unsigned short	brsv;	/*Bad Return Severity*/
	short		prec;	/*Display Precision*/
	DBLINK		inpa;	/*Input Link A*/
	DBLINK		inpb;	/*Input Link B*/
	DBLINK		inpc;	/*Input Link C*/
	DBLINK		inpd;	/*Input Link D*/
	DBLINK		inpe;	/*Input Link E*/
	DBLINK		inpf;	/*Input Link F*/
	DBLINK		inpg;	/*Input Link G*/
	DBLINK		inph;	/*Input Link H*/
	DBLINK		inpi;	/*Input Link I*/
	DBLINK		inpj;	/*Input Link J*/
	DBLINK		inpk;	/*Input Link K*/
	DBLINK		inpl;	/*Input Link L*/
	DBLINK		inpm;	/*Input Link M*/
	DBLINK		inpn;	/*Input Link N*/
	DBLINK		inpo;	/*Input Link O*/
	DBLINK		inpp;	/*Input Link P*/
	DBLINK		inpq;	/*Input Link Q*/
	DBLINK		inpr;	/*Input Link R*/
	DBLINK		inps;	/*Input Link S*/
	DBLINK		inpt;	/*Input Link T*/
	DBLINK		inpu;	/*Input Link U*/
	char		ufa[40]; /*Input Structure A*/
	char		ufb[40]; /*Input Structure B*/
	char		ufc[40]; /*Input Structure C*/
	char		ufd[40]; /*Input Structure D*/
	char		ufe[40]; /*Input Structure E*/
	char		uff[40]; /*Input Structure F*/
	char		ufg[40]; /*Input Structure G*/
	char		ufh[40]; /*Input Structure H*/
	char		ufi[40]; /*Input Structure I*/
	char		ufj[40]; /*Input Structure J*/
	char		ufk[40]; /*Input Structure K*/
	char		ufl[40]; /*Input Structure L*/
	char		ufm[40]; /*Input Structure M*/
	char		ufn[40]; /*Input Structure N*/
	char		ufo[40]; /*Input Structure O*/
	char		ufp[40]; /*Input Structure P*/
	char		ufq[40]; /*Input Structure Q*/
	char		ufr[40]; /*Input Structure R*/
	char		ufs[40]; /*Input Structure S*/
	char		uft[40]; /*Input Structure T*/
	char		ufu[40]; /*Input Structure U*/
	void *a;	/*Value of Input A*/
	void *b;	/*Value of Input B*/
	void *c;	/*Value of Input C*/
	void *d;	/*Value of Input D*/
	void *e;	/*Value of Input E*/
	void *f;	/*Value of Input F*/
	void *g;	/*Value of Input G*/
	void *h;	/*Value of Input H*/
	void *i;	/*Value of Input I*/
	void *j;	/*Value of Input J*/
	void *k;	/*Value of Input K*/
	void *l;	/*Value of Input L*/
	void *m;	/*Value of Input M*/
	void *n;	/*Value of Input N*/
	void *o;	/*Value of Input O*/
	void *p;	/*Value of Input P*/
	void *q;	/*Value of Input Q*/
	void *r;	/*Value of Input R*/
	void *s;	/*Value of Input S*/
	void *t;	/*Value of Input T*/
	void *u;	/*Value of Input U*/
	unsigned short	fta;	/*Type of A*/
	unsigned short	ftb;	/*Type of B*/
	unsigned short	ftc;	/*Type of C*/
	unsigned short	ftd;	/*Type of D*/
	unsigned short	fte;	/*Type of E*/
	unsigned short	ftf;	/*Type of F*/
	unsigned short	ftg;	/*Type of G*/
	unsigned short	fth;	/*Type of H*/
	unsigned short	fti;	/*Type of I*/
	unsigned short	ftj;	/*Type of J*/
	unsigned short	ftk;	/*Type of K*/
	unsigned short	ftl;	/*Type of L*/
	unsigned short	ftm;	/*Type of M*/
	unsigned short	ftn;	/*Type of N*/
	unsigned short	fto;	/*Type of O*/
	unsigned short	ftp;	/*Type of P*/
	unsigned short	ftq;	/*Type of Q*/
	unsigned short	ftr;	/*Type of R*/
	unsigned short	fts;	/*Type of S*/
	unsigned short	ftt;	/*Type of T*/
	unsigned short	ftu;	/*Type of U*/
	unsigned long	noa;	/*No. in A*/
	unsigned long	nob;	/*No. in B*/
	unsigned long	noc;	/*No. in C*/
	unsigned long	nod;	/*No. in D*/
	unsigned long	noe;	/*No. in E*/
	unsigned long	nof;	/*No. in F*/
	unsigned long	nog;	/*No. in G*/
	unsigned long	noh;	/*No. in H*/
	unsigned long	noi;	/*No. in I*/
	unsigned long	noj;	/*No. in J*/
	unsigned long	nok;	/*No. in K*/
	unsigned long	nol;	/*No. in L*/
	unsigned long	nom;	/*No. in M*/
	unsigned long	non;	/*No. in N*/
	unsigned long	noo;	/*No. in O*/
	unsigned long	nop;	/*No. in P*/
	unsigned long	noq;	/*No. in Q*/
	unsigned long	nor;	/*No. in R*/
	unsigned long	nos;	/*No. in S*/
	unsigned long	not;	/*No. in T*/
	unsigned long	nou;	/*No. in U*/
	DBLINK		outa;	/*Output Link A*/
	DBLINK		outb;	/*Output Link B*/
	DBLINK		outc;	/*Output Link C*/
	DBLINK		outd;	/*Output Link D*/
	DBLINK		oute;	/*Output Link E*/
	DBLINK		outf;	/*Output Link F*/
	DBLINK		outg;	/*Output Link G*/
	DBLINK		outh;	/*Output Link H*/
	DBLINK		outi;	/*Output Link I*/
	DBLINK		outj;	/*Output Link J*/
	DBLINK		outk;	/*Output Link K*/
	DBLINK		outl;	/*Output Link L*/
	DBLINK		outm;	/*Output Link M*/
	DBLINK		outn;	/*Output Link N*/
	DBLINK		outo;	/*Output Link O*/
	DBLINK		outp;	/*Output Link P*/
	DBLINK		outq;	/*Output Link Q*/
	DBLINK		outr;	/*Output Link R*/
	DBLINK		outs;	/*Output Link S*/
	DBLINK		outt;	/*Output Link T*/
	DBLINK		outu;	/*Output Link U*/
	char		ufva[40]; /*Output Structure A*/
	char		ufvb[40]; /*Output Structure B*/
	char		ufvc[40]; /*Output Structure C*/
	char		ufvd[40]; /*Output Structure D*/
	char		ufve[40]; /*Output Structure E*/
	char		ufvf[40]; /*Output Structure F*/
	char		ufvg[40]; /*Output Structure G*/
	char		ufvh[40]; /*Output Structure H*/
	char		ufvi[40]; /*Output Structure I*/
	char		ufvj[40]; /*Output Structure J*/
	char		ufvk[40]; /*Output Structure K*/
	char		ufvl[40]; /*Output Structure L*/
	char		ufvm[40]; /*Output Structure M*/
	char		ufvn[40]; /*Output Structure N*/
	char		ufvo[40]; /*Output Structure O*/
	char		ufvp[40]; /*Output Structure P*/
	char		ufvq[40]; /*Output Structure Q*/
	char		ufvr[40]; /*Output Structure R*/
	char		ufvs[40]; /*Output Structure S*/
	char		ufvt[40]; /*Output Structure T*/
	char		ufvu[40]; /*Output Structure U*/
	void *vala;	/*Value of Output A*/
	void *valb;	/*Value of Output B*/
	void *valc;	/*Value of Output C*/
	void *vald;	/*Value of Output D*/
	void *vale;	/*Value of Output E*/
	void *valf;	/*Value of Output F*/
	void *valg;	/*Value of Output G*/
	void *valh;	/*Value of Output H*/
	void *vali;	/*Value of Output I*/
	void *valj;	/*Value of Output J*/
	void *valk;	/*Value of Output K*/
	void *vall;	/*Value of Output L*/
	void *valm;	/*Value of Output M*/
	void *valn;	/*Value of Output N*/
	void *valo;	/*Value of Output O*/
	void *valp;	/*Value of Output P*/
	void *valq;	/*Value of Output Q*/
	void *valr;	/*Value of Output R*/
	void *vals;	/*Value of Output S*/
	void *valt;	/*Value of Output T*/
	void *valu;	/*Value of Output U*/
	void *ovla;	/*Old Output A*/
	void *ovlb;	/*Old Output B*/
	void *ovlc;	/*Old Output C*/
	void *ovld;	/*Old Output D*/
	void *ovle;	/*Old Output E*/
	void *ovlf;	/*Old Output F*/
	void *ovlg;	/*Old Output G*/
	void *ovlh;	/*Old Output H*/
	void *ovli;	/*Old Output I*/
	void *ovlj;	/*Old Output J*/
	void *ovlk;	/*Old Output K*/
	void *ovll;	/*Old Output L*/
	void *ovlm;	/*Old Output M*/
	void *ovln;	/*Old Output N*/
	void *ovlo;	/*Old Output O*/
	void *ovlp;	/*Old Output P*/
	void *ovlq;	/*Old Output Q*/
	void *ovlr;	/*Old Output R*/
	void *ovls;	/*Old Output S*/
	void *ovlt;	/*Old Output T*/
	void *ovlu;	/*Old Output U*/
	unsigned short	ftva;	/*Type of VALA*/
	unsigned short	ftvb;	/*Type of VALB*/
	unsigned short	ftvc;	/*Type of VALC*/
	unsigned short	ftvd;	/*Type of VALD*/
	unsigned short	ftve;	/*Type of VALE*/
	unsigned short	ftvf;	/*Type of VALF*/
	unsigned short	ftvg;	/*Type of VALG*/
	unsigned short	ftvh;	/*Type of VALH*/
	unsigned short	ftvi;	/*Type of VALI*/
	unsigned short	ftvj;	/*Type of VALJ*/
	unsigned short	ftvk;	/*Type of VALK*/
	unsigned short	ftvl;	/*Type of VALL*/
	unsigned short	ftvm;	/*Type of VALM*/
	unsigned short	ftvn;	/*Type of VALN*/
	unsigned short	ftvo;	/*Type of VALO*/
	unsigned short	ftvp;	/*Type of VALP*/
	unsigned short	ftvq;	/*Type of VALQ*/
	unsigned short	ftvr;	/*Type of VALR*/
	unsigned short	ftvs;	/*Type of VALS*/
	unsigned short	ftvt;	/*Type of VALT*/
	unsigned short	ftvu;	/*Type of VALU*/
	unsigned long	nova;	/*No. in VALA*/
	unsigned long	novb;	/*No. in VALB*/
	unsigned long	novc;	/*No. in VALC*/
	unsigned long	novd;	/*No. in VALD*/
	unsigned long	nove;	/*No. in VALE*/
	unsigned long	novf;	/*No. in VALF*/
	unsigned long	novg;	/*No. in VALG*/
	unsigned long	novh;	/*No. in VAlH*/
	unsigned long	novi;	/*No. in VALI*/
	unsigned long	novj;	/*No. in VALJ*/
	unsigned long	novk;	/*No. in VALK*/
	unsigned long	novl;	/*No. in VALL*/
	unsigned long	novm;	/*No. in VALM*/
	unsigned long	novn;	/*No. in VALN*/
	unsigned long	novo;	/*No. in VALO*/
	unsigned long	novp;	/*No. in VALP*/
	unsigned long	novq;	/*No. in VALQ*/
	unsigned long	novr;	/*No. in VALR*/
	unsigned long	novs;	/*No. in VALS*/
	unsigned long	novt;	/*No. in VALT*/
	unsigned long	novu;	/*No. in VALU*/
	unsigned long	tova;	/*Total bytes for VALA*/
	unsigned long	tovb;	/*Total bytes for VALB*/
	unsigned long	tovc;	/*Total bytes for VALC*/
	unsigned long	tovd;	/*Total bytes for VALD*/
	unsigned long	tove;	/*Total bytes for VALE*/
	unsigned long	tovf;	/*Total bytes for VALF*/
	unsigned long	tovg;	/*Total bytes for VALG*/
	unsigned long	tovh;	/*Total bytes for VAlH*/
	unsigned long	tovi;	/*Total bytes for VALI*/
	unsigned long	tovj;	/*Total bytes for VALJ*/
	unsigned long	tovk;	/*Total bytes for VALK*/
	unsigned long	tovl;	/*Total bytes for VALL*/
	unsigned long	tovm;	/*Total bytes for VALM*/
	unsigned long	tovn;	/*Total bytes for VALN*/
	unsigned long	tovo;	/*Total bytes for VALO*/
	unsigned long	tovp;	/*Total bytes for VALP*/
	unsigned long	tovq;	/*Total bytes for VALQ*/
	unsigned long	tovr;	/*Total bytes for VALR*/
	unsigned long	tovs;	/*Total bytes for VALS*/
	unsigned long	tovt;	/*Total bytes for VALT*/
	unsigned long	tovu;	/*Total bytes for VALU*/
} genSubRecord;
#define genSubRecordNAME	0
#define genSubRecordDESC	1
#define genSubRecordASG	2
#define genSubRecordSCAN	3
#define genSubRecordPINI	4
#define genSubRecordPHAS	5
#define genSubRecordEVNT	6
#define genSubRecordTSE	7
#define genSubRecordTSEL	8
#define genSubRecordDTYP	9
#define genSubRecordDISV	10
#define genSubRecordDISA	11
#define genSubRecordSDIS	12
#define genSubRecordMLOK	13
#define genSubRecordMLIS	14
#define genSubRecordDISP	15
#define genSubRecordPROC	16
#define genSubRecordSTAT	17
#define genSubRecordSEVR	18
#define genSubRecordNSTA	19
#define genSubRecordNSEV	20
#define genSubRecordACKS	21
#define genSubRecordACKT	22
#define genSubRecordDISS	23
#define genSubRecordLCNT	24
#define genSubRecordPACT	25
#define genSubRecordPUTF	26
#define genSubRecordRPRO	27
#define genSubRecordASP	28
#define genSubRecordPPN	29
#define genSubRecordPPNN	30
#define genSubRecordSPVT	31
#define genSubRecordRSET	32
#define genSubRecordDSET	33
#define genSubRecordDPVT	34
#define genSubRecordRDES	35
#define genSubRecordLSET	36
#define genSubRecordPRIO	37
#define genSubRecordTPRO	38
#define genSubRecordBKPT	39
#define genSubRecordUDF	40
#define genSubRecordTIME	41
#define genSubRecordFLNK	42
#define genSubRecordVERS	43
#define genSubRecordVAL	44
#define genSubRecordOVAL	45
#define genSubRecordSADR	46
#define genSubRecordOSAD	47
#define genSubRecordLFLG	48
#define genSubRecordEFLG	49
#define genSubRecordSUBL	50
#define genSubRecordINAM	51
#define genSubRecordSNAM	52
#define genSubRecordONAM	53
#define genSubRecordSTYP	54
#define genSubRecordBRSV	55
#define genSubRecordPREC	56
#define genSubRecordINPA	57
#define genSubRecordINPB	58
#define genSubRecordINPC	59
#define genSubRecordINPD	60
#define genSubRecordINPE	61
#define genSubRecordINPF	62
#define genSubRecordINPG	63
#define genSubRecordINPH	64
#define genSubRecordINPI	65
#define genSubRecordINPJ	66
#define genSubRecordINPK	67
#define genSubRecordINPL	68
#define genSubRecordINPM	69
#define genSubRecordINPN	70
#define genSubRecordINPO	71
#define genSubRecordINPP	72
#define genSubRecordINPQ	73
#define genSubRecordINPR	74
#define genSubRecordINPS	75
#define genSubRecordINPT	76
#define genSubRecordINPU	77
#define genSubRecordUFA	78
#define genSubRecordUFB	79
#define genSubRecordUFC	80
#define genSubRecordUFD	81
#define genSubRecordUFE	82
#define genSubRecordUFF	83
#define genSubRecordUFG	84
#define genSubRecordUFH	85
#define genSubRecordUFI	86
#define genSubRecordUFJ	87
#define genSubRecordUFK	88
#define genSubRecordUFL	89
#define genSubRecordUFM	90
#define genSubRecordUFN	91
#define genSubRecordUFO	92
#define genSubRecordUFP	93
#define genSubRecordUFQ	94
#define genSubRecordUFR	95
#define genSubRecordUFS	96
#define genSubRecordUFT	97
#define genSubRecordUFU	98
#define genSubRecordA	99
#define genSubRecordB	100
#define genSubRecordC	101
#define genSubRecordD	102
#define genSubRecordE	103
#define genSubRecordF	104
#define genSubRecordG	105
#define genSubRecordH	106
#define genSubRecordI	107
#define genSubRecordJ	108
#define genSubRecordK	109
#define genSubRecordL	110
#define genSubRecordM	111
#define genSubRecordN	112
#define genSubRecordO	113
#define genSubRecordP	114
#define genSubRecordQ	115
#define genSubRecordR	116
#define genSubRecordS	117
#define genSubRecordT	118
#define genSubRecordU	119
#define genSubRecordFTA	120
#define genSubRecordFTB	121
#define genSubRecordFTC	122
#define genSubRecordFTD	123
#define genSubRecordFTE	124
#define genSubRecordFTF	125
#define genSubRecordFTG	126
#define genSubRecordFTH	127
#define genSubRecordFTI	128
#define genSubRecordFTJ	129
#define genSubRecordFTK	130
#define genSubRecordFTL	131
#define genSubRecordFTM	132
#define genSubRecordFTN	133
#define genSubRecordFTO	134
#define genSubRecordFTP	135
#define genSubRecordFTQ	136
#define genSubRecordFTR	137
#define genSubRecordFTS	138
#define genSubRecordFTT	139
#define genSubRecordFTU	140
#define genSubRecordNOA	141
#define genSubRecordNOB	142
#define genSubRecordNOC	143
#define genSubRecordNOD	144
#define genSubRecordNOE	145
#define genSubRecordNOF	146
#define genSubRecordNOG	147
#define genSubRecordNOH	148
#define genSubRecordNOI	149
#define genSubRecordNOJ	150
#define genSubRecordNOK	151
#define genSubRecordNOL	152
#define genSubRecordNOM	153
#define genSubRecordNON	154
#define genSubRecordNOO	155
#define genSubRecordNOP	156
#define genSubRecordNOQ	157
#define genSubRecordNOR	158
#define genSubRecordNOS	159
#define genSubRecordNOT	160
#define genSubRecordNOU	161
#define genSubRecordOUTA	162
#define genSubRecordOUTB	163
#define genSubRecordOUTC	164
#define genSubRecordOUTD	165
#define genSubRecordOUTE	166
#define genSubRecordOUTF	167
#define genSubRecordOUTG	168
#define genSubRecordOUTH	169
#define genSubRecordOUTI	170
#define genSubRecordOUTJ	171
#define genSubRecordOUTK	172
#define genSubRecordOUTL	173
#define genSubRecordOUTM	174
#define genSubRecordOUTN	175
#define genSubRecordOUTO	176
#define genSubRecordOUTP	177
#define genSubRecordOUTQ	178
#define genSubRecordOUTR	179
#define genSubRecordOUTS	180
#define genSubRecordOUTT	181
#define genSubRecordOUTU	182
#define genSubRecordUFVA	183
#define genSubRecordUFVB	184
#define genSubRecordUFVC	185
#define genSubRecordUFVD	186
#define genSubRecordUFVE	187
#define genSubRecordUFVF	188
#define genSubRecordUFVG	189
#define genSubRecordUFVH	190
#define genSubRecordUFVI	191
#define genSubRecordUFVJ	192
#define genSubRecordUFVK	193
#define genSubRecordUFVL	194
#define genSubRecordUFVM	195
#define genSubRecordUFVN	196
#define genSubRecordUFVO	197
#define genSubRecordUFVP	198
#define genSubRecordUFVQ	199
#define genSubRecordUFVR	200
#define genSubRecordUFVS	201
#define genSubRecordUFVT	202
#define genSubRecordUFVU	203
#define genSubRecordVALA	204
#define genSubRecordVALB	205
#define genSubRecordVALC	206
#define genSubRecordVALD	207
#define genSubRecordVALE	208
#define genSubRecordVALF	209
#define genSubRecordVALG	210
#define genSubRecordVALH	211
#define genSubRecordVALI	212
#define genSubRecordVALJ	213
#define genSubRecordVALK	214
#define genSubRecordVALL	215
#define genSubRecordVALM	216
#define genSubRecordVALN	217
#define genSubRecordVALO	218
#define genSubRecordVALP	219
#define genSubRecordVALQ	220
#define genSubRecordVALR	221
#define genSubRecordVALS	222
#define genSubRecordVALT	223
#define genSubRecordVALU	224
#define genSubRecordOVLA	225
#define genSubRecordOVLB	226
#define genSubRecordOVLC	227
#define genSubRecordOVLD	228
#define genSubRecordOVLE	229
#define genSubRecordOVLF	230
#define genSubRecordOVLG	231
#define genSubRecordOVLH	232
#define genSubRecordOVLI	233
#define genSubRecordOVLJ	234
#define genSubRecordOVLK	235
#define genSubRecordOVLL	236
#define genSubRecordOVLM	237
#define genSubRecordOVLN	238
#define genSubRecordOVLO	239
#define genSubRecordOVLP	240
#define genSubRecordOVLQ	241
#define genSubRecordOVLR	242
#define genSubRecordOVLS	243
#define genSubRecordOVLT	244
#define genSubRecordOVLU	245
#define genSubRecordFTVA	246
#define genSubRecordFTVB	247
#define genSubRecordFTVC	248
#define genSubRecordFTVD	249
#define genSubRecordFTVE	250
#define genSubRecordFTVF	251
#define genSubRecordFTVG	252
#define genSubRecordFTVH	253
#define genSubRecordFTVI	254
#define genSubRecordFTVJ	255
#define genSubRecordFTVK	256
#define genSubRecordFTVL	257
#define genSubRecordFTVM	258
#define genSubRecordFTVN	259
#define genSubRecordFTVO	260
#define genSubRecordFTVP	261
#define genSubRecordFTVQ	262
#define genSubRecordFTVR	263
#define genSubRecordFTVS	264
#define genSubRecordFTVT	265
#define genSubRecordFTVU	266
#define genSubRecordNOVA	267
#define genSubRecordNOVB	268
#define genSubRecordNOVC	269
#define genSubRecordNOVD	270
#define genSubRecordNOVE	271
#define genSubRecordNOVF	272
#define genSubRecordNOVG	273
#define genSubRecordNOVH	274
#define genSubRecordNOVI	275
#define genSubRecordNOVJ	276
#define genSubRecordNOVK	277
#define genSubRecordNOVL	278
#define genSubRecordNOVM	279
#define genSubRecordNOVN	280
#define genSubRecordNOVO	281
#define genSubRecordNOVP	282
#define genSubRecordNOVQ	283
#define genSubRecordNOVR	284
#define genSubRecordNOVS	285
#define genSubRecordNOVT	286
#define genSubRecordNOVU	287
#define genSubRecordTOVA	288
#define genSubRecordTOVB	289
#define genSubRecordTOVC	290
#define genSubRecordTOVD	291
#define genSubRecordTOVE	292
#define genSubRecordTOVF	293
#define genSubRecordTOVG	294
#define genSubRecordTOVH	295
#define genSubRecordTOVI	296
#define genSubRecordTOVJ	297
#define genSubRecordTOVK	298
#define genSubRecordTOVL	299
#define genSubRecordTOVM	300
#define genSubRecordTOVN	301
#define genSubRecordTOVO	302
#define genSubRecordTOVP	303
#define genSubRecordTOVQ	304
#define genSubRecordTOVR	305
#define genSubRecordTOVS	306
#define genSubRecordTOVT	307
#define genSubRecordTOVU	308
#endif /*INCgenSubH*/
#ifdef GEN_SIZE_OFFSET
int genSubRecordSizeOffset(dbRecordType *pdbRecordType)
{
    genSubRecord *prec = 0;
  pdbRecordType->papFldDes[0]->size=sizeof(prec->name);
  pdbRecordType->papFldDes[0]->offset=(short)((char *)&prec->name - (char *)prec);
  pdbRecordType->papFldDes[1]->size=sizeof(prec->desc);
  pdbRecordType->papFldDes[1]->offset=(short)((char *)&prec->desc - (char *)prec);
  pdbRecordType->papFldDes[2]->size=sizeof(prec->asg);
  pdbRecordType->papFldDes[2]->offset=(short)((char *)&prec->asg - (char *)prec);
  pdbRecordType->papFldDes[3]->size=sizeof(prec->scan);
  pdbRecordType->papFldDes[3]->offset=(short)((char *)&prec->scan - (char *)prec);
  pdbRecordType->papFldDes[4]->size=sizeof(prec->pini);
  pdbRecordType->papFldDes[4]->offset=(short)((char *)&prec->pini - (char *)prec);
  pdbRecordType->papFldDes[5]->size=sizeof(prec->phas);
  pdbRecordType->papFldDes[5]->offset=(short)((char *)&prec->phas - (char *)prec);
  pdbRecordType->papFldDes[6]->size=sizeof(prec->evnt);
  pdbRecordType->papFldDes[6]->offset=(short)((char *)&prec->evnt - (char *)prec);
  pdbRecordType->papFldDes[7]->size=sizeof(prec->tse);
  pdbRecordType->papFldDes[7]->offset=(short)((char *)&prec->tse - (char *)prec);
  pdbRecordType->papFldDes[8]->size=sizeof(prec->tsel);
  pdbRecordType->papFldDes[8]->offset=(short)((char *)&prec->tsel - (char *)prec);
  pdbRecordType->papFldDes[9]->size=sizeof(prec->dtyp);
  pdbRecordType->papFldDes[9]->offset=(short)((char *)&prec->dtyp - (char *)prec);
  pdbRecordType->papFldDes[10]->size=sizeof(prec->disv);
  pdbRecordType->papFldDes[10]->offset=(short)((char *)&prec->disv - (char *)prec);
  pdbRecordType->papFldDes[11]->size=sizeof(prec->disa);
  pdbRecordType->papFldDes[11]->offset=(short)((char *)&prec->disa - (char *)prec);
  pdbRecordType->papFldDes[12]->size=sizeof(prec->sdis);
  pdbRecordType->papFldDes[12]->offset=(short)((char *)&prec->sdis - (char *)prec);
  pdbRecordType->papFldDes[13]->size=sizeof(prec->mlok);
  pdbRecordType->papFldDes[13]->offset=(short)((char *)&prec->mlok - (char *)prec);
  pdbRecordType->papFldDes[14]->size=sizeof(prec->mlis);
  pdbRecordType->papFldDes[14]->offset=(short)((char *)&prec->mlis - (char *)prec);
  pdbRecordType->papFldDes[15]->size=sizeof(prec->disp);
  pdbRecordType->papFldDes[15]->offset=(short)((char *)&prec->disp - (char *)prec);
  pdbRecordType->papFldDes[16]->size=sizeof(prec->proc);
  pdbRecordType->papFldDes[16]->offset=(short)((char *)&prec->proc - (char *)prec);
  pdbRecordType->papFldDes[17]->size=sizeof(prec->stat);
  pdbRecordType->papFldDes[17]->offset=(short)((char *)&prec->stat - (char *)prec);
  pdbRecordType->papFldDes[18]->size=sizeof(prec->sevr);
  pdbRecordType->papFldDes[18]->offset=(short)((char *)&prec->sevr - (char *)prec);
  pdbRecordType->papFldDes[19]->size=sizeof(prec->nsta);
  pdbRecordType->papFldDes[19]->offset=(short)((char *)&prec->nsta - (char *)prec);
  pdbRecordType->papFldDes[20]->size=sizeof(prec->nsev);
  pdbRecordType->papFldDes[20]->offset=(short)((char *)&prec->nsev - (char *)prec);
  pdbRecordType->papFldDes[21]->size=sizeof(prec->acks);
  pdbRecordType->papFldDes[21]->offset=(short)((char *)&prec->acks - (char *)prec);
  pdbRecordType->papFldDes[22]->size=sizeof(prec->ackt);
  pdbRecordType->papFldDes[22]->offset=(short)((char *)&prec->ackt - (char *)prec);
  pdbRecordType->papFldDes[23]->size=sizeof(prec->diss);
  pdbRecordType->papFldDes[23]->offset=(short)((char *)&prec->diss - (char *)prec);
  pdbRecordType->papFldDes[24]->size=sizeof(prec->lcnt);
  pdbRecordType->papFldDes[24]->offset=(short)((char *)&prec->lcnt - (char *)prec);
  pdbRecordType->papFldDes[25]->size=sizeof(prec->pact);
  pdbRecordType->papFldDes[25]->offset=(short)((char *)&prec->pact - (char *)prec);
  pdbRecordType->papFldDes[26]->size=sizeof(prec->putf);
  pdbRecordType->papFldDes[26]->offset=(short)((char *)&prec->putf - (char *)prec);
  pdbRecordType->papFldDes[27]->size=sizeof(prec->rpro);
  pdbRecordType->papFldDes[27]->offset=(short)((char *)&prec->rpro - (char *)prec);
  pdbRecordType->papFldDes[28]->size=sizeof(prec->asp);
  pdbRecordType->papFldDes[28]->offset=(short)((char *)&prec->asp - (char *)prec);
  pdbRecordType->papFldDes[29]->size=sizeof(prec->ppn);
  pdbRecordType->papFldDes[29]->offset=(short)((char *)&prec->ppn - (char *)prec);
  pdbRecordType->papFldDes[30]->size=sizeof(prec->ppnn);
  pdbRecordType->papFldDes[30]->offset=(short)((char *)&prec->ppnn - (char *)prec);
  pdbRecordType->papFldDes[31]->size=sizeof(prec->spvt);
  pdbRecordType->papFldDes[31]->offset=(short)((char *)&prec->spvt - (char *)prec);
  pdbRecordType->papFldDes[32]->size=sizeof(prec->rset);
  pdbRecordType->papFldDes[32]->offset=(short)((char *)&prec->rset - (char *)prec);
  pdbRecordType->papFldDes[33]->size=sizeof(prec->dset);
  pdbRecordType->papFldDes[33]->offset=(short)((char *)&prec->dset - (char *)prec);
  pdbRecordType->papFldDes[34]->size=sizeof(prec->dpvt);
  pdbRecordType->papFldDes[34]->offset=(short)((char *)&prec->dpvt - (char *)prec);
  pdbRecordType->papFldDes[35]->size=sizeof(prec->rdes);
  pdbRecordType->papFldDes[35]->offset=(short)((char *)&prec->rdes - (char *)prec);
  pdbRecordType->papFldDes[36]->size=sizeof(prec->lset);
  pdbRecordType->papFldDes[36]->offset=(short)((char *)&prec->lset - (char *)prec);
  pdbRecordType->papFldDes[37]->size=sizeof(prec->prio);
  pdbRecordType->papFldDes[37]->offset=(short)((char *)&prec->prio - (char *)prec);
  pdbRecordType->papFldDes[38]->size=sizeof(prec->tpro);
  pdbRecordType->papFldDes[38]->offset=(short)((char *)&prec->tpro - (char *)prec);
  pdbRecordType->papFldDes[39]->size=sizeof(prec->bkpt);
  pdbRecordType->papFldDes[39]->offset=(short)((char *)&prec->bkpt - (char *)prec);
  pdbRecordType->papFldDes[40]->size=sizeof(prec->udf);
  pdbRecordType->papFldDes[40]->offset=(short)((char *)&prec->udf - (char *)prec);
  pdbRecordType->papFldDes[41]->size=sizeof(prec->time);
  pdbRecordType->papFldDes[41]->offset=(short)((char *)&prec->time - (char *)prec);
  pdbRecordType->papFldDes[42]->size=sizeof(prec->flnk);
  pdbRecordType->papFldDes[42]->offset=(short)((char *)&prec->flnk - (char *)prec);
  pdbRecordType->papFldDes[43]->size=sizeof(prec->vers);
  pdbRecordType->papFldDes[43]->offset=(short)((char *)&prec->vers - (char *)prec);
  pdbRecordType->papFldDes[44]->size=sizeof(prec->val);
  pdbRecordType->papFldDes[44]->offset=(short)((char *)&prec->val - (char *)prec);
  pdbRecordType->papFldDes[45]->size=sizeof(prec->oval);
  pdbRecordType->papFldDes[45]->offset=(short)((char *)&prec->oval - (char *)prec);
  pdbRecordType->papFldDes[46]->size=sizeof(prec->sadr);
  pdbRecordType->papFldDes[46]->offset=(short)((char *)&prec->sadr - (char *)prec);
  pdbRecordType->papFldDes[47]->size=sizeof(prec->osad);
  pdbRecordType->papFldDes[47]->offset=(short)((char *)&prec->osad - (char *)prec);
  pdbRecordType->papFldDes[48]->size=sizeof(prec->lflg);
  pdbRecordType->papFldDes[48]->offset=(short)((char *)&prec->lflg - (char *)prec);
  pdbRecordType->papFldDes[49]->size=sizeof(prec->eflg);
  pdbRecordType->papFldDes[49]->offset=(short)((char *)&prec->eflg - (char *)prec);
  pdbRecordType->papFldDes[50]->size=sizeof(prec->subl);
  pdbRecordType->papFldDes[50]->offset=(short)((char *)&prec->subl - (char *)prec);
  pdbRecordType->papFldDes[51]->size=sizeof(prec->inam);
  pdbRecordType->papFldDes[51]->offset=(short)((char *)&prec->inam - (char *)prec);
  pdbRecordType->papFldDes[52]->size=sizeof(prec->snam);
  pdbRecordType->papFldDes[52]->offset=(short)((char *)&prec->snam - (char *)prec);
  pdbRecordType->papFldDes[53]->size=sizeof(prec->onam);
  pdbRecordType->papFldDes[53]->offset=(short)((char *)&prec->onam - (char *)prec);
  pdbRecordType->papFldDes[54]->size=sizeof(prec->styp);
  pdbRecordType->papFldDes[54]->offset=(short)((char *)&prec->styp - (char *)prec);
  pdbRecordType->papFldDes[55]->size=sizeof(prec->brsv);
  pdbRecordType->papFldDes[55]->offset=(short)((char *)&prec->brsv - (char *)prec);
  pdbRecordType->papFldDes[56]->size=sizeof(prec->prec);
  pdbRecordType->papFldDes[56]->offset=(short)((char *)&prec->prec - (char *)prec);
  pdbRecordType->papFldDes[57]->size=sizeof(prec->inpa);
  pdbRecordType->papFldDes[57]->offset=(short)((char *)&prec->inpa - (char *)prec);
  pdbRecordType->papFldDes[58]->size=sizeof(prec->inpb);
  pdbRecordType->papFldDes[58]->offset=(short)((char *)&prec->inpb - (char *)prec);
  pdbRecordType->papFldDes[59]->size=sizeof(prec->inpc);
  pdbRecordType->papFldDes[59]->offset=(short)((char *)&prec->inpc - (char *)prec);
  pdbRecordType->papFldDes[60]->size=sizeof(prec->inpd);
  pdbRecordType->papFldDes[60]->offset=(short)((char *)&prec->inpd - (char *)prec);
  pdbRecordType->papFldDes[61]->size=sizeof(prec->inpe);
  pdbRecordType->papFldDes[61]->offset=(short)((char *)&prec->inpe - (char *)prec);
  pdbRecordType->papFldDes[62]->size=sizeof(prec->inpf);
  pdbRecordType->papFldDes[62]->offset=(short)((char *)&prec->inpf - (char *)prec);
  pdbRecordType->papFldDes[63]->size=sizeof(prec->inpg);
  pdbRecordType->papFldDes[63]->offset=(short)((char *)&prec->inpg - (char *)prec);
  pdbRecordType->papFldDes[64]->size=sizeof(prec->inph);
  pdbRecordType->papFldDes[64]->offset=(short)((char *)&prec->inph - (char *)prec);
  pdbRecordType->papFldDes[65]->size=sizeof(prec->inpi);
  pdbRecordType->papFldDes[65]->offset=(short)((char *)&prec->inpi - (char *)prec);
  pdbRecordType->papFldDes[66]->size=sizeof(prec->inpj);
  pdbRecordType->papFldDes[66]->offset=(short)((char *)&prec->inpj - (char *)prec);
  pdbRecordType->papFldDes[67]->size=sizeof(prec->inpk);
  pdbRecordType->papFldDes[67]->offset=(short)((char *)&prec->inpk - (char *)prec);
  pdbRecordType->papFldDes[68]->size=sizeof(prec->inpl);
  pdbRecordType->papFldDes[68]->offset=(short)((char *)&prec->inpl - (char *)prec);
  pdbRecordType->papFldDes[69]->size=sizeof(prec->inpm);
  pdbRecordType->papFldDes[69]->offset=(short)((char *)&prec->inpm - (char *)prec);
  pdbRecordType->papFldDes[70]->size=sizeof(prec->inpn);
  pdbRecordType->papFldDes[70]->offset=(short)((char *)&prec->inpn - (char *)prec);
  pdbRecordType->papFldDes[71]->size=sizeof(prec->inpo);
  pdbRecordType->papFldDes[71]->offset=(short)((char *)&prec->inpo - (char *)prec);
  pdbRecordType->papFldDes[72]->size=sizeof(prec->inpp);
  pdbRecordType->papFldDes[72]->offset=(short)((char *)&prec->inpp - (char *)prec);
  pdbRecordType->papFldDes[73]->size=sizeof(prec->inpq);
  pdbRecordType->papFldDes[73]->offset=(short)((char *)&prec->inpq - (char *)prec);
  pdbRecordType->papFldDes[74]->size=sizeof(prec->inpr);
  pdbRecordType->papFldDes[74]->offset=(short)((char *)&prec->inpr - (char *)prec);
  pdbRecordType->papFldDes[75]->size=sizeof(prec->inps);
  pdbRecordType->papFldDes[75]->offset=(short)((char *)&prec->inps - (char *)prec);
  pdbRecordType->papFldDes[76]->size=sizeof(prec->inpt);
  pdbRecordType->papFldDes[76]->offset=(short)((char *)&prec->inpt - (char *)prec);
  pdbRecordType->papFldDes[77]->size=sizeof(prec->inpu);
  pdbRecordType->papFldDes[77]->offset=(short)((char *)&prec->inpu - (char *)prec);
  pdbRecordType->papFldDes[78]->size=sizeof(prec->ufa);
  pdbRecordType->papFldDes[78]->offset=(short)((char *)&prec->ufa - (char *)prec);
  pdbRecordType->papFldDes[79]->size=sizeof(prec->ufb);
  pdbRecordType->papFldDes[79]->offset=(short)((char *)&prec->ufb - (char *)prec);
  pdbRecordType->papFldDes[80]->size=sizeof(prec->ufc);
  pdbRecordType->papFldDes[80]->offset=(short)((char *)&prec->ufc - (char *)prec);
  pdbRecordType->papFldDes[81]->size=sizeof(prec->ufd);
  pdbRecordType->papFldDes[81]->offset=(short)((char *)&prec->ufd - (char *)prec);
  pdbRecordType->papFldDes[82]->size=sizeof(prec->ufe);
  pdbRecordType->papFldDes[82]->offset=(short)((char *)&prec->ufe - (char *)prec);
  pdbRecordType->papFldDes[83]->size=sizeof(prec->uff);
  pdbRecordType->papFldDes[83]->offset=(short)((char *)&prec->uff - (char *)prec);
  pdbRecordType->papFldDes[84]->size=sizeof(prec->ufg);
  pdbRecordType->papFldDes[84]->offset=(short)((char *)&prec->ufg - (char *)prec);
  pdbRecordType->papFldDes[85]->size=sizeof(prec->ufh);
  pdbRecordType->papFldDes[85]->offset=(short)((char *)&prec->ufh - (char *)prec);
  pdbRecordType->papFldDes[86]->size=sizeof(prec->ufi);
  pdbRecordType->papFldDes[86]->offset=(short)((char *)&prec->ufi - (char *)prec);
  pdbRecordType->papFldDes[87]->size=sizeof(prec->ufj);
  pdbRecordType->papFldDes[87]->offset=(short)((char *)&prec->ufj - (char *)prec);
  pdbRecordType->papFldDes[88]->size=sizeof(prec->ufk);
  pdbRecordType->papFldDes[88]->offset=(short)((char *)&prec->ufk - (char *)prec);
  pdbRecordType->papFldDes[89]->size=sizeof(prec->ufl);
  pdbRecordType->papFldDes[89]->offset=(short)((char *)&prec->ufl - (char *)prec);
  pdbRecordType->papFldDes[90]->size=sizeof(prec->ufm);
  pdbRecordType->papFldDes[90]->offset=(short)((char *)&prec->ufm - (char *)prec);
  pdbRecordType->papFldDes[91]->size=sizeof(prec->ufn);
  pdbRecordType->papFldDes[91]->offset=(short)((char *)&prec->ufn - (char *)prec);
  pdbRecordType->papFldDes[92]->size=sizeof(prec->ufo);
  pdbRecordType->papFldDes[92]->offset=(short)((char *)&prec->ufo - (char *)prec);
  pdbRecordType->papFldDes[93]->size=sizeof(prec->ufp);
  pdbRecordType->papFldDes[93]->offset=(short)((char *)&prec->ufp - (char *)prec);
  pdbRecordType->papFldDes[94]->size=sizeof(prec->ufq);
  pdbRecordType->papFldDes[94]->offset=(short)((char *)&prec->ufq - (char *)prec);
  pdbRecordType->papFldDes[95]->size=sizeof(prec->ufr);
  pdbRecordType->papFldDes[95]->offset=(short)((char *)&prec->ufr - (char *)prec);
  pdbRecordType->papFldDes[96]->size=sizeof(prec->ufs);
  pdbRecordType->papFldDes[96]->offset=(short)((char *)&prec->ufs - (char *)prec);
  pdbRecordType->papFldDes[97]->size=sizeof(prec->uft);
  pdbRecordType->papFldDes[97]->offset=(short)((char *)&prec->uft - (char *)prec);
  pdbRecordType->papFldDes[98]->size=sizeof(prec->ufu);
  pdbRecordType->papFldDes[98]->offset=(short)((char *)&prec->ufu - (char *)prec);
  pdbRecordType->papFldDes[99]->size=sizeof(prec->a);
  pdbRecordType->papFldDes[99]->offset=(short)((char *)&prec->a - (char *)prec);
  pdbRecordType->papFldDes[100]->size=sizeof(prec->b);
  pdbRecordType->papFldDes[100]->offset=(short)((char *)&prec->b - (char *)prec);
  pdbRecordType->papFldDes[101]->size=sizeof(prec->c);
  pdbRecordType->papFldDes[101]->offset=(short)((char *)&prec->c - (char *)prec);
  pdbRecordType->papFldDes[102]->size=sizeof(prec->d);
  pdbRecordType->papFldDes[102]->offset=(short)((char *)&prec->d - (char *)prec);
  pdbRecordType->papFldDes[103]->size=sizeof(prec->e);
  pdbRecordType->papFldDes[103]->offset=(short)((char *)&prec->e - (char *)prec);
  pdbRecordType->papFldDes[104]->size=sizeof(prec->f);
  pdbRecordType->papFldDes[104]->offset=(short)((char *)&prec->f - (char *)prec);
  pdbRecordType->papFldDes[105]->size=sizeof(prec->g);
  pdbRecordType->papFldDes[105]->offset=(short)((char *)&prec->g - (char *)prec);
  pdbRecordType->papFldDes[106]->size=sizeof(prec->h);
  pdbRecordType->papFldDes[106]->offset=(short)((char *)&prec->h - (char *)prec);
  pdbRecordType->papFldDes[107]->size=sizeof(prec->i);
  pdbRecordType->papFldDes[107]->offset=(short)((char *)&prec->i - (char *)prec);
  pdbRecordType->papFldDes[108]->size=sizeof(prec->j);
  pdbRecordType->papFldDes[108]->offset=(short)((char *)&prec->j - (char *)prec);
  pdbRecordType->papFldDes[109]->size=sizeof(prec->k);
  pdbRecordType->papFldDes[109]->offset=(short)((char *)&prec->k - (char *)prec);
  pdbRecordType->papFldDes[110]->size=sizeof(prec->l);
  pdbRecordType->papFldDes[110]->offset=(short)((char *)&prec->l - (char *)prec);
  pdbRecordType->papFldDes[111]->size=sizeof(prec->m);
  pdbRecordType->papFldDes[111]->offset=(short)((char *)&prec->m - (char *)prec);
  pdbRecordType->papFldDes[112]->size=sizeof(prec->n);
  pdbRecordType->papFldDes[112]->offset=(short)((char *)&prec->n - (char *)prec);
  pdbRecordType->papFldDes[113]->size=sizeof(prec->o);
  pdbRecordType->papFldDes[113]->offset=(short)((char *)&prec->o - (char *)prec);
  pdbRecordType->papFldDes[114]->size=sizeof(prec->p);
  pdbRecordType->papFldDes[114]->offset=(short)((char *)&prec->p - (char *)prec);
  pdbRecordType->papFldDes[115]->size=sizeof(prec->q);
  pdbRecordType->papFldDes[115]->offset=(short)((char *)&prec->q - (char *)prec);
  pdbRecordType->papFldDes[116]->size=sizeof(prec->r);
  pdbRecordType->papFldDes[116]->offset=(short)((char *)&prec->r - (char *)prec);
  pdbRecordType->papFldDes[117]->size=sizeof(prec->s);
  pdbRecordType->papFldDes[117]->offset=(short)((char *)&prec->s - (char *)prec);
  pdbRecordType->papFldDes[118]->size=sizeof(prec->t);
  pdbRecordType->papFldDes[118]->offset=(short)((char *)&prec->t - (char *)prec);
  pdbRecordType->papFldDes[119]->size=sizeof(prec->u);
  pdbRecordType->papFldDes[119]->offset=(short)((char *)&prec->u - (char *)prec);
  pdbRecordType->papFldDes[120]->size=sizeof(prec->fta);
  pdbRecordType->papFldDes[120]->offset=(short)((char *)&prec->fta - (char *)prec);
  pdbRecordType->papFldDes[121]->size=sizeof(prec->ftb);
  pdbRecordType->papFldDes[121]->offset=(short)((char *)&prec->ftb - (char *)prec);
  pdbRecordType->papFldDes[122]->size=sizeof(prec->ftc);
  pdbRecordType->papFldDes[122]->offset=(short)((char *)&prec->ftc - (char *)prec);
  pdbRecordType->papFldDes[123]->size=sizeof(prec->ftd);
  pdbRecordType->papFldDes[123]->offset=(short)((char *)&prec->ftd - (char *)prec);
  pdbRecordType->papFldDes[124]->size=sizeof(prec->fte);
  pdbRecordType->papFldDes[124]->offset=(short)((char *)&prec->fte - (char *)prec);
  pdbRecordType->papFldDes[125]->size=sizeof(prec->ftf);
  pdbRecordType->papFldDes[125]->offset=(short)((char *)&prec->ftf - (char *)prec);
  pdbRecordType->papFldDes[126]->size=sizeof(prec->ftg);
  pdbRecordType->papFldDes[126]->offset=(short)((char *)&prec->ftg - (char *)prec);
  pdbRecordType->papFldDes[127]->size=sizeof(prec->fth);
  pdbRecordType->papFldDes[127]->offset=(short)((char *)&prec->fth - (char *)prec);
  pdbRecordType->papFldDes[128]->size=sizeof(prec->fti);
  pdbRecordType->papFldDes[128]->offset=(short)((char *)&prec->fti - (char *)prec);
  pdbRecordType->papFldDes[129]->size=sizeof(prec->ftj);
  pdbRecordType->papFldDes[129]->offset=(short)((char *)&prec->ftj - (char *)prec);
  pdbRecordType->papFldDes[130]->size=sizeof(prec->ftk);
  pdbRecordType->papFldDes[130]->offset=(short)((char *)&prec->ftk - (char *)prec);
  pdbRecordType->papFldDes[131]->size=sizeof(prec->ftl);
  pdbRecordType->papFldDes[131]->offset=(short)((char *)&prec->ftl - (char *)prec);
  pdbRecordType->papFldDes[132]->size=sizeof(prec->ftm);
  pdbRecordType->papFldDes[132]->offset=(short)((char *)&prec->ftm - (char *)prec);
  pdbRecordType->papFldDes[133]->size=sizeof(prec->ftn);
  pdbRecordType->papFldDes[133]->offset=(short)((char *)&prec->ftn - (char *)prec);
  pdbRecordType->papFldDes[134]->size=sizeof(prec->fto);
  pdbRecordType->papFldDes[134]->offset=(short)((char *)&prec->fto - (char *)prec);
  pdbRecordType->papFldDes[135]->size=sizeof(prec->ftp);
  pdbRecordType->papFldDes[135]->offset=(short)((char *)&prec->ftp - (char *)prec);
  pdbRecordType->papFldDes[136]->size=sizeof(prec->ftq);
  pdbRecordType->papFldDes[136]->offset=(short)((char *)&prec->ftq - (char *)prec);
  pdbRecordType->papFldDes[137]->size=sizeof(prec->ftr);
  pdbRecordType->papFldDes[137]->offset=(short)((char *)&prec->ftr - (char *)prec);
  pdbRecordType->papFldDes[138]->size=sizeof(prec->fts);
  pdbRecordType->papFldDes[138]->offset=(short)((char *)&prec->fts - (char *)prec);
  pdbRecordType->papFldDes[139]->size=sizeof(prec->ftt);
  pdbRecordType->papFldDes[139]->offset=(short)((char *)&prec->ftt - (char *)prec);
  pdbRecordType->papFldDes[140]->size=sizeof(prec->ftu);
  pdbRecordType->papFldDes[140]->offset=(short)((char *)&prec->ftu - (char *)prec);
  pdbRecordType->papFldDes[141]->size=sizeof(prec->noa);
  pdbRecordType->papFldDes[141]->offset=(short)((char *)&prec->noa - (char *)prec);
  pdbRecordType->papFldDes[142]->size=sizeof(prec->nob);
  pdbRecordType->papFldDes[142]->offset=(short)((char *)&prec->nob - (char *)prec);
  pdbRecordType->papFldDes[143]->size=sizeof(prec->noc);
  pdbRecordType->papFldDes[143]->offset=(short)((char *)&prec->noc - (char *)prec);
  pdbRecordType->papFldDes[144]->size=sizeof(prec->nod);
  pdbRecordType->papFldDes[144]->offset=(short)((char *)&prec->nod - (char *)prec);
  pdbRecordType->papFldDes[145]->size=sizeof(prec->noe);
  pdbRecordType->papFldDes[145]->offset=(short)((char *)&prec->noe - (char *)prec);
  pdbRecordType->papFldDes[146]->size=sizeof(prec->nof);
  pdbRecordType->papFldDes[146]->offset=(short)((char *)&prec->nof - (char *)prec);
  pdbRecordType->papFldDes[147]->size=sizeof(prec->nog);
  pdbRecordType->papFldDes[147]->offset=(short)((char *)&prec->nog - (char *)prec);
  pdbRecordType->papFldDes[148]->size=sizeof(prec->noh);
  pdbRecordType->papFldDes[148]->offset=(short)((char *)&prec->noh - (char *)prec);
  pdbRecordType->papFldDes[149]->size=sizeof(prec->noi);
  pdbRecordType->papFldDes[149]->offset=(short)((char *)&prec->noi - (char *)prec);
  pdbRecordType->papFldDes[150]->size=sizeof(prec->noj);
  pdbRecordType->papFldDes[150]->offset=(short)((char *)&prec->noj - (char *)prec);
  pdbRecordType->papFldDes[151]->size=sizeof(prec->nok);
  pdbRecordType->papFldDes[151]->offset=(short)((char *)&prec->nok - (char *)prec);
  pdbRecordType->papFldDes[152]->size=sizeof(prec->nol);
  pdbRecordType->papFldDes[152]->offset=(short)((char *)&prec->nol - (char *)prec);
  pdbRecordType->papFldDes[153]->size=sizeof(prec->nom);
  pdbRecordType->papFldDes[153]->offset=(short)((char *)&prec->nom - (char *)prec);
  pdbRecordType->papFldDes[154]->size=sizeof(prec->non);
  pdbRecordType->papFldDes[154]->offset=(short)((char *)&prec->non - (char *)prec);
  pdbRecordType->papFldDes[155]->size=sizeof(prec->noo);
  pdbRecordType->papFldDes[155]->offset=(short)((char *)&prec->noo - (char *)prec);
  pdbRecordType->papFldDes[156]->size=sizeof(prec->nop);
  pdbRecordType->papFldDes[156]->offset=(short)((char *)&prec->nop - (char *)prec);
  pdbRecordType->papFldDes[157]->size=sizeof(prec->noq);
  pdbRecordType->papFldDes[157]->offset=(short)((char *)&prec->noq - (char *)prec);
  pdbRecordType->papFldDes[158]->size=sizeof(prec->nor);
  pdbRecordType->papFldDes[158]->offset=(short)((char *)&prec->nor - (char *)prec);
  pdbRecordType->papFldDes[159]->size=sizeof(prec->nos);
  pdbRecordType->papFldDes[159]->offset=(short)((char *)&prec->nos - (char *)prec);
  pdbRecordType->papFldDes[160]->size=sizeof(prec->not);
  pdbRecordType->papFldDes[160]->offset=(short)((char *)&prec->not - (char *)prec);
  pdbRecordType->papFldDes[161]->size=sizeof(prec->nou);
  pdbRecordType->papFldDes[161]->offset=(short)((char *)&prec->nou - (char *)prec);
  pdbRecordType->papFldDes[162]->size=sizeof(prec->outa);
  pdbRecordType->papFldDes[162]->offset=(short)((char *)&prec->outa - (char *)prec);
  pdbRecordType->papFldDes[163]->size=sizeof(prec->outb);
  pdbRecordType->papFldDes[163]->offset=(short)((char *)&prec->outb - (char *)prec);
  pdbRecordType->papFldDes[164]->size=sizeof(prec->outc);
  pdbRecordType->papFldDes[164]->offset=(short)((char *)&prec->outc - (char *)prec);
  pdbRecordType->papFldDes[165]->size=sizeof(prec->outd);
  pdbRecordType->papFldDes[165]->offset=(short)((char *)&prec->outd - (char *)prec);
  pdbRecordType->papFldDes[166]->size=sizeof(prec->oute);
  pdbRecordType->papFldDes[166]->offset=(short)((char *)&prec->oute - (char *)prec);
  pdbRecordType->papFldDes[167]->size=sizeof(prec->outf);
  pdbRecordType->papFldDes[167]->offset=(short)((char *)&prec->outf - (char *)prec);
  pdbRecordType->papFldDes[168]->size=sizeof(prec->outg);
  pdbRecordType->papFldDes[168]->offset=(short)((char *)&prec->outg - (char *)prec);
  pdbRecordType->papFldDes[169]->size=sizeof(prec->outh);
  pdbRecordType->papFldDes[169]->offset=(short)((char *)&prec->outh - (char *)prec);
  pdbRecordType->papFldDes[170]->size=sizeof(prec->outi);
  pdbRecordType->papFldDes[170]->offset=(short)((char *)&prec->outi - (char *)prec);
  pdbRecordType->papFldDes[171]->size=sizeof(prec->outj);
  pdbRecordType->papFldDes[171]->offset=(short)((char *)&prec->outj - (char *)prec);
  pdbRecordType->papFldDes[172]->size=sizeof(prec->outk);
  pdbRecordType->papFldDes[172]->offset=(short)((char *)&prec->outk - (char *)prec);
  pdbRecordType->papFldDes[173]->size=sizeof(prec->outl);
  pdbRecordType->papFldDes[173]->offset=(short)((char *)&prec->outl - (char *)prec);
  pdbRecordType->papFldDes[174]->size=sizeof(prec->outm);
  pdbRecordType->papFldDes[174]->offset=(short)((char *)&prec->outm - (char *)prec);
  pdbRecordType->papFldDes[175]->size=sizeof(prec->outn);
  pdbRecordType->papFldDes[175]->offset=(short)((char *)&prec->outn - (char *)prec);
  pdbRecordType->papFldDes[176]->size=sizeof(prec->outo);
  pdbRecordType->papFldDes[176]->offset=(short)((char *)&prec->outo - (char *)prec);
  pdbRecordType->papFldDes[177]->size=sizeof(prec->outp);
  pdbRecordType->papFldDes[177]->offset=(short)((char *)&prec->outp - (char *)prec);
  pdbRecordType->papFldDes[178]->size=sizeof(prec->outq);
  pdbRecordType->papFldDes[178]->offset=(short)((char *)&prec->outq - (char *)prec);
  pdbRecordType->papFldDes[179]->size=sizeof(prec->outr);
  pdbRecordType->papFldDes[179]->offset=(short)((char *)&prec->outr - (char *)prec);
  pdbRecordType->papFldDes[180]->size=sizeof(prec->outs);
  pdbRecordType->papFldDes[180]->offset=(short)((char *)&prec->outs - (char *)prec);
  pdbRecordType->papFldDes[181]->size=sizeof(prec->outt);
  pdbRecordType->papFldDes[181]->offset=(short)((char *)&prec->outt - (char *)prec);
  pdbRecordType->papFldDes[182]->size=sizeof(prec->outu);
  pdbRecordType->papFldDes[182]->offset=(short)((char *)&prec->outu - (char *)prec);
  pdbRecordType->papFldDes[183]->size=sizeof(prec->ufva);
  pdbRecordType->papFldDes[183]->offset=(short)((char *)&prec->ufva - (char *)prec);
  pdbRecordType->papFldDes[184]->size=sizeof(prec->ufvb);
  pdbRecordType->papFldDes[184]->offset=(short)((char *)&prec->ufvb - (char *)prec);
  pdbRecordType->papFldDes[185]->size=sizeof(prec->ufvc);
  pdbRecordType->papFldDes[185]->offset=(short)((char *)&prec->ufvc - (char *)prec);
  pdbRecordType->papFldDes[186]->size=sizeof(prec->ufvd);
  pdbRecordType->papFldDes[186]->offset=(short)((char *)&prec->ufvd - (char *)prec);
  pdbRecordType->papFldDes[187]->size=sizeof(prec->ufve);
  pdbRecordType->papFldDes[187]->offset=(short)((char *)&prec->ufve - (char *)prec);
  pdbRecordType->papFldDes[188]->size=sizeof(prec->ufvf);
  pdbRecordType->papFldDes[188]->offset=(short)((char *)&prec->ufvf - (char *)prec);
  pdbRecordType->papFldDes[189]->size=sizeof(prec->ufvg);
  pdbRecordType->papFldDes[189]->offset=(short)((char *)&prec->ufvg - (char *)prec);
  pdbRecordType->papFldDes[190]->size=sizeof(prec->ufvh);
  pdbRecordType->papFldDes[190]->offset=(short)((char *)&prec->ufvh - (char *)prec);
  pdbRecordType->papFldDes[191]->size=sizeof(prec->ufvi);
  pdbRecordType->papFldDes[191]->offset=(short)((char *)&prec->ufvi - (char *)prec);
  pdbRecordType->papFldDes[192]->size=sizeof(prec->ufvj);
  pdbRecordType->papFldDes[192]->offset=(short)((char *)&prec->ufvj - (char *)prec);
  pdbRecordType->papFldDes[193]->size=sizeof(prec->ufvk);
  pdbRecordType->papFldDes[193]->offset=(short)((char *)&prec->ufvk - (char *)prec);
  pdbRecordType->papFldDes[194]->size=sizeof(prec->ufvl);
  pdbRecordType->papFldDes[194]->offset=(short)((char *)&prec->ufvl - (char *)prec);
  pdbRecordType->papFldDes[195]->size=sizeof(prec->ufvm);
  pdbRecordType->papFldDes[195]->offset=(short)((char *)&prec->ufvm - (char *)prec);
  pdbRecordType->papFldDes[196]->size=sizeof(prec->ufvn);
  pdbRecordType->papFldDes[196]->offset=(short)((char *)&prec->ufvn - (char *)prec);
  pdbRecordType->papFldDes[197]->size=sizeof(prec->ufvo);
  pdbRecordType->papFldDes[197]->offset=(short)((char *)&prec->ufvo - (char *)prec);
  pdbRecordType->papFldDes[198]->size=sizeof(prec->ufvp);
  pdbRecordType->papFldDes[198]->offset=(short)((char *)&prec->ufvp - (char *)prec);
  pdbRecordType->papFldDes[199]->size=sizeof(prec->ufvq);
  pdbRecordType->papFldDes[199]->offset=(short)((char *)&prec->ufvq - (char *)prec);
  pdbRecordType->papFldDes[200]->size=sizeof(prec->ufvr);
  pdbRecordType->papFldDes[200]->offset=(short)((char *)&prec->ufvr - (char *)prec);
  pdbRecordType->papFldDes[201]->size=sizeof(prec->ufvs);
  pdbRecordType->papFldDes[201]->offset=(short)((char *)&prec->ufvs - (char *)prec);
  pdbRecordType->papFldDes[202]->size=sizeof(prec->ufvt);
  pdbRecordType->papFldDes[202]->offset=(short)((char *)&prec->ufvt - (char *)prec);
  pdbRecordType->papFldDes[203]->size=sizeof(prec->ufvu);
  pdbRecordType->papFldDes[203]->offset=(short)((char *)&prec->ufvu - (char *)prec);
  pdbRecordType->papFldDes[204]->size=sizeof(prec->vala);
  pdbRecordType->papFldDes[204]->offset=(short)((char *)&prec->vala - (char *)prec);
  pdbRecordType->papFldDes[205]->size=sizeof(prec->valb);
  pdbRecordType->papFldDes[205]->offset=(short)((char *)&prec->valb - (char *)prec);
  pdbRecordType->papFldDes[206]->size=sizeof(prec->valc);
  pdbRecordType->papFldDes[206]->offset=(short)((char *)&prec->valc - (char *)prec);
  pdbRecordType->papFldDes[207]->size=sizeof(prec->vald);
  pdbRecordType->papFldDes[207]->offset=(short)((char *)&prec->vald - (char *)prec);
  pdbRecordType->papFldDes[208]->size=sizeof(prec->vale);
  pdbRecordType->papFldDes[208]->offset=(short)((char *)&prec->vale - (char *)prec);
  pdbRecordType->papFldDes[209]->size=sizeof(prec->valf);
  pdbRecordType->papFldDes[209]->offset=(short)((char *)&prec->valf - (char *)prec);
  pdbRecordType->papFldDes[210]->size=sizeof(prec->valg);
  pdbRecordType->papFldDes[210]->offset=(short)((char *)&prec->valg - (char *)prec);
  pdbRecordType->papFldDes[211]->size=sizeof(prec->valh);
  pdbRecordType->papFldDes[211]->offset=(short)((char *)&prec->valh - (char *)prec);
  pdbRecordType->papFldDes[212]->size=sizeof(prec->vali);
  pdbRecordType->papFldDes[212]->offset=(short)((char *)&prec->vali - (char *)prec);
  pdbRecordType->papFldDes[213]->size=sizeof(prec->valj);
  pdbRecordType->papFldDes[213]->offset=(short)((char *)&prec->valj - (char *)prec);
  pdbRecordType->papFldDes[214]->size=sizeof(prec->valk);
  pdbRecordType->papFldDes[214]->offset=(short)((char *)&prec->valk - (char *)prec);
  pdbRecordType->papFldDes[215]->size=sizeof(prec->vall);
  pdbRecordType->papFldDes[215]->offset=(short)((char *)&prec->vall - (char *)prec);
  pdbRecordType->papFldDes[216]->size=sizeof(prec->valm);
  pdbRecordType->papFldDes[216]->offset=(short)((char *)&prec->valm - (char *)prec);
  pdbRecordType->papFldDes[217]->size=sizeof(prec->valn);
  pdbRecordType->papFldDes[217]->offset=(short)((char *)&prec->valn - (char *)prec);
  pdbRecordType->papFldDes[218]->size=sizeof(prec->valo);
  pdbRecordType->papFldDes[218]->offset=(short)((char *)&prec->valo - (char *)prec);
  pdbRecordType->papFldDes[219]->size=sizeof(prec->valp);
  pdbRecordType->papFldDes[219]->offset=(short)((char *)&prec->valp - (char *)prec);
  pdbRecordType->papFldDes[220]->size=sizeof(prec->valq);
  pdbRecordType->papFldDes[220]->offset=(short)((char *)&prec->valq - (char *)prec);
  pdbRecordType->papFldDes[221]->size=sizeof(prec->valr);
  pdbRecordType->papFldDes[221]->offset=(short)((char *)&prec->valr - (char *)prec);
  pdbRecordType->papFldDes[222]->size=sizeof(prec->vals);
  pdbRecordType->papFldDes[222]->offset=(short)((char *)&prec->vals - (char *)prec);
  pdbRecordType->papFldDes[223]->size=sizeof(prec->valt);
  pdbRecordType->papFldDes[223]->offset=(short)((char *)&prec->valt - (char *)prec);
  pdbRecordType->papFldDes[224]->size=sizeof(prec->valu);
  pdbRecordType->papFldDes[224]->offset=(short)((char *)&prec->valu - (char *)prec);
  pdbRecordType->papFldDes[225]->size=sizeof(prec->ovla);
  pdbRecordType->papFldDes[225]->offset=(short)((char *)&prec->ovla - (char *)prec);
  pdbRecordType->papFldDes[226]->size=sizeof(prec->ovlb);
  pdbRecordType->papFldDes[226]->offset=(short)((char *)&prec->ovlb - (char *)prec);
  pdbRecordType->papFldDes[227]->size=sizeof(prec->ovlc);
  pdbRecordType->papFldDes[227]->offset=(short)((char *)&prec->ovlc - (char *)prec);
  pdbRecordType->papFldDes[228]->size=sizeof(prec->ovld);
  pdbRecordType->papFldDes[228]->offset=(short)((char *)&prec->ovld - (char *)prec);
  pdbRecordType->papFldDes[229]->size=sizeof(prec->ovle);
  pdbRecordType->papFldDes[229]->offset=(short)((char *)&prec->ovle - (char *)prec);
  pdbRecordType->papFldDes[230]->size=sizeof(prec->ovlf);
  pdbRecordType->papFldDes[230]->offset=(short)((char *)&prec->ovlf - (char *)prec);
  pdbRecordType->papFldDes[231]->size=sizeof(prec->ovlg);
  pdbRecordType->papFldDes[231]->offset=(short)((char *)&prec->ovlg - (char *)prec);
  pdbRecordType->papFldDes[232]->size=sizeof(prec->ovlh);
  pdbRecordType->papFldDes[232]->offset=(short)((char *)&prec->ovlh - (char *)prec);
  pdbRecordType->papFldDes[233]->size=sizeof(prec->ovli);
  pdbRecordType->papFldDes[233]->offset=(short)((char *)&prec->ovli - (char *)prec);
  pdbRecordType->papFldDes[234]->size=sizeof(prec->ovlj);
  pdbRecordType->papFldDes[234]->offset=(short)((char *)&prec->ovlj - (char *)prec);
  pdbRecordType->papFldDes[235]->size=sizeof(prec->ovlk);
  pdbRecordType->papFldDes[235]->offset=(short)((char *)&prec->ovlk - (char *)prec);
  pdbRecordType->papFldDes[236]->size=sizeof(prec->ovll);
  pdbRecordType->papFldDes[236]->offset=(short)((char *)&prec->ovll - (char *)prec);
  pdbRecordType->papFldDes[237]->size=sizeof(prec->ovlm);
  pdbRecordType->papFldDes[237]->offset=(short)((char *)&prec->ovlm - (char *)prec);
  pdbRecordType->papFldDes[238]->size=sizeof(prec->ovln);
  pdbRecordType->papFldDes[238]->offset=(short)((char *)&prec->ovln - (char *)prec);
  pdbRecordType->papFldDes[239]->size=sizeof(prec->ovlo);
  pdbRecordType->papFldDes[239]->offset=(short)((char *)&prec->ovlo - (char *)prec);
  pdbRecordType->papFldDes[240]->size=sizeof(prec->ovlp);
  pdbRecordType->papFldDes[240]->offset=(short)((char *)&prec->ovlp - (char *)prec);
  pdbRecordType->papFldDes[241]->size=sizeof(prec->ovlq);
  pdbRecordType->papFldDes[241]->offset=(short)((char *)&prec->ovlq - (char *)prec);
  pdbRecordType->papFldDes[242]->size=sizeof(prec->ovlr);
  pdbRecordType->papFldDes[242]->offset=(short)((char *)&prec->ovlr - (char *)prec);
  pdbRecordType->papFldDes[243]->size=sizeof(prec->ovls);
  pdbRecordType->papFldDes[243]->offset=(short)((char *)&prec->ovls - (char *)prec);
  pdbRecordType->papFldDes[244]->size=sizeof(prec->ovlt);
  pdbRecordType->papFldDes[244]->offset=(short)((char *)&prec->ovlt - (char *)prec);
  pdbRecordType->papFldDes[245]->size=sizeof(prec->ovlu);
  pdbRecordType->papFldDes[245]->offset=(short)((char *)&prec->ovlu - (char *)prec);
  pdbRecordType->papFldDes[246]->size=sizeof(prec->ftva);
  pdbRecordType->papFldDes[246]->offset=(short)((char *)&prec->ftva - (char *)prec);
  pdbRecordType->papFldDes[247]->size=sizeof(prec->ftvb);
  pdbRecordType->papFldDes[247]->offset=(short)((char *)&prec->ftvb - (char *)prec);
  pdbRecordType->papFldDes[248]->size=sizeof(prec->ftvc);
  pdbRecordType->papFldDes[248]->offset=(short)((char *)&prec->ftvc - (char *)prec);
  pdbRecordType->papFldDes[249]->size=sizeof(prec->ftvd);
  pdbRecordType->papFldDes[249]->offset=(short)((char *)&prec->ftvd - (char *)prec);
  pdbRecordType->papFldDes[250]->size=sizeof(prec->ftve);
  pdbRecordType->papFldDes[250]->offset=(short)((char *)&prec->ftve - (char *)prec);
  pdbRecordType->papFldDes[251]->size=sizeof(prec->ftvf);
  pdbRecordType->papFldDes[251]->offset=(short)((char *)&prec->ftvf - (char *)prec);
  pdbRecordType->papFldDes[252]->size=sizeof(prec->ftvg);
  pdbRecordType->papFldDes[252]->offset=(short)((char *)&prec->ftvg - (char *)prec);
  pdbRecordType->papFldDes[253]->size=sizeof(prec->ftvh);
  pdbRecordType->papFldDes[253]->offset=(short)((char *)&prec->ftvh - (char *)prec);
  pdbRecordType->papFldDes[254]->size=sizeof(prec->ftvi);
  pdbRecordType->papFldDes[254]->offset=(short)((char *)&prec->ftvi - (char *)prec);
  pdbRecordType->papFldDes[255]->size=sizeof(prec->ftvj);
  pdbRecordType->papFldDes[255]->offset=(short)((char *)&prec->ftvj - (char *)prec);
  pdbRecordType->papFldDes[256]->size=sizeof(prec->ftvk);
  pdbRecordType->papFldDes[256]->offset=(short)((char *)&prec->ftvk - (char *)prec);
  pdbRecordType->papFldDes[257]->size=sizeof(prec->ftvl);
  pdbRecordType->papFldDes[257]->offset=(short)((char *)&prec->ftvl - (char *)prec);
  pdbRecordType->papFldDes[258]->size=sizeof(prec->ftvm);
  pdbRecordType->papFldDes[258]->offset=(short)((char *)&prec->ftvm - (char *)prec);
  pdbRecordType->papFldDes[259]->size=sizeof(prec->ftvn);
  pdbRecordType->papFldDes[259]->offset=(short)((char *)&prec->ftvn - (char *)prec);
  pdbRecordType->papFldDes[260]->size=sizeof(prec->ftvo);
  pdbRecordType->papFldDes[260]->offset=(short)((char *)&prec->ftvo - (char *)prec);
  pdbRecordType->papFldDes[261]->size=sizeof(prec->ftvp);
  pdbRecordType->papFldDes[261]->offset=(short)((char *)&prec->ftvp - (char *)prec);
  pdbRecordType->papFldDes[262]->size=sizeof(prec->ftvq);
  pdbRecordType->papFldDes[262]->offset=(short)((char *)&prec->ftvq - (char *)prec);
  pdbRecordType->papFldDes[263]->size=sizeof(prec->ftvr);
  pdbRecordType->papFldDes[263]->offset=(short)((char *)&prec->ftvr - (char *)prec);
  pdbRecordType->papFldDes[264]->size=sizeof(prec->ftvs);
  pdbRecordType->papFldDes[264]->offset=(short)((char *)&prec->ftvs - (char *)prec);
  pdbRecordType->papFldDes[265]->size=sizeof(prec->ftvt);
  pdbRecordType->papFldDes[265]->offset=(short)((char *)&prec->ftvt - (char *)prec);
  pdbRecordType->papFldDes[266]->size=sizeof(prec->ftvu);
  pdbRecordType->papFldDes[266]->offset=(short)((char *)&prec->ftvu - (char *)prec);
  pdbRecordType->papFldDes[267]->size=sizeof(prec->nova);
  pdbRecordType->papFldDes[267]->offset=(short)((char *)&prec->nova - (char *)prec);
  pdbRecordType->papFldDes[268]->size=sizeof(prec->novb);
  pdbRecordType->papFldDes[268]->offset=(short)((char *)&prec->novb - (char *)prec);
  pdbRecordType->papFldDes[269]->size=sizeof(prec->novc);
  pdbRecordType->papFldDes[269]->offset=(short)((char *)&prec->novc - (char *)prec);
  pdbRecordType->papFldDes[270]->size=sizeof(prec->novd);
  pdbRecordType->papFldDes[270]->offset=(short)((char *)&prec->novd - (char *)prec);
  pdbRecordType->papFldDes[271]->size=sizeof(prec->nove);
  pdbRecordType->papFldDes[271]->offset=(short)((char *)&prec->nove - (char *)prec);
  pdbRecordType->papFldDes[272]->size=sizeof(prec->novf);
  pdbRecordType->papFldDes[272]->offset=(short)((char *)&prec->novf - (char *)prec);
  pdbRecordType->papFldDes[273]->size=sizeof(prec->novg);
  pdbRecordType->papFldDes[273]->offset=(short)((char *)&prec->novg - (char *)prec);
  pdbRecordType->papFldDes[274]->size=sizeof(prec->novh);
  pdbRecordType->papFldDes[274]->offset=(short)((char *)&prec->novh - (char *)prec);
  pdbRecordType->papFldDes[275]->size=sizeof(prec->novi);
  pdbRecordType->papFldDes[275]->offset=(short)((char *)&prec->novi - (char *)prec);
  pdbRecordType->papFldDes[276]->size=sizeof(prec->novj);
  pdbRecordType->papFldDes[276]->offset=(short)((char *)&prec->novj - (char *)prec);
  pdbRecordType->papFldDes[277]->size=sizeof(prec->novk);
  pdbRecordType->papFldDes[277]->offset=(short)((char *)&prec->novk - (char *)prec);
  pdbRecordType->papFldDes[278]->size=sizeof(prec->novl);
  pdbRecordType->papFldDes[278]->offset=(short)((char *)&prec->novl - (char *)prec);
  pdbRecordType->papFldDes[279]->size=sizeof(prec->novm);
  pdbRecordType->papFldDes[279]->offset=(short)((char *)&prec->novm - (char *)prec);
  pdbRecordType->papFldDes[280]->size=sizeof(prec->novn);
  pdbRecordType->papFldDes[280]->offset=(short)((char *)&prec->novn - (char *)prec);
  pdbRecordType->papFldDes[281]->size=sizeof(prec->novo);
  pdbRecordType->papFldDes[281]->offset=(short)((char *)&prec->novo - (char *)prec);
  pdbRecordType->papFldDes[282]->size=sizeof(prec->novp);
  pdbRecordType->papFldDes[282]->offset=(short)((char *)&prec->novp - (char *)prec);
  pdbRecordType->papFldDes[283]->size=sizeof(prec->novq);
  pdbRecordType->papFldDes[283]->offset=(short)((char *)&prec->novq - (char *)prec);
  pdbRecordType->papFldDes[284]->size=sizeof(prec->novr);
  pdbRecordType->papFldDes[284]->offset=(short)((char *)&prec->novr - (char *)prec);
  pdbRecordType->papFldDes[285]->size=sizeof(prec->novs);
  pdbRecordType->papFldDes[285]->offset=(short)((char *)&prec->novs - (char *)prec);
  pdbRecordType->papFldDes[286]->size=sizeof(prec->novt);
  pdbRecordType->papFldDes[286]->offset=(short)((char *)&prec->novt - (char *)prec);
  pdbRecordType->papFldDes[287]->size=sizeof(prec->novu);
  pdbRecordType->papFldDes[287]->offset=(short)((char *)&prec->novu - (char *)prec);
  pdbRecordType->papFldDes[288]->size=sizeof(prec->tova);
  pdbRecordType->papFldDes[288]->offset=(short)((char *)&prec->tova - (char *)prec);
  pdbRecordType->papFldDes[289]->size=sizeof(prec->tovb);
  pdbRecordType->papFldDes[289]->offset=(short)((char *)&prec->tovb - (char *)prec);
  pdbRecordType->papFldDes[290]->size=sizeof(prec->tovc);
  pdbRecordType->papFldDes[290]->offset=(short)((char *)&prec->tovc - (char *)prec);
  pdbRecordType->papFldDes[291]->size=sizeof(prec->tovd);
  pdbRecordType->papFldDes[291]->offset=(short)((char *)&prec->tovd - (char *)prec);
  pdbRecordType->papFldDes[292]->size=sizeof(prec->tove);
  pdbRecordType->papFldDes[292]->offset=(short)((char *)&prec->tove - (char *)prec);
  pdbRecordType->papFldDes[293]->size=sizeof(prec->tovf);
  pdbRecordType->papFldDes[293]->offset=(short)((char *)&prec->tovf - (char *)prec);
  pdbRecordType->papFldDes[294]->size=sizeof(prec->tovg);
  pdbRecordType->papFldDes[294]->offset=(short)((char *)&prec->tovg - (char *)prec);
  pdbRecordType->papFldDes[295]->size=sizeof(prec->tovh);
  pdbRecordType->papFldDes[295]->offset=(short)((char *)&prec->tovh - (char *)prec);
  pdbRecordType->papFldDes[296]->size=sizeof(prec->tovi);
  pdbRecordType->papFldDes[296]->offset=(short)((char *)&prec->tovi - (char *)prec);
  pdbRecordType->papFldDes[297]->size=sizeof(prec->tovj);
  pdbRecordType->papFldDes[297]->offset=(short)((char *)&prec->tovj - (char *)prec);
  pdbRecordType->papFldDes[298]->size=sizeof(prec->tovk);
  pdbRecordType->papFldDes[298]->offset=(short)((char *)&prec->tovk - (char *)prec);
  pdbRecordType->papFldDes[299]->size=sizeof(prec->tovl);
  pdbRecordType->papFldDes[299]->offset=(short)((char *)&prec->tovl - (char *)prec);
  pdbRecordType->papFldDes[300]->size=sizeof(prec->tovm);
  pdbRecordType->papFldDes[300]->offset=(short)((char *)&prec->tovm - (char *)prec);
  pdbRecordType->papFldDes[301]->size=sizeof(prec->tovn);
  pdbRecordType->papFldDes[301]->offset=(short)((char *)&prec->tovn - (char *)prec);
  pdbRecordType->papFldDes[302]->size=sizeof(prec->tovo);
  pdbRecordType->papFldDes[302]->offset=(short)((char *)&prec->tovo - (char *)prec);
  pdbRecordType->papFldDes[303]->size=sizeof(prec->tovp);
  pdbRecordType->papFldDes[303]->offset=(short)((char *)&prec->tovp - (char *)prec);
  pdbRecordType->papFldDes[304]->size=sizeof(prec->tovq);
  pdbRecordType->papFldDes[304]->offset=(short)((char *)&prec->tovq - (char *)prec);
  pdbRecordType->papFldDes[305]->size=sizeof(prec->tovr);
  pdbRecordType->papFldDes[305]->offset=(short)((char *)&prec->tovr - (char *)prec);
  pdbRecordType->papFldDes[306]->size=sizeof(prec->tovs);
  pdbRecordType->papFldDes[306]->offset=(short)((char *)&prec->tovs - (char *)prec);
  pdbRecordType->papFldDes[307]->size=sizeof(prec->tovt);
  pdbRecordType->papFldDes[307]->offset=(short)((char *)&prec->tovt - (char *)prec);
  pdbRecordType->papFldDes[308]->size=sizeof(prec->tovu);
  pdbRecordType->papFldDes[308]->offset=(short)((char *)&prec->tovu - (char *)prec);
    pdbRecordType->rec_size = sizeof(*prec);
    return(0);
}
#endif /*GEN_SIZE_OFFSET*/
