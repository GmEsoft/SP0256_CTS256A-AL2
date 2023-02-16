/*
 * ============================================================================
 *  Title:    Intellivoice Emulation
 *  Author:   J. Zbiciak
 * ============================================================================
 *  This module actually attempts to emulate the Intellivoice.  Wild!
 * ============================================================================
 *  The Intellivoice is mapped into two locations in memory, $0080-$0081.
 *  (This ignores the separate 8-bit bus that the SPB-640 provides, since
 *  nothing uses it and I haven't emulated it.)
 *
 *  Location $0080 provides an interface to the "Address LoaD" (ALD)
 *  mechanism on the SP0256.  Reads from this address return the current
 *  "Load ReQuest" (LRQ) state in bit 15.  When LRQ is 1 (ie. bit 15 of
 *  location $0080 reads as 1), the SP0256 is ready to receive a new command.
 *  A new command address may then be written to location $0080 to trigger
 *  the playback of a sound.  Note that command address register is actually
 *  a 1-deep FIFO, and so LRQ will go to 1 before the SP0256 is finished
 *  speaking.
 *
 *  Location $0081 provides an interface to the SPB-640's 64-decle speech
 *  FIFO.  Reads from this address return the "FIFO full" state in bit 15.
 *  When bit 15 reads as 0, the FIFO has room for at least 1 more decle.
 *  Writes to this address can either clear the FIFO, or provide new data
 *  to the FIFO.  To clear the FIFO, write a value with Bit 10 == 1.
 *  To put a decle into the FIFO, write a value with Bit 10 == 0.  It's
 *  currently unknown what happens when a program attempts to write to the
 *  FIFO when the FIFO is full.  This emulation drops the extra data.
 *
 *  The exact format of the SP0256 speech data, as well as the overall
 *  system view from the SP0256's perspective is documented elsewhere.
 * ============================================================================
 */

//#define SINGLE_STEP

#define jzp_printf printf
#define jzp_flush()

#undef DEBUG_FIFO
#ifdef DEBUG_FIFO
#define dfprintf(x) jzp_printf x ; jzp_flush()
#else
#define dfprintf(x)
#endif

static int s_debugSample = 0;

#if 1
#define dsprintf(x) if( s_debugSample ) { jzp_printf x ; jzp_flush(); }
#else
#undef DEBUG_SAMPLE
//#define DEBUG_SAMPLE
#ifdef DEBUG_SAMPLE
#define dsprintf(x) jzp_printf x ; jzp_flush()
#else
#define dsprintf(x)
#endif
#endif

static int s_debug = 0;

#if 1
#define jzdprintf(x) if( s_debug ) { jzp_printf x ; jzp_flush(); }
#else
#undef DEBUG
#define DEBUG
#ifdef DEBUG
#define jzdprintf(x) jzp_printf x ; jzp_flush()
#else
#define jzdprintf(x)
#endif
#endif

static int s_debugSingleStep = 0;

#define PER_PAUSE    (64)               /* Equiv timing period for pauses.  */
#define PER_NOISE    (64)               /* Equiv timing period for noise.   */

#define FIFO_ADDR    (0x1800 << 3)      /* SP0256 address of speech FIFO.   */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sp0256.h"

#define CONDFREE(p)  if (p) free(p)

ivoice_t intellivoice;

static int fifoEnabled = 0;
static long nSample = 0;
static int s_nLabels = 0;
static const char* *s_labels = 0;

static const char* opcodes[] = {
    "RTS/SETPAGE  Return/Set Page",
    "SETMODE      Set the Mode and Repeat MSBs",
    "LOAD_4       Load Pitch, Ampl, Coeffs (2 or 3 stages)",
    "LOAD_C       Load Pitch, Ampl, Coeffs (5 or 6 stages)",
    "LOAD_2       Load Per, Ampl, Coefs, Interp",
    "SETMSB_A     Load Ampl and MSBs of 3 Coeffs",
    "SETMSB_6     Load Ampl, and Coeff MSBs",
    "LOAD_E       Load Pitch, Amplitude",
	"LOADALL      Load All Parameters",
    "DELTA_9      Delta update Ampl, Pitch, Coeffs (5 or 6 stages)",
    "SETMSB_5     Load Pitch, Ampl, and Coeff MSBs",
    "DELTA_D      Delta update Ampl, Pitch, Coeffs (2 or 3 stages)",
	"SETMSB_3     Load Pitch, Ampl, MSBs & Interp",
    "JSR          Jump to Subroutine",
    "JMP          Jump to 12-bit/16-bit Abs Addr",
    "PAUSE        Silent pause",
};

/* ======================================================================== */
/*  Internal function prototypes.                                           */
/* ======================================================================== */
static INLINE int16_t  limit (int16_t s);
static INLINE uint32_t bitrev(uint32_t val);
static int             lpc12_update(lpc12_t *f, int, int16_t *, uint32_t *);
static void            lpc12_regdec(lpc12_t *f);
static uint32_t        sp0256_getb(ivoice_t *ivoice, int len);
static void            sp0256_micro(ivoice_t *iv);

/* ======================================================================== */
/*  IVOICE_QTBL  -- Coefficient Quantization Table.  This comes from a      */
/*                  SP0250 data sheet, and should be correct for SP0256.    */
/* ======================================================================== */
static const int16_t qtbl[128] =
{
    0,      9,      17,     25,     33,     41,     49,     57,
    65,     73,     81,     89,     97,     105,    113,    121,
    129,    137,    145,    153,    161,    169,    177,    185,
    193,    201,    209,    217,    225,    233,    241,    249,
    257,    265,    273,    281,    289,    297,    301,    305,
    309,    313,    317,    321,    325,    329,    333,    337,
    341,    345,    349,    353,    357,    361,    365,    369,
    373,    377,    381,    385,    389,    393,    397,    401,
    405,    409,    413,    417,    421,    425,    427,    429,
    431,    433,    435,    437,    439,    441,    443,    445,
    447,    449,    451,    453,    455,    457,    459,    461,
    463,    465,    467,    469,    471,    473,    475,    477,
    479,    481,    482,    483,    484,    485,    486,    487,
    488,    489,    490,    491,    492,    493,    494,    495,
    496,    497,    498,    499,    500,    501,    502,    503,
    504,    505,    506,    507,    508,    509,    510,    511
};

/* ======================================================================== */
/*  LIMIT            -- Limiter function for digital sample output.         */
/* ======================================================================== */
static INLINE int16_t limit(int16_t s)
{
    if (s >  127) return  127;
    if (s < -128) return -128;
    return s;
}

/* ======================================================================== */
/*  AMP_DECODE       -- Decode amplitude register                           */
/* ======================================================================== */
static int amp_decode(uint8_t a)
{
    /* -------------------------------------------------------------------- */
    /*  Amplitude has 3 bits of exponent and 5 bits of mantissa.  This      */
    /*  contradicts USP 4,296,269 but matches the SP0250 Apps Manual.       */
    /* -------------------------------------------------------------------- */
    int expn = (a & 0xE0) >> 5;
    int mant = (a & 0x1F);
    int ampl = mant << expn;

    /* -------------------------------------------------------------------- */
    /*  Careful reading of USP 4,296,279, around line 60 in column 14 on    */
    /*  page 16 of the scan suggests the LSB might be held and injected     */
    /*  into the output while the exponent gets counted down, although      */
    /*  this seems dubious.                                                 */
    /* -------------------------------------------------------------------- */

    return ampl;
}

/* ======================================================================== */
/*  LPC12_UPDATE     -- Update the 12-pole filter, outputting samples.      */
/* ======================================================================== */
static int lpc12_update(lpc12_t *f, int num_samp, int16_t *out, uint32_t *optr)
{
    int i, j;
    int16_t samp;
    int do_int, bit;
    int oidx = *optr;

    /* -------------------------------------------------------------------- */
    /*  Iterate up to the desired number of samples.  We actually may       */
    /*  break out early if our repeat count expires.                        */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < num_samp; i++)
    {
        /* ---------------------------------------------------------------- */
        /*  Generate a series of periodic impulses, or random noise.        */
        /* ---------------------------------------------------------------- */
        do_int = 0;
        samp   = 0;
        bit    = f->rng & 1;
        f->rng = (f->rng >> 1) ^ (bit ? 0x4001 : 0);

        if (f->cnt <= 0)
        {
            if (f->rpt <= 0)      /* Stop if we expire the repeat counter */
            {
                f->cnt = f->rpt = 0;
                break;
            }

			--f->rpt;

            f->cnt = f->per ? f->per : PER_NOISE;
            samp   = f->amp;
            do_int = f->interp;
        }
		
		--f->cnt;

        if (!f->per)
            samp   = bit ? -f->amp : f->amp;

        /* ---------------------------------------------------------------- */
        /*  If we need to, process the interpolation registers.             */
        /* ---------------------------------------------------------------- */
        if (do_int)
        {
            f->r[0] += f->r[14];
            f->r[1] += f->r[15];

            f->amp   = amp_decode(f->r[0]);
            f->per   = f->r[1];

            do_int   = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  Each 2nd order stage looks like one of these.  The App. Manual  */
        /*  gives the first form, the patent gives the second form.         */
        /*  They're equivalent except for time delay.  I implement the      */
        /*  first form.   (Note: 1/Z == 1 unit of time delay.)              */
        /*                                                                  */
        /*          ---->(+)-------->(+)----------+------->                 */
        /*                ^           ^           |                         */
        /*                |           |           |                         */
        /*                |           |           |                         */
        /*               [B]        [2*F]         |                         */
        /*                ^           ^           |                         */
        /*                |           |           |                         */
        /*                |           |           |                         */
        /*                +---[1/Z]<--+---[1/Z]<--+                         */
        /*                                                                  */
        /*                                                                  */
        /*                +---[2*F]<---+                                    */
        /*                |            |                                    */
        /*                |            |                                    */
        /*                v            |                                    */
        /*          ---->(+)-->[1/Z]-->+-->[1/Z]---+------>                 */
        /*                ^                        |                        */
        /*                |                        |                        */
        /*                |                        |                        */
        /*                +-----------[B]<---------+                        */
        /*                                                                  */
        /* ---------------------------------------------------------------- */
        for (j = 0; j < 6; j++)
        {
            samp += (((int)f->b_coef[j] * (int)f->z_data[j][1]) >> 9);
            samp += (((int)f->f_coef[j] * (int)f->z_data[j][0]) >> 8);

            f->z_data[j][1] = f->z_data[j][0];
            f->z_data[j][0] = samp;
        }

        out[oidx++ & SCBUF_MASK] = limit(samp >> 4) * 256;
    }

    *optr = oidx;

    return i;
}

//static const int stage_map[6] = { 0, 1, 2, 3, 4, 5 };

//static const int stage_map[6] = { 5, 4, 3, 2, 1, 0 };
//static const int stage_map[6] = { 4, 2, 0, 5, 3, 1 };
static const int stage_map[6] = { 3, 0, 4, 1, 5, 2 }; // Seems OK for AL2 ...
//static const int stage_map[6] = { 3, 0, 1, 4, 2, 5 };

/* ======================================================================== */
/*  LPC12_REGDEC -- Decode the register set in the filter bank.             */
/* ======================================================================== */
static void lpc12_regdec(lpc12_t *f)
{
    int i;

    /* -------------------------------------------------------------------- */
    /*  Decode the Amplitude and Period registers.  Force cnt to 0 to get   */
    /*  the initial impulse.  (Redundant?)                                  */
    /* -------------------------------------------------------------------- */
    f->amp = amp_decode(f->r[0]);
    f->cnt = 0;
    f->per = f->r[1];

    /* -------------------------------------------------------------------- */
    /*  Decode the filter coefficients from the quant table.                */
    /* -------------------------------------------------------------------- */
    for (i = 0; i < 6; i++)
    {
        #define IQ(x) (((x) & 0x80) ? qtbl[0x7F & -(x)] : -qtbl[(x)])

        f->b_coef[stage_map[i]] = IQ(f->r[2 + 2*i]);
        f->f_coef[stage_map[i]] = IQ(f->r[3 + 2*i]);
    }

    /* -------------------------------------------------------------------- */
    /*  Set the Interp flag based on whether we have interpolation parms    */
    /* -------------------------------------------------------------------- */
    f->interp = f->r[14] || f->r[15];

    return;
}

/* ======================================================================== */
/*  SP0256_DATAFMT   -- Data format table for the SP0256's microsequencer   */
/*                                                                          */
/*  len     4 bits      Length of field to extract                          */
/*  lshift  4 bits      Left-shift amount on field                          */
/*  param   4 bits      Parameter number being updated                      */
/*  delta   1 bit       This is a delta-update.  (Implies sign-extend)      */
/*  field   1 bit       This is a field replace.                            */
/*  clr5    1 bit       Clear F5, B5.                                       */
/*  clrall  1 bit       Clear all before doing this update                  */
/* ======================================================================== */

#define CR(l,s,p,d,f,c5,ca)         \
        (                           \
            (((l)  & 15) <<  0) |   \
            (((s)  & 15) <<  4) |   \
            (((p)  & 15) <<  8) |   \
            (((d)  &  1) << 12) |   \
            (((f)  &  1) << 13) |   \
            (((c5) &  1) << 14) |   \
            (((ca) &  1) << 15)     \
        )

#define CR_DELTA  CR(0,0,0,1,0,0,0)
#define CR_FIELD  CR(0,0,0,0,1,0,0)
#define CR_CLR5   CR(0,0,0,0,0,1,0)
#define CR_CLRL   CR(0,0,0,0,0,0,1)
#define CR_LEN(x) ((x) & 15)
#define CR_SHF(x) (((x) >> 4) & 15)
#define CR_PRM(x) (((x) >> 8) & 15)

enum { AM = 0, PR, B0, F0, B1, F1, B2, F2, B3, F3, B4, F4, B5, F5, IA, IP };

static const uint16_t sp0256_datafmt[] =
{
    /* -------------------------------------------------------------------- */
    /*  OPCODE 1111: PAUSE                                                  */
    /* -------------------------------------------------------------------- */
    /*    0 */  CR( 0,  0,  0,  0,  0,  0,  0),     /*  Clear all   */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0001: LOADALL                                                */
    /* -------------------------------------------------------------------- */
                /* Mode modes x1    */
    /*    1 */  CR( 8,  0,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*    2 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*    3 */  CR( 8,  0,  B0, 0,  0,  0,  0),     /*  B0          */
    /*    4 */  CR( 8,  0,  F0, 0,  0,  0,  0),     /*  F0          */
    /*    5 */  CR( 8,  0,  B1, 0,  0,  0,  0),     /*  B1          */
    /*    6 */  CR( 8,  0,  F1, 0,  0,  0,  0),     /*  F1          */
    /*    7 */  CR( 8,  0,  B2, 0,  0,  0,  0),     /*  B2          */
    /*    8 */  CR( 8,  0,  F2, 0,  0,  0,  0),     /*  F2          */
    /*    9 */  CR( 8,  0,  B3, 0,  0,  0,  0),     /*  B3          */
    /*   10 */  CR( 8,  0,  F3, 0,  0,  0,  0),     /*  F3          */
    /*   11 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
    /*   12 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
    /*   13 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
    /*   14 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
    /*   15 */  CR( 8,  0,  IA, 0,  0,  0,  0),     /*  Amp Interp  */
    /*   16 */  CR( 8,  0,  IP, 0,  0,  0,  0),     /*  Pit Interp  */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0100: LOAD_4                                                 */
    /* -------------------------------------------------------------------- */
                /* Mode 00 and 01           */
    /*   17 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
    /*   18 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*   19 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*   20 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
    /*   21 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
    /*   22 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
                /* Mode 01 only             */
    /*   23 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
    /*   24 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

                /* Mode 10 and 11           */
    /*   25 */  CR( 6,  2,  AM, 0,  0,  0,  1),     /*  Amplitude   */
    /*   26 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*   27 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*   28 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
    /*   29 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
    /*   30 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
                /* Mode 11 only             */
    /*   31 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
    /*   32 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0110: SETMSB_6                                               */
    /* -------------------------------------------------------------------- */
                /* Mode 00 only             */
    /*   33 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 00 and 01           */
    /*   34 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   35 */  CR( 6,  2,  F3, 0,  1,  0,  0),     /*  F3 (5 MSBs) */
    /*   36 */  CR( 6,  2,  F4, 0,  1,  0,  0),     /*  F4 (5 MSBs) */
                /* Mode 01 only             */
    /*   37 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (5 MSBs) */

                /* Mode 10 only             */
    /*   38 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 10 and 11           */
    /*   39 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   40 */  CR( 7,  1,  F3, 0,  1,  0,  0),     /*  F3 (6 MSBs) */
    /*   41 */  CR( 8,  0,  F4, 0,  1,  0,  0),     /*  F4 (6 MSBs) */
                /* Mode 11 only             */
    /*   42 */  CR( 8,  0,  F5, 0,  1,  0,  0),     /*  F5 (6 MSBs) */

    /*   43 */  0,
    /*   44 */  0,

    /* -------------------------------------------------------------------- */
    /*  Opcode 1001: DELTA_9                                                */
    /* -------------------------------------------------------------------- */
                /* Mode 00 and 01           */
    /*   45 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
    /*   46 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
    /*   47 */  CR( 3,  4,  B0, 1,  0,  0,  0),     /*  B0 4 MSBs   */
    /*   48 */  CR( 3,  3,  F0, 1,  0,  0,  0),     /*  F0 5 MSBs   */
    /*   49 */  CR( 3,  4,  B1, 1,  0,  0,  0),     /*  B1 4 MSBs   */
    /*   50 */  CR( 3,  3,  F1, 1,  0,  0,  0),     /*  F1 5 MSBs   */
    /*   51 */  CR( 3,  4,  B2, 1,  0,  0,  0),     /*  B2 4 MSBs   */
    /*   52 */  CR( 3,  3,  F2, 1,  0,  0,  0),     /*  F2 5 MSBs   */
    /*   53 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
    /*   54 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
    /*   55 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
    /*   56 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
                /* Mode 01 only             */
    /*   57 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
    /*   58 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

                /* Mode 10 and 11           */
    /*   59 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
    /*   60 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
    /*   61 */  CR( 4,  1,  B0, 1,  0,  0,  0),     /*  B0 7 MSBs   */
    /*   62 */  CR( 4,  2,  F0, 1,  0,  0,  0),     /*  F0 6 MSBs   */
    /*   63 */  CR( 4,  1,  B1, 1,  0,  0,  0),     /*  B1 7 MSBs   */
    /*   64 */  CR( 4,  2,  F1, 1,  0,  0,  0),     /*  F1 6 MSBs   */
    /*   65 */  CR( 4,  1,  B2, 1,  0,  0,  0),     /*  B2 7 MSBs   */
    /*   66 */  CR( 4,  2,  F2, 1,  0,  0,  0),     /*  F2 6 MSBs   */
    /*   67 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
    /*   68 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
    /*   69 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
    /*   70 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
                /* Mode 11 only             */
    /*   71 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
    /*   72 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

    /* -------------------------------------------------------------------- */
    /*  Opcode 1010: SETMSB_A                                               */
    /* -------------------------------------------------------------------- */
                /* Mode 00 only             */
    /*   73 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 00 and 01           */
    /*   74 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   75 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
    /*   76 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
    /*   77 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */

                /* Mode 10 only             */
    /*   78 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 10 and 11           */
    /*   79 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   80 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
    /*   81 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
    /*   82 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0010: LOAD_2  Mode 00 and 10                                 */
    /*  Opcode 1100: LOAD_C  Mode 00 and 10                                 */
    /* -------------------------------------------------------------------- */
                /* LOAD_2, LOAD_C  Mode 00  */
    /*   83 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   84 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*   85 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
    /*   86 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
    /*   87 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
    /*   88 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
    /*   89 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
    /*   90 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
    /*   91 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*   92 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
    /*   93 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
    /*   94 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
                /* LOAD_2 only              */
    /*   95 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*   96 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

                /* LOAD_2, LOAD_C  Mode 10  */
    /*   97 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*   98 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*   99 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
    /*  100 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
    /*  101 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
    /*  102 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
    /*  103 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
    /*  104 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
    /*  105 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*  106 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
    /*  107 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
    /*  108 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
                /* LOAD_2 only              */
    /*  109 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*  110 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

    /* -------------------------------------------------------------------- */
    /*  OPCODE 1101: DELTA_D                                                */
    /* -------------------------------------------------------------------- */
                /* Mode 00 and 01           */
    /*  111 */  CR( 4,  2,  AM, 1,  0,  0,  1),     /*  Amplitude   */
    /*  112 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
    /*  113 */  CR( 3,  3,  B3, 1,  0,  0,  0),     /*  B3 5 MSBs   */
    /*  114 */  CR( 4,  2,  F3, 1,  0,  0,  0),     /*  F3 6 MSBs   */
    /*  115 */  CR( 4,  1,  B4, 1,  0,  0,  0),     /*  B4 7 MSBs   */
    /*  116 */  CR( 4,  2,  F4, 1,  0,  0,  0),     /*  F4 6 MSBs   */
                /* Mode 01 only             */
    /*  117 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
    /*  118 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

                /* Mode 10 and 11           */
    /*  119 */  CR( 4,  2,  AM, 1,  0,  0,  0),     /*  Amplitude   */
    /*  120 */  CR( 5,  0,  PR, 1,  0,  0,  0),     /*  Period      */
    /*  121 */  CR( 4,  1,  B3, 1,  0,  0,  0),     /*  B3 7 MSBs   */
    /*  122 */  CR( 5,  1,  F3, 1,  0,  0,  0),     /*  F3 7 MSBs   */
    /*  123 */  CR( 5,  0,  B4, 1,  0,  0,  0),     /*  B4 8 MSBs   */
    /*  124 */  CR( 5,  0,  F4, 1,  0,  0,  0),     /*  F4 8 MSBs   */
                /* Mode 11 only             */
    /*  125 */  CR( 5,  0,  B5, 1,  0,  0,  0),     /*  B5 8 MSBs   */
    /*  126 */  CR( 5,  0,  F5, 1,  0,  0,  0),     /*  F5 8 MSBs   */

    /* -------------------------------------------------------------------- */
    /*  OPCODE 1110: LOAD_E                                                 */
    /* -------------------------------------------------------------------- */
    /*  127 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  128 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0010: LOAD_2  Mode 01 and 11                                 */
    /*  Opcode 1100: LOAD_C  Mode 01 and 11                                 */
    /* -------------------------------------------------------------------- */
                /* LOAD_2, LOAD_C  Mode 01  */
    /*  129 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  130 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*  131 */  CR( 3,  4,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
    /*  132 */  CR( 5,  3,  F0, 0,  0,  0,  0),     /*  F0          */
    /*  133 */  CR( 3,  4,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
    /*  134 */  CR( 5,  3,  F1, 0,  0,  0,  0),     /*  F1          */
    /*  135 */  CR( 3,  4,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
    /*  136 */  CR( 5,  3,  F2, 0,  0,  0,  0),     /*  F2          */
    /*  137 */  CR( 4,  3,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*  138 */  CR( 6,  2,  F3, 0,  0,  0,  0),     /*  F3          */
    /*  139 */  CR( 7,  1,  B4, 0,  0,  0,  0),     /*  B4          */
    /*  140 */  CR( 6,  2,  F4, 0,  0,  0,  0),     /*  F4          */
    /*  141 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
    /*  142 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
                /* LOAD_2 only              */
    /*  143 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*  144 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

                /* LOAD_2, LOAD_C  Mode 11  */
    /*  145 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  146 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*  147 */  CR( 6,  1,  B0, 0,  0,  0,  0),     /*  B0 (S=0)    */
    /*  148 */  CR( 6,  2,  F0, 0,  0,  0,  0),     /*  F0          */
    /*  149 */  CR( 6,  1,  B1, 0,  0,  0,  0),     /*  B1 (S=0)    */
    /*  150 */  CR( 6,  2,  F1, 0,  0,  0,  0),     /*  F1          */
    /*  151 */  CR( 6,  1,  B2, 0,  0,  0,  0),     /*  B2 (S=0)    */
    /*  152 */  CR( 6,  2,  F2, 0,  0,  0,  0),     /*  F2          */
    /*  153 */  CR( 6,  1,  B3, 0,  0,  0,  0),     /*  B3 (S=0)    */
    /*  154 */  CR( 7,  1,  F3, 0,  0,  0,  0),     /*  F3          */
    /*  155 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
    /*  156 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
    /*  157 */  CR( 8,  0,  B5, 0,  0,  0,  0),     /*  B5          */
    /*  158 */  CR( 8,  0,  F5, 0,  0,  0,  0),     /*  F5          */
                /* LOAD_2 only              */
    /*  159 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*  160 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0011: SETMSB_3                                               */
    /*  Opcode 0101: SETMSB_5                                               */
    /* -------------------------------------------------------------------- */
                /* Mode 00 only             */
    /*  161 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 00 and 01           */
    /*  162 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  163 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*  164 */  CR( 5,  3,  F0, 0,  1,  0,  0),     /*  F0 (5 MSBs) */
    /*  165 */  CR( 5,  3,  F1, 0,  1,  0,  0),     /*  F1 (5 MSBs) */
    /*  166 */  CR( 5,  3,  F2, 0,  1,  0,  0),     /*  F2 (5 MSBs) */
                /* SETMSB_3 only            */
    /*  167 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*  168 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

                /* Mode 10 only             */
    /*  169 */  CR( 0,  0,  0,  0,  0,  0,  0),
                /* Mode 10 and 11           */
    /*  170 */  CR( 6,  2,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  171 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*  172 */  CR( 6,  2,  F0, 0,  1,  0,  0),     /*  F0 (6 MSBs) */
    /*  173 */  CR( 6,  2,  F1, 0,  1,  0,  0),     /*  F1 (6 MSBs) */
    /*  174 */  CR( 6,  2,  F2, 0,  1,  0,  0),     /*  F2 (6 MSBs) */
                /* SETMSB_3 only            */
    /*  175 */  CR( 5,  0,  IA, 0,  0,  0,  0),     /*  Ampl. Intr. */
    /*  176 */  CR( 5,  0,  IP, 0,  0,  0,  0),     /*  Per. Intr.  */

    /* -------------------------------------------------------------------- */
    /*  Opcode 0001: LOADALL                                                */
    /* -------------------------------------------------------------------- */
                /* Mode x0    */
    /*  177 */  CR( 8,  0,  AM, 0,  0,  0,  0),     /*  Amplitude   */
    /*  178 */  CR( 8,  0,  PR, 0,  0,  0,  0),     /*  Period      */
    /*  179 */  CR( 8,  0,  B0, 0,  0,  0,  0),     /*  B0          */
    /*  180 */  CR( 8,  0,  F0, 0,  0,  0,  0),     /*  F0          */
    /*  181 */  CR( 8,  0,  B1, 0,  0,  0,  0),     /*  B1          */
    /*  182 */  CR( 8,  0,  F1, 0,  0,  0,  0),     /*  F1          */
    /*  183 */  CR( 8,  0,  B2, 0,  0,  0,  0),     /*  B2          */
    /*  184 */  CR( 8,  0,  F2, 0,  0,  0,  0),     /*  F2          */
    /*  185 */  CR( 8,  0,  B3, 0,  0,  0,  0),     /*  B3          */
    /*  186 */  CR( 8,  0,  F3, 0,  0,  0,  0),     /*  F3          */
    /*  187 */  CR( 8,  0,  B4, 0,  0,  0,  0),     /*  B4          */
    /*  188 */  CR( 8,  0,  F4, 0,  0,  0,  0),     /*  F4          */
    /*  189 */  CR( 8,  0,  IA, 0,  0,  0,  0),     /*  Amp Interp  */
    /*  190 */  CR( 8,  0,  IP, 0,  0,  0,  0),     /*  Pit Interp  */

};



static const int16_t sp0256_df_idx[16 * 8] =
{
    /*  OPCODE 0000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
    /*  OPCODE 1000 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
    /*  OPCODE 0100 */      17, 22,     17, 24,     25, 30,     25, 32,
    /*  OPCODE 1100 */      83, 94,     129,142,    97, 108,    145,158,
    /*  OPCODE 0010 */      83, 96,     129,144,    97, 110,    145,160,
    /*  OPCODE 1010 */      73, 77,     74, 77,     78, 82,     79, 82,
    /*  OPCODE 0110 */      33, 36,     34, 37,     38, 41,     39, 42,
    /*  OPCODE 1110 */      127,128,    127,128,    127,128,    127,128,
    /*  OPCODE 0001 */      177,190,    1,  16,     177,190,    1,  16,
    /*  OPCODE 1001 */      45, 56,     45, 58,     59, 70,     59, 72,
    /*  OPCODE 0101 */      161,166,    162,166,    169,174,    170,174,
    /*  OPCODE 1101 */      111,116,    111,118,    119,124,    119,126,
    /*  OPCODE 0011 */      161,168,    162,168,    169,176,    170,176,
    /*  OPCODE 1011 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
    /*  OPCODE 0111 */      -1, -1,     -1, -1,     -1, -1,     -1, -1,
    /*  OPCODE 1111 */      0,  0,      0,  0,      0,  0,      0,  0
};

/* ======================================================================== */
/*  BITREV       -- Bit-reverse a 32-bit number.                            */
/* ======================================================================== */
static INLINE uint32_t bitrev(uint32_t val)
{
    val = ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16);
    val = ((val & 0xFF00FF00) >>  8) | ((val & 0x00FF00FF) <<  8);
    val = ((val & 0xF0F0F0F0) >>  4) | ((val & 0x0F0F0F0F) <<  4);
    val = ((val & 0xCCCCCCCC) >>  2) | ((val & 0x33333333) <<  2);
    val = ((val & 0xAAAAAAAA) >>  1) | ((val & 0x55555555) <<  1);

    return val;
}

/* ======================================================================== */
/*  SP0256_GETB  -- Get up to 8 bits at the current PC.                     */
/* ======================================================================== */
static uint32_t sp0256_getb(ivoice_t *ivoice, int len)
{
    uint32_t data = 0;
    uint32_t d0, d1;

    /* -------------------------------------------------------------------- */
    /*  Fetch data from the FIFO or from the MASK                           */
    /* -------------------------------------------------------------------- */
    if (ivoice->fifo_sel)
    {
        d0 = ivoice->fifo[(ivoice->fifo_tail    ) & 63];
        d1 = ivoice->fifo[(ivoice->fifo_tail + 1) & 63];

        data = ((d1 << 10) | d0) >> ivoice->fifo_bitp;

#ifdef DEBUG_FIFO
        dfprintf(("IV: RD_FIFO %.3X %d.%d %d\n", data & ((1u << len) - 1),
                ivoice->fifo_tail, ivoice->fifo_bitp, ivoice->fifo_head));
#endif

        /* ---------------------------------------------------------------- */
        /*  Note the PC doesn't advance when we execute from FIFO.          */
        /*  Just the FIFO's bit-pointer advances.   (That's not REALLY      */
        /*  what happens, but that's roughly how it behaves.)               */
        /* ---------------------------------------------------------------- */
        ivoice->fifo_bitp += len;
        if (ivoice->fifo_bitp >= 10)
        {
            ivoice->fifo_tail++;
            ivoice->fifo_bitp -= 10;
        }
    } else
    {
        /* ---------------------------------------------------------------- */
        /*  Figure out which ROMs are being fetched into, and grab two      */
        /*  adjacent bytes.  The byte we're interested in is extracted      */
        /*  from the appropriate bit-boundary between them.                 */
        /* ---------------------------------------------------------------- */
        int idx0 = (ivoice->pc    ) >> 3, page0 = idx0 >> 12;
        int idx1 = (ivoice->pc + 8) >> 3, page1 = idx1 >> 12;

        idx0 &= 0xFFF;
        idx1 &= 0xFFF;

        d0 = d1 = 0;

        if (ivoice->rom[page0]) d0 = ivoice->rom[page0][idx0];
        if (ivoice->rom[page1]) d1 = ivoice->rom[page1][idx1];

        data = ((d1 << 8) | d0) >> (ivoice->pc & 7);

        ivoice->pc += len;
    }

    /* -------------------------------------------------------------------- */
    /*  Mask data to the requested length.                                  */
    /* -------------------------------------------------------------------- */
    data &= ((1u << len) - 1);

    return data;
}

/* ======================================================================== */
/*  SP0256_MICRO -- Emulate the microsequencer in the SP0256.  Executes     */
/*                  instructions either until the repeat count != 0 or      */
/*                  the sequencer gets halted by a RTS to 0.                */
/* ======================================================================== */
static void sp0256_micro(ivoice_t *iv)
{
    uint8_t  immed4;
    uint8_t  opcode;
    uint16_t cr;
    int      ctrl_xfer = 0;
    int      repeat    = 0;
    int      i, idx0, idx1;

    /* -------------------------------------------------------------------- */
    /*  Only execute instructions while the filter is not busy.             */
    /* -------------------------------------------------------------------- */
    while (iv->filt.rpt <= 0 && iv->filt.cnt <= 0)
    {
        /* ---------------------------------------------------------------- */
        /*  If the CPU is halted, see if we have a new command pending      */
        /*  in the Address LoaD buffer.                                     */
        /* ---------------------------------------------------------------- */
        if (iv->halted && !iv->lrq)
        {
			int data = iv->ald >> 4;
			jzdprintf(( "\nfetch => %02X: %s\n", data, data < s_nLabels ? s_labels[data] : "---" ));
            iv->pc       = iv->ald | (0x1000 << 3);
            iv->fifo_sel = 0;
            iv->halted   = 0;
            iv->lrq      = 0x8000;
            iv->ald      = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  If we're still halted, do nothing.                              */
        /* ---------------------------------------------------------------- */
        if (iv->halted)
        {
			jzdprintf(( "\nfetch => HALT\n" ));
            iv->filt.rpt = 1;
            iv->filt.cnt = 0;
            iv->lrq      = 0x8000;
            iv->ald      = 0;
            return;
        }

        /* ---------------------------------------------------------------- */
        /*  Fetch the first 8 bits of the opcode, which are always in the   */
        /*  same approximate format -- immed4 followed by opcode.           */
        /* ---------------------------------------------------------------- */
        immed4 = sp0256_getb(iv, 4);
        opcode = sp0256_getb(iv, 4);
        repeat = 0;
        ctrl_xfer = 0;

        jzdprintf(("$%.4X.%.1X: OPCODE %d%d%d%d.%d%d - %s\n",
                (iv->pc >> 3) - 1, iv->pc & 7,
                !!(opcode & 1), !!(opcode & 2),
                !!(opcode & 4), !!(opcode & 8),
                !!(iv->mode&4), !!(iv->mode&2),
				opcodes[opcode&15]
				));

        /* ---------------------------------------------------------------- */
        /*  Handle the special cases for specific opcodes.                  */
        /* ---------------------------------------------------------------- */
        switch (opcode)
        {
            /* ------------------------------------------------------------ */
            /*  OPCODE 0000:  RTS / SETPAGE                                 */
            /* ------------------------------------------------------------ */
            case 0x0:
            {
                /* -------------------------------------------------------- */
                /*  If immed4 != 0, then this is a SETPAGE instruction.     */
                /* -------------------------------------------------------- */
                if (immed4)     /* SETPAGE */
                {
                    iv->page = bitrev(immed4) >> 13;
                } else
                /* -------------------------------------------------------- */
                /*  Otherwise, this is an RTS / HLT.                        */
                /* -------------------------------------------------------- */
                {
                    uint32_t btrg;

                    /* ---------------------------------------------------- */
                    /*  Figure out our branch target.                       */
                    /* ---------------------------------------------------- */
                    btrg = iv->stack;

                    iv->stack = 0;

                    /* ---------------------------------------------------- */
                    /*  If the branch target is zero, this is a HLT.        */
                    /*  Otherwise, it's an RTS, so set the PC.              */
                    /* ---------------------------------------------------- */
                    if (!btrg)
                    {
                        iv->halted = 1;
                        iv->pc     = 0;
                        ctrl_xfer  = 1;
                    } else
                    {
                        iv->pc    = btrg;
                        ctrl_xfer = 1;
                    }
                }

                break;
            }

            /* ------------------------------------------------------------ */
            /*  OPCODE 0111:  JMP          Jump to 12-bit/16-bit Abs Addr   */
            /*  OPCODE 1011:  JSR          Jump to Subroutine               */
            /* ------------------------------------------------------------ */
            case 0xE:
            case 0xD:
            {
                int btrg;

                /* -------------------------------------------------------- */
                /*  Figure out our branch target.                           */
                /* -------------------------------------------------------- */
                btrg = iv->page                           |
                       (bitrev(immed4)             >> 17) |
                       (bitrev(sp0256_getb(iv, 8)) >> 21);
                ctrl_xfer = 1;

                /* -------------------------------------------------------- */
                /*  If this is a JSR, push our return address on the        */
                /*  stack.  Make sure it's byte aligned.                    */
                /* -------------------------------------------------------- */
                if (opcode == 0xD)
                    iv->stack = (iv->pc + 7) & ~7;

                /* -------------------------------------------------------- */
                /*  Jump to the new location!                               */
                /* -------------------------------------------------------- */
                iv->pc = btrg;
                break;
            }

            /* ------------------------------------------------------------ */
            /*  OPCODE 1000:  SETMODE      Set the Mode and Repeat MSBs     */
            /* ------------------------------------------------------------ */
            case 0x1:
            {
                iv->mode = ((immed4 & 8) >> 2) | (immed4 & 4) |
                           ((immed4 & 3) << 4);
                break;
            }

            /* ------------------------------------------------------------ */
            /*  OPCODE 0001:  LOADALL      Load All Parameters              */
            /*  OPCODE 0010:  LOAD_2       Load Per, Ampl, Coefs, Interp.   */
            /*  OPCODE 0011:  SETMSB_3     Load Pitch, Ampl, MSBs, & Intrp  */
            /*  OPCODE 0100:  LOAD_4       Load Pitch, Ampl, Coeffs         */
            /*  OPCODE 0101:  SETMSB_5     Load Pitch, Ampl, and Coeff MSBs */
            /*  OPCODE 0110:  SETMSB_6     Load Ampl, and Coeff MSBs.       */
            /*  OPCODE 1001:  DELTA_9      Delta update Ampl, Pitch, Coeffs */
            /*  OPCODE 1010:  SETMSB_A     Load Ampl and MSBs of 3 Coeffs   */
            /*  OPCODE 1100:  LOAD_C       Load Pitch, Ampl, Coeffs         */
            /*  OPCODE 1101:  DELTA_D      Delta update Ampl, Pitch, Coeffs */
            /*  OPCODE 1110:  LOAD_E       Load Pitch, Amplitude            */
            /*  OPCODE 1111:  PAUSE        Silent pause                     */
            /* ------------------------------------------------------------ */
            default:
            {
                repeat    = immed4 | (iv->mode & 0x30);
                break;
            }
        }
        if (opcode != 1) // SETMODE
			iv->mode &= 0xF;

        /* ---------------------------------------------------------------- */
        /*  If this was a control transfer, handle setting "fifo_sel"       */
        /*  and all that ugliness.                                          */
        /* ---------------------------------------------------------------- */
        if (ctrl_xfer)
        {
            jzdprintf(("jumping to $%.4X.%.1X: ", iv->pc >> 3, iv->pc & 7));

            /* ------------------------------------------------------------ */
            /*  Set our "FIFO Selected" flag based on whether we're going   */
            /*  to the FIFO's address.                                      */
            /* ------------------------------------------------------------ */
            iv->fifo_sel = fifoEnabled && ( iv->pc == FIFO_ADDR );

            jzdprintf(("%s ", iv->fifo_sel ? "FIFO" : "ROM"));

            /* ------------------------------------------------------------ */
            /*  Control transfers to the FIFO cause it to discard the       */
            /*  partial decle that's at the front of the FIFO.              */
            /* ------------------------------------------------------------ */
            if (iv->fifo_sel && iv->fifo_bitp)
            {
                jzdprintf(("bitp = %d -> Flush", iv->fifo_bitp));

                /* Discard partially-read decle. */
                if (iv->fifo_tail < iv->fifo_head) iv->fifo_tail++;
                iv->fifo_bitp = 0;
            }

            jzdprintf(("\n"));

            continue;
        }

        /* ---------------------------------------------------------------- */
        /*  Otherwise, if we have a repeat count, then go grab the data     */
        /*  block and feed it to the filter.                                */
        /* ---------------------------------------------------------------- */
        if (!repeat) 
			continue;


		if ( s_debugSingleStep )
		{
			jzp_printf("NEXT:"); jzp_flush();
        {
        char buf[1024];
				fgets(buf,sizeof(buf),stdin); // if (opcode != 0xF) repeat <<= 3;
				if ( toupper(*buf) == 'C' ) // (C)ontinue
					s_debugSingleStep = 0;
			}
        }

        iv->filt.rpt = repeat;
        jzdprintf(("repeat = %d\n", repeat));

        /* clear delay line on new opcode */
        for (i = 0; i < 6; i++)
             iv->filt.z_data[i][0] = iv->filt.z_data[i][1] = 0;

        i = (opcode << 3) | (iv->mode & 6);
        idx0 = sp0256_df_idx[i++];
        idx1 = sp0256_df_idx[i  ];

        assert(idx0 >= 0 && idx1 >= 0 && idx1 >= idx0);

        /* ---------------------------------------------------------------- */
        /*  If we're in one of the 10-pole modes (x0), clear F5/B5.         */
        /* ---------------------------------------------------------------- */
        if ((iv->mode & 2) == 0)
            iv->filt.r[F5] = iv->filt.r[B5] = 0;


        /* ---------------------------------------------------------------- */
        /*  Step through control words in the description for data block.   */
        /* ---------------------------------------------------------------- */
        for (i = idx0; i <= idx1; i++)
        {
            int len, shf, delta, field, prm, clrL;
            int8_t value;

            /* ------------------------------------------------------------ */
            /*  Get the control word and pull out some important fields.    */
            /* ------------------------------------------------------------ */
            cr = sp0256_datafmt[i];

            len   = CR_LEN(cr);
            shf   = CR_SHF(cr);
            prm   = CR_PRM(cr);
            clrL  = cr & CR_CLRL;
            delta = cr & CR_DELTA;
            field = cr & CR_FIELD;
            value = 0;

            jzdprintf(("$%.4X.%.1X: len=%2d shf=%2d prm=%2d d=%d f=%d ",
                     iv->pc >> 3, iv->pc & 7, len, shf, prm, !!delta, !!field));
            /* ------------------------------------------------------------ */
            /*  Clear any registers that were requested to be cleared.      */
            /* ------------------------------------------------------------ */
            if (clrL)
            {
                iv->filt.r[F0] = iv->filt.r[B0] = 0;
                iv->filt.r[F1] = iv->filt.r[B1] = 0;
                iv->filt.r[F2] = iv->filt.r[B2] = 0;
            }

            /* ------------------------------------------------------------ */
            /*  If this entry has a bitfield with it, grab the bitfield.    */
            /* ------------------------------------------------------------ */
            if (len)
            {
                value = sp0256_getb(iv, len);
            }
            else
            {
                jzdprintf((" (no update)\n"));
                continue;
            }

            /* ------------------------------------------------------------ */
            /*  Sign extend if this is a delta update.                      */
            /* ------------------------------------------------------------ */
            if (delta)  /* Sign extend */
            {
                if (value & (1u << (len - 1))) value |= -(1u << len);
            }

            /* ------------------------------------------------------------ */
            /*  Shift the value to the appropriate precision.               */
            /* ------------------------------------------------------------ */
            if (shf)
                value = value < 0 ? -(-value << shf) : (value << shf);

            jzdprintf(("v=%.2X (%c%.2X)  ", value & 0xFF,
                     value & 0x80 ? '-' : '+',
                     0xFF & (value & 0x80 ? -value : value)));

            iv->silent = 0;

            /* ------------------------------------------------------------ */
            /*  If this is a field-replace, insert the field.               */
            /* ------------------------------------------------------------ */
            if (field)
            {
                jzdprintf(("--field-> r[%2d] = %.2X -> ", prm, iv->filt.r[prm]));

                iv->filt.r[prm] &= ~(~0u << shf); /* Clear the old bits.    */
                iv->filt.r[prm] |= value;         /* Merge in the new bits. */

                jzdprintf(("%.2X\n", iv->filt.r[prm]));

                continue;
            }

            /* ------------------------------------------------------------ */
            /*  If this is a delta update, add to the appropriate field.    */
            /* ------------------------------------------------------------ */
            if (delta)
            {
                jzdprintf(("--delta-> r[%2d] = %.2X -> ", prm, iv->filt.r[prm]));

                iv->filt.r[prm] += value;

                jzdprintf(("%.2X\n", iv->filt.r[prm]));

                continue;
            }

            /* ------------------------------------------------------------ */
            /*  Otherwise, just write the new value.                        */
            /* ------------------------------------------------------------ */
            iv->filt.r[prm] = value;
            jzdprintf(("--value-> r[%2d] = %.2X\n", prm, iv->filt.r[prm]));
        }

        /* ---------------------------------------------------------------- */
        /*  Most opcodes clear IA, IP.                                      */
        /* ---------------------------------------------------------------- */
        if (opcode != 0x1 && opcode != 0x2 && opcode != 0x3)
        {
            iv->filt.r[IA] = 0;
            iv->filt.r[IP] = 0;
        }

        /* ---------------------------------------------------------------- */
        /*  Special case:  Set PAUSE's equivalent period.                   */
        /* ---------------------------------------------------------------- */
        if (opcode == 0xF)
        {
            iv->silent     = 1;
            iv->filt.r[AM] = 0;
            iv->filt.r[PR] = PER_PAUSE;
        }

        /* ---------------------------------------------------------------- */
        /*  Now that we've updated the registers, go decode them.           */
        /* ---------------------------------------------------------------- */
        lpc12_regdec(&iv->filt);

        /* ---------------------------------------------------------------- */
        /*  Break out since we now have a repeat count.                     */
        /* ---------------------------------------------------------------- */
        break;
    }
}

/* ======================================================================== */
/*  IVOICE_RD    -- Handle reads from the Intellivoice.                     */
/* ======================================================================== */
uint32_t ivoice_rd(uint32_t addr)
{
    ivoice_t *ivoice = &intellivoice;

    /* -------------------------------------------------------------------- */
    /*  Address 0x80 returns the SP0256 LRQ status on bit 15.               */
    /* -------------------------------------------------------------------- */
    if (addr == 0)
    {
        return ivoice->lrq;
    }

    /* -------------------------------------------------------------------- */
    /*  Address 0x81 returns the SPB640 FIFO full status on bit 15.         */
    /* -------------------------------------------------------------------- */
    if (addr == 1)
    {
        return (ivoice->fifo_head - ivoice->fifo_tail) >= 64 ? 0x8000 : 0;
    }

    /* -------------------------------------------------------------------- */
    /*  Just return 255 for all other addresses in our range.               */
    /* -------------------------------------------------------------------- */
    return 0x00FF;
}

/* ======================================================================== */
/*  IVOICE_WR    -- Handle writes to the Intellivoice.                      */
/* ======================================================================== */
void ivoice_wr(uint32_t addr, uint32_t data)
{
    ivoice_t *ivoice = &intellivoice;

    /* -------------------------------------------------------------------- */
    /*  Ignore writes outside 0x80, 0x81.                                   */
    /* -------------------------------------------------------------------- */
    if (addr > 1) return;

    /* -------------------------------------------------------------------- */
    /*  Address 0x80 is for Address Loads (essentially speech commands).    */
    /* -------------------------------------------------------------------- */
    if (addr == 0)
    {
        /* ---------------------------------------------------------------- */
        /*  Drop writes to the ALD register if we're busy.                  */
        /* ---------------------------------------------------------------- */
        if (!ivoice->lrq)
            return;

        /* ---------------------------------------------------------------- */
        /*  Set LRQ to "busy" and load the 8 LSBs of the data into the ALD  */
        /*  reg.  We take the command address, and multiply by 2 bytes to   */
        /*  get the new PC address.                                         */
        /* ---------------------------------------------------------------- */
        ivoice->lrq = 0;
        ivoice->ald = (0xFF & data) << 4;

        return;
    }

    /* -------------------------------------------------------------------- */
    /*  Address 0x81 is for FIFOing up decles.  The FIFO is 64 decles       */
    /*  long.  The Head pointer points to where we insert new decles and    */
    /*  the Tail pointer is where we pull them from.                        */
    /* -------------------------------------------------------------------- */
    if (addr == 1)
    {
        /* ---------------------------------------------------------------- */
        /*  If Bit 10 is set, just reset the FIFO and SP0256.               */
        /* ---------------------------------------------------------------- */
        if (data & 0x400)
        {
            ivoice->fifo_head = ivoice->fifo_tail = ivoice->fifo_bitp = 0;

            memset(&ivoice->filt, 0, sizeof(ivoice->filt));
            ivoice->halted   = 1;
            ivoice->filt.rpt = -1;
            ivoice->filt.rng = 1;
            ivoice->lrq      = 0x8000;
            ivoice->ald      = 0x0000;
            ivoice->pc       = 0x0000;
            ivoice->stack    = 0x0000;
            ivoice->fifo_sel = 0;
            ivoice->mode     = 0;
            ivoice->page     = 0x1000 << 3;
            ivoice->silent   = 1;
            return;
        }

        /* ---------------------------------------------------------------- */
        /*  If the FIFO is full, drop the data.                             */
        /* ---------------------------------------------------------------- */
        if ((ivoice->fifo_head - ivoice->fifo_tail) >= 64)
        {
            jzdprintf(("IV: Dropped FIFO write\n"));
            return;
        }

        /* ---------------------------------------------------------------- */
        /*  FIFO up the lower 10 bits of the data.                          */
        /* ---------------------------------------------------------------- */
#ifdef DEBUG_FIFO
        dfprintf(("IV: WR_FIFO %.3X %d.%d %d\n", data & 0x3FF,
                ivoice->fifo_tail, ivoice->fifo_bitp, ivoice->fifo_head));
#endif
        ivoice->fifo[ivoice->fifo_head++ & 63] = data & 0x3FF;

        return;
    }
}

/* ======================================================================== */
/*  IVOICE_RESET -- Resets the Intellivoice                                 */
/* ======================================================================== */
void ivoice_reset(void)
{
    /* -------------------------------------------------------------------- */
    /*  Do a software-style reset of the Intellivoice.                      */
    /* -------------------------------------------------------------------- */
    ivoice_wr(1, 0x400);
}


/* ======================================================================== */
/*  IVOICE_INIT  -- Makes a new Intellivoice                                */
/* ======================================================================== */
int ivoice_init
(
	const uint8_t			*mask
)
{
    ivoice_t *ivoice = &intellivoice;

    /* -------------------------------------------------------------------- */
    /*  First, lets zero out the structure to be safe.                      */
    /* -------------------------------------------------------------------- */
    memset(ivoice, 0, sizeof(ivoice_t));

    /* -------------------------------------------------------------------- */
    /*  Configure our internal variables.                                   */
    /* -------------------------------------------------------------------- */
    ivoice->rom[1]     = mask;
	ivoice->filt.rng   = 1;

    /* -------------------------------------------------------------------- */
    /*  Set up the microsequencer's initial state.                          */
    /* -------------------------------------------------------------------- */
    ivoice->halted   = 1;
    ivoice->filt.rpt = -1;
    ivoice->lrq      = 0x8000;
    ivoice->page     = 0x1000 << 3;
    ivoice->silent   = 1;

    return 0;
}

// BEGIN GmEsoft additions

void sp0256_setFifoEnabled( int enabled )
{
}

uint32_t sp0256_getStatus()
{
    return ivoice_rd( 0 );
}

int sp0256_halted()
{
	return intellivoice.halted;
}

void sp0256_sendCommand( unsigned cmd )
{
	ivoice_wr( 0, cmd );
}

int sp0256_isNextSample()
{
    ivoice_t *ivoice = &intellivoice;
	return 0;
}

int sp0256_getNextSample()
{
    ivoice_t *ivoice = &intellivoice;
	uint32_t optr = 0;
	int16_t out = 0;

	if (ivoice->filt.rpt <= 0 && ivoice->filt.cnt <= 0)
        sp0256_micro(ivoice);

	if  (	ivoice->halted 
		||	( ivoice->silent && ivoice->filt.rpt <= 0 && ivoice->filt.cnt <= 0 ) 
		)
	{
		out = 0;
    } 
	else 
	{
		lpc12_update(&ivoice->filt, 1, &out, &optr);
    }

	//dsprintf(( "%d\t%d\n", nSample++, (int8_t)(out >> 8) ));
	dsprintf(( "%ld\t%4d\t%*c\n", nSample++, (int8_t)(out >> 8), (int8_t)(out >> 9)+64, '+' ));

	return out;
}

int sp0256_exec()
{
    ivoice_t *ivoice = &intellivoice;

    /* ------------------------------------------------------------ */
    /*  If our repeat count expired, emulate the microsequencer.    */
    /* ------------------------------------------------------------ */
    if (ivoice->filt.rpt <= 0 && ivoice->filt.cnt <= 0)
        sp0256_micro(ivoice);

	return 0;
}

void sp0256_setLabels( int nLabels, const char *labels[] )
{
	s_nLabels = nLabels;
	s_labels = labels;
}

void sp0256_setDebug( int debug )
{
	s_debug = debug & 1;
	s_debugSample = debug & 2;
	s_debugSingleStep = debug & 4;
}

// END   GmEsoft additions

/* ======================================================================== */
/*  This program is free software; you can redistribute it and/or modify    */
/*  it under the terms of the GNU General Public License as published by    */
/*  the Free Software Foundation; either version 2 of the License, or       */
/*  (at your option) any later version.                                     */
/*                                                                          */
/*  This program is distributed in the hope that it will be useful,         */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of          */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       */
/*  General Public License for more details.                                */
/*                                                                          */
/*  You should have received a copy of the GNU General Public License along */
/*  with this program; if not, write to the Free Software Foundation, Inc., */
/*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.             */
/* ======================================================================== */
/*                 Copyright (c) 1998-2000, Joseph Zbiciak                  */
/* ======================================================================== */
