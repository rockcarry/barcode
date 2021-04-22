#ifndef __BARCODE_H__
#define __BARCODE_H__

int barcode_code128_bufsize(char *str);
int barcode_str2code128(char *str, char *bufcode, int bufsize);

#endif
