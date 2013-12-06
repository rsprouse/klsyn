/*              K L S Y N . C                 D. H. K L A T T           */
 
/*    Copyright 1984 by Dennis H. Klatt */
/*  revised for IBM PC by Keith Johnson, Aug. 1987 */
 
/*      COMMAND HANDLER FOR THE KLATT SOFTWARE SYNTHESIZER              */
 
/*   Compile by make klsyn */
 
/* EDIT HISTORY:
 * 000001 10-Mar-83 DK    Initial creation
 * 000002  5-Mar-83 DK    Fix read-config bug
 * 000003  7-Jul-83 DK    Read ".par" file on startup if it exists
 * 000004 26-Jul-83 DK    Minor changes to bring up on VAX
 * 000005 23-Oct-83 DK    Replace rwwave.c by writwave.c and malloc() call
 * 000006 22-Nov-83 DK    Add and reorder pars
 * 000007 23-Nov-83 DK    Replace cpsw by outselect,AVp,rseed,nfcasc
 * 000008  7-Dec-83 DK    Rename 'C,P', add 'ss' par, update F1,... at 'ui' rate
 * 000009 16-Dec-83 DK    Add fast plot of par data as entered if in VT125 mode
 * 000010 10-Jan-84 DK    Fix computation of actual source waveform for os=4
 * 000011 10-Jan-84 DK ---->  Put Version 1.0 in [sysmgr.exe]
 * 000012 27-Jan-84 DK    Fix wave header so change in samrat noted
 * 000013 28-Jan-84 DK    Add fixed F7-F8 option to cascade branch, vt125 default
 * 000014 31-Jan-84 DK ---->  Put Version 1.1 in [sysmgr.exe]
 * 000015  3-Feb-84 DK    Make more columns in printout from prpars()
 * 000016 14-Feb-84 DK    Fix 'os' when 'os' > 12
 * 000017 14-Feb-84 DK ---->  Put Version 1.2 in [sysmgr.exe]
 * 000018 17-Feb-84 DK    Minor changes to print statements
 * 000019  2-Mar-84 DK    Turn la50_end_text_out() into a .doc file for listing
 * 000020 16-Mar-84 DK    Fix bug in parwave, fund period was not a func(samrate)
 * 000021 16-Mar-84 DK ---->  Put Version 1.3 in [sysmgr.exe]
 * 000022 14-May-84 DK    Read/write .doc files (.con & .par don't exist anymore)
 * 000023 14-May-84 DK ---->  Put Version 1.4 in [sysmgr.exe]
 * 000024  1-Jun-84 DK    Typing value=-1 results in value set to current val
 * 000025 15-Nov-85 DK    Add pars dF and db, extend range and linearize tl,
 *             replace no by oq (open quotient)
 * 000026 26-Nov-85 DK    Split off code involving graphical input of data
 * 000027 28-Feb-86 DK    Add new synth pars for F1,B1 increase during open phase
             Add triangular glottal waveform option (ss=3)
 * 000028  3-Mar-86 DK    Print current value during draw
 * 000029 25-Mar-86 DK    Add plot formants command 'G'
 * 000030 14-Aug-87 KJ  Revise graphics and input for PC type machines
 * 000031 19-Oct-87 KJ,YYQ Revise parwav for 16 bit architecture, fix graphics
             bug in pcplot
 * 000032 27-aug-90 KJ  revise for Microsoft-c  removed all graphics calls
			draw command is now called "enter" and it asks for
			time and value entries from the keyboard.  
			also revised writwv for c-speech header.
			fixed bug in print_outmax.
			added batch mode -b.
			revised KLSYN.MAN to reflect changes.
 * 000033 12-mar-96 KJ  convert back to 32 bit architecture, for sun sparc
                        -revise conio calls
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "klsyn.h"

#define TAGGED 1
 
char date[] = {"KLSYN    Version 1.5   D.H. Klatt         28-Mar-86"};
char d2[] = {  "KLSYN13  Update        Keith Johnson      12-Nov-13"};
 
FILE *fopen(), *fconf, *odev, *odoc;
int pdata[MAX_VAR_PARS][MAX_FRAMES];  /* Parameter data [np][nf] */
char firstname[256],cname[260],*pcname,wname[260],param[5];
char string[260],dname[260];
char request,sym1,sym2;
int i,n,npar1,lastnf,lastval;
int nsamtot,nskip;
int debug_level =0; /* for ESPS routines */
int16_t *iwave;
int defval[NPAR];

/*    State Variables */
char user;      /* If == NOVICE, print everything */
char ipsw;      /* TRUE if default pars inserted in par tracks */
int batch=0;  /* 0=normal mode, 1=batch mode */
int gain_control=0; /* 0=normal mode, 1=automatic gain control */
int np;         /* Current parameter */
int nf;         /* Current time frame */
int val;        /* Current value of current par for cur frame */

int npars = NPAR;              /* Number of parameters in config definition */
int totdur;        /* Utterance duration in msec */
int ms_frame;        /* Number of msec in a frame */
int nframes;            /* # frames (Dur in ms divided by framedur */
int nvar;               /* Number of variable parameters in config */
int dispars[NPAR];      /* Flags for other pars to be displayed */
 
/*    Variables known to parwv.c */
int pars[NPAR];
long sigmx;
int initsw;
 
/*    Speaker Definition */
int spkrdef[] = {
	0,      /* OUTSELECT (Select which output waveform to save)  */
	11025,  /* SAMRAT                         */
	55,     /* NSAMP_PER_FRAME (= SAMRAT * N_MSEC_FRAME) / 1000) */
	1,      /* RANSEED (Initial value for variable "nrand")      */
	5,      /* NFCASC (Number formants in cascade vocal tract)   */
	2,      /* SOURCE_SELECT (1->impulse, 2-> natural) */
};
 
int forfreq[10],nforfreq;    /* Formant freq estimates (or f0) */
int time_in_ms;     /* Time in msec corresp. to cursor position */
char symdigit[5] = {'1', '2', '3', '4', '5' };

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                              M A I N                                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int main(int argc,char *argv[]) {

    int i;
     /*    Print version number */
    printf("\n%s\n", date);
    printf("%s\n\n",d2);
    
    for (i=0; i<NPAR; i++) defval[i]=cdefval[i];
	
    OUTSELECT= defval[find_config_loc("os")];
    RANSEED  = defval[find_config_loc("rs")];
    NFCASC   = defval[find_config_loc("nf")];
    SOURCE_SELECT = defval[find_config_loc("ss")];
    setlimits(0);        /* Set nframes and/or NSAMP_PER_FRAME */
    initpars();            /* Put defaults in time functions */
    
    /*    Decode arguments */
    for (i=1; i<argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'n') user = NOVICE;      /* Verbose printout */
			else if (argv[i][1] == 'b') {
				request='s'; /* go straight to synthesis */
				batch=1;    /* in batch mode */
			}
			else if (argv[i][1] == 'g') gain_control=1;  /* auto gain */
			else {
argerr:       printf("\t  ERROR: Illegal argument to KLSYN\n");
				helpa();            /* Print argument options */
				exit(1);
			}
		}
		else {
			if (argv[i][0] == '?') {
						helpa();
						exit(1);
			}
			if (firstname[0] != '\0') {        /* name already entered */
						goto argerr;
			}
			strcpy(firstname,&argv[i][0]);
			makefilenames(firstname);
            pcname=dname;
			/* read_doc(dname); */
            // read_klt(dname);
		}
	}

/*    Main loop */
    if (user == NOVICE) helpr();            /* Print command options */
    while (1) {
	    if (user == NOVICE) printf("KLSYN Command (or '?')");
	    if (batch==0) request = get_request();
        actonrequest(request,batch);     /* Act on request */
    }
}  /* end of main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      A C T - O N - R E Q U E S T                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void actonrequest(char request,int batch)
{
    extern float dBconvert(long );
    long int i; /* for dbconvert */
    float db;	/* output of dbconvert */

    if ((request == 'q')||(request == 'Q')) exit(1);    /*  quit */
 
    if ((request == 'p')||(request == 'P')) {
        putconfig(stdout,0);   /* Print configuration */
        prpars(stdout,0);   /* print time-varied parameters */
    }

    else if ((request == 'c')||(request == 'C')) {
        if ((np = get_par_name()) >= 0) settypicalpar(np);  /* Set typical value for a parameter */
    }
 
    else if ((request == 'r')||(request == 'R')) {  /* reset varied parameter */
        if ((np = get_par_name()) >= 0) {
            if (cv[np] != FIXED) {
                if (cv[np] == VARRIED) cv[np] = VARIABLE;
                clearpar(np);
            }

        }
    }
    
    else if ((request == 'e')||(request == 'E')) drawparam();      /* Draw parameters versus time */

    else if ((request == 's')||(request == 'S')) {
// Commented out by RLS 20131126
//        printf("\n\tSynthesis requested");
        initsw = 0;
        synthesize(batch);	       /* Synthesize a waveform */

        i=sigmx;
        if (i<=0) i=1;
        db=dBconvert(i);
        if (gain_control==1) {  /* check output amplitude, reset gain and synthesize again */
            if (db>=0.0) defval[find_config_loc("g0")]-=(db+1);
            if (db < -2.0) defval[find_config_loc("g0")]+=(-1*db);
            if (db>=0.0 || db < -2.0) {
                printf("\n\tResynthesizing with adjusted gain value");
                init_static_vars();  /* reset resonators etc */
                initsw = 0;
                free(iwave);  /* release this memory */
                synthesize(1); /* second time is in batch mode */
            }
            i=sigmx;
            if (i<=0) i=1;
            db=dBconvert(i);
        }

// Commented out by RLS 20131126
//        if ((odoc = fopen(dname,"w")) != NULL) {
//            if (TAGGED==1) {
//                putconfig(odoc,TAGGED);
//                prpars(odoc,TAGGED);
//                print_outmax(odoc,TAGGED);
//            } else {
//                fprintf(odoc,"\t\t\tSynthesis specification for file:    '%s'\n", wname);
//                fprintf(odoc, "\n");
//                print_outmax(odoc,0);        /* print signal max in db */
//                putconfig(odoc,0);    /* save configuration in .doc file */
//                fprintf(odoc, "\n\n");
//                prpars(odoc,0);
//                fprintf(odoc, "\n\n");
//            }
            /*    Save array in output file */
//            writwave(wname,iwave,nsamtot,SAMRAT);
//            printf("\tWaveform saved in file\t\t'%s'\n", wname);
//            fclose(odoc);
//            printf("\tSynthesis config saved in file\t'%s'\n",dname);
//        }
//        else {  printf("\tERROR: Can't open output file '%s'\n", dname);    }
// Commented out by RLS 20131125
//        exit(1);        /* Quit so synthesizer init next time used */
    }
    else helpr();            /* Print command menu */
} /* end of actonrequest() */
 

void setlimits(int verbose) {
/*    Set number of frames in utterance, and number of samples per frame */
    SAMRAT   = defval[find_config_loc("sr")];
    totdur   = defval[find_config_loc("du")];
    ms_frame = defval[find_config_loc("ui")];

    /* this line gives the wrong answer for sampling rates not divisible by
	1000.  Like 11025, 22050, etc.  new loop control needed in parwave */
    NSAMP_PER_FRAME = ms_frame * (SAMRAT / 1000);
    nframes = (totdur + ms_frame - 1) / ms_frame;    /* Round up */
    if (nframes > MAX_FRAMES) {
            printf("\n\tToo many frames for parameter buffer,");
            nframes = MAX_FRAMES;
            printf(" duration truncated to %d ms\n", totdur);
    }
    totdur  = nframes * ms_frame;
 
    if (verbose) {
        printf("\tUtterance duration = %d ms,  %d ms per frame\n",totdur, ms_frame);
        printf("\ttherefore, there are %d time frames to be synthesized\n\n",nframes);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                   F I N D - C O N F I G - L O C                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int find_config_loc(char *arg) {
 
    int n;
 
    for (n=0; n<npars; n++) {
        if (arg[0] == symb1[n]) {
			if (arg[1] == symb2[n]) {
				return(n);
			}
        }
    }
    printf("ERROR in find_config_loc('%s'), symbol doesn't exist\n", arg);
    return(0);
}
 
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      P U T - C O N F I G - F I L E                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void putconfig(FILE *dev,int tagged) {
    int i, n;
    
    if (tagged==1) {
        fprintf(dev,"<config>\n");
        for (i=0;i<npars;i++) {
            if (cdefval[i] != defval[i]) fprintf(dev,"%c%c\t%5d\n",symb1[i],symb2[i],defval[i]);
        }
        fprintf(dev,"</config>\n");
    }
    else {
        fprintf(dev, "\n  CURRENT CONFIGURATION:\n");
        fprintf(dev, "    %2d parameters\n\n", npars);
        for (n=0; n<2; n++) fprintf(dev, "       SYM V/C  MIN   VAL   MAX");
        fprintf(dev, "\n");
        for (n=0; n<2; n++) fprintf(dev, "       ------------------------");
        fprintf(dev, "\n");
        i = 0;
        for (n=0; n<npars; n++) {
            fprintf(dev, "       %c%c   %c  %4d %5d %5d",
                    symb1[n], symb2[n], cv[n], minval[n], defval[n], maxval[n]);
            if (++i == 2) {
                i = 0;
                fprintf(dev, "\n");
                }
        }
        if (i != 0) fprintf(dev, "\n");
    }
    
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      G E T - C O N F I G - F I L E                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void read_doc(char *fname)   {
    FILE *fp;
    int n, count, i, time;
    char s11, s21, vc1, s12, s22, vc2, c;
    int mn1, v1, mx1, mn2, v2, mx2;
    char line[512],subline[256],word[12];
    char vn[30][2];  /* array to store time var parameter names */
    int vl[30];
    
    if ((fp = fopen(fname,"r")) == NULL) {
        printf("\tUnable to find %s, exiting.\n",fname);
    }
    while (fgets(line, sizeof line, fp) != NULL) {
        if ((sscanf(line,"%d %s",&n, subline)==2) && (strcmp(subline, "parameters")==0)) {
                npars=n;
                printf("\tReading %d parameters from file\n",npars);
        }
        if (sscanf(line," %c%c %c %d %d %d %c%c %c %d %d %d",&s11, &s21, &vc1, &mn1, &v1, &mx1, &s12, &s22, &vc2, &mn2, &v2, &mx2)==12) {
            symb1[0]=s11; symb2[0]=s21; cv[0]=vc1; minval[0]=mn1; defval[0]=v1; maxval[0]=mx1;
            symb1[1]=s12; symb2[1]=s22; cv[1]=vc2; minval[1]=mn2; defval[1]=v2; maxval[1]=mx2;

            fgets(line,sizeof line, fp);  // get next line
            n=2;
            while ((count = sscanf(line," %c%c %c %d %d %d %c%c %c %d %d %d",&s11, &s21, &vc1, &mn1, &v1, &mx1, &s12, &s22, &vc2, &mn2, &v2, &mx2))==12) {
                symb1[n]=s11; symb2[n]=s21; cv[n]=vc1; minval[n]=mn1; defval[n]=v1; maxval[n]=mx1;
                symb1[n+1]=s12; symb2[n+1]=s22; cv[n+1]=vc2; minval[n+1]=mn2; defval[n+1]=v2; maxval[n+1]=mx2;
                n=n+2;
                fgets(line,sizeof line,fp);  // get next line
            }
            if (count==6) {
                symb1[n]=s11; symb2[n]=s21; cv[n]=vc1; minval[n]=mn1; defval[n]=v1; maxval[n]=mx1;
            }
            if (n+1 != npars) printf("\tWarning: found %d parameters instead of %d\n",n+1, npars);
        
        /* Check how many parameters are varied */
            nvar = 0 ;
            for(n=0; n<npars; n++)  if (cv[n] == VARRIED)  nvar++ ;
            printf("\t%d parameters are time-varying.\n\n", nvar) ;
            OUTSELECT= defval[find_config_loc("os")];
            RANSEED  = defval[find_config_loc("rs")];
            NFCASC   = defval[find_config_loc("nf")];
            SOURCE_SELECT = defval[find_config_loc("ss")];
            setlimits(1);    /* Set nframes and/or NSAMP_PER_FRAME */
            
            /* Set all params to config values */
            for(np=0; np<npars; np++) clearpar(np);
            ipsw = MYTRUE;
        }

        if (sscanf(line,"Varied Parameters%c",&c)==1) {
            /* read header line: time plus varied parameters - gets(line), parse line*/
            fgets(line,sizeof line, fp);
            n = sscanf(line,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
                       word,vn[0],vn[1],vn[2],vn[3],vn[4],vn[5],vn[6],vn[7],vn[8],vn[9],vn[10],vn[11],
                       vn[12],vn[13],vn[14],vn[15],vn[16],vn[17],vn[18],vn[19],vn[20],vn[21],vn[22],vn[23],
                       vn[24],vn[25],vn[26],vn[27],vn[28],vn[29]);
    
            n--;  /* number of varied parameters */
            if (n != nvar) printf("\tWarning: found a discrepancy in the number of varied parameters\n");
            
            printf("\t  now reading a table of %d time-varying parameters.\n",n);
            for (nf=0;nf<nframes;nf++) {
                fgets(line,sizeof line, fp);
                n = sscanf(line,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                           &time, &vl[0],&vl[1],&vl[2],&vl[3],&vl[4],&vl[5],&vl[6],&vl[7],&vl[8],&vl[9],&vl[10],&vl[11],
                           &vl[12],&vl[13],&vl[14],&vl[15],&vl[16],&vl[17],&vl[18],&vl[19],&vl[20],&vl[21],&vl[22],&vl[23],
                           &vl[24],&vl[25],&vl[26],&vl[27],&vl[28],&vl[29]);
                for (i=0;i<n-1;i++) {
                    np = find_config_loc(vn[i]);
                    pdata[np][nf]=vl[i];
                }
            }
            
        }
    }
    fclose(fp);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                        S E T - T Y P I C A L - P A R                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void settypicalpar(int np) {
 
    if (cv[np] == VARRIED) {
        printf(	 "\tThis parameter already varied, change in default has no effect\n");
        return;
    }
    printf("\tChange value of %c%c (min = %d, max = %d) to\n", symb1[np], symb2[np], minval[np], maxval[np]);
    val = get_value(defval[np]);
    defval[np] = val;
    clearpar(np);            /* Put new value in time function */
 
    OUTSELECT= defval[find_config_loc("os")];
    RANSEED  = defval[find_config_loc("rs")];
    NFCASC   = defval[find_config_loc("nf")];
    SOURCE_SELECT = defval[find_config_loc("ss")];
    if  ((np == find_config_loc("du"))
      || (np == find_config_loc("ui"))
      || (np == find_config_loc("sr")) ) setlimits(1); /* Reset nframes and/or NSAMP_PER_FRAME */
    if ((user == NOVICE) || (val != defval[np])) putconfig(stdout,0);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                        D R A W - P A R A M E T E R                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void drawparam(void) {
    int nfsw;
 
/*    Initialize paramter buffer if not done yet */
    if (ipsw != MYTRUE) initpars();

/*    Select paramter (loop here until user types 'q' or <CR> */
    while ((np = get_par_name()) >= 0) {
        if (cv[np] == FIXED) {
            printf("\tERROR: \'%c%c\' Not a variable param", sym1, sym2);
            printf(" try again\n");
        }
        else {
            if (cv[np] != VARRIED) {
                nvar++;
                cv[np] = VARRIED;
            }
            nf = 0;
            val = pdata[np][nf];
            printf("\nEmpty line terminates input\n");

           /* Read first point  */
            gettimval(&nf,&val);
            if (nf >= 0) {
                nfsw = nf; /* Set switches to set initial point */
                lastnf = nf;
                while (1) {
                    lastval = val;
                    gettimval(&nf,&val);
                    if (nf < 0)  break;
                    if (nfsw != -1) {   /* Must wait before setting first point */
                        pdata[np][nfsw] = lastval;
                        nfsw = -1;
                    }
		/* fill frames btw nf and lastnf by linear interpolation */
                    fill_frames(np,nf,lastnf,val,lastval);
                    lastnf = nf;
                } /* while */
            } /* if (nf */
        } /* else { */
    } /* while ((np */
} /* drawparam */
 

/*------------gettimval---------------------------Aug 87   KJ----*/
void gettimval(int *gtime,int *gvalue) {

    *gtime=get_time();
    if (*gtime == -1) return;
    *gvalue=get_value(*gvalue);

}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                            N A M E - F I L E                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void namefile(void) {
 
trynam: printf("\n\tFirst name for '.wav' & '.klt' output files:    ");
    scanf("%s",firstname);
    if (strlen(firstname) == 0) {
        printf("\n\tERROR: Name has no chars, try again\n");
        goto trynam;
    }
    makefilenames(firstname);
}
 
 
void makefilenames(char *fname) {

    strcpy(wname,fname);
    strcat (wname,".wav");
    
    strcpy(dname,fname);
    strcat(dname,".klt");
}
 
 
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                            P U T P A R S                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void putpars() {
 
/*    Make header */
    fprintf(odev, "Parameter data:\n");
    fprintf(odev, "  %d params were varied and are thus saved here\n",nvar);
    fprintf(odev, "  %d is the number of frames in each param track\n",nframes);
 
/*    Parameter tracks */
    n = 0;
    for (np=0; np<npars; np++) {
	if (cv[np] == VARRIED) {
	   n++;    /* Count # of varied pars for consistency check */
	   fprintf(odev, "%c%c\n", symb1[np], symb2[np]);
	   for (nf=0; nf<nframes; nf++) fprintf(odev, "%d\n", pdata[np][nf]);
	}
    }
    if (n != nvar) {
       printf("\tERROR: Header info about nvar incorrect, try again\n");
       nvar = n;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                S Y N T H E S I Z E - W A V E F O R M                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void synthesize(int batch) {
 
// Commented out by RLS 20131126
//    printf("\n\tSynthesize waveform.\n");
//    if ((batch==0) || (firstname[0]=='\0')) namefile();
//    else makefilenames(firstname); /* don't ask for filename in batch mode */

    nsamtot = nframes * NSAMP_PER_FRAME;

	/*    Allocate core area to put synthetic waveform */
    if ((iwave = (int16_t *) calloc(nsamtot,sizeof(int16_t))) == NULL) {
        printf("ERROR in Klsyn.c: no memory available for wave\n");
        return;
    }
 
    if (ipsw != MYTRUE) initpars(); /* Stick default values in param tracks */

    /*    Main loop, for each frame of parameter data */
    nsamtot = 0;
    for (nf=0; nf<nframes; nf++) {
        /* Move pars into output parameter buffer */
        nskip = 0;
        for (np=0; np<npars; np++) {
            if (cv[np] == FIXED) nskip++;
            else if (cv[np] == VARRIED) pars[np-nskip] = pdata[np][nf];
            else pars[np-nskip] = defval[np];
        }
        
        /* Compute waveform chunk from paramter data */
        parwav(&iwave[nsamtot]);
        nsamtot += NSAMP_PER_FRAME;
    }
}
 
void print_outmax(FILE *odoc, int tagged) {
 
    float db;
    extern float dBconvert(long );
    long i;
 
    i = sigmx;
    if (i <= 0)    i = 1;
    db = dBconvert(i);
    printf("\n\tMax output signal (overload if greater than 0.0 dB) is  %3.1f dB  \n", db);
    printf("\tTotal number of waveform samples = %d\n\n", nsamtot);

    if (tagged == 0) {
        fprintf(odoc,"\n\n\tMax output signal (overload if greater than 0.0 dB) is  %3.1f dB  \n", db);
        fprintf(odoc,"\tTotal number of waveform samples = %d\n\n", nsamtot);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*            I N I T - P A R A M E T E R - T R A C K S                  */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void initpars(void) {
    int np;
    
    ipsw = MYTRUE;
    nvar = 0;
    for (np=0; np<npars; np++) {
        if (cv[np] != FIXED) {
            if (cv[np] == VARRIED) cv[np] = VARIABLE;
            clearpar(np);
        }
    }
}
 
 
void clearpar(int np) {
 
  for (nf=0; nf<nframes; nf++) {
      val = defval[np];
      if (np >= MAX_VAR_PARS) {
          printf("  ERROR: np=%d > max=%d, ignore request\n", np, MAX_VAR_PARS);
      }
      else if (nf >= MAX_FRAMES) {
          printf("  ERROR: Time frame nf=%d > max=%d, ignored\n", nf, MAX_FRAMES);
      }
      else pdata[np][nf] = val;
      
  }
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                     f i l l _ f r a m e s                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void fill_frames(int np,int nf,int lastnf,int val,int lastval) {
    float dval,dnf;
    int nf1,nf2,val1,nfinc;
 
    /* See if line to be drawn forward in time or backward */
    if (nf > lastnf) nfinc = 1;
    else nfinc = -1;

    /*    Linear interpolation */
    nf1 = lastnf;
    nf2 = nf;
    val1 = lastval;
    dval = (val - lastval);
    dnf = nf2 - nf1;
    for (nf=nf1; nf!= nf2; nf+=nfinc) {
	val = val1 + ((dval*(nf-nf1))/dnf);
	pdata[np][nf] = val;
    }
    nf = nf2;
    val = val1 + dval;
    pdata[np][nf] = val;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      D E C O D E - P A R A M                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int decodparam(void) {

    int nxx;
 
    nxx = 0;
    for (nxx=0; nxx<npars; nxx++) {
	if ((sym1 == symb1[nxx]) && ( sym2 == symb2[nxx])) return(nxx);
    }
    return(-1);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      C H E C K - L I M I T S                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int checklimits(void) {
    char command;
ch1:
    if (val > maxval[np]) {
       printf("\t  ERROR: %d is bigger than maxval[%c%c] = %d\n", val, symb1[np], symb2[np], maxval[np]);
       printf("\t  Override this suggested maximum [y] [n] ");
       command = get_request();
       if (command == 'y') {
           maxval[np] = val;
           printf("\t\tmaxval[%c%c] is now %d\n", symb1[np], symb2[np], maxval[np]);
       }
       else {
           printf("\t  Please type requested value again\n");
           return(-2);
       }
    }
    if (val < minval[np]) {
        if (val == -1) {
            val = getpval();
            printf("\t  Use current value == %d\n", val);
            goto ch1;
        }
        printf("\t  ERROR: %d is smaller than minval[%c%c] = %d\n", val, symb1[np], symb2[np], minval[np]);
        printf("\t  Override this suggested minimum [y] [n] ");
        command = get_request();
        if (command == 'y') {
            minval[np] = val;
            printf("\t\tminval[%c%c] is now %d\n", symb1[np], symb2[np], minval[np]);
        }
        else {
            printf("\t  Please type requested value again\n");
            return(-2);
        }
    }
    return(val);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                      G E T P V A L                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int getpval(void) {
 
    if (np >= MAX_VAR_PARS) {
       printf("  ERROR: np=%d > max=%d in getpval()", np, MAX_VAR_PARS);
       return(0);
    }
    if (nf >= MAX_FRAMES) {
       printf("  ERROR: frame =%d > max=%d in getpval()", nf, MAX_FRAMES);
       return(0);
    }
    return(pdata[np][nf]);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*              P R I N T - P A R A M E T E R - T R A C K S              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void prpars(FILE *dev,int tagged) {
    int i, n, npar1;
    
    if (tagged==1){
        if (nvar == 0) return;
        fprintf(dev,"<pars>\n");
        
        fprintf(dev,"<time>");
        for (n=0; n<nframes; n++) fprintf(dev,"\t%5d",ms_frame*n);
        fprintf(dev,"\t</time>\n");
        
        for (i=0;i<npars;i++) {
            if (cv[i] == VARRIED) {
                
                fprintf(dev,"<%c%c>",symb1[i],symb2[i]);
                for (n=0; n<nframes; n++) fprintf(dev,"\t%5d",pdata[i][n]);
                fprintf(dev,"\t</%c%c>\n",symb1[i],symb2[i]);
                
            }
        }
        fprintf(dev,"</pars>\n");
    }
    else {
        if (nvar == 0) fprintf(dev, "\tNo parameters are varied\n");
        else {
            fprintf(dev, "\n\nVaried Parameters:\n");
            if (nvar == 0) fprintf(dev, "    none");
            else {
                fprintf(dev, "time");
                for (npar1=0; npar1<npars; npar1++) {
                    if (cv[npar1] == VARRIED)
                        fprintf(dev, "   %c%c", symb1[npar1], symb2[npar1]);
                }
            }
            fprintf(dev, "\n");

            for (n=0; n<nframes; n++) {
                fprintf(dev, "%4d", ms_frame*n);
                for (npar1=0; npar1<npars; npar1++) {
                    if (cv[npar1] == VARRIED) fprintf(dev, "%5d", pdata[npar1][n]);
                }
                fprintf(dev, "\n");
            }
            fprintf(dev, "\n");
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - R E Q U E S T                        */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char get_request(void) {
 
        char req,junk;
 
        printf("> ");
        req = getchar();
        junk = getchar();  /* for the end of line char */
        return(req);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - P A R - N A M E                      */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int get_par_name(void) {
 
    char garb;
 
    np = -1;
    while (np == -1) {
        if (user == NOVICE) printf("\n    2-char name for desired param (or <CR> to quit): ");
        else printf("\n    Par: ");
        sym1 = '\0';
        sym2 = '\0';
        scanf("%c", &garb);
        if (garb != '\n') {
            sym1 = garb;
            scanf("%c", &garb);
            if (garb != '\n') {
                sym2 = garb;
                while (garb != '\n') scanf("%c", &garb);
            }
        }
        if ((sym1 == '\0') || (sym1 == 'q')) return(-2); /* Request to quit */
        if ((np = decodparam()) == -1) {
            if (sym1 != '?') printf("\tERROR: '%c%c' is not a legal parameter\n", sym1, sym2);
            putconfig(stdout,0);
        }
    }
    return(np);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - T I M E                              */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int get_time(void) {
 
    int nms;
 
    if (user == NOVICE) {
repett: printf("\n\tTime in msec (limits are 0 to %d), or <CR> to quit: ",
              totdur);
    }
    else  printf("\tTime:  ");
    nms = get_digits(stdin); /* Time in msec */
    if (nms < 0) {
       if (nms == -1) return(-1);  /* Request to quit */
       if (nms != -2) printf("\t  ERROR in requested value\n");
       goto repett;
    }
    nms = nms / ms_frame;  /* Convert to number of frames */
    if (nms == nframes) nms--;        /* Assume user meant last possible frame */
    else if (nms > nframes) {
       printf("\t  ERROR in requested value\n");
       goto repett;
    }
    return(nms);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - V A L U E                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int get_value(int arg) {

    if (user == NOVICE) {
gv1:   printf("\tValue (was %d, limits are %d to %d): ",
             arg, minval[np], maxval[np]);
    }
    else printf("\tValue (was %d): ", arg);
    val = get_digits(stdin);
    if (val < -1) {
        if (val != -2) printf("\t  ERROR: Negative values are meaningless\n");
        goto gv1;
    }
    val = checklimits();
    if (val < 0) goto gv1; /* checklimits sets to -2 if user doesn't override */
    return(val);
}
 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                          G E T - D I G I T S                          */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int get_digits(FILE *dev) {
 
    int dig,nn;
    char digitarray[9],c,c2;
 
    /*    Strip off leading white space */
    do fscanf(dev, "%c", &c); while ((c == ' ') || (c == '\t'));
    if (c == '\n') return(-1);

    /*    Read digits and place in array digitarray[] */
    nn = 0;
    while ((c == '-') || ((c >= '0') && (c <= '9'))) {
       if (nn >= 8) {
           printf("\t  ERROR: Digit string too long, value set to 0\n");
           return(0);
       }
       digitarray[nn++] = c;
       fscanf(dev, "%c", &c);
    }
    if ((c != ' ') && (c != '\t') && (c != '\n')) {
        do fscanf(dev, "%c", &c2); while (c2 != '\n');
        if (c == 'q') return(-1);
        if (c == '?') return(-2);
        printf("\t  ERROR: Alphabetic char '%c' in a digit string\n", c);
        return(-2);
    }
    digitarray[nn] = '\0';
 
/*    Convert from ascii string to integer */
    dig = atoi(digitarray);
    return(dig);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                  H E L P - L I S T - A R G U M E N T S                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void helpa(void) {
    printf("\n    Required arguments for KLSYN:\n");
    printf("\tnone\n");
    printf("\n    Optional arguments:\n");
    printf("\t-n\tNovice user, verbose printout please\n");
	printf("\t-b\tBatch mode, synthesize .klt file without asking\n");
	printf("\t-g\tUse automatic gain control to set parameter G0 \n");
    printf("\tname\tFirst name of '.klt' files to be read\n");
    printf("\n");
}
 

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                  H E L P - A C T - O N - R E Q U E S T                */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void helpr(void) {
    printf("\tLegal KLSYN commands:\n");
    printf("\t?\tHELP, please list legal KLSYN commands\n");
    printf("\tp\tPRINT set of synthesis parameter default values\n");
    printf("\tc\tCHANGE default value for a synthesis parameter\n");
    printf("\tr\tRESET varied parameter to default value\n");
    printf("\te\tENTER synthesis parameter time function\n");
    printf("\ts\tSYNTHESIZE waveform file, save everything, quit\n");
    printf("\tq\tQUIT program, save nothing\n");
    printf("\n");
}
