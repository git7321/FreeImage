/* APNG Disassembler 2.5
 * Copyright (C) Max Stepin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

unsigned short swap16(unsigned short data) {return((data & 0xFF) << 8) | ((data >> 8) & 0xFF);}
unsigned int swap32(unsigned int data) {return((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | ((data >> 8) & 0xFF00) | ((data >> 24) & 0xFF);}

#define PNG_ZBUF_SIZE 32768

#define PNG_DISPOSE_OP_NONE 0x00
#define PNG_DISPOSE_OP_BACKGROUND 0x01
#define PNG_DISPOSE_OP_PREVIOUS 0x02

#define PNG_BLEND_OP_SOURCE 0x00
#define PNG_BLEND_OP_OVER 0x01

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

#define ROWBYTES(pixel_bits, width) \
 ((pixel_bits) >= 8 ? \
 ((width) * (((unsigned int)(pixel_bits)) >> 3)) : \
 (( ((width) * ((unsigned int)(pixel_bits))) + 7) >> 3) )

unsigned char png_sign[8] = {137, 80, 78, 71, 13, 10, 26, 10};
int mask4[2]={240,15};
int shift4[2]={4,0};
int mask2[4]={192,48,12,3};
int shift2[4]={6,4,2,0};
int mask1[8]={128,64,32,16,8,4,2,1};
int shift1[8]={7,6,5,4,3,2,1,0};
unsigned int keep_original = 1;
unsigned char pal[256][3];
unsigned char trns[256];
unsigned int palsize, trnssize;
unsigned short trns1, trns2, trns3;
int trns_idx;

int read32(unsigned int *val, FILE * f1)
{
 unsigned int res;
 if (fread(&res, 1, 4, f1) != 4) return 1;
 *val = swap32(res);
 return 0;
}

int read16(unsigned short *val, FILE * f1)
{
 unsigned short res;
 if (fread(&res, 1, 2, f1) != 2) return 1;
 *val = swap16(res);
 return 0;
}

unsigned short readshort(unsigned char * p)
{
 return ((unsigned short)(*p)<<8)+(unsigned short)(*(p+1));
}

void read_sub_row(unsigned char * row, unsigned int rowbytes, unsigned int bpp)
{
 unsigned int i;

 for (i=bpp; i<rowbytes; i++)
 row[i] += row[i-bpp];
}

void read_up_row(unsigned char * row, unsigned char * prev_row, unsigned int rowbytes, unsigned int bpp)
{
 unsigned int i;

 if (prev_row)
 for (i=0; i<rowbytes; i++)
 row[i] += prev_row[i];
}

void read_average_row(unsigned char * row, unsigned char * prev_row, unsigned int rowbytes, unsigned int bpp)
{
 unsigned int i;

 if (prev_row)
 {
 for (i=0; i<bpp; i++)
 row[i] += prev_row[i]>>1;
 for (i=bpp; i<rowbytes; i++)
 row[i] += (prev_row[i] + row[i-bpp])>>1;
 }
 else
 {
 for (i=bpp; i<rowbytes; i++)
 row[i] += row[i-bpp]>>1;
 }
}

void read_paeth_row(unsigned char * row, unsigned char * prev_row, unsigned int rowbytes, unsigned int bpp)
{
 unsigned int i;
 int a, b, c, pa, pb, pc, p;

 if (prev_row)
 {
 for (i=0; i<bpp; i++)
 row[i] += prev_row[i];
 for (i=bpp; i<rowbytes; i++)
 {
 a = row[i-bpp];
 b = prev_row[i];
 c = prev_row[i-bpp];
 p = b - c;
 pc = a - c;
 pa = abs(p);
 pb = abs(pc);
 pc = abs(p + pc);
 row[i] += ((pa <= pb && pa <= pc) ? a : (pb <= pc) ? b : c);
 }
 }
 else
 {
 for (i=bpp; i<rowbytes; i++)
 row[i] += row[i-bpp];
 }
}

void unpack(z_stream zstream, unsigned char * dst, unsigned int dst_size, unsigned char * src, unsigned int src_size, unsigned int h, unsigned int rowbytes, unsigned char bpp)
{
 unsigned int j;
 unsigned char * row = dst;
 unsigned char * prev_row = NULL;

 zstream.next_out = dst;
 zstream.avail_out = dst_size;
 zstream.next_in = src;
 zstream.avail_in = src_size;
 inflate(&zstream, Z_FINISH);
 inflateReset(&zstream);

 for (j=0; j<h; j++)
 {
 switch (*row++)
 {
 case 0: break;
 case 1: read_sub_row(row, rowbytes, bpp); break;
 case 2: read_up_row(row, prev_row, rowbytes, bpp); break;
 case 3: read_average_row(row, prev_row, rowbytes, bpp); break;
 case 4: read_paeth_row(row, prev_row, rowbytes, bpp); break;
 }
 prev_row = row;
 row += rowbytes;
 }
}

void compose0(unsigned char * dst1, unsigned int dstbytes1, unsigned char * dst2, unsigned int dstbytes2, unsigned char * src, unsigned int srcbytes, unsigned int w, unsigned int h, unsigned int bop, unsigned char depth)
{
 unsigned int i, j, g, a;
 unsigned char * sp;
 unsigned char * dp1;
 unsigned int * dp2;

 for (j=0; j<h; j++)
 {
 sp = src+1;
 dp1 = dst1;
 dp2 = (unsigned int*)dst2;

 if (bop == PNG_BLEND_OP_SOURCE)
 {
 switch (depth)
 {
 case 16: for (i=0; i<w; i++) { a = 0xFF; if (trnssize && readshort(sp)==trns1) a = 0; *dp1++ = *sp; *dp2++ = (a << 24) + (*sp << 16) + (*sp << 8) + *sp; sp+=2; } break;
 case 8: for (i=0; i<w; i++) { a = 0xFF; if (trnssize && *sp==trns1) a = 0; *dp1++ = *sp; *dp2++ = (a << 24) + (*sp << 16) + (*sp << 8) + *sp; sp++; } break;
 case 4: for (i=0; i<w; i++) { g = (sp[i>>1] & mask4[i&1]) >> shift4[i&1]; a = 0xFF; if (trnssize && g==trns1) a = 0; *dp1++ = g*0x11; *dp2++ = (a<<24) + g*0x111111; } break;
 case 2: for (i=0; i<w; i++) { g = (sp[i>>2] & mask2[i&3]) >> shift2[i&3]; a = 0xFF; if (trnssize && g==trns1) a = 0; *dp1++ = g*0x55; *dp2++ = (a<<24) + g*0x555555; } break;
 case 1: for (i=0; i<w; i++) { g = (sp[i>>3] & mask1[i&7]) >> shift1[i&7]; a = 0xFF; if (trnssize && g==trns1) a = 0; *dp1++ = g*0xFF; *dp2++ = (a<<24) + g*0xFFFFFF; } break;
 }
 }
 else
 {
 switch (depth)
 {
 case 16: for (i=0; i<w; i++, dp1++, dp2++) { if (readshort(sp) != trns1) { *dp1 = *sp; *dp2 = 0xFF000000 + (*sp << 16) + (*sp << 8) + *sp; } sp+=2; } break;
 case 8: for (i=0; i<w; i++, dp1++, dp2++) { if (*sp != trns1) { *dp1 = *sp; *dp2 = 0xFF000000 + (*sp << 16) + (*sp << 8) + *sp; } sp++; } break;
 case 4: for (i=0; i<w; i++, dp1++, dp2++) { g = (sp[i>>1] & mask4[i&1]) >> shift4[i&1]; if (g != trns1) { *dp1 = g*0x11; *dp2 = 0xFF000000+g*0x111111; } } break;
 case 2: for (i=0; i<w; i++, dp1++, dp2++) { g = (sp[i>>2] & mask2[i&3]) >> shift2[i&3]; if (g != trns1) { *dp1 = g*0x55; *dp2 = 0xFF000000+g*0x555555; } } break;
 case 1: for (i=0; i<w; i++, dp1++, dp2++) { g = (sp[i>>3] & mask1[i&7]) >> shift1[i&7]; if (g != trns1) { *dp1 = g*0xFF; *dp2 = 0xFF000000+g*0xFFFFFF; } } break;
 }
 }

 src += srcbytes;
 dst1 += dstbytes1;
 dst2 += dstbytes2;
 }
}

void compose2(unsigned char * dst1, unsigned int dstbytes1, unsigned char * dst2, unsigned int dstbytes2, unsigned char * src, unsigned int srcbytes, unsigned int w, unsigned int h, unsigned int bop, unsigned char depth)
{
 unsigned int i, j;
 unsigned int r, g, b, a;
 unsigned char * sp;
 unsigned char * dp1;
 unsigned int * dp2;

 for (j=0; j<h; j++)
 {
 sp = src+1;
 dp1 = dst1;
 dp2 = (unsigned int*)dst2;

 if (bop == PNG_BLEND_OP_SOURCE)
 {
 if (depth == 8)
 {
 for (i=0; i<w; i++)
 {
 r = *sp++;
 g = *sp++;
 b = *sp++;
 a = 0xFF;
 if (trnssize && r==trns1 && g==trns2 && b==trns3)
 a = 0;
 *dp1++ = r; *dp1++ = g; *dp1++ = b;
 *dp2++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 }
 else
 {
 for (i=0; i<w; i++, sp+=6)
 {
 r = *sp;
 g = *(sp+2);
 b = *(sp+4);
 a = 0xFF;
 if (trnssize && readshort(sp)==trns1 && readshort(sp+2)==trns2 && readshort(sp+4)==trns3)
 a = 0;
 *dp1++ = r; *dp1++ = g; *dp1++ = b;
 *dp2++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 }
 }
 else
 {
 if (depth == 8)
 {
 for (i=0; i<w; i++, sp+=3, dp1+=3, dp2++)
 if ((*sp != trns1) || (*(sp+1) != trns2) || (*(sp+2) != trns3))
 {
 *dp1 = *sp; *(dp1+1) = *(sp+1); *(dp1+2) = *(sp+2);
 *dp2 = 0xFF000000 + (*(sp+2) << 16) + (*(sp+1) << 8) + *sp;
 }
 }
 else
 {
 for (i=0; i<w; i++, sp+=6, dp1+=3, dp2++)
 if ((readshort(sp) != trns1) || (readshort(sp+2) != trns2) || (readshort(sp+4) != trns3))
 {
 *dp1 = *sp; *(dp1+1) = *(sp+2); *(dp1+2) = *(sp+4);
 *dp2 = 0xFF000000 + (*(sp+4) << 16) + (*(sp+2) << 8) + *sp;
 }
 }
 }
 src += srcbytes;
 dst1 += dstbytes1;
 dst2 += dstbytes2;
 }
}

void compose3(unsigned char * dst1, unsigned int dstbytes1, unsigned char * dst2, unsigned int dstbytes2, unsigned char * src, unsigned int srcbytes, unsigned int w, unsigned int h, unsigned int bop, unsigned char depth)
{
 unsigned int i, j;
 unsigned int r, g, b, a;
 unsigned int r2, g2, b2, a2;
 int u, v, al;
 unsigned char col;
 unsigned char * sp;
 unsigned char * dp1;
 unsigned int * dp2;

 for (j=0; j<h; j++)
 {
 sp = src+1;
 dp1 = dst1;
 dp2 = (unsigned int*)dst2;

 for (i=0; i<w; i++)
 {
 switch (depth)
 {
 case 1: col = (sp[i>>3] & mask1[i&7]) >> shift1[i&7]; break;
 case 2: col = (sp[i>>2] & mask2[i&3]) >> shift2[i&3]; break;
 case 4: col = (sp[i>>1] & mask4[i&1]) >> shift4[i&1]; break;
 default: col = sp[i];
 }

 r = pal[col][0];
 g = pal[col][1];
 b = pal[col][2];
 a = trns[col];

 if (bop == PNG_BLEND_OP_SOURCE)
 {
 *dp1++ = col;
 *dp2++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 else
 {
 if (a == 255)
 {
 *dp1++ = col;
 *dp2++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 else
 if (a != 0)
 {
 if ((a2 = (*dp2)>>24) != 0)
 {
 keep_original = 0;
 u = a*255;
 v = (255-a)*a2;
 al = 255*255-(255-a)*(255-a2);
 r2 = ((*dp2)&255);
 g2 = (((*dp2)>>8)&255);
 b2 = (((*dp2)>>16)&255);
 r = (r*u + r2*v)/al;
 g = (g*u + g2*v)/al;
 b = (b*u + b2*v)/al;
 a = al/255;
 }
 *dp1++ = col;
 *dp2++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 else
 {
 dp1++;
 dp2++;
 }
 }
 }
 src += srcbytes;
 dst1 += dstbytes1;
 dst2 += dstbytes2;
 }
}

void compose4(unsigned char * dst, unsigned int dstbytes, unsigned char * src, unsigned int srcbytes, unsigned int w, unsigned int h, unsigned int bop, unsigned char depth)
{
 unsigned int i, j, step;
 unsigned int g, a, g2, a2;
 int u, v, al;
 unsigned char * sp;
 unsigned char * dp;

 step = (depth+7)/8;

 for (j=0; j<h; j++)
 {
 sp = src+1;
 dp = dst;

 if (bop == PNG_BLEND_OP_SOURCE)
 {
 for (i=0; i<w; i++)
 {
 g = *sp; sp += step;
 a = *sp; sp += step;
 *dp++ = g;
 *dp++ = a;
 }
 }
 else
 {
 for (i=0; i<w; i++)
 {
 g = *sp; sp += step;
 a = *sp; sp += step;
 if (a == 255)
 {
 *dp++ = g;
 *dp++ = a;
 }
 else
 if (a != 0)
 {
 if ((a2 = *(dp+1)) != 0)
 {
 u = a*255;
 v = (255-a)*a2;
 al = 255*255-(255-a)*(255-a2);
 g2 = ((*dp)&255);
 g = (g*u + g2*v)/al;
 a = al/255;
 }
 *dp++ = g;
 *dp++ = a;
 }
 else
 dp+=2;
 }
 }
 src += srcbytes;
 dst += dstbytes;
 }
}

void compose6(unsigned char * dst, unsigned int dstbytes, unsigned char * src, unsigned int srcbytes, unsigned int w, unsigned int h, unsigned int bop, unsigned char depth)
{
 unsigned int i, j, step;
 unsigned int r, g, b, a;
 unsigned int r2, g2, b2, a2;
 int u, v, al;
 unsigned char * sp;
 unsigned int * dp;

 step = (depth+7)/8;

 for (j=0; j<h; j++)
 {
 sp = src+1;
 dp = (unsigned int*)dst;

 if (bop == PNG_BLEND_OP_SOURCE)
 {
 for (i=0; i<w; i++)
 {
 r = *sp; sp += step;
 g = *sp; sp += step;
 b = *sp; sp += step;
 a = *sp; sp += step;
 *dp++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 }
 else
 {
 for (i=0; i<w; i++)
 {
 r = *sp; sp += step;
 g = *sp; sp += step;
 b = *sp; sp += step;
 a = *sp; sp += step;
 if (a == 255)
 *dp++ = (a << 24) + (b << 16) + (g << 8) + r;
 else
 if (a != 0)
 {
 if ((a2 = (*dp)>>24) != 0)
 {
 u = a*255;
 v = (255-a)*a2;
 al = 255*255-(255-a)*(255-a2);
 r2 = ((*dp)&255);
 g2 = (((*dp)>>8)&255);
 b2 = (((*dp)>>16)&255);
 r = (r*u + r2*v)/al;
 g = (g*u + g2*v)/al;
 b = (b*u + b2*v)/al;
 a = al/255;
 }
 *dp++ = (a << 24) + (b << 16) + (g << 8) + r;
 }
 else
 dp++;
 }
 }
 src += srcbytes;
 dst += dstbytes;
 }
}

int LoadAPNG(char * szIn, unsigned int *pWidth, unsigned int *pHeight, unsigned int *pChannels, unsigned char *pColType, unsigned int *pFirst, unsigned int *pLast, unsigned char **ppOut1, unsigned char **ppOut2, unsigned short **ppDelays)
{
 unsigned char sig[8];
 unsigned int i, j;
 unsigned int len, chunk, seq, crc;
 unsigned int w, h, w0, h0, x0, y0;
 unsigned char depth, coltype, compr, filter, interl;
 unsigned char pixeldepth, bpp;
 unsigned int rowbytes, frames, loops, first_visible, cur_frame, channels;
 unsigned int outrow1, outrow2, outimg1, outimg2;
 unsigned short d1, d2;
 unsigned char c, dop, bop;
 int imagesize, zbuf_size, zsize;
 z_stream zstream;
 unsigned char * pOut1;
 unsigned char * pOut2;
 unsigned char * pTemp;
 unsigned char * pData;
 unsigned char * pImg1;
 unsigned char * pImg2;
 unsigned char * pDst1;
 unsigned char * pDst2;
 unsigned short * pDelays;
 FILE * f1;
 int res = 1;

 if ((f1 = fopen(szIn, "rb")) == 0)
 {
 printf("Error: can't open '%s'\n", szIn);
 return res;
 }

 printf("Reading '%s'...\n", szIn);

 frames = 0;
 loops = 0;
 first_visible = 0;
 cur_frame = 0;
 zsize = 0;
 x0 = 0;
 y0 = 0;
 bop = PNG_BLEND_OP_SOURCE;
 palsize = 0;
 trnssize = 0;
 trns_idx = -1;

 memset(trns, 255, 256);

 zstream.zalloc = Z_NULL;
 zstream.zfree = Z_NULL;
 zstream.opaque = Z_NULL;
 inflateInit(&zstream);

 do
 {
 if (fread(sig, 1, 8, f1) != 8)
 {
 printf("Error: can't read the sig\n");
 break;
 }
 if (memcmp(sig, png_sign, 8) != 0)
 {
 printf("Error: wrong PNG sig\n");
 break;
 }

 if (read32(&len, f1)) break;
 if (read32(&chunk, f1)) break;

 if (len != 13 || chunk != 0x49484452)
 {
 printf("Error: missing IHDR\n");
 break;
 }

 if (read32(&w, f1)) break;
 if (read32(&h, f1)) break;
 w0 = w;
 h0 = h;
 if (fread(&depth, 1, 1, f1) != 1) break;
 if (fread(&coltype, 1, 1, f1) != 1) break;
 if (fread(&compr, 1, 1, f1) != 1) break;
 if (fread(&filter, 1, 1, f1) != 1) break;
 if (fread(&interl, 1, 1, f1) != 1) break;
 if (read32(&crc, f1)) break;

 channels = 1;
 if (coltype == 2)
 channels = 3;
 else
 if (coltype == 4)
 channels = 2;
 else
 if (coltype == 6)
 channels = 4;

 pixeldepth = depth*channels;
 bpp = (pixeldepth + 7) >> 3;
 rowbytes = ROWBYTES(pixeldepth, w);

 imagesize = (rowbytes + 1) * h;
 zbuf_size = imagesize + ((imagesize + 7) >> 3) + ((imagesize + 63) >> 6) + 11;

 outrow1 = w*channels;
 outrow2 = w*4;
 outimg1 = h*outrow1;
 outimg2 = h*outrow2;

 pOut1 = (unsigned char *)malloc((frames+1)*outimg1);
 pOut2 = (unsigned char *)malloc((frames+1)*outimg2);
 pDelays = (unsigned short *)malloc((frames+1)*4);
 pTemp = (unsigned char *)malloc(imagesize);
 pData = (unsigned char *)malloc(zbuf_size);
 if (!pOut1 || !pOut2 || !pDelays || !pTemp || !pData)
 {
 printf("Error: not enough memory\n");
 break;
 }

 pImg1 = pOut1;
 pImg2 = pOut2;
 memset(pOut1, 0, outimg1);
 memset(pOut2, 0, outimg2);
 pDelays[0] = 0;
 pDelays[1] = 0;

 while ( !feof(f1) )
 {
 if (read32(&len, f1)) break;
 if (read32(&chunk, f1)) break;

 if (chunk == 0x504C5445)
 {
 unsigned int col;
 for (i=0; i<len; i++)
 {
 if (fread(&c, 1, 1, f1) != 1) break;
 col = i/3;
 if (col<256)
 {
 pal[col][i%3] = c;
 palsize = col+1;
 }
 }
 if (read32(&crc, f1)) break;
 }
 else
 if (chunk == 0x74524E53)
 {
 for (i=0; i<len; i++)
 {
 if (fread(&c, 1, 1, f1) != 1) break;
 if (i<256)
 {
 trns[i] = c;
 trnssize = i+1;
 if (c == 0 && coltype == 3 && trns_idx == -1)
 trns_idx = i;
 }
 }
 if (coltype == 0)
 {
 if (trnssize == 2)
 {
 trns1 = readshort(&trns[0]);
 switch (depth)
 {
 case 16: trns[1] = trns[0]; trns[0] = 0; break;
 case 4: trns[1] *= 0x11; break;
 case 2: trns[1] *= 0x55; break;
 case 1: trns[1] *= 0xFF; break;
 }
 trns_idx = trns[1];
 }
 else
 trnssize = 0;
 }
 else
 if (coltype == 2)
 {
 if (trnssize == 6)
 {
 trns1 = readshort(&trns[0]);
 trns2 = readshort(&trns[2]);
 trns3 = readshort(&trns[4]);
 if (depth == 16)
 {
 trns[1] = trns[0]; trns[0] = 0;
 trns[3] = trns[2]; trns[2] = 0;
 trns[5] = trns[4]; trns[4] = 0;
 }
 }
 else
 trnssize = 0;
 }
 if (read32(&crc, f1)) break;
 }
 else
 if (chunk == 0x6163544C)
 {
 if (read32(&frames, f1)) break;
 if (read32(&loops, f1)) break;
 if (read32(&crc, f1)) break;
 free(pOut1);
 free(pOut2);
 free(pDelays);
 pOut1 = (unsigned char *)malloc((frames+1)*outimg1);
 pOut2 = (unsigned char *)malloc((frames+1)*outimg2);
 pDelays = (unsigned short *)malloc((frames+1)*4);
 pImg1 = pOut1;
 pImg2 = pOut2;
 memset(pOut1, 0, outimg1);
 memset(pOut2, 0, outimg2);
 }
 else
 if (chunk == 0x6663544C)
 {
 if (zsize == 0)
 first_visible = 1;
 else
 {
 if (dop == PNG_DISPOSE_OP_PREVIOUS)
 {
 if (coltype != 6)
 memcpy(pImg1 + outimg1, pImg1, outimg1);
 if (coltype != 4)
 memcpy(pImg2 + outimg2, pImg2, outimg2);
 }

 pDst1 = pImg1 + y0*outrow1 + x0*channels;
 pDst2 = pImg2 + y0*outrow2 + x0*4;
 unpack(zstream, pTemp, imagesize, pData, zsize, h0, rowbytes, bpp);
 switch (coltype)
 {
 case 0: compose0(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 2: compose2(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 3: compose3(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 4: compose4(pDst1, outrow1, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 6: compose6( pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 }
 zsize = 0;

 if (dop != PNG_DISPOSE_OP_PREVIOUS)
 {
 if (coltype != 6)
 memcpy(pImg1 + outimg1, pImg1, outimg1);
 if (coltype != 4)
 memcpy(pImg2 + outimg2, pImg2, outimg2);

 if (dop == PNG_DISPOSE_OP_BACKGROUND)
 {
 pDst1 += outimg1;
 pDst2 += outimg2;

 for (j=0; j<h0; j++)
 {
 switch (coltype)
 {
 case 0: memset(pDst2, 0, w0*4); if (trnssize) memset(pDst1, trns_idx, w0); else keep_original = 0; break;
 case 2: memset(pDst2, 0, w0*4); if (trnssize) for (i=0; i<w0; i++) { pDst1[i*3] = trns[1]; pDst1[i*3+1] = trns[3]; pDst1[i*3+2] = trns[5]; } else keep_original = 0; break;
 case 3: memset(pDst2, 0, w0*4); if (trns_idx >= 0) memset(pDst1, trns_idx, w0); else keep_original = 0; break;
 case 4: memset(pDst1, 0, w0*2); break;
 case 6: memset(pDst2, 0, w0*4); break;
 }
 pDst1 += outrow1;
 pDst2 += outrow2;
 }
 }
 }
 }

 pImg1 += outimg1;
 pImg2 += outimg2;

 if (read32(&seq, f1)) break;
 if (read32(&w0, f1)) break;
 if (read32(&h0, f1)) break;
 if (read32(&x0, f1)) break;
 if (read32(&y0, f1)) break;
 if (read16(&d1, f1)) break;
 if (read16(&d2, f1)) break;
 if (fread(&dop, 1, 1, f1) != 1) break;
 if (fread(&bop, 1, 1, f1) != 1) break;
 if (read32(&crc, f1)) break;

 if (cur_frame == 0)
 {
 bop = PNG_BLEND_OP_SOURCE;
 if (dop == PNG_DISPOSE_OP_PREVIOUS)
 dop = PNG_DISPOSE_OP_BACKGROUND;
 }

 if (coltype<=3 && trnssize==0)
 bop = PNG_BLEND_OP_SOURCE;

 rowbytes = ROWBYTES(pixeldepth, w0);

 cur_frame++;
 pDelays[cur_frame*2] = d1;
 pDelays[cur_frame*2+1] = d2;
 }
 else
 if (chunk == 0x49444154)
 {
 if (fread(pData + zsize, 1, len, f1) != len) break;
 zsize += len;
 if (read32(&crc, f1)) break;
 }
 else
 if (chunk == 0x66644154)
 {
 if (read32(&seq, f1)) break;
 len -= 4;
 if (fread(pData + zsize, 1, len, f1) != len) break;
 zsize += len;
 if (read32(&crc, f1)) break;
 }
 else
 if (chunk == 0x49454E44)
 {
 pDst1 = pImg1 + y0*outrow1 + x0*channels;
 pDst2 = pImg2 + y0*outrow2 + x0*4;
 unpack(zstream, pTemp, imagesize, pData, zsize, h0, rowbytes, bpp);
 switch (coltype)
 {
 case 0: compose0(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 2: compose2(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 3: compose3(pDst1, outrow1, pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 4: compose4(pDst1, outrow1, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 case 6: compose6( pDst2, outrow2, pTemp, rowbytes+1, w0, h0, bop, depth); break;
 }

 if (cur_frame == frames)
 res = 0;

 break;
 }
 else
 {
 c = (unsigned char)(chunk>>24);
 if (notabc(c)) break;
 c = (unsigned char)((chunk>>16) & 0xFF);
 if (notabc(c)) break;
 c = (unsigned char)((chunk>>8) & 0xFF);
 if (notabc(c)) break;
 c = (unsigned char)(chunk & 0xFF);
 if (notabc(c)) break;

 fseek(f1, len, SEEK_CUR);
 if (read32(&crc, f1)) break;
 }
 }

 *pWidth = w;
 *pHeight = h;
 *pChannels = channels;
 *pColType = coltype;
 *pFirst = first_visible;
 *pLast = cur_frame;
 *ppOut1 = pOut1;
 *ppOut2 = pOut2;
 *ppDelays = pDelays;

 free(pData);
 free(pTemp);

 } while (0);

 inflateEnd(&zstream);
 fclose(f1);
 return res;
}

void write_chunk(FILE * f, const char * name, unsigned char * data, unsigned int length)
{
 unsigned int crc = crc32(0, Z_NULL, 0);
 unsigned int len = swap32(length);

 fwrite(&len, 1, 4, f);
 fwrite(name, 1, 4, f);
 crc = crc32(crc, (const Bytef *)name, 4);

 if (data != NULL && length > 0)
 {
 fwrite(data, 1, length, f);
 crc = crc32(crc, data, length);
 }

 crc = swap32(crc);
 fwrite(&crc, 1, 4, f);
}

void write_IDATs(FILE * f, unsigned char * data, unsigned int length, unsigned int idat_size)
{
 unsigned int z_cmf = data[0];
 if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
 {
 if (length >= 2)
 {
 unsigned int z_cinfo = z_cmf >> 4;
 unsigned int half_z_window_size = 1 << (z_cinfo + 7);
 while (idat_size <= half_z_window_size && half_z_window_size >= 256)
 {
 z_cinfo--;
 half_z_window_size >>= 1;
 }
 z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);
 if (data[0] != (unsigned char)z_cmf)
 {
 data[0] = (unsigned char)z_cmf;
 data[1] &= 0xe0;
 data[1] += (unsigned char)(0x1f - ((z_cmf << 8) + data[1]) % 0x1f);
 }
 }
 }

 while (length > 0)
 {
 unsigned int ds = length;
 if (ds > PNG_ZBUF_SIZE)
 ds = PNG_ZBUF_SIZE;

 write_chunk(f, "IDAT", data, ds);

 data += ds;
 length -= ds;
 }
}

void SavePNG(char * szPath, unsigned char * pdata, unsigned short * delays, unsigned int w, unsigned int h, unsigned int first, unsigned int last, unsigned int bpp, unsigned char coltype)
{
 struct IHDR
 {
 unsigned int mWidth;
 unsigned int mHeight;
 unsigned char mDepth;
 unsigned char mColorType;
 unsigned char mCompression;
 unsigned char mFilterMethod;
 unsigned char mInterlaceMethod;
 } ihdr = { swap32(w), swap32(h), 8, coltype, 0, 0, 0 };

 char szOut[256];

 z_stream zstream1;
 z_stream zstream2;
 FILE * f;
 unsigned int i, j, n, len;

 unsigned int rowbytes = w * bpp;
 unsigned int idat_size = (rowbytes + 1) * h;
 unsigned int zbuf_size = idat_size + ((idat_size + 7) >> 3) + ((idat_size + 63) >> 6) + 11;

 unsigned char * row_buf = (unsigned char *)malloc(rowbytes + 1);
 unsigned char * sub_row = (unsigned char *)malloc(rowbytes + 1);
 unsigned char * up_row = (unsigned char *)malloc(rowbytes + 1);
 unsigned char * avg_row = (unsigned char *)malloc(rowbytes + 1);
 unsigned char * paeth_row = (unsigned char *)malloc(rowbytes + 1);
 unsigned char * zbuf1 = (unsigned char *)malloc(zbuf_size);
 unsigned char * zbuf2 = (unsigned char *)malloc(zbuf_size);

 if (!row_buf || !sub_row || !up_row || !avg_row || !paeth_row || !zbuf1 || !zbuf2)
 return;

 row_buf[0] = 0;
 sub_row[0] = 1;
 up_row[0] = 2;
 avg_row[0] = 3;
 paeth_row[0] = 4;

 zstream1.data_type = Z_BINARY;
 zstream1.zalloc = Z_NULL;
 zstream1.zfree = Z_NULL;
 zstream1.opaque = Z_NULL;
 deflateInit2(&zstream1, Z_BEST_COMPRESSION, 8, 15, 8, Z_DEFAULT_STRATEGY);

 zstream2.data_type = Z_BINARY;
 zstream2.zalloc = Z_NULL;
 zstream2.zfree = Z_NULL;
 zstream2.opaque = Z_NULL;
 deflateInit2(&zstream2, Z_BEST_COMPRESSION, 8, 15, 8, Z_FILTERED);

 len = sprintf(szOut, "%d", last);

 for (n=first; n<=last; n++)
 {
 printf("extracting frame %d of %d\n", n, last);

 if (n > 0)
 {
 sprintf(szOut, "%s%.*d.txt", szPath, len, n);
 if ((f = fopen(szOut, "wt")) != 0)
 {
 fprintf(f, "delay=%d/%d\n", delays[n*2], delays[n*2+1]);
 fclose(f);
 }
 }

 sprintf(szOut, "%s%.*d.png", szPath, len, n);
 if ((f = fopen(szOut, "wb")) != 0)
 {
 int a, b, c, pa, pb, pc, p, v;
 unsigned char * prev;
 unsigned char * row;

 fwrite(png_sign, 1, 8, f);
 write_chunk(f, "IHDR", (unsigned char *)(&ihdr), 13);

 if (palsize > 0)
 write_chunk(f, "PLTE", (unsigned char *)(&pal), palsize*3);

 if (trnssize > 0)
 write_chunk(f, "tRNS", trns, trnssize);

 zstream1.next_out = zbuf1;
 zstream1.avail_out = zbuf_size;
 zstream2.next_out = zbuf2;
 zstream2.avail_out = zbuf_size;

 prev = NULL;
 row = pdata + n*h*rowbytes;

 for (j=0; j<h; j++)
 {
 unsigned char * out;
 unsigned int sum = 0;
 unsigned char * best_row = row_buf;
 unsigned int mins = ((unsigned int)(-1)) >> 1;

 out = row_buf+1;
 for (i=0; i<rowbytes; i++)
 {
 v = out[i] = row[i];
 sum += (v < 128) ? v : 256 - v;
 }
 mins = sum;

 sum = 0;
 out = sub_row+1;
 for (i=0; i<bpp; i++)
 {
 v = out[i] = row[i];
 sum += (v < 128) ? v : 256 - v;
 }
 for (i=bpp; i<rowbytes; i++)
 {
 v = out[i] = row[i] - row[i-bpp];
 sum += (v < 128) ? v : 256 - v;
 if (sum > mins) break;
 }
 if (sum < mins)
 {
 mins = sum;
 best_row = sub_row;
 }

 if (prev)
 {
 sum = 0;
 out = up_row+1;
 for (i=0; i<rowbytes; i++)
 {
 v = out[i] = row[i] - prev[i];
 sum += (v < 128) ? v : 256 - v;
 if (sum > mins) break;
 }
 if (sum < mins)
 {
 mins = sum;
 best_row = up_row;
 }

 sum = 0;
 out = avg_row+1;
 for (i=0; i<bpp; i++)
 {
 v = out[i] = row[i] - prev[i]/2;
 sum += (v < 128) ? v : 256 - v;
 }
 for (i=bpp; i<rowbytes; i++)
 {
 v = out[i] = row[i] - (prev[i] + row[i-bpp])/2;
 sum += (v < 128) ? v : 256 - v;
 if (sum > mins) break;
 }
 if (sum < mins)
 {
 mins = sum;
 best_row = avg_row;
 }

 sum = 0;
 out = paeth_row+1;
 for (i=0; i<bpp; i++)
 {
 v = out[i] = row[i] - prev[i];
 sum += (v < 128) ? v : 256 - v;
 }
 for (i=bpp; i<rowbytes; i++)
 {
 a = row[i-bpp];
 b = prev[i];
 c = prev[i-bpp];
 p = b - c;
 pc = a - c;
 pa = abs(p);
 pb = abs(pc);
 pc = abs(p + pc);
 p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;
 v = out[i] = row[i] - p;
 sum += (v < 128) ? v : 256 - v;
 if (sum > mins) break;
 }
 if (sum < mins)
 {
 best_row = paeth_row;
 }
 }
 zstream1.next_in = row_buf;
 zstream1.avail_in = rowbytes + 1;
 deflate(&zstream1, Z_NO_FLUSH);

 zstream2.next_in = best_row;
 zstream2.avail_in = rowbytes + 1;
 deflate(&zstream2, Z_NO_FLUSH);

 prev = row;
 row += rowbytes;
 }
 deflate(&zstream1, Z_FINISH);
 deflate(&zstream2, Z_FINISH);

 if (zstream1.total_out <= zstream2.total_out)
 write_IDATs(f, zbuf1, zstream1.total_out, idat_size);
 else
 write_IDATs(f, zbuf2, zstream2.total_out, idat_size);

 deflateReset(&zstream1);
 zstream1.data_type = Z_BINARY;
 deflateReset(&zstream2);
 zstream2.data_type = Z_BINARY;

 write_chunk(f, "IEND", 0, 0);
 fclose(f);
 }
 else
 printf("Error: can't open the file '%s'\n", szOut);
 }

 deflateEnd(&zstream1);
 deflateEnd(&zstream2);
 free(zbuf1);
 free(zbuf2);
 free(row_buf);
 free(sub_row);
 free(up_row);
 free(avg_row);
 free(paeth_row);
}

int __cdecl main(int argc, char** argv)
{
 char * szIn;
 char * szImg;
 char szOut[256];
 unsigned int w, h, first, last, channels, i, j;
 unsigned char coltype;
 unsigned char * pOut1 = NULL;
 unsigned char * pOut2 = NULL;
 unsigned short * pDelays = NULL;

 if (argc > 1)
 szIn = argv[1];
 else
 {
 printf("APNG Disassembler 2.5\n\n"
 "Usage: apngdis apng.png [name]\n");
 return 1;
 }

 if (LoadAPNG(szIn, &w, &h, &channels, &coltype, &first, &last, &pOut1, &pOut2, &pDelays) != 0)
 {
 printf("Error: can't load '%s'\n", szIn);
 return 1;
 }

 strcpy(szOut, szIn);
 for (i=j=0; szOut[i]!=0; i++)
 {
 if (szOut[i] == '\\' || szOut[i] == '/' || szOut[i] == ':')
 j = i+1;
 }
 szOut[j] = 0;

 if (argc > 2)
 {
 szImg = argv[2];

 for (i=j=0; szImg[i]!=0; i++)
 {
 if (szImg[i] == '\\' || szImg[i] == '/' || szImg[i] == ':')
 j = i+1;
 if (szImg[i] == '.')
 szImg[i] = 0;
 }
 strcat(szOut, szImg+j);
 }
 else
 strcat(szOut, "frame");

 if (coltype == 6)
 SavePNG(szOut, pOut2, pDelays, w, h, first, last, 4, 6);
 else
 if (coltype == 4)
 SavePNG(szOut, pOut1, pDelays, w, h, first, last, channels, coltype);
 else
 if (keep_original)
 SavePNG(szOut, pOut1, pDelays, w, h, first, last, channels, coltype);
 else
 SavePNG(szOut, pOut2, pDelays, w, h, first, last, 4, 6);

 free(pOut1);
 free(pOut2);
 free(pDelays);

 printf("all done\n");

 return 0;
}
