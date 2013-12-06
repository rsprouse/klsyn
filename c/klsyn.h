/*	K L S Y N . H			D. H. Klatt			*/

#ifndef KLSYN_H
#define KLSYN_H 1

#include <stdint.h>

#define NPAR             49
#define MAX_VAR_PARS     49
#define FIXED           'C'
#define VARIABLE        'v'
#define VARRIED         'V'
#define NOVICE          'n'
#define MYTRUE            't'
#define MAX_FRAMES      200

#define OUTSELECT	  spkrdef[0]
#define SAMRAT		  spkrdef[1]
#define NSAMP_PER_FRAME   spkrdef[2]     /* Number of samples per frame */
#define RANSEED		  spkrdef[3]
#define NFCASC		  spkrdef[4]
#define SOURCE_SELECT	  spkrdef[5]

/* First char of 2-char symbol for parameter */
char symb1[NPAR] = { 's','n','d','s','u','r','f','a',
    'F','b','F','b','F','b','F','b','F','b','f','b','f','b','f','b',
    'a','o','a','t','a','s',
    'a','p','a','p','a','p','a','p','a','p','a','p',
    'a','a','a','o','g','d','d'};

/* 2nd char of 2-char symbol for parameter */
char symb2[NPAR] = { 'r','f','u','s','i','s','0','v','1','1','2','2',
	'3','3','4','4','5','5','6','6','z','z','p','p','h','q','t','l','f',
	'k','1','1','2','2','3','3','4','4','5','5','6','6','n','b',
    'p','s','0','F','b'};

/* Constant/variable switch for each param */
char cv[NPAR] = { 'C','C','C','C','C','C','v','v','v','v','v','v','v',
	'v','v','v','v','v','v','v','v','v','v','v','v','v','v','v','v','v',
	'v','v','v','v','v','v','v','v','v','v','v','v','v','v','v','C','v','v','v'};

/* Maximaum values for all parameters */
int maxval[NPAR] = { 22050, 8, 5000, 2, 20, 99, 500, 80, 1300, 1000, 3000, 1000, 
	4800, 1000, 4990, 1000, 4990, 1500, 4990, 4000, 800, 1000, 500, 1000, 80, 80, 
	80, 34, 80, 100, 80, 1000, 80, 1000, 80, 1000, 80, 1000, 80, 1500, 80, 
	4000, 80, 80, 80, 20, 80, 100, 400};

/* Minimum values for all parameters */
int minval[NPAR] = { 5000, 1, 30, 1, 1, 1, 50, 0, 180, 30, 550, 40, 1200, 60, 2400, 
	100, 3000, 100, 3000, 100, 180, 40, 180, 40, 0, 10, 0, 0, 0, 0, 0, 30, 0, 40, 
	0, 60, 0, 100, 0, 100, 0, 100, 0, 0, 0, 0, 0, 0, 0 };

/* Default values for all parameters */
static int cdefval[NPAR] = { 11025, 5, 500, 2, 5, 1, 100, 60, 350, 60, 850, 70, 2500,
	150, 3250, 200, 3700, 200, 4990, 500, 280, 90, 280, 90, 0, 50, 0, 0, 0, 0, 0, 80, 
	0, 200, 0, 350, 0, 500, 0, 600, 0, 800, 0, 0, 0, 0, 60, 0, 0 };


void helpa(void);
void makefilenames(char *);
void read_doc(char *);
void readconfig(void);
void clearpar(int np);
void helpr(void);
char get_request(void);
void actonrequest(char ,int);
void putconfig(FILE *,int);
int get_par_name(void);
void settypicalpar(int);
void drawparam(void);
void synthesize(int );
int find_config_loc(char *);
extern void init_static_vars(void);
void print_outmax(FILE *, int);
void writwave(char *,short *,int ,int );
void prpars(FILE *,int);
int get_digits(FILE *);
void setlimits(int);
int get_value(int );
void initpars(void);
void gettimval(int *,int *);
void fill_frames(int ,int ,int ,int ,int );
int get_time(void);
int get_value(int );
void namefile(void);
extern void parwav(int16_t *);
int getpval(void);
int checklimits(void);

#endif /* KLSYN_H */
