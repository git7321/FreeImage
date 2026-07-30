/* pngwio.c - functions for data output
 *
 * libpng version 1.5.20
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "png.h"

#ifdef PNG_WRITE_SUPPORTED

void PNGAPI
png_write_data(png_structp png_ptr, png_const_bytep data, png_size_t length)
{
   if (png_ptr->write_data_fn != NULL )
      (*(png_ptr->write_data_fn))(png_ptr, (png_bytep)data, length);
}

#ifdef PNG_STDIO_SUPPORTED
#ifndef USE_FAR_KEYWORD
void PNGCBAPI
png_default_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_size_t check;

   if (png_ptr == NULL)
      return;

   check = fwrite(data, 1, length, (png_FILE_p)(png_ptr->io_ptr));
}
#else

#define NEAR_BUF_SIZE 1024
#define MIN(a,b) (a <= b ? a : b)

void PNGCBAPI
png_default_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
   png_uint_32 check;
   png_byte *near_data;
   png_FILE_p io_ptr;

   if (png_ptr == NULL)
      return;

   near_data = (png_byte *)CVT_PTR_NOCHECK(data);
   io_ptr = (png_FILE_p)CVT_PTR(png_ptr->io_ptr);

   if ((png_bytep)near_data == data)
   {
      check = fwrite(near_data, 1, length, io_ptr);
   }

   else
   {
      png_byte buf[NEAR_BUF_SIZE];
      png_size_t written, remaining, err;
      check = 0;
      remaining = length;

      do
      {
         written = MIN(NEAR_BUF_SIZE, remaining);
         png_memcpy(buf, data, written);
         err = fwrite(buf, 1, written, io_ptr);

         if (err != written)
            break;

         else
            check += err;

         data += written;
         remaining -= written;
      }
      while (remaining != 0);
   }
}

#endif
#endif

#ifdef PNG_WRITE_FLUSH_SUPPORTED
void PNGAPI
png_flush(png_structp png_ptr)
{
   if (png_ptr->output_flush_fn != NULL)
      (*(png_ptr->output_flush_fn))(png_ptr);
}

#  ifdef PNG_STDIO_SUPPORTED
void PNGCBAPI
png_default_flush(png_structp png_ptr)
{
   png_FILE_p io_ptr;

   if (png_ptr == NULL)
      return;

   io_ptr = (png_FILE_p)CVT_PTR((png_ptr->io_ptr));
   fflush(io_ptr);
}
#  endif
#endif

void PNGAPI
png_set_write_fn(png_structp png_ptr, png_voidp io_ptr,
    png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn)
{
   if (png_ptr == NULL)
      return;

   png_ptr->io_ptr = io_ptr;

#ifdef PNG_STDIO_SUPPORTED
   if (write_data_fn != NULL)
      png_ptr->write_data_fn = write_data_fn;

   else
      png_ptr->write_data_fn = png_default_write_data;
#else
   png_ptr->write_data_fn = write_data_fn;
#endif

#ifdef PNG_WRITE_FLUSH_SUPPORTED
#  ifdef PNG_STDIO_SUPPORTED

   if (output_flush_fn != NULL)
      png_ptr->output_flush_fn = output_flush_fn;

   else
      png_ptr->output_flush_fn = png_default_flush;

#  else
   png_ptr->output_flush_fn = output_flush_fn;
#  endif
#else
   PNG_UNUSED(output_flush_fn)
#endif /* PNG_WRITE_FLUSH_SUPPORTED */

   if (png_ptr->read_data_fn != NULL)
   {
      png_ptr->read_data_fn = NULL;
   }
}

#ifdef USE_FAR_KEYWORD
#  ifdef _MSC_VER
void * PNGAPI png_far_to_near(png_structp png_ptr, png_voidp ptr, int check)
{
   void *near_ptr;
   void FAR *far_ptr;
   FP_OFF(near_ptr) = FP_OFF(ptr);
   far_ptr = (void FAR *)near_ptr;

   return(near_ptr);
}
#  else
void * PNGAPI png_far_to_near(png_structp png_ptr, png_voidp ptr, int check)
{
   void *near_ptr;
   void FAR *far_ptr;
   near_ptr = (void FAR *)ptr;
   far_ptr = (void FAR *)near_ptr;

   return(near_ptr);
}
#  endif
#endif
#endif /* PNG_WRITE_SUPPORTED */
