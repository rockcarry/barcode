#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "barcode.h"

enum {
    CODE_ID_CODE_C = 99,
    CODE_ID_CODE_B,
    CODE_ID_CODE_A,
    CODE_ID_FNC1,
    CODE_ID_START_A,
    CODE_ID_START_B,
    CODE_ID_START_C,
    CODE_ID_STOP,
};

static char* ID2CODE_TAB[] = {
"bbsbbssbbss", "bbssbbsbbss", "bbssbbssbbs", "bssbssbbsss", "bssbsssbbss", "bsssbssbbss", "bssbbssbsss", "bssbbsssbss", "bsssbbssbss", "bbssbssbsss",
"bbssbsssbss", "bbsssbssbss", "bsbbssbbbss", "bssbbsbbbss", "bssbbssbbbs", "bsbbbssbbss", "bssbbbsbbss", "bssbbbssbbs", "bbssbbbssbs", "bbssbsbbbss",
"bbssbssbbbs", "bbsbbbssbss", "bbssbbbsbss", "bbbsbbsbbbs", "bbbsbssbbss", "bbbssbsbbss", "bbbssbssbbs", "bbbsbbssbss", "bbbssbbsbss", "bbbssbbssbs",
"bbsbbsbbsss", "bbsbbsssbbs", "bbsssbbsbbs", "bsbsssbbsss", "bsssbsbbsss", "bsssbsssbbs", "bsbbsssbsss", "bsssbbsbsss", "bsssbbsssbs", "bbsbsssbsss",
"bbsssbsbsss", "bbsssbsssbs", "bsbbsbbbsss", "bsbbsssbbbs", "bsssbbsbbbs", "bsbbbsbbsss", "bsbbbsssbbs", "bsssbbbsbbs", "bbbsbbbsbbs", "bbsbsssbbbs",
"bbsssbsbbbs", "bbsbbbsbsss", "bbsbbbsssbs", "bbsbbbsbbbs", "bbbsbsbbsss", "bbbsbsssbbs", "bbbsssbsbbs", "bbbsbbsbsss", "bbbsbbsssbs", "bbbsssbbsbs",
"bbbsbbbbsbs", "bbssbssssbs", "bbbbsssbsbs", "bsbssbbssss", "bsbssssbbss", "bssbsbbssss", "bssbssssbbs", "bssssbsbbss", "bssssbssbbs", "bsbbssbssss",
"bsbbssssbss", "bssbbsbssss", "bssbbssssbs", "bssssbbsbss", "bssssbbssbs", "bbssssbssbs", "bbssbsbssss", "bbbbsbbbsbs", "bbssssbsbss", "bsssbbbbsbs",
"bsbssbbbbss", "bssbsbbbbss", "bssbssbbbbs", "bsbbbbssbss", "bssbbbbsbss", "bssbbbbssbs", "bbbbsbssbss", "bbbbssbsbss", "bbbbssbssbs", "bbsbbsbbbbs",
"bbsbbbbsbbs", "bbbbsbbsbbs", "bsbsbbbbsss", "bsbsssbbbbs", "bsssbsbbbbs", "bsbbbbsbsss", "bsbbbbsssbs", "bbbbsbsbsss", "bbbbsbsssbs", "bsbbbsbbbbs",
"bsbbbbsbbbs", "bbbsbsbbbbs", "bbbbsbsbbbs", "bbsbssssbss", "bbsbssbssss", "bbsbssbbbss", "bbsssbbbsbs",
};

static int isnum(char asc)                 { return asc >= '0' && asc <= '9'; }
static int num_to_code128c_id(char num[2]) { return (num[0] - '0') * 10 + (num[1] - '0'); }

static int ascii_to_code128ab_id(char asc, int *id)
{
    if (asc >= 0 && asc < ' ') {
        *id = asc + 64;
        return CODE_ID_CODE_A;
    } else if (asc >= ' ' && asc <= 0x7F) {
        *id = asc - ' ';
        return CODE_ID_CODE_B;
    } else {
        *id = 0; // unsupported
        return CODE_ID_CODE_B;
    }
}

static int append_code(char **code, int *len, int id, int *checksum, int *idxcs)
{
    if (*len < 12) return -1;
    strncat(*code, ID2CODE_TAB[id], 11);
//  printf("%s\n", ID2CODE_TAB[id]);
    *code+= 11; *len -= 11;
    if (*idxcs == 0) *checksum = id;
    else             *checksum+= *idxcs * id;
    (*idxcs)++;
    return 0;
}

int barcode_code128_bufsize(char *str) { return (strlen(str) * 2 + 3) * 11 + 2 + 1; }

int barcode_str2code128(char *str, char *bufcode, int bufsize)
{
    int len = strlen(str), id, checksum, lasttype, curtype, i = 0, j = 0;
    char *temp = bufcode;

    if (!str || !bufcode || bufsize <= 0) return -1;
    *bufcode = '\0';

    if (len >= 2 && isnum(str[0]) && isnum(str[1])) {
        lasttype = CODE_ID_CODE_C ;
        id       = CODE_ID_START_C;
    } else {
        lasttype = ascii_to_code128ab_id(str[0], &id);
        id       = (lasttype == CODE_ID_CODE_A) ? CODE_ID_START_A : CODE_ID_START_B;
    }
    if (append_code(&temp, &bufsize, id, &checksum, &j) != 0) return -1;

    while (i < len) {
        if (i + 1 < len && isnum(str[i]) && isnum(str[i+1])) {
            if (lasttype != CODE_ID_CODE_C) {
                lasttype  = CODE_ID_CODE_C;
                if (append_code(&temp, &bufsize, lasttype, &checksum, &j) != 0) return -1;
            }
            id = num_to_code128c_id(str + i);
            i += 2;
        } else {
            curtype = ascii_to_code128ab_id(str[i], &id);
            if (lasttype != curtype) {
                lasttype  = curtype;
                if (append_code(&temp, &bufsize, lasttype, &checksum, &j) != 0) return -1;
            }
            i += 1;
        }
        if (append_code(&temp, &bufsize, id, &checksum, &j) != 0) return -1;
    }

    checksum %= 103;
    if (append_code(&temp, &bufsize, checksum    , &checksum, &j) != 0) return -1;
    if (append_code(&temp, &bufsize, CODE_ID_STOP, &checksum, &j) != 0) return -1;
    if (bufsize < 3) return -1;
    else strncat(temp, "bb", 2);
    return 0;
}

#ifdef _TEST_
#include "bmpfile.h"

static void bmp_fillrect(BMP *pb, int x, int y, int w, int h, int r, int g, int b)
{
    int i, j;
    for (i=0; i<h; i++) {
        for (j=0; j<w; j++) {
            bmp_setpixel(pb, x+j, y+i, r, g, b);
        }
    }
}

#define BARCODE_LINE_SIZE  3
int main(int argc, char *argv[])
{
    char *str = "hello world !", *buf;
    int   size, w, h, n, i, j;
    BMP   mybmp = {0};

    if (argc > 1) str = argv[1];
//  printf("str: %s\n", str);

    // allocate buffer for barcode encoding
    size = barcode_code128_bufsize(str);
    buf  = malloc(size);

    // do encoding
    barcode_str2code128(str, buf, size);

    // create bmp object according to barcode size
    n = strlen(buf);
    w = (n + 10) * BARCODE_LINE_SIZE;
    h =  w / 6;
    bmp_create(&mybmp, w, h);

    // clear bmp object to white color
    for (i=0; i<h; i++) {
        for (j=0; j<w; j++) {
            bmp_setpixel(&mybmp, j, i, 255, 255, 255);
        }
    }

    // draw line on bmp object
    for (i=0; i<n; i++) {
        int c = (buf[i] == 'b') ? 0 : 255;
        bmp_fillrect(&mybmp, (5 + i) * BARCODE_LINE_SIZE, h / 10, BARCODE_LINE_SIZE, h - (h / 10) * 2, c, c, c);
    }

    bmp_save(&mybmp, "barcode.bmp");
    bmp_free(&mybmp);
//  printf("%s\n", buf);
    free(buf);
    return 0;
}
#endif
