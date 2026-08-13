/*
 * transupp.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1997-2019, Thomas G. Lane, Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2017, 2021, 2026, D. R. Commander.
 * Copyright (C) 2026, Ricardo M. Ferreira.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file contains declarations for image transformation routines and
 * other utility code used by the jpegtran sample application.  These are
 * NOT part of the core JPEG library.  But we keep these routines separate
 * from jpegtran.c to ease the task of maintaining jpegtran-like programs
 * that have other user interfaces.
 *
 * NOTE: all the routines declared here have very specific requirements
 * about when they are to be executed during the reading and writing of the
 * source and destination files.  See the comments in transupp.c, or see
 * jpegtran.c for an example of correct usage.
 */

#ifndef TRANSFORMS_SUPPORTED
#define TRANSFORMS_SUPPORTED  1
#endif

typedef enum {
  JXFORM_NONE,            /* no transformation */
  JXFORM_FLIP_H,          /* horizontal flip */
  JXFORM_FLIP_V,          /* vertical flip */
  JXFORM_TRANSPOSE,       /* transpose across UL-to-LR axis */
  JXFORM_TRANSVERSE,      /* transpose across UR-to-LL axis */
  JXFORM_ROT_90,          /* 90-degree clockwise rotation */
  JXFORM_ROT_180,         /* 180-degree rotation */
  JXFORM_ROT_270,         /* 270-degree clockwise (or 90 ccw) */
  JXFORM_WIPE,            /* wipe */
  JXFORM_DROP,            /* drop */
  JXFORM_ROLL             /* roll (shift with wraparound) */
} JXFORM_CODE;

typedef enum {
  JCROP_UNSET,
  JCROP_POS,
  JCROP_NEG,
  JCROP_FORCE,
  JCROP_REFLECT
} JCROP_CODE;

typedef struct {
  JXFORM_CODE transform;        /* image transform operator */
  boolean perfect;              /* if TRUE, fail if partial MCUs are requested */
  boolean trim;                 /* if TRUE, trim partial MCUs as needed */
  boolean force_grayscale;      /* if TRUE, convert color image to grayscale */
  boolean crop;                 /* if TRUE, crop or wipe source image, or drop */
  boolean slow_hflip;  /* For best performance, the JXFORM_FLIP_H transform
                          normally modifies the source coefficients in place.
                          Setting this to TRUE will instead use a slower,
                          double-buffered algorithm, which leaves the source
                          coefficients in tact (necessary if other transformed
                          images must be generated from the same set of
                          coefficients. */

  JDIMENSION crop_width;        /* Width of selected region */
  JCROP_CODE crop_width_set;    /* (force-disables adjustment) */
  JDIMENSION crop_height;       /* Height of selected region */
  JCROP_CODE crop_height_set;   /* (force-disables adjustment) */
  JDIMENSION crop_xoffset;      /* X offset of selected region */
  JCROP_CODE crop_xoffset_set;  /* (negative measures from right edge) */
  JDIMENSION crop_yoffset;      /* Y offset of selected region */
  JCROP_CODE crop_yoffset_set;  /* (negative measures from bottom edge) */

  /* Drop parameters: set by caller for drop request */
  j_decompress_ptr drop_ptr;
  jvirt_barray_ptr *drop_coef_arrays;

  /* Internal workspace: caller should not touch these */
  int num_components;           /* # of components in workspace */
  jvirt_barray_ptr *workspace_coef_arrays; /* workspace for transformations */
  JDIMENSION output_width;      /* cropped destination dimensions */
  JDIMENSION output_height;
  JDIMENSION x_crop_offset;     /* destination crop offsets measured in iMCUs */
  JDIMENSION y_crop_offset;
  JDIMENSION drop_width;        /* drop/wipe dimensions measured in iMCUs */
  JDIMENSION drop_height;
  int iMCU_sample_width;        /* destination iMCU size */
  int iMCU_sample_height;
} jpeg_transform_info;

#if TRANSFORMS_SUPPORTED

/* Parse a crop specification (written in X11 geometry style) */
EXTERN(boolean) jtransform_parse_crop_spec(jpeg_transform_info *info,
                                           const char *spec);
/* Request any required workspace */
EXTERN(boolean) jtransform_request_workspace(j_decompress_ptr srcinfo,
                                             jpeg_transform_info *info);
/* Adjust output image parameters */
EXTERN(jvirt_barray_ptr *) jtransform_adjust_parameters
  (j_decompress_ptr srcinfo, j_compress_ptr dstinfo,
   jvirt_barray_ptr *src_coef_arrays, jpeg_transform_info *info);
/* Execute the actual transformation, if any */
EXTERN(void) jtransform_execute_transform(j_decompress_ptr srcinfo,
                                          j_compress_ptr dstinfo,
                                          jvirt_barray_ptr *src_coef_arrays,
                                          jpeg_transform_info *info);
EXTERN(boolean) jtransform_perfect_transform(JDIMENSION image_width,
                                             JDIMENSION image_height,
                                             int MCU_width, int MCU_height,
                                             jpeg_transform_info *info);

#define jtransform_execute_transformation       jtransform_execute_transform

#endif /* TRANSFORMS_SUPPORTED */

typedef enum {
  JCOPYOPT_NONE,           /* copy no optional markers */
  JCOPYOPT_COMMENTS,       /* copy only comment (COM) markers */
  JCOPYOPT_ALL,            /* copy all optional markers */
  JCOPYOPT_ALL_EXCEPT_ICC, /* copy all optional markers except APP2 */
  JCOPYOPT_ICC             /* copy only ICC profile (APP2) markers */
} JCOPY_OPTION;

#define JCOPYOPT_DEFAULT  JCOPYOPT_COMMENTS    /* recommended default */

/* Setup decompression object to save desired markers in memory */
EXTERN(void) jcopy_markers_setup(j_decompress_ptr srcinfo,
                                 JCOPY_OPTION option);
/* Copy markers saved in the given source object to the destination object */
EXTERN(void) jcopy_markers_execute(j_decompress_ptr srcinfo,
                                   j_compress_ptr dstinfo,
                                   JCOPY_OPTION option);
