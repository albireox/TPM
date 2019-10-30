#include <vxWorks.h>
#include <semLib.h>
#include "ellLib.h"
#include "fast_lock.h"
#include "link.h"
#include "tsDefs.h"
#ifndef INCwaveAnlH
#define INCwaveAnlH
typedef struct waveAnlRecord {
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
	struct putNotify *ppn;	/*addr of PUTNOTIFY*/
	struct pnWaitNode *ppnn;	/*next record PUTNOTIFY*/
	struct scan_element *spvt;	/*Scan Private*/
	struct rset	*rset;	/*Address of RSET*/
	struct dset	*dset;	/*DSET address*/
	void		*dpvt;	/*Device Private*/
	struct dbRecordType *rdes;	/*Address of dbRecordType*/
	struct lockRecord *lset;	/*Lock Set*/
	unsigned short	prio;	/*Scheduling Priority*/
	unsigned char	tpro;	/*Trace Processing*/
	char bkpt;	/*Break Point*/
	unsigned char	udf;	/*Undefined*/
	TS_STAMP	time;	/*Time*/
	DBLINK		flnk;	/*Forward Process Link*/
	double		max;	/*Maximum value in data set*/
	double		min;	/*Minimum value in data set*/
	double		pkpk;	/*Peak to peak value data set*/
	double		mean;	/*Mean value of data set*/
	double		var;	/*Variance of data set*/
	double		sdev;	/*Standard deviation of data set*/
	double		madv;	/*Mean absolute deviation of data set*/
	double		fwhm;	/*Full width half maximum*/
	unsigned long	nelm;	/*Max umber of elements*/
	unsigned long	nuse;	/*Number of used*/
	double * xptr;	/*X axis scale array pointer*/
	double		xres;	/*X axis resoultion of samples*/
	DBLINK		xrsl;	/*Time resoultion location*/
	double		oxrs;	/*Old X axis resoultion of samples*/
	double		xoff;	/*X axis offset*/
	DBLINK		xofl;	/*X axis offset location*/
	double		oxof;	/*Old X axis offset location*/
	double		bgri;	/*Beginning of region of interest*/
	double		enri;	/*End of region of interest*/
	unsigned long	bgix;	/*Index of beginning of region of interest*/
	unsigned long	enix;	/*Index of end of region of interest*/
	double		thld;	/*Threshold*/
	double		blof;	/*Baseline offset value*/
	DBLINK		blol;	/*Baseline offset location*/
	double * bptr;	/*Buffer Pointer*/
	long		sadr;	/*Subroutine Address*/
	char		inam[40]; /*Init Subr. Name*/
	char		snam[40]; /*Pre-Process Subr. Name*/
	short		pass;	/*Post-Process Subr. Name*/
	unsigned short	brsv;	/*Bad Return Severity*/
	double * val;	/*Ptr to output array*/
	double		vala;	/*User value A*/
	double		valb;	/*User value B*/
	double		valc;	/*User value C*/
	double		vald;	/*User value D*/
	double		vale;	/*User value E*/
	double		valf;	/*User value F*/
	double		valg;	/*User value G*/
	double		valh;	/*User value H*/
	DBLINK		inp;	/*Input Specification*/
	DBLINK		inpa;	/*User Input Link A*/
	DBLINK		inpb;	/*User Input Link B*/
	DBLINK		inpc;	/*User Input Link C*/
	DBLINK		inpd;	/*User Input Link D*/
	DBLINK		inpe;	/*User Input Link E*/
	DBLINK		inpf;	/*User Input Link F*/
	DBLINK		inpg;	/*User Input Link G*/
	DBLINK		inph;	/*User Input Link H*/
	double		a;	/*Value of User Input A*/
	double		b;	/*Value of User Input B*/
	double		c;	/*Value of User Input C*/
	double		d;	/*Value of User Input D*/
	double		e;	/*Value of User Input E*/
	double		f;	/*Value of User Input F*/
	double		g;	/*Value of User Input G*/
	double		h;	/*Value of User Input H*/
	short		prec;	/*Display Precision*/
	char		egux[16]; /*X Axis Engineering Units*/
	float		horx;	/*X Axis High Operating Range*/
	float		lorx;	/*X Axis Low Operating Range*/
	char		eguy[16]; /*Y Axis Engineering Units*/
	float		hory;	/*Y Axis High Operating Range*/
	float		lory;	/*Y Axis Low Operating Range*/
} waveAnlRecord;
#define waveAnlRecordNAME	0
#define waveAnlRecordDESC	1
#define waveAnlRecordASG	2
#define waveAnlRecordSCAN	3
#define waveAnlRecordPINI	4
#define waveAnlRecordPHAS	5
#define waveAnlRecordEVNT	6
#define waveAnlRecordTSE	7
#define waveAnlRecordTSEL	8
#define waveAnlRecordDTYP	9
#define waveAnlRecordDISV	10
#define waveAnlRecordDISA	11
#define waveAnlRecordSDIS	12
#define waveAnlRecordMLOK	13
#define waveAnlRecordMLIS	14
#define waveAnlRecordDISP	15
#define waveAnlRecordPROC	16
#define waveAnlRecordSTAT	17
#define waveAnlRecordSEVR	18
#define waveAnlRecordNSTA	19
#define waveAnlRecordNSEV	20
#define waveAnlRecordACKS	21
#define waveAnlRecordACKT	22
#define waveAnlRecordDISS	23
#define waveAnlRecordLCNT	24
#define waveAnlRecordPACT	25
#define waveAnlRecordPUTF	26
#define waveAnlRecordRPRO	27
#define waveAnlRecordASP	28
#define waveAnlRecordPPN	29
#define waveAnlRecordPPNN	30
#define waveAnlRecordSPVT	31
#define waveAnlRecordRSET	32
#define waveAnlRecordDSET	33
#define waveAnlRecordDPVT	34
#define waveAnlRecordRDES	35
#define waveAnlRecordLSET	36
#define waveAnlRecordPRIO	37
#define waveAnlRecordTPRO	38
#define waveAnlRecordBKPT	39
#define waveAnlRecordUDF	40
#define waveAnlRecordTIME	41
#define waveAnlRecordFLNK	42
#define waveAnlRecordMAX	43
#define waveAnlRecordMIN	44
#define waveAnlRecordPKPK	45
#define waveAnlRecordMEAN	46
#define waveAnlRecordVAR	47
#define waveAnlRecordSDEV	48
#define waveAnlRecordMADV	49
#define waveAnlRecordFWHM	50
#define waveAnlRecordNELM	51
#define waveAnlRecordNUSE	52
#define waveAnlRecordXPTR	53
#define waveAnlRecordXRES	54
#define waveAnlRecordXRSL	55
#define waveAnlRecordOXRS	56
#define waveAnlRecordXOFF	57
#define waveAnlRecordXOFL	58
#define waveAnlRecordOXOF	59
#define waveAnlRecordBGRI	60
#define waveAnlRecordENRI	61
#define waveAnlRecordBGIX	62
#define waveAnlRecordENIX	63
#define waveAnlRecordTHLD	64
#define waveAnlRecordBLOF	65
#define waveAnlRecordBLOL	66
#define waveAnlRecordBPTR	67
#define waveAnlRecordSADR	68
#define waveAnlRecordINAM	69
#define waveAnlRecordSNAM	70
#define waveAnlRecordPASS	71
#define waveAnlRecordBRSV	72
#define waveAnlRecordVAL	73
#define waveAnlRecordVALA	74
#define waveAnlRecordVALB	75
#define waveAnlRecordVALC	76
#define waveAnlRecordVALD	77
#define waveAnlRecordVALE	78
#define waveAnlRecordVALF	79
#define waveAnlRecordVALG	80
#define waveAnlRecordVALH	81
#define waveAnlRecordINP	82
#define waveAnlRecordINPA	83
#define waveAnlRecordINPB	84
#define waveAnlRecordINPC	85
#define waveAnlRecordINPD	86
#define waveAnlRecordINPE	87
#define waveAnlRecordINPF	88
#define waveAnlRecordINPG	89
#define waveAnlRecordINPH	90
#define waveAnlRecordA	91
#define waveAnlRecordB	92
#define waveAnlRecordC	93
#define waveAnlRecordD	94
#define waveAnlRecordE	95
#define waveAnlRecordF	96
#define waveAnlRecordG	97
#define waveAnlRecordH	98
#define waveAnlRecordPREC	99
#define waveAnlRecordEGUX	100
#define waveAnlRecordHORX	101
#define waveAnlRecordLORX	102
#define waveAnlRecordEGUY	103
#define waveAnlRecordHORY	104
#define waveAnlRecordLORY	105
#endif /*INCwaveAnlH*/
#ifdef GEN_SIZE_OFFSET
int waveAnlRecordSizeOffset(dbRecordType *pdbRecordType)
{
    waveAnlRecord *prec = 0;
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
  pdbRecordType->papFldDes[43]->size=sizeof(prec->max);
  pdbRecordType->papFldDes[43]->offset=(short)((char *)&prec->max - (char *)prec);
  pdbRecordType->papFldDes[44]->size=sizeof(prec->min);
  pdbRecordType->papFldDes[44]->offset=(short)((char *)&prec->min - (char *)prec);
  pdbRecordType->papFldDes[45]->size=sizeof(prec->pkpk);
  pdbRecordType->papFldDes[45]->offset=(short)((char *)&prec->pkpk - (char *)prec);
  pdbRecordType->papFldDes[46]->size=sizeof(prec->mean);
  pdbRecordType->papFldDes[46]->offset=(short)((char *)&prec->mean - (char *)prec);
  pdbRecordType->papFldDes[47]->size=sizeof(prec->var);
  pdbRecordType->papFldDes[47]->offset=(short)((char *)&prec->var - (char *)prec);
  pdbRecordType->papFldDes[48]->size=sizeof(prec->sdev);
  pdbRecordType->papFldDes[48]->offset=(short)((char *)&prec->sdev - (char *)prec);
  pdbRecordType->papFldDes[49]->size=sizeof(prec->madv);
  pdbRecordType->papFldDes[49]->offset=(short)((char *)&prec->madv - (char *)prec);
  pdbRecordType->papFldDes[50]->size=sizeof(prec->fwhm);
  pdbRecordType->papFldDes[50]->offset=(short)((char *)&prec->fwhm - (char *)prec);
  pdbRecordType->papFldDes[51]->size=sizeof(prec->nelm);
  pdbRecordType->papFldDes[51]->offset=(short)((char *)&prec->nelm - (char *)prec);
  pdbRecordType->papFldDes[52]->size=sizeof(prec->nuse);
  pdbRecordType->papFldDes[52]->offset=(short)((char *)&prec->nuse - (char *)prec);
  pdbRecordType->papFldDes[53]->size=sizeof(prec->xptr);
  pdbRecordType->papFldDes[53]->offset=(short)((char *)&prec->xptr - (char *)prec);
  pdbRecordType->papFldDes[54]->size=sizeof(prec->xres);
  pdbRecordType->papFldDes[54]->offset=(short)((char *)&prec->xres - (char *)prec);
  pdbRecordType->papFldDes[55]->size=sizeof(prec->xrsl);
  pdbRecordType->papFldDes[55]->offset=(short)((char *)&prec->xrsl - (char *)prec);
  pdbRecordType->papFldDes[56]->size=sizeof(prec->oxrs);
  pdbRecordType->papFldDes[56]->offset=(short)((char *)&prec->oxrs - (char *)prec);
  pdbRecordType->papFldDes[57]->size=sizeof(prec->xoff);
  pdbRecordType->papFldDes[57]->offset=(short)((char *)&prec->xoff - (char *)prec);
  pdbRecordType->papFldDes[58]->size=sizeof(prec->xofl);
  pdbRecordType->papFldDes[58]->offset=(short)((char *)&prec->xofl - (char *)prec);
  pdbRecordType->papFldDes[59]->size=sizeof(prec->oxof);
  pdbRecordType->papFldDes[59]->offset=(short)((char *)&prec->oxof - (char *)prec);
  pdbRecordType->papFldDes[60]->size=sizeof(prec->bgri);
  pdbRecordType->papFldDes[60]->offset=(short)((char *)&prec->bgri - (char *)prec);
  pdbRecordType->papFldDes[61]->size=sizeof(prec->enri);
  pdbRecordType->papFldDes[61]->offset=(short)((char *)&prec->enri - (char *)prec);
  pdbRecordType->papFldDes[62]->size=sizeof(prec->bgix);
  pdbRecordType->papFldDes[62]->offset=(short)((char *)&prec->bgix - (char *)prec);
  pdbRecordType->papFldDes[63]->size=sizeof(prec->enix);
  pdbRecordType->papFldDes[63]->offset=(short)((char *)&prec->enix - (char *)prec);
  pdbRecordType->papFldDes[64]->size=sizeof(prec->thld);
  pdbRecordType->papFldDes[64]->offset=(short)((char *)&prec->thld - (char *)prec);
  pdbRecordType->papFldDes[65]->size=sizeof(prec->blof);
  pdbRecordType->papFldDes[65]->offset=(short)((char *)&prec->blof - (char *)prec);
  pdbRecordType->papFldDes[66]->size=sizeof(prec->blol);
  pdbRecordType->papFldDes[66]->offset=(short)((char *)&prec->blol - (char *)prec);
  pdbRecordType->papFldDes[67]->size=sizeof(prec->bptr);
  pdbRecordType->papFldDes[67]->offset=(short)((char *)&prec->bptr - (char *)prec);
  pdbRecordType->papFldDes[68]->size=sizeof(prec->sadr);
  pdbRecordType->papFldDes[68]->offset=(short)((char *)&prec->sadr - (char *)prec);
  pdbRecordType->papFldDes[69]->size=sizeof(prec->inam);
  pdbRecordType->papFldDes[69]->offset=(short)((char *)&prec->inam - (char *)prec);
  pdbRecordType->papFldDes[70]->size=sizeof(prec->snam);
  pdbRecordType->papFldDes[70]->offset=(short)((char *)&prec->snam - (char *)prec);
  pdbRecordType->papFldDes[71]->size=sizeof(prec->pass);
  pdbRecordType->papFldDes[71]->offset=(short)((char *)&prec->pass - (char *)prec);
  pdbRecordType->papFldDes[72]->size=sizeof(prec->brsv);
  pdbRecordType->papFldDes[72]->offset=(short)((char *)&prec->brsv - (char *)prec);
  pdbRecordType->papFldDes[73]->size=sizeof(prec->val);
  pdbRecordType->papFldDes[73]->offset=(short)((char *)&prec->val - (char *)prec);
  pdbRecordType->papFldDes[74]->size=sizeof(prec->vala);
  pdbRecordType->papFldDes[74]->offset=(short)((char *)&prec->vala - (char *)prec);
  pdbRecordType->papFldDes[75]->size=sizeof(prec->valb);
  pdbRecordType->papFldDes[75]->offset=(short)((char *)&prec->valb - (char *)prec);
  pdbRecordType->papFldDes[76]->size=sizeof(prec->valc);
  pdbRecordType->papFldDes[76]->offset=(short)((char *)&prec->valc - (char *)prec);
  pdbRecordType->papFldDes[77]->size=sizeof(prec->vald);
  pdbRecordType->papFldDes[77]->offset=(short)((char *)&prec->vald - (char *)prec);
  pdbRecordType->papFldDes[78]->size=sizeof(prec->vale);
  pdbRecordType->papFldDes[78]->offset=(short)((char *)&prec->vale - (char *)prec);
  pdbRecordType->papFldDes[79]->size=sizeof(prec->valf);
  pdbRecordType->papFldDes[79]->offset=(short)((char *)&prec->valf - (char *)prec);
  pdbRecordType->papFldDes[80]->size=sizeof(prec->valg);
  pdbRecordType->papFldDes[80]->offset=(short)((char *)&prec->valg - (char *)prec);
  pdbRecordType->papFldDes[81]->size=sizeof(prec->valh);
  pdbRecordType->papFldDes[81]->offset=(short)((char *)&prec->valh - (char *)prec);
  pdbRecordType->papFldDes[82]->size=sizeof(prec->inp);
  pdbRecordType->papFldDes[82]->offset=(short)((char *)&prec->inp - (char *)prec);
  pdbRecordType->papFldDes[83]->size=sizeof(prec->inpa);
  pdbRecordType->papFldDes[83]->offset=(short)((char *)&prec->inpa - (char *)prec);
  pdbRecordType->papFldDes[84]->size=sizeof(prec->inpb);
  pdbRecordType->papFldDes[84]->offset=(short)((char *)&prec->inpb - (char *)prec);
  pdbRecordType->papFldDes[85]->size=sizeof(prec->inpc);
  pdbRecordType->papFldDes[85]->offset=(short)((char *)&prec->inpc - (char *)prec);
  pdbRecordType->papFldDes[86]->size=sizeof(prec->inpd);
  pdbRecordType->papFldDes[86]->offset=(short)((char *)&prec->inpd - (char *)prec);
  pdbRecordType->papFldDes[87]->size=sizeof(prec->inpe);
  pdbRecordType->papFldDes[87]->offset=(short)((char *)&prec->inpe - (char *)prec);
  pdbRecordType->papFldDes[88]->size=sizeof(prec->inpf);
  pdbRecordType->papFldDes[88]->offset=(short)((char *)&prec->inpf - (char *)prec);
  pdbRecordType->papFldDes[89]->size=sizeof(prec->inpg);
  pdbRecordType->papFldDes[89]->offset=(short)((char *)&prec->inpg - (char *)prec);
  pdbRecordType->papFldDes[90]->size=sizeof(prec->inph);
  pdbRecordType->papFldDes[90]->offset=(short)((char *)&prec->inph - (char *)prec);
  pdbRecordType->papFldDes[91]->size=sizeof(prec->a);
  pdbRecordType->papFldDes[91]->offset=(short)((char *)&prec->a - (char *)prec);
  pdbRecordType->papFldDes[92]->size=sizeof(prec->b);
  pdbRecordType->papFldDes[92]->offset=(short)((char *)&prec->b - (char *)prec);
  pdbRecordType->papFldDes[93]->size=sizeof(prec->c);
  pdbRecordType->papFldDes[93]->offset=(short)((char *)&prec->c - (char *)prec);
  pdbRecordType->papFldDes[94]->size=sizeof(prec->d);
  pdbRecordType->papFldDes[94]->offset=(short)((char *)&prec->d - (char *)prec);
  pdbRecordType->papFldDes[95]->size=sizeof(prec->e);
  pdbRecordType->papFldDes[95]->offset=(short)((char *)&prec->e - (char *)prec);
  pdbRecordType->papFldDes[96]->size=sizeof(prec->f);
  pdbRecordType->papFldDes[96]->offset=(short)((char *)&prec->f - (char *)prec);
  pdbRecordType->papFldDes[97]->size=sizeof(prec->g);
  pdbRecordType->papFldDes[97]->offset=(short)((char *)&prec->g - (char *)prec);
  pdbRecordType->papFldDes[98]->size=sizeof(prec->h);
  pdbRecordType->papFldDes[98]->offset=(short)((char *)&prec->h - (char *)prec);
  pdbRecordType->papFldDes[99]->size=sizeof(prec->prec);
  pdbRecordType->papFldDes[99]->offset=(short)((char *)&prec->prec - (char *)prec);
  pdbRecordType->papFldDes[100]->size=sizeof(prec->egux);
  pdbRecordType->papFldDes[100]->offset=(short)((char *)&prec->egux - (char *)prec);
  pdbRecordType->papFldDes[101]->size=sizeof(prec->horx);
  pdbRecordType->papFldDes[101]->offset=(short)((char *)&prec->horx - (char *)prec);
  pdbRecordType->papFldDes[102]->size=sizeof(prec->lorx);
  pdbRecordType->papFldDes[102]->offset=(short)((char *)&prec->lorx - (char *)prec);
  pdbRecordType->papFldDes[103]->size=sizeof(prec->eguy);
  pdbRecordType->papFldDes[103]->offset=(short)((char *)&prec->eguy - (char *)prec);
  pdbRecordType->papFldDes[104]->size=sizeof(prec->hory);
  pdbRecordType->papFldDes[104]->offset=(short)((char *)&prec->hory - (char *)prec);
  pdbRecordType->papFldDes[105]->size=sizeof(prec->lory);
  pdbRecordType->papFldDes[105]->offset=(short)((char *)&prec->lory - (char *)prec);
    pdbRecordType->rec_size = sizeof(*prec);
    return(0);
}
#endif /*GEN_SIZE_OFFSET*/
