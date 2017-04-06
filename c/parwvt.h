/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                     */
/*                    P A R W A V T A B . H                            */
/*                                                                     */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This data file is included in PARWAVE.C */

#ifndef PARWVT_H
#define PARWVT_H 1

/*
  A B0 macro is defined in termios.h and can interfere with the variable
  of the same name in this file. The termios B0 macro is related to
  baud rate of serial modems. We aren't likely to need this definition
  and it should be safe to undef in the case that termios is included
  somewhere in the compilation toolchain before this file.
*/
#ifdef B0
#undef B0
#endif

/* VARIABLES TO HOLD SPEAKER DEFINITION FROM HOST:                    */

static int outsl;   /* Output waveform selector			      */
static int samrate; /* Number of output samples per second	      */
static int ranseed; /* Seed specifying initial number for ran # gener */
static int FLPhz ;  /* Frequeny of glottal downsample low-pass filter */
static int BLPhz ;  /* Bandwidt of glottal downsample low-pass filter */
static int nfcascade; /* Number of formants in cascade vocal tract    */
static int glsource;  /* 1->impulsive, 2->natural voicing source      */

/* VARIABLES TO HOLD INPUT PARAMETERS FROM HOST:                      */

static int F0hz;  /* Voicing fund freq in Hz	       	      */
static int F1hz  ;  /*  First formant freq in Hz,  200 to 1300      */
static int dF1hz ;  /*  Increment to F1hz during open phase of cycle*/
static int F2hz  ;  /* Second formant freq in Hz,  550 to 3000      */
static int F3hz  ;  /*  Third formant freq in Hz, 1200 to 4999      */
static int F4hz  ;  /* Fourth formant freq in Hz, 1200 to 4999      */
static int F5hz  ;  /*  Fifth formant freq in Hz, 1200 to 4999      */
static int F6hz  ;  /*  Sixth formant freq in Hz, 1200 to 4999      */
static int FNZhz ;  /*     Nasal zero freq in Hz,  248 to  528      */
static int FNPhz ;  /*     Nasal pole freq in Hz,  248 to  528      */

static int B1hz  ;  /*    First formant bw in Hz,   40 to 1000      */
static int dB1hz ;  /*  Increment to B1hz during open phase of cycle*/
static int B2hz  ;  /*   Second formant bw in Hz,   40 to 1000      */
static int B3hz  ;  /*    Third formant bw in Hz,   40 to 1000      */
static int B4hz  ;  /*    Fourth formant bw in Hz,  40 to 1000      */
static int B5hz  ;  /*    Fifth formant bw in Hz,   40 to 1000      */
static int B6hz  ;  /*    Sixth formant bw in Hz,   40 to 2000      */
static int B1phz ;  /* Par. 1st formant bw in Hz,   40 to 1000      */
static int B2phz ;  /* Par. 2nd formant bw in Hz,   40 to 1000      */
static int B3phz ;  /* Par. 3rd formant bw in Hz,   40 to 1000      */
static int B4phz ;  /*  Par. 4th formant bw in Hz,  40 to 1000      */
static int B5phz ;  /* Par. 5th formant bw in Hz,   40 to 1000      */
static int B6phz ;  /* Par. 6th formant bw in Hz,   40 to 2000      */
static int BNZhz ;  /*       Nasal zero bw in Hz,   40 to 1000      */
static int BNPhz ;  /*       Nasal pole bw in Hz,   40 to 1000      */

static int AVdb  ;  /*      Amp of voicing in dB,    0 to   70      */
static int AVpdb ;  /* Amp of voicing, par in dB,    0 to   70      */
static int AP    ;  /*   Amp of aspiration in dB,    0 to   70      */
static int AF    ;  /*    Amp of frication in dB,    0 to   80      */
static int A1    ;  /* Amp of par 1st formant in dB, 0 to   80      */
static int ANP   ;  /* Amp of par nasal pole in dB,  0 to   80      */
static int A2    ;  /* Amp of F2 frication in dB,    0 to   80      */
static int A3    ;  /* Amp of F3 frication in dB,    0 to   80      */
static int A4    ;  /* Amp of F4 frication in dB,    0 to   80      */
static int A5    ;  /* Amp of F5 frication in dB,    0 to   80      */
static int A6    ;  /* Amp of F6 (same as r6pa),     0 to   80      */
static int AB    ;  /* Amp of bypass fric. in dB,    0 to   80      */
static int TLTdb ;  /* Voicing spectral tilt in dB,  0 to   24      */
static int Kopen ;  /* # of samples in open period, 10 to   65      */
static int Aturb ;  /* Breathiness in voicing,       0 to   80      */
static int Kskew ;  /* Skewness of alternate periods,0 to   40      */
                    /* in sample#/2                                 */
static int Gain0 ;  /* Overall gain, 60 dB is unity  0 to   60	    */

/* SAME VARIABLES CONVERTED TO LINEAR FLOATING POINT */
static float amp_parF1;	/* A1 converted to linear gain		    */
static float amp_parFNP; /* AP converted to linear gain		    */
static float amp_parF2;	/* A2 converted to linear gain		    */
static float amp_parF3;	/* A3 converted to linear gain		    */
static float amp_parF4;	/* A4 converted to linear gain		    */
static float amp_parF5;	/* A5 converted to linear gain		    */
static float amp_parF6;	/* A6 converted to linear gain		    */
static float amp_bypas;	/* AB converted to linear gain		    */
static float amp_voice;	/* AVdb converted to linear gain	    */
static float par_amp_voice; /* AVpdb converted to linear gain	    */
static float amp_aspir;	/* AP converted to linear gain		    */
static float amp_frica;	/* AF converted to linear gain		    */
static float amp_breth;	/* ATURB converted to linear gain	    */
static float amp_gain0;	/* G0 converted to linear gain		    */

/* COUNTERS */

static int ns    ;  /* Number of samples into current frame         */
static int nper  ;  /* Current loc in voicing period   40000 samp/s */
static int n4    ;  /* Counter of 4 samples in glottal source loop  */

/* COUNTER LIMITS */

static int T0    ;  /* Fundamental period in output samples times 4 */
static int nopen ;  /* Number of samples in open phase of period    */
static int nmod  ;  /* Position in period to begin noise amp. modul */
static int nspfr ;  /* Number of samples in a parameter frame       */

/* ALL-PURPOSE TEMPORARY VARIABLES */

static int temp    ;
static long ltemp  ;
static float temp1 ;

/* VARIABLES THAT HAVE TO STICK AROUND FOR AWHILE, AND THUS "temp" */
/* IS NOT APPROPRIATE */

static int nrand   ;  /* Varible used by random number generator      */
static int skew    ;  /* Alternating jitter, in half-period units     */

static float a     ;  /* Makes waveshape of glottal pulse when open   */
static float b     ;  /* Makes waveshape of glottal pulse when open   */
static float voice ;  /* Current sample of voicing waveform           */
static float vwave ;  /* Ditto, but before multiplication by AVdb     */
static float noise ;  /* Output of random number generator            */
static float frics ;  /* Frication sound source                       */
static float aspiration; /* Aspiration sound source                   */
static float sourc ;  /* Sound source if all-parallel config used     */
static float casc_next_in;  /* Input to next used resonator of casc   */
static float out   ;  /* Output of cascade branch, also final output  */
static float rnzout;  /* Output of cascade nazal zero resonator	      */
static float glotout; /* Output of glottal sound source		      */
static float par_glotout; /* Output of parallelglottal sound sourc    */
static float outbypas; /* Output signal from bypass path	      */

/* INTERNAL MEMORY FOR DIGITAL RESONATORS AND ANTIRESONATOR           */

static float rnpp_1;  /* Last output sample from parallel nasal pole  */
static float rnpp_2;  /* Second-previous output sample                */

static float r1p_1 ;  /* Last output sample from parallel 1st formant */
static float r1p_2 ;  /* Second-previous output sample                */

static float r2p_1 ;  /* Last output sample from parallel 2nd formant */
static float r2p_2 ;  /* Second-previous output sample                */

static float r3p_1 ;  /* Last output sample from parallel 3rd formant */
static float r3p_2 ;  /* Second-previous output sample                */

static float r4p_1 ;  /* Last output sample from parallel 4th formant */
static float r4p_2 ;  /* Second-previous output sample                */

static float r5p_1 ;  /* Last output sample from parallel 5th formant */
static float r5p_2 ;  /* Second-previous output sample                */

static float r6p_1 ;  /* Last output sample from parallel 6th formant */
static float r6p_2 ;  /* Second-previous output sample                */

static float r1c_1 ;  /* Last output sample from cascade 1st formant  */
static float r1c_2 ;  /* Second-previous output sample                */

static float r2c_1 ;  /* Last output sample from cascade 2nd formant  */
static float r2c_2 ;  /* Second-previous output sample                */

static float r3c_1 ;  /* Last output sample from cascade 3rd formant  */
static float r3c_2 ;  /* Second-previous output sample                */

static float r4c_1 ;  /* Last output sample from cascade 4th formant  */
static float r4c_2 ;  /* Second-previous output sample                */

static float r5c_1 ;  /* Last output sample from cascade 5th formant  */
static float r5c_2 ;  /* Second-previous output sample                */

static float r6c_1 ;  /* Last output sample from cascade 6th formant  */
static float r6c_2 ;  /* Second-previous output sample                */

static float rnpc_1;  /* Last output sample from cascade nasal pole   */
static float rnpc_2;  /* Second-previous output sample                */

static float rnz_1 ;  /* Last output sample from cascade nasal zero   */
static float rnz_2 ;  /* Second-previous output sample                */

static float rgl_1 ;  /* Last output crit-damped glot low-pass filter */
static float rgl_2 ;  /* Second-previous output sample                */

static float rlp_1 ;  /* Last output from downsamp low-pass filter    */
static float rlp_2 ;  /* Second-previous output sample                */

static float vlast ;  /* Previous output of voice                     */
static float nlast ;  /* Previous output of random number generator   */
static float glotlast; /* Previous value of glotout                   */

/* COEFFICIENTS FOR DIGITAL RESONATORS AND ANTIRESONATOR */

static float rnppa ;   /* "a" coefficient for parallel nasal pole     */
static float rnppb ;   /* "b" coefficient                             */
static float rnppc ;   /* "c" coefficient                             */

static float r1pa  ;  /* "a" coef for par. 1st formant                */
static float r1pb  ;  /* "b" coefficient                              */
static float r1pc  ;  /* "c" coefficient                              */

static float r2pa  ;  /* Could be same as A2 if all integer impl.     */
static float r2pb  ;  /* "b" coefficient                              */
static float r2pc  ;  /* "c" coefficient                              */

static float r3pa  ;  /* Could be same as A3 if all integer impl.     */
static float r3pb  ;  /* "b" coefficient                              */
static float r3pc  ;  /* "c" coefficient                              */

static float r4pa  ;  /* Could be same as A4 if all integer impl.     */
static float r4pb  ;  /* "b" coefficient                              */
static float r4pc  ;  /* "c" coefficient                              */

static float r5pa  ;  /* Could be same as A5 if all integer impl.     */
static float r5pb  ;  /* "b" coefficient                              */
static float r5pc  ;  /* "c" coefficient                              */

static float r6pa  ;  /* Could be same as A6 if all integer impl.     */
static float r6pb  ;  /* "b" coefficient                              */
static float r6pc  ;  /* "c" coefficient                              */

static float r1ca  ;  /* Could be same as r1pa if all integer impl.   */
static float r1cb  ;  /* Could be same as r1pb if all integer impl.   */
static float r1cc  ;  /* Could be same as r1pc if all integer impl.   */

static float r2ca  ;   /* "a" coefficient for cascade 2nd formant     */
static float r2cb  ;   /* "b" coefficient                             */
static float r2cc  ;   /* "c" coefficient                             */

static float r3ca  ;   /* "a" coefficient for cascade 3rd formant     */
static float r3cb  ;   /* "b" coefficient                             */
static float r3cc  ;   /* "c" coefficient                             */

static float r4ca  ;   /* "a" coefficient for cascade 4th formant     */
static float r4cb  ;   /* "b" coefficient                             */
static float r4cc  ;   /* "c" coefficient (same as R4Cccoef)          */

static float r5ca  ;   /* "a" coefficient for cascade 5th formant     */
static float r5cb  ;   /* "b" coefficient                             */
static float r5cc  ;   /* "c" coefficient (same as R5Cccoef)          */

static float r6ca  ;   /* "a" coefficient for cascade 5th formant     */
static float r6cb  ;   /* "b" coefficient                             */
static float r6cc  ;   /* "c" coefficient (same as R5Cccoef)          */

static float rnpca ;   /* "a" coefficient for cascade nasal pole      */
static float rnpcb ;   /* "b" coefficient                             */
static float rnpcc ;   /* "c" coefficient                             */

static float rnza  ;   /* "a" coefficient for cascade nasal zero      */
static float rnzb  ;   /* "b" coefficient                             */
static float rnzc  ;   /* "c" coefficient                             */

static float rgla  ;   /* "a" coefficient for crit-damp glot filter   */
static float rglb  ;   /* "b" coefficient                             */
static float rglc  ;   /* "c" coefficient                             */

static float rlpa  ;   /* "a" coefficient for downsam low-pass filter */
static float rlpb  ;   /* "b" coefficient                             */
static float rlpc  ;   /* "c" coefficient                             */

static float decay ;   /* TLTdb converted to exponential time const   */
static float onemd ;   /* in voicing one-pole low-pass filter         */

/* CONSTANTS AND TABLES TO BE PUT IN ROM                              */

#define CASCADE_PARALLEL      1 /* Normal synthesizer config          */
#define ALL_PARALLEL          2 /* Only use parallel branch           */


/*
 * Constant B0 controls shape of glottal pulse as a function
 * of desired duration of open phase N0
 * (Note that N0 is specified in terms of 40,000 samples/sec of speech)
 *
 *    Assume voicing waveform V(t) has form: k1 t**2 - k2 t**3
 *
 *    If the radiation characterivative, a temporal derivative
 *      is folded in, and we go from continuous time to discrete
 *      integers n:  dV/dt = vwave[n]
 *                         = sum over i=1,2,...,n of { a - (i * b) }
 *                         = a n  -  b/2 n**2
 *
 *      where the  constants a and b control the detailed shape
 *      and amplitude of the voicing waveform over the open
 *      potion of the voicing cycle "nopen".
 *
 *    Let integral of dV/dt have no net dc flow --> a = (b * nopen) / 3
 *
 *    Let maximum of dUg(n)/dn be constant --> b = gain / (nopen * nopen)
 *      meaning as nopen gets bigger, V has bigger peak proportional to n
 *
 *    Thus, to generate the table below for 40 <= nopen <= 263:
 *
 *      B0[nopen - 40] = 1920000 / (nopen * nopen)
 */
short B0[224] = {
        1200,   1142,   1088,   1038,   991,
        948,    907,    869,    833,    799,
        768,    738,    710,    683,    658,
        634,    612,    590,    570,    551,
        533,    515,    499,    483,    468,
        454,    440,    427,    415,    403,
        391,    380,    370,    360,    350,
        341,    332,    323,    315,    307,
        300,    292,    285,    278,    272,
        265,    259,    253,    247,    242,
        237,    231,    226,    221,    217,
        212,    208,    204,    199,    195,
        192,    188,    184,    180,    177,
        174,    170,    167,    164,    161,
        158,    155,    153,    150,    147,
        145,    142,    140,    137,    135,
        133,    131,    128,    126,    124,
        122,    120,    119,    117,    115,
        113,    111,    110,    108,    106,
        105,    103,    102,    100,    99,
        97,     96,     95,     93,     92,
        91,     90,     88,     87,     86,
        85,     84,     83,     82,     80,
        79,     78,     77,     76,     75,
        75,     74,     73,     72,     71,
        70,     69,     68,     68,     67,
        66,     65,     64,     64,     63,
        62,     61,     61,     60,     59,
        59,     58,     57,     57,     56,
        56,     55,     55,     54,     54,
        53,     53,     52,     52,     51,
        51,     50,     50,     49,     49,
        48,     48,     47,     47,     46,
        46,     45,     45,     44,     44,
        43,     43,     42,     42,     41,
        41,     41,     41,     40,     40,
        39,     39,     38,     38,     38,
        38,     37,     37,     36,     36,
        36,     36,     35,     35,     35,
        35,     34,     34,     33,     33,
        33,     33,     32,     32,     32,
        32,     31,     31,     31,     31,
        30,     30,     30,     30,     29,
        29,     29,     29,     28,     28,
        28,     28,     27,     27
};

/*
 * Convertion table, db to linear, 87 dB --> 32767
 *                                 86 dB --> 29491 (1 dB down = 0.5**1/6)
 *                                 ...
 *                                 81 dB --> 16384 (6 dB down = 0.5)
 *                                 ...
 *                                  0 dB -->     0
 *
 * The just noticeable difference for a change in intensity of a vowel
 *   is approximately 1 dB.  Thus all amplitudes are quantized to 1 dB
 *   steps.
 */

float amptable[88] = {
        0.,     0.,     0.,     0.,     0.,
	0.,     0.,     0.,     0.,     0.,
	0.,     0.,     0.,     6.,     7.,
	8.,     9.,    10.,    11.,    13.,
	14.,    16.,    18.,    20.,    22.,
	25.,    28.,    32.,    35.,    40.,
	45.,    51.,    57.,    64.,    71.,
	80.,    90.,   101.,   114.,   128.,
	142.,   159.,   179.,   202.,   227.,
	256.,   284.,   318.,   359.,   405.,
	455.,   512.,   568.,   638.,   719.,
	811.,   911.,  1024.,  1137.,  1276.,
	1438.,  1622.,  1823.,  2048.,  2273.,
	2552.,  2875.,  3244.,  3645.,  4096.,
	4547.,  5104.,  5751.,  6488.,  7291.,
	8192.,  9093., 10207., 11502., 12976.,
	14582., 16384., 18350., 20644., 23429.,
	26214., 29491., 32767.
};

char *spdef_name[] = {
	"OUTs",
	"Srat",
	"NSfr",
	"Flp ",
	"BWlp",
	"RANs",
	"NFca",
	"VSsw"
};

char *par_name[] = {
	"F0  ",
	"AV  ",
	"F1  ",
	"BW1 ",
	"F2  ",
	"BW2 ",
	"F3  ",
	"BW3 ",
	"F4  ",
	"BW4 ",
	"F5  ",
	"BW5 ",
	"F6  ",
	"BW6 ",
	"Fnz ",
	"BWnz",
	"Fnp ",
	"BWnp",
	"Aasp",
	"Nopn",
	"Atur",
	"tilt",
	"Afrc",
	"skew",
	"A1  ",
	"BWp1",
	"A2  ",
	"BWp2",
	"A3  ",
	"BWp3",
	"A4  ",
	"BWp4",
	"A5  ",
	"BWp5",
	"A6  ",
	"BWp6",
	"AN  ",
	"AB  ",
	"AVpa",
	"G0  "
};
/* function prototypes for use within parwav.c */
void gen_noise(void);
void gethost(void);
float impulsive_source(void);
void setR1(int ,int );
float DBtoLIN(int );
float natural_source(void);
float triangular_source(void);
float square_source(void);
void pitch_synch_par_reset(void);
void resonlp(void);
void resoncnz(void);
void resoncnp(void);
void resonc8(void);
void resonc7(void);
void resonc6(void);
void resonc5(void);
void resonc4(void);
void resonc3(void);
void resonc2(void);
void resonc1(void);
void no_rad_char(float );
void reson1p(void);
void reson2p(void);
void reson3p(void);
void reson4p(void);
void reson5p(void);
void reson6p(void);
void resonnpp(void);
void getmax(long ,long *);
short parwv_truncate(long );
void setabc(int ,int ,float *,float *,float *);
void setzeroabc(int ,int ,float *,float *,float *);
void resonglot(void);
void overload_warning(long );
float dBconvert(long );
void pr_pars(void);

#endif /* PARWVT_H */
