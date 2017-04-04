/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                        P A R W A V E . C                            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*   Copyright            1982                    by Dennis H. Klatt
 *
 *      Klatt synthesizer
 *         Modified version of synthesizer described in
 *         J. Acoust. Soc. Am., Mar. 1980. -- new voicing
 *         source.
 *
 *   This software may not be resold or used in any commercial product.
 *
 * Edit history
 * 000001 10-Mar-83 DK  Initial creation.
 * 000002  5-May-83 DK  Fix bug in operation of parallel F1
 * 000003  7-Jul-83 DK  Allow parallel B1 to vary, and if ALL_PARALLEL,
 *                      also allow B2 and B3 to vary
 * 000004 26-Jul-83 DK  Get rid of mulsh, use short for VAX
 * 000005 24-Oct-83 DK  Split off parwavtab.c, change short to int
 * 000006 16-Nov-83 DK  Make samrate a variable, use exp(), cos() rand()
 * 000007 17-Nov-83 DK  Convert to float, remove  cpsw, add set outsl
 * 000008 28-Nov-83 DK  Add simple impulsive glottal source option
 * 000009  7-Dec-83 DK  Use spkrdef[7] to select impulse or natural voicing
 *                       and update cascade F1,..,F6 at update times
 * 000010 19-Dec-83 DK  Add subroutine no_rad_char() to get rid of rad char
 * 000011 28-Jan-84 DK  Allow up to 8 formants in cascade branch F7 fixed
 *                       at 6.5 kHz, F8 fixed at 7.5 kHz
 * 000012 14-Feb-84 DK  Fix bug in 'os' options so os>12 works
 * 000013 17-May-84 DK  Add G0 code
 * 000014 14-Nov-85 DK  Compensate resonator history variables when change F1,
 *                       optional pitch-synch F1,B1 change if dF & db non-zero,
                         linearize TLTdb,
                         no -> oq
 * 000015 24-Feb-86 DK  Reverse polarity of waveform when even # of resonators
 * 000016 28-Feb-86 DK  Change F1,F2,F3 when change requested, also change F1
                         pitch-synch, use modified Fujisaki compensation to
                         y(nt-T) and y(nT-2T) for F1,F2 & F3 of cascade branch
                        Add triangular glottal source (ss=2) with param "assym"
 * 000017 14-Oct-87 KJ & YYQ convert to 16 bit integers for PC version
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include "parwvt.h"

#define N_SPDEF_PARS    8
#define N_VARIABLE_PARS 40   /* these is used in pr_pars */

#define IMPULSIVE       1       /* spkrdef[7] sets glsource to IMPULSIVE or */
#define NATURAL         2       /*                              NATURAL.    */
#define TRIANGULAR      3       /*                              TRIANGULAR  */

extern int spkrdef[],pars[];            /* From KLSYN.C   */
extern long sigmx;                      /* From KLSYN.C   */
extern int initsw;

/* CONVERT FRAME OF PARAMETER DATA TO A WAVEFORM CHUNK
 * Synthesize nspfr samples of waveform and store in jwave[]. */

int ntime;                                      /* TEMPORARY */
static float anorm1;  /* Normalizing scale factor for F1 sudden change */
static float anorm2;  /* Normalizing scale factor for F2 sudden change */
static float anorm3;  /* Normalizing scale factor for F3 sudden change */
static int F1last,F2last,F3last; /* for sudden formant changes */
static int disptcum,F1hzmod,B1hzmod;
static float r8ca,r8cb,r8cc,r7ca,r7cb,r7cc,r8c_1,r8c_2,r7c_1,r7c_2;
/* static float peakval = -2200.;  not used */
static float slopet1,slopet2,Afinal,maxt1,maxt2;        /* For triangle */
static int nfirsthalf,nsecondhalf,assym,as;   /* For triangle */
/* static int nc;  not used */

void parwav(int16_t *jwave) {
/* Initialize synthesizer and get specification for current speech
   frame from host microcomputer */
    gethost();

/* MAIN LOOP, for each output sample of current frame: */
    for (ns=0; ns<nspfr; ns++) {

/*    Get low-passed random number for aspiration and frication noise */
        gen_noise();                    /* output variable is 'noise' */

/*    Amplitude modulate noise (reduce noise amplitude during semi-closed
 *    portion of glottal period) if voicing simultaneously present.
 *    nmod=T0 if voiceless, nmod=nopen if voiced  */
/* BUG: This is backward (reduced during open phase) if impulsive source */
        if (nper > nmod) {
            noise *= 0.5;
        }

/*    Compute frication noise */
        frics = amp_frica * noise;

/*  Compute voicing waveform */
/*    (run glottal source simulation at 4 times normal sample rate to minimize
 *    quantization noise in period of female voice) */

        for (n4=0; n4<4; n4++) {

/*        Use impulsive glottal source */
            if (glsource == IMPULSIVE) {
                voice = impulsive_source();
/*            Set F1 and BW1 pitch synchrounously */
                if (nper == (T0 - nopen)) {
                    setR1(F1hz+dF1hz,B1hz+dB1hz);       /* Glottis opens */
                }
                if (nper == T0) {
                    setR1(F1hz,B1hz);                   /* Glottis closes */
                }
            }
/*        Or use a more-natural-shaped source waveform with excitation
          occurring both upon opening and upon closure, strongest at closure */
            else {
                if (glsource == NATURAL) {
                    voice = natural_source();
                }
                else if (glsource == TRIANGULAR){
                    voice = triangular_source();
                }
                else {
                    voice = square_source();
                }
/*            Modify F1 and BW1 pitch synchrounously */
                if (nper == nopen) {
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz,B1hz);
                    }
                    F1hzmod = 0;                /* Glottis closes */
                    B1hzmod = 0;
                }
                if (nper == T0) {
                    F1hzmod = dF1hz;            /* Glottis opens */
                    B1hzmod = dB1hz;
                    if ((F1hzmod+B1hzmod) > 0) {
                        setR1(F1hz+F1hzmod,B1hz+B1hzmod);
                    }
                }
            }

/*        Reset period when counter 'nper' reaches T0 */
            if (nper >= T0) {
                nper = 0;
                pitch_synch_par_reset();
            }

/*        Low-pass filter voicing waveform before downsampling from 4*samrate
 *        to samrate samples/sec.  Resonator f=.09*samrate, bw=.06*samrate */
            resonlp();          /* in=voice, out=voice */

/*        Increment counter that keeps track of 4*samrate samples/sec */
            nper++;
        }

/*    Tilt spectrum of voicing source down by soft low-pass filtering, amount
 *    of tilt determined by TLTdb */
        voice = (voice * onemd) + (vlast * decay);
        vlast = voice;

/*    Add breathiness during glottal open phase */
/* BUG: This is backward (on during closed phase) if impulsive source */
        if (nper < nopen) {
/*        Amount of breathiness determined by parameter Aturb */
/*        Use nrand rather than noise because noise is low-passed */
            voice += amp_breth * nrand;
        }

/*    Set amplitude of voicing */
        glotout = amp_voice * voice;
        par_glotout = par_amp_voice * voice;

/*    Compute aspiration amplitude and add to voicing source */
        aspiration = amp_aspir * noise;
        glotout += aspiration;
        par_glotout += aspiration;

/*  Cascade vocal tract, excited by laryngeal sources.
 *    Nasal antiresonator, then formants FNP, F5, F4, F3, F2, F1 */

        resoncnz();             /* in=glotout, out=rnzout   */

        resoncnp();             /* in=rnzout, out=rnpc_1 */

        casc_next_in = rnpc_1;
/*    Make signal out of cascade branch have right polarity */
        if ((nfcascade & 07776) == nfcascade)  casc_next_in = -rnpc_1;

        if (nfcascade >= 8) {
            resonc8();          /* Do not use unless samrat = 16000 */
            casc_next_in = r8c_1;
        }

        if (nfcascade >= 7) {
            resonc7();          /* Do not use unless samrat = 16000 */
            casc_next_in = r7c_1;
        }

        if (nfcascade >= 6) {
            resonc6();          /* Do not use unless long vocal tract or */
            casc_next_in = r6c_1;       /* samrat increased */
        }

        if (nfcascade >= 5) {
            resonc5();
            casc_next_in = r5c_1;
        }

        if (nfcascade >= 4) {
            resonc4();
            casc_next_in = r4c_1;
        }

        if (nfcascade >= 3) {
            resonc3();
            casc_next_in = r3c_1;
        }

        if (nfcascade >= 2) {
            resonc2();
            casc_next_in = r2c_1;
        }

        if (nfcascade >= 1) {
            resonc1();
        }

        if (outsl > 0) {
            if (outsl < 13) {
                if (outsl ==  1)        out = voice;
                if (outsl ==  2)        out = aspiration;
                if (outsl ==  3)        out = frics;
                if (outsl ==  4)        out = glotout * 2.0;
                if (outsl ==  5)        out = par_glotout;
                if (outsl ==  6)        out = rnzout;
                if (outsl ==  7)        out = rnpc_1;
                if (outsl ==  8)        out = r5c_1;
                if (outsl ==  9)        out = r4c_1;
                if (outsl == 10)        out = r3c_1;
                if (outsl == 11)        out = r2c_1;
                if (outsl == 12)        out = r1c_1;
                if (outsl <= 4) {
                    no_rad_char(out);   /* Take out effects of radiation char */
                }
                goto skip;
            }
        }
        out = r1c_1;

/*    Excite parallel F1 and FNP by voicing waveform */

        sourc = par_glotout;        /* Source is voicing plus aspiration */
        reson1p();                 /* in=sourc, out=rnpp_1 */
        resonnpp();                /* in=sourc, out=r1c_1 */
        out += rnpp_1 + r1p_1;     /* Add in phase, boost lows for nasalized */

/*  Standard parallel vocal tract
 *  Formants F6,F5,F4,F3,F2, outputs added with alternating sign */

/*    Sound sourc for other parallel resonators is frication plus */
/*    first difference of voicing waveform */
        sourc = frics + par_glotout - glotlast;
        glotlast = par_glotout;

        reson6p();              /* in=sourc, out=r6p_1 */
        out = r6p_1 - out;

        reson5p();              /* in=sourc, out=r5p_1 */
        out = r5p_1 - out;

        reson4p();              /* in=sourc, out=r4p_1 */
        out = r4p_1 - out;

        reson3p();              /* in=sourc, out=r3p_1 */
        out = r3p_1 - out;

        reson2p();              /* in=sourc, out=r2p_1 */
        out = r2p_1 - out;

        outbypas = amp_bypas * sourc;
        out = outbypas - out;

        if (outsl > 12) {
                if (outsl == 13)        out = r6p_1;
                if (outsl == 14)        out = r5p_1;
                if (outsl == 15)        out = r4p_1;
                if (outsl == 16)        out = r3p_1;
                if (outsl == 17)        out = r2p_1;
                if (outsl == 18)        out = r1p_1;
                if (outsl == 19)        out = rnpp_1;
                if (outsl == 20)        out = outbypas;
        }

skip:   ltemp = out * amp_gain0;                /* Convert back to integer */
        getmax(ltemp,&sigmx);
		*jwave++ = parwv_truncate(-ltemp);
    }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                   S U B R O U T I N E   G E T H O S T               */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Get variable parameters from host computer,
 *   initially also get definition of fixed pars */

        static float minus_pi_t,two_pi_t;
        int m;

void gethost(void) {

/* static int F2last, F3last;  defined as global for set_static_vars */

/*  Initialize speaker definition */
        if (initsw == 0) {
            initsw++;

/*        Read Speaker Definition from Host */
            outsl     = spkrdef[ 0];    /* Select which waveform to output */
            samrate   = spkrdef[ 1];    /* Sampling rate                   */
            nspfr     = spkrdef[ 2];    /* # of samples per frame of pars  */
            ranseed   = spkrdef[ 3];    /* Seed for ran num gen            */
            nfcascade = spkrdef[ 4];    /* Number formants in cascade tract*/
            glsource  = spkrdef[ 5];    /* 1->impulsive, 2->natural voicing*/

            FLPhz = 950 * (samrate / 10000);
            BLPhz = 630 * (samrate / 10000);
            minus_pi_t = -3.14159 / samrate;
            two_pi_t = -2. * minus_pi_t;
            setabc(FLPhz,BLPhz,&rlpa,&rlpb,&rlpc);
            srand(ranseed);  /* Init ran # generator */
        }

/*    Read  speech frame definition into temp store
 *    and move some parameters into active use immediately
 *    (voice-excited ones are updated pitch synchronously
 *    to avoid waveform glitches). */

        F0hz = pars[ 0];
        AVdb  = pars[ 1] - 7;
            if (AVdb < 0)    AVdb = 0;

        F1hz  = pars[ 2];
        B1hz  = pars[ 3];
        dF1hz  = pars[40];      /* F1 increment during open phase of cycle */
        dB1hz  = pars[41];      /* B1 increment during open phase of cycle */

        F2hz  = pars[ 4];
        B2hz  = pars[ 5];

        F3hz  = pars[ 6];
        B3hz  = pars[ 7];

        F4hz  = pars[ 8];
        B4hz  = pars[ 9];

        F5hz  = pars[10];
        B5hz  = pars[11];

        F6hz  = pars[12];        /* f  of parallel 6th formant */
        B6hz  = pars[13];        /* bw of cascade 6th formant (doesn't exist) */

        FNZhz = pars[14];
        BNZhz = pars[15];

        FNPhz = pars[16];
        BNPhz = pars[17];

        AP    = pars[18];
            amp_aspir = DBtoLIN(AP) * .05;
        Kopen = pars[19];       /* Open quotient in percent of T0 */
        as = pars[42];          /* assymetry of triangular pulse, 0-100 */

        Aturb = pars[20];
        TLTdb = pars[21];

        AF    = pars[22];
            amp_frica = DBtoLIN(AF) * 0.25;
        Kskew = pars[23];
        AVpdb = pars[38];
            par_amp_voice = DBtoLIN(AVpdb);

        A1    = pars[24];
            amp_parF1 = DBtoLIN(A1) * 0.4;
        B1phz = pars[25];

        A2    = pars[26];
            amp_parF2 = DBtoLIN(A2) * 0.15;
        B2phz = pars[27];

        A3    = pars[28];
            amp_parF3 = DBtoLIN(A3) * 0.06;
        B3phz = pars[29];

        A4    = pars[30];
            amp_parF4 = DBtoLIN(A4) * 0.04;
        B4phz = pars[31];

        A5    = pars[32];
            amp_parF5 = DBtoLIN(A5) * 0.022;
        B5phz = pars[33];

        A6    = pars[34];
            amp_parF6 = DBtoLIN(A6) * 0.03;
        B6phz = pars[35];

        ANP   = pars[36];
            amp_parFNP = DBtoLIN(ANP) * 0.6;

        AB    = pars[37];
            amp_bypas = DBtoLIN(AB) * 0.05;

        Gain0 = pars[39] - 3;
            if (Gain0 <= 0) {
                Gain0 = 57;
            }
            amp_gain0 = DBtoLIN(Gain0);

             /*pr_pars(); DEBUG tool */   

/*    Set coefficients of variable cascade resonators */

        if (nfcascade >= 8)    setabc(7500,600,&r8ca,&r8cb,&r8cc);
        if (nfcascade >= 7)    setabc(6500,500,&r7ca,&r7cb,&r7cc);
        if (nfcascade >= 6)    setabc(F6hz,B6hz,&r6ca,&r6cb,&r6cc);
        setabc(F5hz,B5hz,&r5ca,&r5cb,&r5cc);
        setabc(F4hz,B4hz,&r4ca,&r4cb,&r4cc);
        setabc(F3hz,B3hz,&r3ca,&r3cb,&r3cc);
/*    Adjust memory variables to compensate for sudden change to F3 */
        if ((F3last != 0) && (F3hz < F3last)) {
            anorm3 = F3hz / anorm3;
            r3c_1 = r3c_1 * anorm3;
            r3c_2 = r3c_2 * anorm3;
        }
        F3last = F3hz;
        anorm3 = F3hz;          /* Save to use next time in denom of divide */

        setabc(F2hz,B2hz,&r2ca,&r2cb,&r2cc);
/*    Adjust memory variables to compensate for sudden change to F2 */
        if ((F2last != 0) && (F2hz < F2last)) {
            anorm2 = F2hz / anorm2;
            r2c_1 = r2c_1 * anorm2;
            r2c_2 = r2c_2 * anorm2;
        }
        F2last = F2hz;
        anorm2 = F2hz;          /* Save to use next time in denom of divide */

        setR1(F1hz+F1hzmod,B1hz+B1hzmod);

/*    Note that R1c not only set here (new F1,B1 used immediately),
       but also changed pitch-synchrounously in glottal source code */

/*    Set coeficients of nasal resonator and zero antiresonator */
        setabc(FNPhz,BNPhz,&rnpca,&rnpcb,&rnpcc);
        setzeroabc(FNZhz,BNZhz,&rnza,&rnzb,&rnzc);

/*    Set coefficients of parallel resonators, and amplitude of outputs */
/*    Note that R1p is not changed pitch synchronously, only R1c */
        setabc(F1hz,B1hz,&r1pa,&r1pb,&r1pc);
        r1pa *= amp_parF1;
        setabc(FNPhz,BNPhz,&rnppa,&rnppb,&rnppc);
        rnppa *= amp_parFNP;
        setabc(F2hz,B2phz,&r2pa,&r2pb,&r2pc);
        r2pa *= amp_parF2;
        setabc(F3hz,B3phz,&r3pa,&r3pb,&r3pc);
        r3pa *= amp_parF3;
        setabc(F4hz,B4phz,&r4pa,&r4pb,&r4pc);
        r4pa *= amp_parF4;
        setabc(F5hz,B5phz,&r5pa,&r5pb,&r5pc);
        r5pa *= amp_parF5;
        setabc(F6hz,B6phz,&r6pa,&r6pb,&r6pc);
        r6pa *= amp_parF6;

        disptcum += nspfr;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*       S U B R O U T I N E   I M P U L S I V E - S O U R C E         */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static float doublet[] = { 0., 13000000., -13000000. };

float impulsive_source(void) {

        if (nper < 3) {
            vwave = doublet[nper];
        }
        else {
            vwave = 0.;
        }

/*    Low-pass filter the differenciated impulse with a critically-damped
      second-order filter, time constant proportional to Kopen */
        resonglot();
        return(rgl_1);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         S U B R O U T I N E   N A T U R A L - S O U R C E           */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  Vwave is the differentiated glottal flow waveform, there is a weak
    spectral zero around 800 Hz, magic constants a,b reset pitch-synch */

float natural_source(void) {

/*    See if glottis open */
        if (nper < nopen) {
            a -= b;
            vwave += a;
            return(vwave * 0.03);
        }

/*    Glottis closed */
        else {
            vwave = 0.;
            return(0.);
        }
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*         S U B R O U T I N E   T R I A N G U L A R - S O U R C E     */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*  Vwave is the differentiated glottal flow waveform, there are strong
    spectral zeros whose locations depend on nopen and pulse symmetry */

float triangular_source() {

/*    See if glottis open */
        if (nper < nopen) {
            if (nper < nfirsthalf) {
                vwave += slopet1;
                if (vwave > maxt1)    return(maxt1);
            }
            else {
                vwave += slopet2;
                if (vwave < maxt2)    return(maxt2);
            }
            return(vwave);
        }

/*    Glottis closed */
        else {
            return(0.);
        }
}


float square_source() {
    
    /*    See if glottis open */
    if (nper < nopen) {
        vwave = -1750;
        return(vwave);
    }
        /*    Glottis closed */
    else {
        vwave = 1750;
        return(vwave);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*  S U B R O U T I N E   P I T C H _ S Y N C _ P A R _ U P D A T E    */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Reset selected parameters pitch-synchronously */

void pitch_synch_par_reset(void) {

        extern float lineartilt[];

        if (F0hz > 0) {
            T0 = 40 * ((float)samrate / (F0hz*10));         /* Period in samp*4 */
                amp_voice = DBtoLIN(AVdb);

/*        Duration of period before amplitude modulation */
            nmod = T0;
            if (AVdb > 0) {
                nmod = nopen;
            }

/*        Breathiness of voicing waveform */
            amp_breth = DBtoLIN(Aturb) * 0.1;

/*        Set open phase of glottal period */
/*        where  40 <= open phase <= 263 */
            nopen = T0*((float)Kopen/100) ;  /* Was   nopen = 4 * Kopen; */
            if ((glsource == IMPULSIVE) && (nopen > 263))    nopen = 263;
            if ((glsource == NATURAL) && (nopen > 263)) {
                temp1 = nopen / 40.0;
                printf(
"\n\tWarning: At f0 = %d, requested nopen = %5.3f ms truncated to 6.6 ms\n\t",
                 F0hz, temp1);
                nopen = 263;
            }
            if (nopen >= (T0-1)) {
                nopen = T0 - 2;
                printf(
                "Warning: glottal open period cannot exceed T0, truncated\n");
            }

            if (nopen < 40) {
                 nopen = 40;    /* F0 max = 1000 Hz */
                printf(
                "Warning: minimum glottal open period is 1.0 ms, truncated\n");
            }


/*        Reset a & b, which determine shape of "natural" glottal waveform */
            b = Bzero[nopen-40];
                a = (b * nopen) * .333;

/*        Reset width of "impulsive" glottal pulse */
            temp = samrate / nopen;             /* WAS 11000 !!! */
            setabc(0,temp,&rgla,&rglb,&rglc);
/*        Make gain at F1 about constant */
            temp1 = nopen *.00833;
            rgla *= temp1 * temp1;


/*        Reset legs of triangular glottal pulse */
            if (glsource == TRIANGULAR) {
                assym = (nopen*(as-50))/100;  /* as=50 is symmetrical  CHECK */
                nfirsthalf = (nopen>>1) + assym;
                if (nfirsthalf >= nopen)    nfirsthalf = nopen -1;
                if (nfirsthalf <= 0)            nfirsthalf = 1;
                nsecondhalf = nopen - nfirsthalf;
                Afinal = -7000.;
                maxt2 = Afinal * 0.25;
                slopet2 = Afinal / nsecondhalf;
                vwave = -(Afinal * nsecondhalf) / nfirsthalf;   /* CHECK */
                maxt1 = vwave * 0.25;
                slopet1 = - vwave / nfirsthalf;
/* OUT
printf("Reset nfh=%d nsh=%d  max1=%6.1f max2=%6.1f  slop1=%6.3f slop2=%6.3f\n\t",
 nfirsthalf,nsecondhalf, maxt1,maxt2, slopet1,slopet2);
END OUT */
            }

/*        Truncate skewness so as not to exceed duration of closed phase
          of glottal period */
            temp = T0 - nopen;
            if (Kskew > temp) {
                printf(
                "Kskew duration=%d > glottal closed period=%d, truncate\n",
                 Kskew, T0-nopen);
                Kskew = temp;
            }
            if (skew >= 0) {
                skew = Kskew;   /* Reset skew to requested Kskew */
            }
            else {
                skew = - Kskew;
            }
/*        Add skewness to closed portion of voicing period */
            T0 = T0 + skew;
            skew = - skew;
        }

        else {
            T0 = 4;                     /* Default for f0 undefined */
            amp_voice = 0.;
            nmod = T0;
            amp_breth = 0.;
            a = 0.;
            b = 0.;
        }

/*    Reset these pars pitch synchronously or at update rate if f0=0 */
        if ((T0 != 4) || (ns == 0)) {

/*        Set one-pole low-pass filter that tilts glottal source */
            if (TLTdb < 0)    TLTdb = 0;
            if (TLTdb > 34)   TLTdb = 34;
            decay = lineartilt[TLTdb];
            onemd = 1. - decay;
        }
}

/* Linearize the TILT variable */
/*  E.g. if you request 3 dB of tilt at 2500 Hz, decay = .233 */

float lineartilt[35] = {
        .000, .100, .167, .233, .300, .367, .433, .467, .500, .533,
        .567, .600, .633, .667, .700, .730, .750, .770, .790, .810,
        .825, .840, .855, .870, .885, .900, .915, .925, .935, .945,
        .955, .965, .975, .985, .995
};




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                   S U B R O U T I N E   S E T R 1                   */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Impliment pitch-synch change to F1,B1 so both rise when glottis open */

void setR1(int Fx,int Bx) {
 /* int Fx;         Desired F1 value */
 /* int Bx;         Desired B1 value */

/* static int F1last; defined global for set_static_vars */

            setabc(Fx,Bx,&r1ca,&r1cb,&r1cc);
/*        Adjust memory variables to have proper levels for a given sudden
          change to F1hz and B1hz.
          Approximate r1c_n' = r1c_n * sqrt(r1ca/r1calast)
          by r1c_n' = r1c_n * (F1hz/F1hzlast) */

	    if ((F1last != 0) && (Fx < F1last)) {
                anorm1 = Fx / anorm1;
/*            For reasons that I don't understand, amplitude compensation
              only needed when a formant goes down in frequency */
                r1c_1 = r1c_1 * anorm1;
                r1c_2 = r1c_2 * anorm1;
            }
	    F1last = Fx;	/* For print only */
            anorm1 = Fx;        /* Save to use next time in denom of divide */
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                   S U B R O U T I N E   S E T A B C                 */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert formant freqencies and bandwidth into
 *      resonator difference equation constants */

void setabc(int f,int bw,float *acoef,float *bcoef,float *ccoef) {

        float r;
        double arg;


/*    Let r  =  exp(-pi bw t) */

        arg = minus_pi_t * bw;
        r = exp(arg);

/*    Let c  =  -r**2 */

        *ccoef = -(r * r);

/*    Let b = r * 2*cos(2 pi f t) */

        arg = two_pi_t * f;
        *bcoef = r * cos(arg) * 2.;

/*    Let a = 1.0 - b - c */

        *acoef = 1.0 - *bcoef - *ccoef;

/*    Debugging printout *
      printf("f=%4d bw=%3d acoef=%8.5f bcoef=%8.5f ccoef=%8.5f\n",
          f, bw, *acoef, *bcoef, *ccoef);
*/
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*              S U B R O U T I N E   S E T Z E R O A B C              */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert formant freqencies and bandwidth into
 *      anti-resonator difference equation constants */

void setzeroabc(int f,int bw,float *acoef,float *bcoef,float *ccoef) {

        float r;
        double arg,exp(),cos();

/*    First compute ordinary resonator coefficients */
/*    Let r  =  exp(-pi bw t) */

        arg = minus_pi_t * bw;
        r = exp(arg);

/*    Let c  =  -r**2 */

        *ccoef = -(r * r);

/*    Let b = r * 2*cos(2 pi f t) */

        arg = two_pi_t * f;
        *bcoef = r * cos(arg) * 2.;

/*    Let a = 1.0 - b - c */

        *acoef = 1. - *bcoef - *ccoef;

/*    Now convert to antiresonator coefficients (a'=1/a, b'=b/a, c'=c/a) */
        *acoef = 1.0 / *acoef;
        *ccoef *= -*acoef;
        *bcoef *= -*acoef;

/*    Debugging printout *
      printf("fz=%3d bw=%3d acoef=%8.5f bcoef=%8.5f ccoef=%8.5f\n",
          f, bw, *acoef, *bcoef, *ccoef);
*/
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*            S U B R O U T I N E   G E N - N O I S E                  */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Random number generator (return a number between -8191 and +8191) */

void gen_noise(void) {

	nrand = (rand()>>17) - 8192;
	
/*    Tilt down noise spectrum by soft low-pass filter having
 *    a pole near the origin in the z-plane, i.e.
 *    output = input + (0.75 * lastoutput) */

        noise = nrand + (0.75 * nlast);
        nlast = noise;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                   S U B R O U T I N E   D B t o L I N               */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*      Convert from decibels to a linear scale factor
 */

float DBtoLIN(int dB) {

/*    Check limits or argument (can be removed in final product) */
        if (dB < 0) {
            printf("Try to compute amptable[%d]\n", dB);
            return(0);
        }
        return(amptable[dB] * .001);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                   S U B R O U T I N E   R E S O N L P               */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Critically-damped Low-Pass Resonator of Impulsive Glottal Source */

void resonglot(void) {

	register long temp3,temp4;

	temp4 = rglc * rgl_2;          /*   (ccoef * old2)     */
	rgl_2 = rgl_1;

	temp3 = rglb * rgl_1;          /* + (bcoef * old1)     */
	temp4 += temp3;

	temp3 = rgla * vwave;          /* + (acoef * input)    */
	rgl_1 = temp4 + temp3;
}


/* Low-Pass Downsampling Resonator of Glottal Source */

void resonlp(void) {

	register long temp3,temp4;             /* this was register integer */

	temp4 = rlpc * rlp_2;          /*   (ccoef * old2)     */
	rlp_2 = rlp_1;
/*      printf(" rlpc = %8.5f rlp_2 = %8.5f\n",rlpc,rlp_2); */
	temp3 = rlpb * rlp_1;          /* + (bcoef * old1)     */
	temp4 += temp3;

	temp3 = rlpa * voice;          /* + (acoef * input)    */
	rlp_1 = temp4 + temp3;
	voice = rlp_1;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                S U B R O U T I N E   R E S O N C                    */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Nasal Antiresonator of Cascade Vocal Tract:
 *  Output = (rnza * input) + (rnzb * oldin1) + (rnzc * oldin2) */

void resoncnz(void) {

	register long temp3,temp4;

	temp4 = rnzc * rnz_2;          /*   (ccoef * oldin2)   */
	rnz_2 = rnz_1;

	temp3 = rnzb * rnz_1;          /* + (bcoef * oldin1)   */
	temp4 += temp3;

	temp3 = rnza * glotout;            /* + (acoef * input)    */
	rnz_1 = glotout;
	rnzout = temp4 + temp3;
}

/* Nasal Resonator of Cascade Vocal Tract */

void resoncnp(void) {

	register long temp3,temp4;

	temp4 = rnpcc * rnpc_2;        /*   (ccoef * old2)     */
	rnpc_2 = rnpc_1;

	temp3 = rnpcb * rnpc_1;          /* + (bcoef * old1)     */
	temp4 += temp3;

	temp3 = rnpca * rnzout;          /* + (acoef * input)    */
	rnpc_1 = temp4 + temp3;
}

/* Eighth cascaded Formant */

void resonc8(void) {

	register long temp3,temp4;

	temp4 = r8cc * r8c_2;          /*   (ccoef * old2)     */
	r8c_2 = r8c_1;

	temp3 = r8cb * r8c_1;          /* + (bcoef * old1)     */
	temp4 += temp3;

	temp3 = r8ca * casc_next_in;   /* + (acoef * input)    */
	r8c_1 = temp4 + temp3;
}

/* Seventh cascaded Formant */

void resonc7(void) {

        register long temp3,temp4;

        temp4 = r7cc * r7c_2;          /*   (ccoef * old2)     */
        r7c_2 = r7c_1;

        temp3 = r7cb * r7c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r7ca * casc_next_in;   /* + (acoef * input)    */
        r7c_1 = temp4 + temp3;
}

/* Sixth cascaded Formant */

void resonc6(void) {

        register  long temp3,temp4;


        temp4 = r6cc * r6c_2;          /*   (ccoef * old2)     */
        r6c_2 = r6c_1;

        temp3 = r6cb * r6c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r6ca * casc_next_in;   /* + (acoef * input)    */
        r6c_1 = temp4 + temp3;
}

/* Fifth Formant */

void resonc5(void) {

        register long temp3,temp4;

        temp4 = r5cc * r5c_2;          /*   (ccoef * old2)     */
        r5c_2 = r5c_1;

        temp3 = r5cb * r5c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r5ca * casc_next_in;   /* + (acoef * input)    */
        r5c_1 = temp4 + temp3;
}

/* Fourth Formant */

void resonc4(void) {

        register long temp3,temp4;

        temp4 = r4cc * r4c_2;          /*   (ccoef * old2)     */
        r4c_2 = r4c_1;

        temp3 = r4cb * r4c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r4ca * casc_next_in;   /* + (acoef * input)    */
        r4c_1 = temp4 + temp3;
}

/* Third Formant */

void resonc3(void) {

        register long temp3,temp4;

        temp4 = r3cc * r3c_2;          /*   (ccoef * old2)     */
        r3c_2 = r3c_1;

        temp3 = r3cb * r3c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r3ca * casc_next_in;   /* + (acoef * input)    */
        r3c_1 = temp4 + temp3;
}

/* Second Formant */

void resonc2(void) {

        register long temp3,temp4;

        temp4 = r2cc * r2c_2;          /*   (ccoef * old2)     */
        r2c_2 = r2c_1;

        temp3 = r2cb * r2c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r2ca * casc_next_in;   /* + (acoef * input)    */
        r2c_1 = temp4 + temp3;
}

/* First Formant of Cascade Vocal Tract */

void resonc1(void) {

        register long temp3,temp4;

        temp4 = r1cc * r1c_2;          /*   (ccoef * old2)     */
        r1c_2 = r1c_1;

        temp3 = r1cb * r1c_1;          /* + (bcoef * old1)     */
        temp4 += temp3;

        temp3 = r1ca * casc_next_in;   /* + (acoef * input)    */
        r1c_1 = temp4 + temp3;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                  S U B R O U T I N E   R E S O N P                  */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*   Output = (acoef * input) + (bcoef * old1) + (ccoef * old2); */

/* Sixth Formant of Parallel Vocal Tract */

void reson6p(void) {

        register long temp3,temp4;

        temp4 = r6pc * r6p_2;
        r6p_2 = r6p_1;

        temp3 = r6pb * r6p_1;
        temp4 += temp3;

        temp3 = r6pa * sourc;
        r6p_1 = temp4 + temp3;
}

/* Fifth Formant of Parallel Vocal Tract */

void reson5p(void) {

        register long temp3,temp4;

        temp4 = r5pc * r5p_2;
        r5p_2 = r5p_1;

        temp3 = r5pb * r5p_1;
        temp4 += temp3;

        temp3 = r5pa * sourc;
        r5p_1 = temp4 + temp3;
}

/* Fourth Formant of Parallel Vocal Tract */

void reson4p(void) {

        register long temp3,temp4;

        temp4 = r4pc * r4p_2;
        r4p_2 = r4p_1;

        temp3 = r4pb * r4p_1;
        temp4 += temp3;

        temp3 = r4pa * sourc;
        r4p_1 = temp4 + temp3;
}

/* Third Formant of Parallel Vocal Tract */

void reson3p(void) {

        register long temp3,temp4;

        temp4 = r3pc * r3p_2;
        r3p_2 = r3p_1;

        temp3 = r3pb * r3p_1;
        temp4 += temp3;

        temp3 = r3pa * sourc;
        r3p_1 = temp4 + temp3;
}

/* Second Formant of Parallel Vocal Tract */

void reson2p(void) {

        register long temp3,temp4;

        temp4 = r2pc * r2p_2;
        r2p_2 = r2p_1;

        temp3 = r2pb * r2p_1;
        temp4 += temp3;

        temp3 = r2pa * sourc;
        r2p_1 = temp4 + temp3;
}


/* First Formant of Parallel Vocal Tract */

void reson1p(void) {

        register long temp3,temp4;

        temp4 = r1pc * r1p_2;
        r1p_2 = r1p_1;

        temp3 = r1pb * r1p_1;
        temp4 += temp3;

        temp3 = r1pa * sourc;
        r1p_1 = temp4 + temp3;
}

/* Nasal Formant of Parallel Vocal Tract */

void resonnpp(void) {

        register long temp3,temp4;

        temp4 = rnppc * rnpp_2;
        rnpp_2 = rnpp_1;

        temp3 = rnppb * rnpp_1;
        temp4 += temp3;

        temp3 = rnppa * sourc;
        rnpp_1 = temp4 + temp3;
}




/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*               S U B R O U T I N E   N O - R A D - C H A R           */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define ACOEF           0.005
#define BCOEF           (1.0 - ACOEF)   /* Slight decay to remove dc */

void no_rad_char(float in) {

        static float lastin;

        out = (ACOEF * in) + (BCOEF * lastin);
        lastin = out;
        out = -100. * out;      /* Scale up to make visible */
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                  S U B R O U T I N E   G E T M A X                  */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Find absolute maximum of arg1 & arg2, save in arg2 */

void getmax(long arg1,long *arg2) {

	if (arg1 < 0) arg1 = - arg1;

	if (arg1 > *arg2) *arg2 = arg1;
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                S U B R O U T I N E   P R - P A R S                  */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void pr_pars(void) {

        int m4;

	m4 = 0;
	printf("\n  Speaker-defining Constants:\n");
	for (m=0; m<N_SPDEF_PARS; m++) {
	    printf("    %s %5d\t", spdef_name[m], spkrdef[m]);
	    if (++m4 >= 4) {
		m4 = 0;
		printf("\n");
	    }
	}
	if (m4 != 0) printf("\n");

	m4 = 0;
	printf("  Par values for this frame:\n");
	for (m=0; m<N_VARIABLE_PARS; m++) {
	    printf("    %s %5d\t", par_name[m], pars[m]);
	    if (++m4 >= 4) {
		m4 = 0;
		printf("\n");
	    }
	}
	if (m4 != 0) printf("\n");
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                 S U B R O U T I N E   T R U N C A T E               */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Truncate arg to fit into 16-bit word */

short parwv_truncate(long arg) {

	short iarg;               /* 16 bit size argument */

        if (arg < -32768) {
            overload_warning(-arg);
            arg = -32768;
        }
        if (arg >  32767) {
            overload_warning(arg);
            arg =  32767;
        }
	iarg = (short) arg;       /* from long to short int */

        return(iarg);
}

void overload_warning(long arg) {

    static int warnsw;
    extern float dBconvert();

    if (warnsw == 0) {
        warnsw++;
        printf("\n* * * WARNING: ");
        printf(" Signal at output of synthesizer (+%3.1f dB) exceeds 0 dB\n", dBconvert(arg));
        printf("    at time = %d\n", 1000*disptcum/samrate);
        printf(" Output waveform will be TRUNCATED\n");

        pr_pars();
    }
}

float dBconvert(long arg) {

        double x,log10();
        float db;

        x = arg / 32767.;
        x = log10(x);
        db = 20. * x;
        return(db);
}

/* for synthesizer restart (after adjusting g0) it is necessary to initialize
   these static variables */
void init_static_vars()
{
    nper=0;  /* int */
    skew=0;  /* int */
    F1last=0;  /* int */
    F2last=0;  /* int */
    F3last=0;  /* int */
    a=0.0; b=0.0;
    vwave=0.0;
    rnpp_1=0.0;
    rnpp_2=0.0;
    r1p_1=0.0;
    r1p_2=0.0;
    r2p_1=0.0;
    r2p_2=0.0;
    r3p_1=0.0;
    r3p_2=0.0;
    r4p_1=0.0;
    r4p_2=0.0;
    r5p_1=0.0;
    r5p_2=0.0;
    r6p_1=0.0;
    r6p_2=0.0;
    r1c_1=0.0;
    r1c_2=0.0;
    r2c_1=0.0;
    r2c_2=0.0;
    r3c_1=0.0;
    r3c_2=0.0;
    r4c_1=0.0;
    r4c_2=0.0;
    r5c_1=0.0;
    r5c_2=0.0;
    r6c_1=0.0;
    r6c_2=0.0;
    rnpc_1=0.0;
    rnpc_2=0.0;
    rnz_1=0.0;
    rnz_2=0.0;
    rgl_1=0.0;
    rgl_2=0.0;
    rlp_1=0.0;
    rlp_2=0.0;
    vlast=0.0;
    nlast=0.0;
    glotlast=0.0;
    anorm1=0.0;
    anorm2=0.0;
    anorm3=0.0;
}






