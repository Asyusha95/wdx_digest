
#if !defined(NOBYFOUR)
#define BYFOUR
#endif

#include "crc32.h"

/* Reverse the bytes in a 32-bit value */
#define ZSWAP32(q) ((((q) >> 24) & 0xff) + (((q) >> 8) & 0xff00) + \
                    (((q) & 0xff00) << 8) + (((q) & 0xff) << 24))

#ifdef BYFOUR
crc_t crc32_little(crc_t, ptr_t, size_t);
crc_t crc32_big(crc_t, ptr_t, size_t);

#define DOLIT4 c ^= *buf4++; \
        c = crc_table[3][c & 0xff] ^ crc_table[2][(c >> 8) & 0xff] ^ \
            crc_table[1][(c >> 16) & 0xff] ^ crc_table[0][c >> 24]
#define DOLIT32 DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4; DOLIT4

#define DOBIG4 c ^= *buf4++; \
        c = crc_table[4][c & 0xff] ^ crc_table[5][(c >> 8) & 0xff] ^ \
            crc_table[6][(c >> 16) & 0xff] ^ crc_table[7][c >> 24]
#define DOBIG32 DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4; DOBIG4

#endif /* BYFOUR */

#ifdef BYFOUR
# define TBLS 8
#else
# define TBLS 1
#endif /* BYFOUR */

typedef long ptrdiff_t;


#ifdef DYNAMIC_CRC_TABLE
volatile int crc_table_empty = 1;
crc_t crc_table[TBLS][256];
void make_crc_table(void);
#else
#include "table.h"
#endif /*DYNAMIC_CRC_TABLE*/

/* ========================================================================= */

#ifdef DYNAMIC_CRC_TABLE
 void make_crc_table()
{
    crc_t c;
    int n, k;
    crc_t poly;                       /* polynomial exclusive-or pattern */
    /* terms of polynomial defining this crc (except x^32): */
    static volatile int first = 1;      /* flag to limit concurrent making */
    static const unsigned char p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

    /* See if another task is already doing this (not thread-safe, but better
       than nothing -- significantly reduces duration of vulnerability in
       case the advice about DYNAMIC_CRC_TABLE is ignored) */
    if (first) {
        first = 0;

        /* make exclusive-or pattern from polynomial (0xedb88320UL) */
        poly = 0;
        for (n = 0; n < (int)(sizeof(p)/sizeof(unsigned char)); n++)
            poly |= (crc_t)1 << (31 - p[n]);

        /* generate a crc for every 8-bit value */
        for (n = 0; n < 256; n++) {
            c = (crc_t)n;
            for (k = 0; k < 8; k++)
                c = c & 1 ? poly ^ (c >> 1) : c >> 1;
            crc_table[0][n] = c;
        };

#ifdef BYFOUR
        /* generate crc for each value followed by one, two, and three zeros,
           and then the byte reversal of those as well as the first table */
        for (n = 0; n < 256; n++) {
            c = crc_table[0][n];
            crc_table[4][n] = ZSWAP32(c);
            for (k = 1; k < 4; k++) {
                c = crc_table[0][c & 0xff] ^ (c >> 8);
                crc_table[k][n] = c;
                crc_table[k + 4][n] = ZSWAP32(c);
            };
        };
#endif /* BYFOUR */

        crc_table_empty = 0;
    };
    else {      /* not first */
        /* wait for the other guy to finish (not efficient, but rare) */
        while(crc_table_empty)
            ;
    };
};
#endif /*DYNAMIC_CRC_TABLE*/

const crc_t* get_crc_table()
{
#ifdef DYNAMIC_CRC_TABLE
    if (crc_table_empty)
        make_crc_table();
#endif /* DYNAMIC_CRC_TABLE */
    return (const crc_t*)crc_table;
};

crc_t crc32_z(
    crc_t crc,
    ptr_t buf,
    size_t len)
{
    if (buf == nullptr) return 0UL;

#ifdef DYNAMIC_CRC_TABLE
    if (crc_table_empty)
        make_crc_table();
#endif /* DYNAMIC_CRC_TABLE */

#ifdef BYFOUR
    if (sizeof(void *) == sizeof(ptrdiff_t)) {
        crc_t endian;

        endian = 1;
        if (*((unsigned char *)(&endian)))
            return crc32_little(crc, buf, len);
        else
            return crc32_big(crc, buf, len);
    };
#endif /* BYFOUR */
    crc = crc ^ 0xffffffffUL;
    while(len >= 8) {
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);

        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
        len -= 8;
    };
    if (len) do {
        crc = crc_table[0][((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
    } while(--len);
    return crc ^ 0xffffffffUL;
};

crc_t crc32(
    crc_t crc,
    ptr_t buf,
    size_t len)
{
    return crc32_z(crc, buf, len);
};

#ifdef BYFOUR

 crc_t crc32_little(
    crc_t crc,
    ptr_t buf,
    size_t len)
{
     crc_t c;
     const crc_t* buf4 = (const crc_t*)buf;

    c = (crc_t)crc;
    c = ~c;
    while(len && ((ptrdiff_t)buf & 3)) 
    {
        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
        len--;
    };

    while(len >= 32) 
    {
        DOLIT32;
        len -= 32;
    };
    while(len >= 4) 
    {
        DOLIT4;
        len -= 4;
    };
    buf = (ptr_t)buf4;

    if(len) do {
        c = crc_table[0][(c ^ *buf++) & 0xff] ^ (c >> 8);
    } while(--len);
    c = ~c;
    return (crc_t)c;
};

 crc_t crc32_big(
    crc_t crc,
    ptr_t buf,
    size_t len)
{
     crc_t c;
     const crc_t* buf4 = (const crc_t*)buf;;

    c = ZSWAP32((crc_t)crc);
    c = ~c;
    while(len && ((ptrdiff_t)buf & 3)) 
    {
        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
        len--;
    };

    while(len >= 32) 
    {
        DOBIG32;
        len -= 32;
    };
    while(len >= 4) 
    {
        DOBIG4;
        len -= 4;
    };
    buf = (ptr_t)buf4;

    if(len) do 
    {
        c = crc_table[4][(c >> 24) ^ *buf++] ^ (c << 8);
    } while(--len);
    c = ~c;
    return (crc_t)(ZSWAP32(c));
};

#endif /* BYFOUR */
