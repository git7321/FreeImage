/* pngmem.c - stub functions for memory allocation
 *
 * libpng version 1.5.20
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 */

#include "png.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)

PNG_FUNCTION(png_voidp,PNGCAPI
png_create_struct,(int type),PNG_ALLOCATED)
{
#  ifdef PNG_USER_MEM_SUPPORTED
   return (png_create_struct_2(type, NULL, NULL));
}

PNG_FUNCTION(png_voidp,PNGCAPI
png_create_struct_2,(int type, png_malloc_ptr malloc_fn, png_voidp mem_ptr),
   PNG_ALLOCATED)
{
#  endif /* PNG_USER_MEM_SUPPORTED */
   png_size_t size;
   png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
      size = png_sizeof(png_info);

   else if (type == PNG_STRUCT_PNG)
      size = png_sizeof(png_struct);

   else
      return (NULL);

#  ifdef PNG_USER_MEM_SUPPORTED
   if (malloc_fn != NULL)
   {
      png_struct dummy_struct;
      png_structp png_ptr = &dummy_struct;
      png_ptr->mem_ptr=mem_ptr;
      struct_ptr = (*(malloc_fn))(png_ptr, size);

      if (struct_ptr != NULL)
         png_memset(struct_ptr, 0, size);

      return (struct_ptr);
   }
#  endif /* PNG_USER_MEM_SUPPORTED */

# if defined(_MSC_VER) && defined(MAXSEG_64K)
   struct_ptr = (png_voidp)halloc(size, 1);
# else
   struct_ptr = (png_voidp)malloc(size);
# endif

   if (struct_ptr != NULL)
      png_memset(struct_ptr, 0, size);

   return (struct_ptr);
}

void PNGCAPI
png_destroy_struct(png_voidp struct_ptr)
{
#  ifdef PNG_USER_MEM_SUPPORTED
   png_destroy_struct_2(struct_ptr, NULL, NULL);
}

void PNGCAPI
png_destroy_struct_2(png_voidp struct_ptr, png_free_ptr free_fn,
    png_voidp mem_ptr)
{
#  endif /* PNG_USER_MEM_SUPPORTED */
   if (struct_ptr != NULL)
   {
#  ifdef PNG_USER_MEM_SUPPORTED
      if (free_fn != NULL)
      {
         png_struct dummy_struct;
         png_structp png_ptr = &dummy_struct;
         png_ptr->mem_ptr=mem_ptr;
         (*(free_fn))(png_ptr, struct_ptr);
         return;
      }
#  endif /* PNG_USER_MEM_SUPPORTED */
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   hfree(struct_ptr);
# else
   free(struct_ptr);
# endif
   }
}

PNG_FUNCTION(png_voidp,PNGCAPI
png_calloc,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

   ret = (png_malloc(png_ptr, size));

   if (ret != NULL)
      png_memset(ret,0,(png_size_t)size);

   return (ret);
}

PNG_FUNCTION(png_voidp,PNGCAPI
png_malloc,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;

#  ifdef PNG_USER_MEM_SUPPORTED
   if (png_ptr == NULL || size == 0)
      return (NULL);

   if (png_ptr->malloc_fn != NULL)
      ret = ((png_voidp)(*(png_ptr->malloc_fn))(png_ptr, (png_size_t)size));

   else
      ret = (png_malloc_default(png_ptr, size));

   return (ret);
}

PNG_FUNCTION(png_voidp,PNGCAPI
png_malloc_default,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ret;
#  endif /* PNG_USER_MEM_SUPPORTED */

   if (png_ptr == NULL || size == 0)
      return (NULL);

#  ifdef PNG_MAX_MALLOC_64K
   if (size > (png_uint_32)65536L)
   {
      return NULL;
   }
#  endif

# if defined(_MSC_VER) && defined(MAXSEG_64K)
   if (size != (unsigned long)size)
      ret = NULL;
   else
      ret = halloc(size, 1);
# else
   if (size != (size_t)size)
      ret = NULL;
   else
      ret = malloc((size_t)size);
# endif

   return (ret);
}

void PNGCAPI
png_free(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

#  ifdef PNG_USER_MEM_SUPPORTED
   if (png_ptr->free_fn != NULL)
   {
      (*(png_ptr->free_fn))(png_ptr, ptr);
      return;
   }

   else
      png_free_default(png_ptr, ptr);
}

void PNGCAPI
png_free_default(png_structp png_ptr, png_voidp ptr)
{
   if (png_ptr == NULL || ptr == NULL)
      return;

#  endif /* PNG_USER_MEM_SUPPORTED */

# if defined(_MSC_VER) && defined(MAXSEG_64K)
   hfree(ptr);
# else
   free(ptr);
# endif
}

PNG_FUNCTION(png_voidp,PNGCAPI
png_malloc_warn,(png_structp png_ptr, png_alloc_size_t size),PNG_ALLOCATED)
{
   png_voidp ptr;
   png_uint_32 save_flags;
   if (png_ptr == NULL)
      return (NULL);

   save_flags = png_ptr->flags;
   png_ptr->flags|=PNG_FLAG_MALLOC_NULL_MEM_OK;
   ptr = (png_voidp)png_malloc((png_structp)png_ptr, size);
   png_ptr->flags=save_flags;
   return(ptr);
}

#ifdef PNG_USER_MEM_SUPPORTED
void PNGCAPI
png_set_mem_fn(png_structp png_ptr, png_voidp mem_ptr, png_malloc_ptr
  malloc_fn, png_free_ptr free_fn)
{
   if (png_ptr != NULL)
   {
      png_ptr->mem_ptr = mem_ptr;
      png_ptr->malloc_fn = malloc_fn;
      png_ptr->free_fn = free_fn;
   }
}

png_voidp PNGCAPI
png_get_mem_ptr(png_const_structp png_ptr)
{
   if (png_ptr == NULL)
      return (NULL);

   return ((png_voidp)png_ptr->mem_ptr);
}
#endif /* PNG_USER_MEM_SUPPORTED */
#endif /* PNG_READ_SUPPORTED || PNG_WRITE_SUPPORTED */
