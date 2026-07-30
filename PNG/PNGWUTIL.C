/* pngwutil.c - utilities to write a PNG file
 *
 * libpng version 1.5.20
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "png.h"

#ifdef PNG_WRITE_SUPPORTED

#ifdef PNG_WRITE_INT_FUNCTIONS_SUPPORTED
void PNGAPI
png_save_uint_32(png_bytep buf, png_uint_32 i)
{
   buf[0] = (png_byte)((i >> 24) & 0xff);
   buf[1] = (png_byte)((i >> 16) & 0xff);
   buf[2] = (png_byte)((i >> 8) & 0xff);
   buf[3] = (png_byte)(i & 0xff);
}

#ifdef PNG_SAVE_INT_32_SUPPORTED
void PNGAPI
png_save_int_32(png_bytep buf, png_int_32 i)
{
   buf[0] = (png_byte)((i >> 24) & 0xff);
   buf[1] = (png_byte)((i >> 16) & 0xff);
   buf[2] = (png_byte)((i >> 8) & 0xff);
   buf[3] = (png_byte)(i & 0xff);
}
#endif

void PNGAPI
png_save_uint_16(png_bytep buf, unsigned int i)
{
   buf[0] = (png_byte)((i >> 8) & 0xff);
   buf[1] = (png_byte)(i & 0xff);
}
#endif

void PNGAPI
png_write_sig(png_structp png_ptr)
{
   png_byte png_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_SIGNATURE;
#endif

   png_write_data(png_ptr, &png_signature[png_ptr->sig_bytes],
      (png_size_t)(8 - png_ptr->sig_bytes));

   if (png_ptr->sig_bytes < 3)
      png_ptr->mode |= PNG_HAVE_PNG_SIGNATURE;
}

static void PNGAPI
png_write_chunk_header(png_structp png_ptr, png_uint_32 chunk_name,
    png_uint_32 length)
{
   png_byte buf[8];

#if defined(PNG_DEBUG) && (PNG_DEBUG > 0)
   PNG_CSTRING_FROM_CHUNK(buf, chunk_name);
   png_debug2(0, "Writing %s chunk, length = %lu", buf, (unsigned long)length);
#endif

   if (png_ptr == NULL)
      return;

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_HDR;
#endif

   png_save_uint_32(buf, length);
   png_save_uint_32(buf + 4, chunk_name);
   png_write_data(png_ptr, buf, 8);

   png_ptr->chunk_name = chunk_name;
   png_reset_crc(png_ptr);
   png_calculate_crc(png_ptr, buf + 4, 4);

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_DATA;
#endif
}

void PNGAPI
png_write_chunk_start(png_structp png_ptr, png_const_bytep chunk_string,
    png_uint_32 length)
{
   png_write_chunk_header(png_ptr, PNG_CHUNK_FROM_STRING(chunk_string), length);
}

void PNGAPI
png_write_chunk_data(png_structp png_ptr, png_const_bytep data,
    png_size_t length)
{
   if (png_ptr == NULL)
      return;

   if (data != NULL && length > 0)
   {
      png_write_data(png_ptr, data, length);

      png_calculate_crc(png_ptr, data, length);
   }
}

void PNGAPI
png_write_chunk_end(png_structp png_ptr)
{
   png_byte buf[4];

   if (png_ptr == NULL) return;

#ifdef PNG_IO_STATE_SUPPORTED
   png_ptr->io_state = PNG_IO_WRITING | PNG_IO_CHUNK_CRC;
#endif

   png_save_uint_32(buf, png_ptr->crc);

   png_write_data(png_ptr, buf, (png_size_t)4);
}

static void PNGAPI
png_write_complete_chunk(png_structp png_ptr, png_uint_32 chunk_name,
   png_const_bytep data, png_size_t length)
{
   if (png_ptr == NULL)
      return;

   png_write_chunk_header(png_ptr, chunk_name, (png_uint_32)length);
   png_write_chunk_data(png_ptr, data, length);
   png_write_chunk_end(png_ptr);
}

void PNGAPI
png_write_chunk(png_structp png_ptr, png_const_bytep chunk_string,
   png_const_bytep data, png_size_t length)
{
   png_write_complete_chunk(png_ptr, PNG_CHUNK_FROM_STRING(chunk_string), data,
      length);
}

static void PNGAPI
png_zlib_claim(png_structp png_ptr, png_uint_32 state)
{
   if (!(png_ptr->zlib_state & PNG_ZLIB_IN_USE))
   {
      if (png_ptr->zlib_state != state)
      {
         int ret = Z_OK;
         png_const_charp who = "-";

         if (png_ptr->zlib_state != PNG_ZLIB_UNINITIALIZED)
         {
            ret = deflateEnd(&png_ptr->zstream);
            who = "end";
            png_ptr->zlib_state = PNG_ZLIB_UNINITIALIZED;
         }

         if (ret == Z_OK) switch (state)
         {
# ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
            case PNG_ZLIB_FOR_TEXT:
               ret = deflateInit2(&png_ptr->zstream,
                  png_ptr->zlib_text_level, png_ptr->zlib_text_method,
                  png_ptr->zlib_text_window_bits,
                  png_ptr->zlib_text_mem_level, png_ptr->zlib_text_strategy);
               who = "text";
               break;
# endif

            case PNG_ZLIB_FOR_IDAT:
               ret = deflateInit2(&png_ptr->zstream, png_ptr->zlib_level,
                   png_ptr->zlib_method, png_ptr->zlib_window_bits,
                   png_ptr->zlib_mem_level, png_ptr->zlib_strategy);
               who = "IDAT";
               break;

            default:
               ;
         }

         if (ret == Z_OK)
            png_ptr->zlib_state = state;
      }

      png_ptr->zlib_state |= PNG_ZLIB_IN_USE;
   }
}

static void PNGAPI
png_zlib_release(png_structp png_ptr)
{
   if (png_ptr->zlib_state & PNG_ZLIB_IN_USE)
   {
      png_ptr->zlib_state &= ~PNG_ZLIB_IN_USE;

   }
}

#ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED

typedef struct
{
   png_const_bytep input;
   png_size_t input_len;
   int num_output_ptr;
   int max_output_ptr;
   png_bytep *output_ptr;
} compression_state;

static int PNGAPI
png_text_compress(png_structp png_ptr,
    png_const_charp text, png_size_t text_len, int compression,
    compression_state *comp)
{
   int ret;

   comp->num_output_ptr = 0;
   comp->max_output_ptr = 0;
   comp->output_ptr = NULL;
   comp->input = NULL;
   comp->input_len = text_len;

   if (compression == PNG_TEXT_COMPRESSION_NONE)
   {
      comp->input = (png_const_bytep)text;
      return((int)text_len);
   }

   png_zlib_claim(png_ptr, PNG_ZLIB_FOR_TEXT);
   png_ptr->zstream.avail_in = (uInt)text_len;

   png_ptr->zstream.next_in = (Bytef *)text;
   png_ptr->zstream.avail_out = png_ptr->zbuf_size;
   png_ptr->zstream.next_out = png_ptr->zbuf;

   do
   {
      ret = deflate(&png_ptr->zstream, Z_NO_FLUSH);

      if (!(png_ptr->zstream.avail_out))
      {
         if (comp->num_output_ptr >= comp->max_output_ptr)
         {
            int old_max;

            old_max = comp->max_output_ptr;
            comp->max_output_ptr = comp->num_output_ptr + 4;
            if (comp->output_ptr != NULL)
            {
               png_bytepp old_ptr;

               old_ptr = comp->output_ptr;

               comp->output_ptr = (png_bytepp)png_malloc(png_ptr,
                   (comp->max_output_ptr * png_sizeof(png_bytep)));

               png_memcpy(comp->output_ptr, old_ptr, old_max
                   * png_sizeof(png_bytep));

               png_free(png_ptr, old_ptr);
            }
            else
               comp->output_ptr = (png_bytepp)png_malloc(png_ptr,
                   (comp->max_output_ptr * png_sizeof(png_bytep)));
         }

         comp->output_ptr[comp->num_output_ptr] =
             (png_bytep)png_malloc(png_ptr, png_ptr->zbuf_size);

         png_memcpy(comp->output_ptr[comp->num_output_ptr], png_ptr->zbuf,
             png_ptr->zbuf_size);

         comp->num_output_ptr++;

         png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
         png_ptr->zstream.next_out = png_ptr->zbuf;
      }
   } while (png_ptr->zstream.avail_in);

   do
   {
      ret = deflate(&png_ptr->zstream, Z_FINISH);

      if (ret == Z_OK)
      {
         if (!(png_ptr->zstream.avail_out))
         {
            if (comp->num_output_ptr >= comp->max_output_ptr)
            {
               int old_max;

               old_max = comp->max_output_ptr;
               comp->max_output_ptr = comp->num_output_ptr + 4;
               if (comp->output_ptr != NULL)
               {
                  png_bytepp old_ptr;

                  old_ptr = comp->output_ptr;

                  comp->output_ptr = (png_bytepp)png_malloc(png_ptr,
                      (png_alloc_size_t)(comp->max_output_ptr *
                      png_sizeof(png_charp)));

                  png_memcpy(comp->output_ptr, old_ptr,
                      old_max * png_sizeof(png_charp));

                  png_free(png_ptr, old_ptr);
               }

               else
                  comp->output_ptr = (png_bytepp)png_malloc(png_ptr,
                      (png_alloc_size_t)(comp->max_output_ptr *
                      png_sizeof(png_charp)));
            }

            comp->output_ptr[comp->num_output_ptr] =
                (png_bytep)png_malloc(png_ptr,
                (png_alloc_size_t)png_ptr->zbuf_size);

            png_memcpy(comp->output_ptr[comp->num_output_ptr], png_ptr->zbuf,
                png_ptr->zbuf_size);

            comp->num_output_ptr++;

            png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
            png_ptr->zstream.next_out = png_ptr->zbuf;
         }
      }
   } while (ret != Z_STREAM_END);

   text_len = png_ptr->zbuf_size * comp->num_output_ptr;

   if (png_ptr->zstream.avail_out < png_ptr->zbuf_size)
      text_len += png_ptr->zbuf_size - (png_size_t)png_ptr->zstream.avail_out;

   return((int)text_len);
}

static void PNGAPI
png_write_compressed_data_out(png_structp png_ptr, compression_state *comp,
   png_size_t data_len)
{
   int i;

   if (comp->input)
   {
      png_write_chunk_data(png_ptr, comp->input, data_len);

      return;
   }

#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
   if (data_len >= 2 && comp->input_len < 16384 && png_ptr->zbuf_size > 1)
   {
      unsigned int z_cmf;

      if (comp->num_output_ptr)
        z_cmf = comp->output_ptr[0][0];
      else
        z_cmf = png_ptr->zbuf[0];

      if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
      {
         unsigned int z_cinfo;
         unsigned int half_z_window_size;
         png_size_t uncompressed_text_size = comp->input_len;

         z_cinfo = z_cmf >> 4;
         half_z_window_size = 1 << (z_cinfo + 7);

         while (uncompressed_text_size <= half_z_window_size &&
             half_z_window_size >= 256)
         {
            z_cinfo--;
            half_z_window_size >>= 1;
         }

         z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);

         if (comp->num_output_ptr)
         {

           if (comp->output_ptr[0][0] != z_cmf)
           {
              int tmp;

              comp->output_ptr[0][0] = (png_byte)z_cmf;
              tmp = comp->output_ptr[0][1] & 0xe0;
              tmp += 0x1f - ((z_cmf << 8) + tmp) % 0x1f;
              comp->output_ptr[0][1] = (png_byte)tmp;
           }
         }
         else
         {
            int tmp;

            png_ptr->zbuf[0] = (png_byte)z_cmf;
            tmp = png_ptr->zbuf[1] & 0xe0;
            tmp += 0x1f - ((z_cmf << 8) + tmp) % 0x1f;
            png_ptr->zbuf[1] = (png_byte)tmp;
         }
      }
   }
#endif /* PNG_WRITE_OPTIMIZE_CMF_SUPPORTED */

   for (i = 0; i < comp->num_output_ptr; i++)
   {
      png_write_chunk_data(png_ptr, comp->output_ptr[i],
          (png_size_t)png_ptr->zbuf_size);

      png_free(png_ptr, comp->output_ptr[i]);
   }

   if (comp->max_output_ptr != 0)
      png_free(png_ptr, comp->output_ptr);

   if (png_ptr->zstream.avail_out < (png_uint_32)png_ptr->zbuf_size)
      png_write_chunk_data(png_ptr, png_ptr->zbuf,
          (png_size_t)(png_ptr->zbuf_size - png_ptr->zstream.avail_out));

   png_zlib_release(png_ptr);
}
#endif /* PNG_WRITE_COMPRESSED_TEXT_SUPPORTED */

void PNGAPI
png_write_IHDR(png_structp png_ptr, png_uint_32 width, png_uint_32 height,
    int bit_depth, int color_type, int compression_type, int filter_type,
    int interlace_type)
{
   png_byte buf[13];
   png_debug(1, "in png_write_IHDR");

   switch (color_type)
   {
      case PNG_COLOR_TYPE_GRAY:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
#ifdef PNG_WRITE_16BIT_SUPPORTED
            case 16:
#endif
            png_ptr->channels = 1; break;

            default:
               ;
         }
         break;

      case PNG_COLOR_TYPE_RGB:
#ifdef PNG_WRITE_16BIT_SUPPORTED
         if (bit_depth != 8 && bit_depth != 16)
#else
         if (bit_depth != 8)
#endif
            ;

         png_ptr->channels = 3;
         break;

      case PNG_COLOR_TYPE_PALETTE:
         switch (bit_depth)
         {
            case 1:
            case 2:
            case 4:
            case 8:
               png_ptr->channels = 1;
               break;

            default:
               ;
         }
         break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
         if (bit_depth != 8 && bit_depth != 16)
            ;

         png_ptr->channels = 2;
         break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
#ifdef PNG_WRITE_16BIT_SUPPORTED
         if (bit_depth != 8 && bit_depth != 16)
#else
         if (bit_depth != 8)
#endif
         ;

         png_ptr->channels = 4;
         break;

      default:
         ;
   }

   if (compression_type != PNG_COMPRESSION_TYPE_BASE)
   {
      compression_type = PNG_COMPRESSION_TYPE_BASE;
   }

   if (
#ifdef PNG_MNG_FEATURES_SUPPORTED
       !((png_ptr->mng_features_permitted & PNG_FLAG_MNG_FILTER_64)!=0 &&
       ((png_ptr->mode&PNG_HAVE_PNG_SIGNATURE) == 0) &&
       (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_RGB_ALPHA) &&
       (filter_type == PNG_INTRAPIXEL_DIFFERENCING)) &&
#endif
       filter_type != PNG_FILTER_TYPE_BASE)
   {
      filter_type = PNG_FILTER_TYPE_BASE;
   }

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   if (interlace_type != PNG_INTERLACE_NONE &&
       interlace_type != PNG_INTERLACE_ADAM7)
   {
      interlace_type = PNG_INTERLACE_ADAM7;
   }
#else
   interlace_type=PNG_INTERLACE_NONE;
#endif

   png_ptr->bit_depth = (png_byte)bit_depth;
   png_ptr->color_type = (png_byte)color_type;
   png_ptr->interlaced = (png_byte)interlace_type;
#ifdef PNG_MNG_FEATURES_SUPPORTED
   png_ptr->filter_type = (png_byte)filter_type;
#endif
   png_ptr->compression_type = (png_byte)compression_type;
   png_ptr->width = width;
   png_ptr->height = height;

   png_ptr->pixel_depth = (png_byte)(bit_depth * png_ptr->channels);
   png_ptr->rowbytes = PNG_ROWBYTES(png_ptr->pixel_depth, width);
   png_ptr->usr_width = png_ptr->width;
   png_ptr->usr_bit_depth = png_ptr->bit_depth;
   png_ptr->usr_channels = png_ptr->channels;

   png_save_uint_32(buf, width);
   png_save_uint_32(buf + 4, height);
   buf[8] = (png_byte)bit_depth;
   buf[9] = (png_byte)color_type;
   buf[10] = (png_byte)compression_type;
   buf[11] = (png_byte)filter_type;
   buf[12] = (png_byte)interlace_type;

   png_write_complete_chunk(png_ptr, png_IHDR, buf, (png_size_t)13);

   png_ptr->zstream.zalloc = png_zalloc;
   png_ptr->zstream.zfree = png_zfree;
   png_ptr->zstream.opaque = (voidpf)png_ptr;

   if ((png_ptr->do_filter) == PNG_NO_FILTERS)
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE ||
          png_ptr->bit_depth < 8)
         png_ptr->do_filter = PNG_FILTER_NONE;

      else
         png_ptr->do_filter = PNG_ALL_FILTERS;
   }

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_STRATEGY))
   {
      if (png_ptr->do_filter != PNG_FILTER_NONE)
         png_ptr->zlib_strategy = Z_FILTERED;

      else
         png_ptr->zlib_strategy = Z_DEFAULT_STRATEGY;
   }

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_LEVEL))
      png_ptr->zlib_level = Z_DEFAULT_COMPRESSION;

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_MEM_LEVEL))
      png_ptr->zlib_mem_level = 8;

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_WINDOW_BITS))
      png_ptr->zlib_window_bits = 15;

   if (!(png_ptr->flags & PNG_FLAG_ZLIB_CUSTOM_METHOD))
      png_ptr->zlib_method = 8;

#ifdef PNG_WRITE_COMPRESSED_TEXT_SUPPORTED
#ifdef PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED
   if (!(png_ptr->flags & PNG_FLAG_ZTXT_CUSTOM_STRATEGY))
      png_ptr->zlib_text_strategy = Z_DEFAULT_STRATEGY;

   if (!(png_ptr->flags & PNG_FLAG_ZTXT_CUSTOM_LEVEL))
      png_ptr->zlib_text_level = png_ptr->zlib_level;

   if (!(png_ptr->flags & PNG_FLAG_ZTXT_CUSTOM_MEM_LEVEL))
      png_ptr->zlib_text_mem_level = png_ptr->zlib_mem_level;

   if (!(png_ptr->flags & PNG_FLAG_ZTXT_CUSTOM_WINDOW_BITS))
      png_ptr->zlib_text_window_bits = png_ptr->zlib_window_bits;

   if (!(png_ptr->flags & PNG_FLAG_ZTXT_CUSTOM_METHOD))
      png_ptr->zlib_text_method = png_ptr->zlib_method;
#else
   png_ptr->zlib_text_strategy = Z_DEFAULT_STRATEGY;
   png_ptr->zlib_text_level = png_ptr->zlib_level;
   png_ptr->zlib_text_mem_level = png_ptr->zlib_mem_level;
   png_ptr->zlib_text_window_bits = png_ptr->zlib_window_bits;
   png_ptr->zlib_text_method = png_ptr->zlib_method;
#endif /* PNG_WRITE_CUSTOMIZE_ZTXT_COMPRESSION_SUPPORTED */
#endif /* PNG_WRITE_COMPRESSED_TEXT_SUPPORTED */

   png_ptr->zlib_state = PNG_ZLIB_UNINITIALIZED;
   png_ptr->mode = PNG_HAVE_IHDR;
}

void PNGAPI
png_write_PLTE(png_structp png_ptr, png_const_colorp palette,
    png_uint_32 num_pal)
{
   png_uint_32 max_palette_length, i;
   png_const_colorp pal_ptr;
   png_byte buf[3];

   png_debug(1, "in png_write_PLTE");
   max_palette_length = (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE) ?
      (1 << png_ptr->bit_depth) : PNG_MAX_PALETTE_LENGTH;

   if ((
#ifdef PNG_MNG_FEATURES_SUPPORTED
       (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0 &&
#endif
       num_pal == 0) || num_pal > max_palette_length)
   {
      if (png_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
      {
      }

      else
      {
         return;
      }
   }

   if ((png_ptr->color_type & PNG_COLOR_MASK_COLOR) == 0)
   {
      return;
   }

   png_ptr->num_palette = (png_uint_16)num_pal;
   png_debug1(3, "num_palette = %d", png_ptr->num_palette);

   png_write_chunk_header(png_ptr, png_PLTE, (png_uint_32)(num_pal * 3));

   for (i = 0, pal_ptr = palette; i < num_pal; i++, pal_ptr++)
   {
      buf[0] = pal_ptr->red;
      buf[1] = pal_ptr->green;
      buf[2] = pal_ptr->blue;
      png_write_chunk_data(png_ptr, buf, (png_size_t)3);
   }

   png_write_chunk_end(png_ptr);
   png_ptr->mode |= PNG_HAVE_PLTE;
}

void PNGAPI
png_write_IDAT(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_debug(1, "in png_write_IDAT");

#ifdef PNG_WRITE_OPTIMIZE_CMF_SUPPORTED
   if ((png_ptr->mode & PNG_HAVE_IDAT) == 0 &&
       png_ptr->compression_type == PNG_COMPRESSION_TYPE_BASE)
   {
      unsigned int z_cmf = data[0];

      if ((z_cmf & 0x0f) == 8 && (z_cmf & 0xf0) <= 0x70)
      {
         if (length >= 2 &&
             png_ptr->height < 16384 && png_ptr->width < 16384)
         {
            unsigned int z_cinfo;
            unsigned int half_z_window_size;
            png_uint_32 uncompressed_idat_size = png_ptr->height *
                ((png_ptr->width *
                png_ptr->channels * png_ptr->bit_depth + 15) >> 3);

            if (png_ptr->interlaced)
               uncompressed_idat_size += ((png_ptr->height + 7)/8) *
                   (png_ptr->bit_depth < 8 ? 12 : 6);

            z_cinfo = z_cmf >> 4;
            half_z_window_size = 1 << (z_cinfo + 7);

            while (uncompressed_idat_size <= half_z_window_size &&
                half_z_window_size >= 256)
            {
               z_cinfo--;
               half_z_window_size >>= 1;
            }

            z_cmf = (z_cmf & 0x0f) | (z_cinfo << 4);
            if (data[0] != z_cmf)
            {
               int tmp;
               data[0] = (png_byte)z_cmf;
               tmp = data[1] & 0xe0;
               tmp += 0x1f - ((z_cmf << 8) + tmp) % 0x1f;
               data[1] = (png_byte)tmp;
            }
         }
      }
   }
#endif /* PNG_WRITE_OPTIMIZE_CMF_SUPPORTED */

   png_write_complete_chunk(png_ptr, png_IDAT, data, length);
   png_ptr->mode |= PNG_HAVE_IDAT;

   png_ptr->zstream.next_out = png_ptr->zbuf;
   png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
}

void PNGAPI
png_write_IEND(png_structp png_ptr)
{
   png_debug(1, "in png_write_IEND");

   png_write_complete_chunk(png_ptr, png_IEND, NULL, (png_size_t)0);
   png_ptr->mode |= PNG_HAVE_IEND;
}

#ifdef PNG_WRITE_gAMA_SUPPORTED
void PNGAPI
png_write_gAMA_fixed(png_structp png_ptr, png_fixed_point file_gamma)
{
   png_byte buf[4];

   png_debug(1, "in png_write_gAMA");

   png_save_uint_32(buf, (png_uint_32)file_gamma);
   png_write_complete_chunk(png_ptr, png_gAMA, buf, (png_size_t)4);
}
#endif

#ifdef PNG_WRITE_sRGB_SUPPORTED
void PNGAPI
png_write_sRGB(png_structp png_ptr, int srgb_intent)
{
   png_byte buf[1];

   png_debug(1, "in png_write_sRGB");

   buf[0]=(png_byte)srgb_intent;
   png_write_complete_chunk(png_ptr, png_sRGB, buf, (png_size_t)1);
}
#endif

#ifdef PNG_WRITE_iCCP_SUPPORTED
void PNGAPI
png_write_iCCP(png_structp png_ptr, png_const_charp name, int compression_type,
    png_const_charp profile, int profile_len)
{
   png_size_t name_len;
   compression_state comp;
   int embedded_profile_len = 0;
   PNG_UNUSED(compression_type)

   png_debug(1, "in png_write_iCCP");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;
   comp.input_len = 0;

   name_len = png_strlen(name);

   if (profile == NULL)
      profile_len = 0;

   if (profile_len > 3)
      embedded_profile_len =
      ((*( (png_const_bytep)profile    ))<<24) |
      ((*( (png_const_bytep)profile + 1))<<16) |
      ((*( (png_const_bytep)profile + 2))<< 8) |
      ((*( (png_const_bytep)profile + 3))    );

   if (embedded_profile_len < 0)
   {
      return;
   }

   if (profile_len < embedded_profile_len)
   {
      return;
   }

   if (profile_len > embedded_profile_len)
   {
      profile_len = embedded_profile_len;
   }

   if (profile_len != 0)
      profile_len = png_text_compress(png_ptr, profile,
       (png_size_t)profile_len, PNG_COMPRESSION_TYPE_BASE, &comp);

   png_write_chunk_header(png_ptr, png_iCCP,
       (png_uint_32)(name_len + profile_len + 2));

   png_write_chunk_data(png_ptr, (png_bytep)name, name_len);

   {
      png_byte buffer[2];
      buffer[0] = 0;
      buffer[1] = (png_byte)(0xFFU & compression_type);
      png_write_chunk_data(png_ptr, buffer, 2);
   }

   if (profile_len != 0)
   {
      png_write_compressed_data_out(png_ptr, &comp, profile_len);
   }

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_sPLT_SUPPORTED
void PNGAPI
png_write_sPLT(png_structp png_ptr, png_const_sPLT_tp spalette)
{
   png_size_t name_len;
   png_byte entrybuf[10];
   png_size_t entry_size = (spalette->depth == 8 ? 6 : 10);
   png_size_t palette_size = (png_size_t)(entry_size * spalette->nentries);
   png_sPLT_entryp ep;

   png_debug(1, "in png_write_sPLT");

   name_len = png_strlen(spalette->name);

   png_write_chunk_header(png_ptr, png_sPLT,
       (png_uint_32)(name_len + 2 + palette_size));

   png_write_chunk_data(png_ptr, (png_bytep)spalette->name,
       (png_size_t)(name_len + 1));

   png_write_chunk_data(png_ptr, &spalette->depth, (png_size_t)1);

   for (ep = spalette->entries; ep<spalette->entries + spalette->nentries; ep++)
   {
      if (spalette->depth == 8)
      {
         entrybuf[0] = (png_byte)ep->red;
         entrybuf[1] = (png_byte)ep->green;
         entrybuf[2] = (png_byte)ep->blue;
         entrybuf[3] = (png_byte)ep->alpha;
         png_save_uint_16(entrybuf + 4, ep->frequency);
      }

      else
      {
         png_save_uint_16(entrybuf + 0, ep->red);
         png_save_uint_16(entrybuf + 2, ep->green);
         png_save_uint_16(entrybuf + 4, ep->blue);
         png_save_uint_16(entrybuf + 6, ep->alpha);
         png_save_uint_16(entrybuf + 8, ep->frequency);
      }

      png_write_chunk_data(png_ptr, entrybuf, (png_size_t)entry_size);
   }

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_sBIT_SUPPORTED
void PNGAPI
png_write_sBIT(png_structp png_ptr, png_const_color_8p sbit, int color_type)
{
   png_byte buf[4];
   png_size_t size;

   png_debug(1, "in png_write_sBIT");

   if ((color_type & PNG_COLOR_MASK_COLOR) != 0)
   {
      png_byte maxbits;

      maxbits = (png_byte)(color_type==PNG_COLOR_TYPE_PALETTE ? 8 :
          png_ptr->usr_bit_depth);

      if (sbit->red == 0 || sbit->red > maxbits ||
          sbit->green == 0 || sbit->green > maxbits ||
          sbit->blue == 0 || sbit->blue > maxbits)
      {
         return;
      }

      buf[0] = sbit->red;
      buf[1] = sbit->green;
      buf[2] = sbit->blue;
      size = 3;
   }

   else
   {
      if (sbit->gray == 0 || sbit->gray > png_ptr->usr_bit_depth)
      {
         return;
      }

      buf[0] = sbit->gray;
      size = 1;
   }

   if ((color_type & PNG_COLOR_MASK_ALPHA) != 0)
   {
      if (sbit->alpha == 0 || sbit->alpha > png_ptr->usr_bit_depth)
      {
         return;
      }

      buf[size++] = sbit->alpha;
   }

   png_write_complete_chunk(png_ptr, png_sBIT, buf, size);
}
#endif

#ifdef PNG_WRITE_cHRM_SUPPORTED
void PNGAPI
png_write_cHRM_fixed(png_structp png_ptr, png_fixed_point white_x,
    png_fixed_point white_y, png_fixed_point red_x, png_fixed_point red_y,
    png_fixed_point green_x, png_fixed_point green_y, png_fixed_point blue_x,
    png_fixed_point blue_y)
{
   png_byte buf[32];

   png_debug(1, "in png_write_cHRM");

#ifdef PNG_CHECK_cHRM_SUPPORTED
   if (png_check_cHRM_fixed(png_ptr, white_x, white_y, red_x, red_y,
       green_x, green_y, blue_x, blue_y))
#endif
   {
      png_save_uint_32(buf, (png_uint_32)white_x);
      png_save_uint_32(buf + 4, (png_uint_32)white_y);

      png_save_uint_32(buf + 8, (png_uint_32)red_x);
      png_save_uint_32(buf + 12, (png_uint_32)red_y);

      png_save_uint_32(buf + 16, (png_uint_32)green_x);
      png_save_uint_32(buf + 20, (png_uint_32)green_y);

      png_save_uint_32(buf + 24, (png_uint_32)blue_x);
      png_save_uint_32(buf + 28, (png_uint_32)blue_y);

      png_write_complete_chunk(png_ptr, png_cHRM, buf, (png_size_t)32);
   }
}
#endif

#ifdef PNG_WRITE_tRNS_SUPPORTED
void PNGAPI
png_write_tRNS(png_structp png_ptr, png_const_bytep trans_alpha,
    png_const_color_16p tran, int num_trans, int color_type)
{
   png_byte buf[6];

   png_debug(1, "in png_write_tRNS");

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (num_trans <= 0 || num_trans > (int)png_ptr->num_palette)
      {
         return;
      }

      png_write_complete_chunk(png_ptr, png_tRNS, trans_alpha,
         (png_size_t)num_trans);
   }

   else if (color_type == PNG_COLOR_TYPE_GRAY)
   {
      if (tran->gray >= (png_uint_16)(1 << png_ptr->bit_depth))
      {
         return;
      }

      png_save_uint_16(buf, tran->gray);
      png_write_complete_chunk(png_ptr, png_tRNS, buf, (png_size_t)2);
   }

   else if (color_type == PNG_COLOR_TYPE_RGB)
   {
      png_save_uint_16(buf, tran->red);
      png_save_uint_16(buf + 2, tran->green);
      png_save_uint_16(buf + 4, tran->blue);
#ifdef PNG_WRITE_16BIT_SUPPORTED
      if (png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4])!=0)
#else
      if ((buf[0] | buf[2] | buf[4]) != 0)
#endif
      {
         return;
      }

      png_write_complete_chunk(png_ptr, png_tRNS, buf, (png_size_t)6);
   }
}
#endif

#ifdef PNG_WRITE_bKGD_SUPPORTED
void PNGAPI
png_write_bKGD(png_structp png_ptr, png_const_color_16p back, int color_type)
{
   png_byte buf[6];

   png_debug(1, "in png_write_bKGD");

   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      if (
#ifdef PNG_MNG_FEATURES_SUPPORTED
          (png_ptr->num_palette != 0 ||
          (png_ptr->mng_features_permitted & PNG_FLAG_MNG_EMPTY_PLTE) == 0) &&
#endif
         back->index >= png_ptr->num_palette)
      {
         return;
      }

      buf[0] = back->index;
      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)1);
   }

   else if ((color_type & PNG_COLOR_MASK_COLOR)!=0)
   {
      png_save_uint_16(buf, back->red);
      png_save_uint_16(buf + 2, back->green);
      png_save_uint_16(buf + 4, back->blue);
#ifdef PNG_WRITE_16BIT_SUPPORTED
      if (png_ptr->bit_depth == 8 && (buf[0] | buf[2] | buf[4])!=0)
#else
      if ((buf[0] | buf[2] | buf[4])!=0)
#endif
      {
         return;
      }

      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)6);
   }

   else
   {
      if (back->gray >= (png_uint_16)(1 << png_ptr->bit_depth))
      {
         return;
      }

      png_save_uint_16(buf, back->gray);
      png_write_complete_chunk(png_ptr, png_bKGD, buf, (png_size_t)2);
   }
}
#endif

#ifdef PNG_WRITE_hIST_SUPPORTED
void PNGAPI
png_write_hIST(png_structp png_ptr, png_const_uint_16p hist, int num_hist)
{
   int i;
   png_byte buf[3];

   png_debug(1, "in png_write_hIST");

   if (num_hist > (int)png_ptr->num_palette)
   {
      png_debug2(3, "num_hist = %d, num_palette = %d", num_hist,
          png_ptr->num_palette);
      return;
   }

   png_write_chunk_header(png_ptr, png_hIST, (png_uint_32)(num_hist * 2));

   for (i = 0; i < num_hist; i++)
   {
      png_save_uint_16(buf, hist[i]);
      png_write_chunk_data(png_ptr, buf, (png_size_t)2);
   }

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_tEXt_SUPPORTED
void PNGAPI
png_write_tEXt(png_structp png_ptr, png_const_charp key, png_const_charp text,
    png_size_t text_len)
{
   png_size_t key_len;

   png_debug(1, "in png_write_tEXt");

   key_len = strlen(key);

   if (text == NULL || *text == '\0')
      text_len = 0;

   else
      text_len = png_strlen(text);

   png_write_chunk_header(png_ptr, png_tEXt,
       (png_uint_32)(key_len + text_len + 1));
   png_write_chunk_data(png_ptr, (png_bytep)key,
       (png_size_t)(key_len + 1));

   if (text_len != 0)
      png_write_chunk_data(png_ptr, (png_const_bytep)text,
          (png_size_t)text_len);

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_zTXt_SUPPORTED
void PNGAPI
png_write_zTXt(png_structp png_ptr, png_const_charp key, png_const_charp text,
    png_size_t text_len, int compression)
{
   png_size_t key_len;
   png_byte buf;
   compression_state comp;

   png_debug(1, "in png_write_zTXt");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;
   comp.input_len = 0;

   key_len = strlen(key);

   if (text == NULL || *text == '\0' || compression==PNG_TEXT_COMPRESSION_NONE)
   {
      png_write_tEXt(png_ptr, key, text, (png_size_t)0);
      return;
   }

   text_len = png_strlen(text);
   text_len = png_text_compress(png_ptr, text, text_len, compression,
       &comp);
   png_write_chunk_header(png_ptr, png_zTXt,
       (png_uint_32)(key_len+text_len + 2));
   png_write_chunk_data(png_ptr, (png_bytep)key,
       (png_size_t)(key_len + 1));

   buf = (png_byte)compression;

   png_write_chunk_data(png_ptr, &buf, (png_size_t)1);
   png_write_compressed_data_out(png_ptr, &comp, text_len);
   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_iTXt_SUPPORTED
void PNGAPI
png_write_iTXt(png_structp png_ptr, int compression, png_const_charp key,
    png_const_charp lang, png_const_charp lang_key, png_const_charp text)
{
   png_size_t lang_len, key_len, lang_key_len, text_len;
   png_byte cbuf[2];
   compression_state comp;

   png_debug(1, "in png_write_iTXt");

   comp.num_output_ptr = 0;
   comp.max_output_ptr = 0;
   comp.output_ptr = NULL;
   comp.input = NULL;

   key_len = png_strlen(key);

   if (lang == NULL)
      lang_len = 0;

   else
      lang_len = png_strlen(lang);

   if (lang_key == NULL)
      lang_key_len = 0;

   else
      lang_key_len = png_strlen(lang_key);

   if (text == NULL)
      text_len = 0;

   else
      text_len = png_strlen(text);

   text_len = png_text_compress(png_ptr, text, text_len, compression - 2,
       &comp);

   png_write_chunk_header(png_ptr, png_iTXt, (png_uint_32)(
        5
        + key_len
        + lang_len
        + lang_key_len
        + text_len));

   png_write_chunk_data(png_ptr, (png_bytep)key, (png_size_t)(key_len + 1));

   if (compression == PNG_ITXT_COMPRESSION_NONE ||
       compression == PNG_TEXT_COMPRESSION_NONE)
      cbuf[0] = 0;

   else
      cbuf[0] = 1;

   cbuf[1] = 0;

   png_write_chunk_data(png_ptr, cbuf, (png_size_t)2);

   cbuf[0] = 0;
   png_write_chunk_data(png_ptr, (lang ? (png_const_bytep)lang : cbuf),
       (png_size_t)(lang_len + 1));

   png_write_chunk_data(png_ptr, (lang_key ? (png_const_bytep)lang_key : cbuf),
       (png_size_t)(lang_key_len + 1));

   png_write_compressed_data_out(png_ptr, &comp, text_len);

   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_oFFs_SUPPORTED
void PNGAPI
png_write_oFFs(png_structp png_ptr, png_int_32 x_offset, png_int_32 y_offset,
    int unit_type)
{
   png_byte buf[9];

   png_debug(1, "in png_write_oFFs");

   png_save_int_32(buf, x_offset);
   png_save_int_32(buf + 4, y_offset);
   buf[8] = (png_byte)unit_type;

   png_write_complete_chunk(png_ptr, png_oFFs, buf, (png_size_t)9);
}
#endif
#ifdef PNG_WRITE_pCAL_SUPPORTED
void PNGAPI
png_write_pCAL(png_structp png_ptr, png_charp purpose, png_int_32 X0,
    png_int_32 X1, int type, int nparams, png_const_charp units,
    png_charpp params)
{
   png_size_t purpose_len, units_len, total_len;
   png_size_tp params_len;
   png_byte buf[10];
   int i;

   png_debug1(1, "in png_write_pCAL (%d parameters)", nparams);

   purpose_len = strlen(purpose) + 1;
   png_debug1(3, "pCAL purpose length = %d", (int)purpose_len);
   units_len = png_strlen(units) + (nparams == 0 ? 0 : 1);
   png_debug1(3, "pCAL units length = %d", (int)units_len);
   total_len = purpose_len + units_len + 10;

   params_len = (png_size_tp)png_malloc(png_ptr,
       (png_alloc_size_t)(nparams * png_sizeof(png_size_t)));

   for (i = 0; i < nparams; i++)
   {
      params_len[i] = png_strlen(params[i]) + (i == nparams - 1 ? 0 : 1);
      png_debug2(3, "pCAL parameter %d length = %lu", i,
          (unsigned long)params_len[i]);
      total_len += params_len[i];
   }

   png_debug1(3, "pCAL total length = %d", (int)total_len);
   png_write_chunk_header(png_ptr, png_pCAL, (png_uint_32)total_len);
   png_write_chunk_data(png_ptr, (png_const_bytep)purpose, purpose_len);
   png_save_int_32(buf, X0);
   png_save_int_32(buf + 4, X1);
   buf[8] = (png_byte)type;
   buf[9] = (png_byte)nparams;
   png_write_chunk_data(png_ptr, buf, (png_size_t)10);
   png_write_chunk_data(png_ptr, (png_const_bytep)units, (png_size_t)units_len);

   for (i = 0; i < nparams; i++)
   {
      png_write_chunk_data(png_ptr, (png_const_bytep)params[i], params_len[i]);
   }

   png_free(png_ptr, params_len);
   png_write_chunk_end(png_ptr);
}
#endif

#ifdef PNG_WRITE_sCAL_SUPPORTED
void PNGAPI
png_write_sCAL_s(png_structp png_ptr, int unit, png_const_charp width,
    png_const_charp height)
{
   png_byte buf[64];
   png_size_t wlen, hlen, total_len;

   png_debug(1, "in png_write_sCAL_s");

   wlen = png_strlen(width);
   hlen = png_strlen(height);
   total_len = wlen + hlen + 2;

   if (total_len > 64)
   {
      return;
   }

   buf[0] = (png_byte)unit;
   png_memcpy(buf + 1, width, wlen + 1);
   png_memcpy(buf + wlen + 2, height, hlen);

   png_debug1(3, "sCAL total length = %u", (unsigned int)total_len);
   png_write_complete_chunk(png_ptr, png_sCAL, buf, total_len);
}
#endif

#ifdef PNG_WRITE_pHYs_SUPPORTED
void PNGAPI
png_write_pHYs(png_structp png_ptr, png_uint_32 x_pixels_per_unit,
    png_uint_32 y_pixels_per_unit,
    int unit_type)
{
   png_byte buf[9];

   png_debug(1, "in png_write_pHYs");

   png_save_uint_32(buf, x_pixels_per_unit);
   png_save_uint_32(buf + 4, y_pixels_per_unit);
   buf[8] = (png_byte)unit_type;

   png_write_complete_chunk(png_ptr, png_pHYs, buf, (png_size_t)9);
}
#endif

#ifdef PNG_WRITE_tIME_SUPPORTED
void PNGAPI
png_write_tIME(png_structp png_ptr, png_const_timep mod_time)
{
   png_byte buf[7];

   png_debug(1, "in png_write_tIME");

   if (mod_time->month  > 12 || mod_time->month  < 1 ||
       mod_time->day    > 31 || mod_time->day    < 1 ||
       mod_time->hour   > 23 || mod_time->second > 60)
   {
      return;
   }

   png_save_uint_16(buf, mod_time->year);
   buf[2] = mod_time->month;
   buf[3] = mod_time->day;
   buf[4] = mod_time->hour;
   buf[5] = mod_time->minute;
   buf[6] = mod_time->second;

   png_write_complete_chunk(png_ptr, png_tIME, buf, (png_size_t)7);
}
#endif

void PNGAPI
png_write_start_row(png_structp png_ptr)
{
#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   static PNG_CONST png_byte png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};
   static PNG_CONST png_byte png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};
   static PNG_CONST png_byte png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};
   static PNG_CONST png_byte png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   png_alloc_size_t buf_size;
   int usr_pixel_depth;

   png_debug(1, "in png_write_start_row");

   usr_pixel_depth = png_ptr->usr_channels * png_ptr->usr_bit_depth;
   buf_size = PNG_ROWBYTES(usr_pixel_depth, png_ptr->width) + 1;

   png_ptr->transformed_pixel_depth = png_ptr->pixel_depth;
   png_ptr->maximum_pixel_depth = (png_byte)usr_pixel_depth;

   png_ptr->row_buf = (png_bytep)png_malloc(png_ptr, buf_size);
   png_ptr->row_buf[0] = PNG_FILTER_VALUE_NONE;

#ifdef PNG_WRITE_FILTER_SUPPORTED
   if (png_ptr->do_filter & PNG_FILTER_SUB)
   {
      png_ptr->sub_row = (png_bytep)png_malloc(png_ptr, png_ptr->rowbytes + 1);

      png_ptr->sub_row[0] = PNG_FILTER_VALUE_SUB;
   }

   if (png_ptr->do_filter & (PNG_FILTER_AVG | PNG_FILTER_UP | PNG_FILTER_PAETH))
   {
      png_ptr->prev_row = (png_bytep)png_calloc(png_ptr, buf_size);

      if (png_ptr->do_filter & PNG_FILTER_UP)
      {
         png_ptr->up_row = (png_bytep)png_malloc(png_ptr,
            png_ptr->rowbytes + 1);

         png_ptr->up_row[0] = PNG_FILTER_VALUE_UP;
      }

      if (png_ptr->do_filter & PNG_FILTER_AVG)
      {
         png_ptr->avg_row = (png_bytep)png_malloc(png_ptr,
             png_ptr->rowbytes + 1);

         png_ptr->avg_row[0] = PNG_FILTER_VALUE_AVG;
      }

      if (png_ptr->do_filter & PNG_FILTER_PAETH)
      {
         png_ptr->paeth_row = (png_bytep)png_malloc(png_ptr,
             png_ptr->rowbytes + 1);

         png_ptr->paeth_row[0] = PNG_FILTER_VALUE_PAETH;
      }
   }
#endif /* PNG_WRITE_FILTER_SUPPORTED */

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   if (png_ptr->interlaced !=0)
   {
      if ((png_ptr->transformations & PNG_INTERLACE) == 0)
      {
         png_ptr->num_rows = (png_ptr->height + png_pass_yinc[0] - 1 -
             png_pass_ystart[0]) / png_pass_yinc[0];

         png_ptr->usr_width = (png_ptr->width + png_pass_inc[0] - 1 -
             png_pass_start[0]) / png_pass_inc[0];
      }

      else
      {
         png_ptr->num_rows = png_ptr->height;
         png_ptr->usr_width = png_ptr->width;
      }
   }

   else
#endif
   {
      png_ptr->num_rows = png_ptr->height;
      png_ptr->usr_width = png_ptr->width;
   }

   png_zlib_claim(png_ptr, PNG_ZLIB_FOR_IDAT);
   png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
   png_ptr->zstream.next_out = png_ptr->zbuf;
}

void PNGAPI
png_write_finish_row(png_structp png_ptr)
{
#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   static PNG_CONST png_byte png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};
   static PNG_CONST png_byte png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};
   static PNG_CONST png_byte png_pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};
   static PNG_CONST png_byte png_pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
#endif

   int ret;

   png_debug(1, "in png_write_finish_row");

   png_ptr->row_number++;

   if (png_ptr->row_number < png_ptr->num_rows)
      return;

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
   if (png_ptr->interlaced != 0)
   {
      png_ptr->row_number = 0;
      if ((png_ptr->transformations & PNG_INTERLACE) != 0)
      {
         png_ptr->pass++;
      }

      else
      {
         do
         {
            png_ptr->pass++;

            if (png_ptr->pass >= 7)
               break;

            png_ptr->usr_width = (png_ptr->width +
                png_pass_inc[png_ptr->pass] - 1 -
                png_pass_start[png_ptr->pass]) /
                png_pass_inc[png_ptr->pass];

            png_ptr->num_rows = (png_ptr->height +
                png_pass_yinc[png_ptr->pass] - 1 -
                png_pass_ystart[png_ptr->pass]) /
                png_pass_yinc[png_ptr->pass];

            if ((png_ptr->transformations & PNG_INTERLACE) != 0)
               break;

         } while (png_ptr->usr_width == 0 || png_ptr->num_rows == 0);

      }

      if (png_ptr->pass < 7)
      {
         if (png_ptr->prev_row != NULL)
            png_memset(png_ptr->prev_row, 0,
                (png_size_t)(PNG_ROWBYTES(png_ptr->usr_channels*
                png_ptr->usr_bit_depth, png_ptr->width)) + 1);

         return;
      }
   }
#endif

   do
   {
      ret = deflate(&png_ptr->zstream, Z_FINISH);

      if (ret == Z_OK)
      {
         if (!(png_ptr->zstream.avail_out))
         {
            png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size);
            png_ptr->zstream.next_out = png_ptr->zbuf;
            png_ptr->zstream.avail_out = (uInt)png_ptr->zbuf_size;
         }
      }

   } while (ret != Z_STREAM_END);

   if (png_ptr->zstream.avail_out < png_ptr->zbuf_size)
   {
      png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size -
          png_ptr->zstream.avail_out);
   }

   png_zlib_release(png_ptr);
   png_ptr->zstream.data_type = Z_BINARY;
}

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
void PNGAPI
png_do_write_interlace(png_row_infop row_info, png_bytep row, int pass)
{
   static PNG_CONST png_byte png_pass_start[7] = {0, 4, 0, 2, 0, 1, 0};
   static PNG_CONST png_byte  png_pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};

   png_debug(1, "in png_do_write_interlace");

   if (pass < 6)
   {
      switch (row_info->pixel_depth)
      {
         case 1:
         {
            png_bytep sp;
            png_bytep dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;
            png_uint_32 row_width = row_info->width;

            dp = row;
            d = 0;
            shift = 7;

            for (i = png_pass_start[pass]; i < row_width;
               i += png_pass_inc[pass])
            {
               sp = row + (png_size_t)(i >> 3);
               value = (int)(*sp >> (7 - (int)(i & 0x07))) & 0x01;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 7;
                  *dp++ = (png_byte)d;
                  d = 0;
               }

               else
                  shift--;

            }
            if (shift != 7)
               *dp = (png_byte)d;

            break;
         }

         case 2:
         {
            png_bytep sp;
            png_bytep dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;
            png_uint_32 row_width = row_info->width;

            dp = row;
            shift = 6;
            d = 0;

            for (i = png_pass_start[pass]; i < row_width;
               i += png_pass_inc[pass])
            {
               sp = row + (png_size_t)(i >> 2);
               value = (*sp >> ((3 - (int)(i & 0x03)) << 1)) & 0x03;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 6;
                  *dp++ = (png_byte)d;
                  d = 0;
               }

               else
                  shift -= 2;
            }
            if (shift != 6)
               *dp = (png_byte)d;

            break;
         }

         case 4:
         {
            png_bytep sp;
            png_bytep dp;
            int shift;
            int d;
            int value;
            png_uint_32 i;
            png_uint_32 row_width = row_info->width;

            dp = row;
            shift = 4;
            d = 0;
            for (i = png_pass_start[pass]; i < row_width;
                i += png_pass_inc[pass])
            {
               sp = row + (png_size_t)(i >> 1);
               value = (*sp >> ((1 - (int)(i & 0x01)) << 2)) & 0x0f;
               d |= (value << shift);

               if (shift == 0)
               {
                  shift = 4;
                  *dp++ = (png_byte)d;
                  d = 0;
               }

               else
                  shift -= 4;
            }
            if (shift != 4)
               *dp = (png_byte)d;

            break;
         }

         default:
         {
            png_bytep sp;
            png_bytep dp;
            png_uint_32 i;
            png_uint_32 row_width = row_info->width;
            png_size_t pixel_bytes;

            dp = row;

            pixel_bytes = (row_info->pixel_depth >> 3);

            for (i = png_pass_start[pass]; i < row_width;
               i += png_pass_inc[pass])
            {
               sp = row + (png_size_t)i * pixel_bytes;

               if (dp != sp)
                  png_memcpy(dp, sp, pixel_bytes);

               dp += pixel_bytes;
            }
            break;
         }
      }
      row_info->width = (row_info->width +
          png_pass_inc[pass] - 1 -
          png_pass_start[pass]) /
          png_pass_inc[pass];

      row_info->rowbytes = PNG_ROWBYTES(row_info->pixel_depth,
          row_info->width);
   }
}
#endif

static void PNGAPI png_write_filtered_row(png_structp png_ptr, png_bytep filtered_row,
   png_size_t row_bytes);

#define PNG_MAXSUM (((png_uint_32)(-1)) >> 1)
#define PNG_HISHIFT 10
#define PNG_LOMASK ((png_uint_32)0xffffL)
#define PNG_HIMASK ((png_uint_32)(~PNG_LOMASK >> PNG_HISHIFT))
void PNGAPI
png_write_find_filter(png_structp png_ptr, png_row_infop row_info)
{
   png_bytep best_row;
#ifdef PNG_WRITE_FILTER_SUPPORTED
   png_bytep prev_row, row_buf;
   png_uint_32 mins, bpp;
   png_byte filter_to_do = png_ptr->do_filter;
   png_size_t row_bytes = row_info->rowbytes;

   png_debug(1, "in png_write_find_filter");

   bpp = (row_info->pixel_depth + 7) >> 3;

   prev_row = png_ptr->prev_row;
#endif
   best_row = png_ptr->row_buf;
#ifdef PNG_WRITE_FILTER_SUPPORTED
   row_buf = best_row;
   mins = PNG_MAXSUM;

   if ((filter_to_do & PNG_FILTER_NONE) && filter_to_do != PNG_FILTER_NONE)
   {
      png_bytep rp;
      png_uint_32 sum = 0;
      png_size_t i;
      int v;

      for (i = 0, rp = row_buf + 1; i < row_bytes; i++, rp++)
      {
         v = *rp;
         sum += 128 - abs(v - 128);
      }

      mins = sum;
   }

   if (filter_to_do == PNG_FILTER_SUB)
   {
      png_bytep rp, lp, dp;
      png_size_t i;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->sub_row + 1; i < bpp;
           i++, rp++, dp++)
      {
         *dp = *rp;
      }

      for (lp = row_buf + 1; i < row_bytes;
         i++, rp++, lp++, dp++)
      {
         *dp = (png_byte)(((int)*rp - (int)*lp) & 0xff);
      }

      best_row = png_ptr->sub_row;
   }

   else if (filter_to_do & PNG_FILTER_SUB)
   {
      png_bytep rp, dp, lp;
      png_uint_32 sum = 0, lmins = mins;
      png_size_t i;
      int v;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->sub_row + 1; i < bpp;
           i++, rp++, dp++)
      {
         v = *dp = *rp;
         sum += 128 - abs(v - 128);
      }

      for (lp = row_buf + 1; i < row_bytes;
         i++, rp++, lp++, dp++)
      {
         v = *dp = (png_byte)(((int)*rp - (int)*lp) & 0xff);
         sum += 128 - abs(v - 128);

         if (sum > lmins)
            break;
      }

      if (sum < mins)
      {
         mins = sum;
         best_row = png_ptr->sub_row;
      }
   }

   if (filter_to_do == PNG_FILTER_UP)
   {
      png_bytep rp, dp, pp;
      png_size_t i;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->up_row + 1,
          pp = prev_row + 1; i < row_bytes;
          i++, rp++, pp++, dp++)
      {
         *dp = (png_byte)(((int)*rp - (int)*pp) & 0xff);
      }

      best_row = png_ptr->up_row;
   }

   else if (filter_to_do & PNG_FILTER_UP)
   {
      png_bytep rp, dp, pp;
      png_uint_32 sum = 0, lmins = mins;
      png_size_t i;
      int v;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->up_row + 1,
          pp = prev_row + 1; i < row_bytes; i++)
      {
         v = *dp++ = (png_byte)(((int)*rp++ - (int)*pp++) & 0xff);
         sum += 128 - abs(v - 128);

         if (sum > lmins)
            break;
      }

      if (sum < mins)
      {
         mins = sum;
         best_row = png_ptr->up_row;
      }
   }

   if (filter_to_do == PNG_FILTER_AVG)
   {
      png_bytep rp, dp, pp, lp;
      png_uint_32 i;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->avg_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         *dp++ = (png_byte)(((int)*rp++ - ((int)*pp++ / 2)) & 0xff);
      }

      for (lp = row_buf + 1; i < row_bytes; i++)
      {
         *dp++ = (png_byte)(((int)*rp++ - (((int)*pp++ + (int)*lp++) / 2))
                 & 0xff);
      }
      best_row = png_ptr->avg_row;
   }

   else if (filter_to_do & PNG_FILTER_AVG)
   {
      png_bytep rp, dp, pp, lp;
      png_uint_32 sum = 0, lmins = mins;
      png_size_t i;
      int v;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->avg_row + 1,
           pp = prev_row + 1; i < bpp; i++)
      {
         v = *dp++ = (png_byte)(((int)*rp++ - ((int)*pp++ / 2)) & 0xff);
         sum += 128 - abs(v - 128);
      }

      for (lp = row_buf + 1; i < row_bytes; i++)
      {
         v = *dp++ =
             (png_byte)(((int)*rp++ - (((int)*pp++ + (int)*lp++) / 2)) & 0xff);
         sum += 128 - abs(v - 128);

         if (sum > lmins)
            break;
      }

      if (sum < mins)
      {
         mins = sum;
         best_row = png_ptr->avg_row;
      }
   }

   if (filter_to_do == PNG_FILTER_PAETH)
   {
      png_bytep rp, dp, pp, cp, lp;
      png_size_t i;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->paeth_row + 1,
          pp = prev_row + 1; i < bpp; i++)
      {
         *dp++ = (png_byte)(((int)*rp++ - (int)*pp++) & 0xff);
      }

      for (lp = row_buf + 1, cp = prev_row + 1; i < row_bytes; i++)
      {
         int a, b, c, pa, pb, pc, p;

         b = *pp++;
         c = *cp++;
         a = *lp++;

         p = b - c;
         pc = a - c;

         pa = abs(p);
         pb = abs(pc);
         pc = abs(p + pc);

         p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

         *dp++ = (png_byte)(((int)*rp++ - p) & 0xff);
      }
      best_row = png_ptr->paeth_row;
   }

   else if (filter_to_do & PNG_FILTER_PAETH)
   {
      png_bytep rp, dp, pp, cp, lp;
      png_uint_32 sum = 0, lmins = mins;
      png_size_t i;
      int v;

      for (i = 0, rp = row_buf + 1, dp = png_ptr->paeth_row + 1,
          pp = prev_row + 1; i < bpp; i++)
      {
         v = *dp++ = (png_byte)(((int)*rp++ - (int)*pp++) & 0xff);
         sum += 128 - abs(v - 128);
      }

      for (lp = row_buf + 1, cp = prev_row + 1; i < row_bytes; i++)
      {
         int a, b, c, pa, pb, pc, p;

         b = *pp++;
         c = *cp++;
         a = *lp++;

#ifndef PNG_SLOW_PAETH
         p = b - c;
         pc = a - c;
         pa = abs(p);
         pb = abs(pc);
         pc = abs(p + pc);
         p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;
#else
         p = a + b - c;
         pa = abs(p - a);
         pb = abs(p - b);
         pc = abs(p - c);

         if (pa <= pb && pa <= pc)
            p = a;

         else if (pb <= pc)
            p = b;

         else
            p = c;
#endif /* PNG_SLOW_PAETH */

         v = *dp++ = (png_byte)(((int)*rp++ - p) & 0xff);
         sum += 128 - abs(v - 128);

         if (sum > lmins)
            break;
      }

      if (sum < mins)
      {
         best_row = png_ptr->paeth_row;
      }
   }
#endif /* PNG_WRITE_FILTER_SUPPORTED */

   png_write_filtered_row(png_ptr, best_row, row_info->rowbytes+1);
}

static void PNGAPI
png_write_filtered_row(png_structp png_ptr, png_bytep filtered_row,
   png_size_t avail)
{
   png_debug(1, "in png_write_filtered_row");
   png_debug1(2, "filter = %d", filtered_row[0]);

   png_ptr->zstream.next_in = filtered_row;
   png_ptr->zstream.avail_in = 0;
   do
   {
      int ret;

      if (png_ptr->zstream.avail_in == 0)
      {
         if (avail > ZLIB_IO_MAX)
         {
            png_ptr->zstream.avail_in  = ZLIB_IO_MAX;
            avail -= ZLIB_IO_MAX;
         }

         else
         {
            png_ptr->zstream.avail_in = (uInt)avail;
            avail = 0;
         }
      }

      ret = deflate(&png_ptr->zstream, Z_NO_FLUSH);

      if (!(png_ptr->zstream.avail_out))
      {
         png_write_IDAT(png_ptr, png_ptr->zbuf, png_ptr->zbuf_size);
      }
   } while (avail > 0 || png_ptr->zstream.avail_in > 0);

   if (png_ptr->prev_row != NULL)
   {
      png_bytep tptr;

      tptr = png_ptr->prev_row;
      png_ptr->prev_row = png_ptr->row_buf;
      png_ptr->row_buf = tptr;
   }

   png_write_finish_row(png_ptr);

#ifdef PNG_WRITE_FLUSH_SUPPORTED
   png_ptr->flush_rows++;

   if (png_ptr->flush_dist > 0 &&
       png_ptr->flush_rows >= png_ptr->flush_dist)
   {
      png_write_flush(png_ptr);
   }
#endif
}
#endif /* PNG_WRITE_SUPPORTED */
