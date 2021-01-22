/* ==============================================================
   Macro definitions for pseudographics characters.
   These characters may be encoded in an alternate character set.
   Therfore, you must alwys use the "F_ALTERNATE" color attribute
   when printing any of these characters.
   ==============================================================  */

#ifndef __SPECCH_H__
#define __SPECCH_H__

#define SC(x) tt_specials[((x) - 1)]
#define SC1 tt_specials[0]     /* double border vertical              */
#define SC2 tt_specials[1]     /* double border horizontal            */
#define SC3 tt_specials[2]     /* double border left top corner       */
#define SC4 tt_specials[3]     /* double border right top corner      */
#define SC5 tt_specials[4]     /* double border left bottom corner    */
#define SC6 tt_specials[5]     /* double border right bottom corner   */
#define SC7 tt_specials[6]     /* single border vertical              */
#define SC8 tt_specials[7]     /* single border horizontal            */
#define SC9 tt_specials[8]     /* single border left top corner       */
#define SC10 tt_specials[9]    /* single border right top corner      */
#define SC11 tt_specials[10]   /* single border left bottom corner    */
#define SC12 tt_specials[11]   /* single border right bottom corner   */
#define SC13 tt_specials[12]   /* 50% grey box (for input fields)     */
#define SC14 tt_specials[13]   /* arrow left                          */
#define SC15 tt_specials[14]   /* arrow right                         */
#define SC16 tt_specials[15]   /* button shadow underneath            */
#define SC17 tt_specials[16]   /* button shadow to the right          */
#define SC18 tt_specials[17]   /* arrow up                            */
#define SC19 tt_specials[18]   /* arrow down                          */
#define SC20 tt_specials[19]   /* "PI" (end of text paragraph marker) */
#define SC21 tt_specials[20]   /* arrow <->                           */
extern char * tt_specials;

#endif // ifndef __SPECCH_H__
