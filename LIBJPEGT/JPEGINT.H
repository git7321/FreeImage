/*
 * jpegint.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * Lossless JPEG Modifications:
 * Copyright (C) 1999, Ken Murchison.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2015-2017, 2019, 2021-2022, 2024-2026, D. R. Commander.
 * Copyright (C) 2015, Google, Inc.
 * Copyright (C) 2021, Alex Richardson.
 * For conditions of distribution and use, see the accompanying README.ijg
 * file.
 *
 * This file provides common declarations for the various JPEG modules.
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */

typedef int JDIFF;

typedef JDIFF FAR *JDIFFROW;    /* pointer to one row of difference values */
typedef JDIFFROW *JDIFFARRAY;   /* ptr to some rows (a 2-D diff array) */
typedef JDIFFARRAY *JDIFFIMAGE; /* a 3-D diff array: top index is color */

typedef enum {            /* Operating modes for buffer controllers */
  JBUF_PASS_THRU,         /* Plain stripwise operation */
  /* Remaining modes require a full-image buffer to have been created */
  JBUF_SAVE_SOURCE,       /* Run source subobject only, save output */
  JBUF_CRANK_DEST,        /* Run dest subobject only, using saved data */
  JBUF_SAVE_AND_PASS      /* Run both subobjects, save output */
} J_BUF_MODE;

/* Values of global_state field (jdapi.c has some dependencies on ordering!) */
#define CSTATE_START     100    /* after create_compress */
#define CSTATE_SCANNING  101    /* start_compress done, write_scanlines OK */
#define CSTATE_RAW_OK    102    /* start_compress done, write_raw_data OK */
#define CSTATE_WRCOEFS   103    /* jpeg_write_coefficients done */
#define DSTATE_START     200    /* after create_decompress */
#define DSTATE_INHEADER  201    /* reading header markers, no SOS yet */
#define DSTATE_READY     202    /* found SOS, ready for start_decompress */
#define DSTATE_PRELOAD   203    /* reading multiscan file in start_decompress*/
#define DSTATE_PRESCAN   204    /* performing dummy pass for 2-pass quant */
#define DSTATE_SCANNING  205    /* start_decompress done, read_scanlines OK */
#define DSTATE_RAW_OK    206    /* start_decompress done, read_raw_data OK */
#define DSTATE_BUFIMAGE  207    /* expecting jpeg_start_output */
#define DSTATE_BUFPOST   208    /* looking for SOS/EOI in jpeg_finish_output */
#define DSTATE_RDCOEFS   209    /* reading file in jpeg_read_coefficients */
#define DSTATE_STOPPING  210    /* looking for EOI in jpeg_finish_decompress */

typedef long JLONG;

#ifdef __UINTPTR_TYPE__
typedef __UINTPTR_TYPE__ JUINTPTR;
#else
typedef size_t JUINTPTR;
#endif

#define IsExtRGB(cs) \
  (cs == JCS_RGB || (cs >= JCS_EXT_RGB && cs <= JCS_EXT_ARGB))

#define LEFT_SHIFT(a, b)  ((JLONG)((unsigned long)(a) << (b)))

struct jpeg_comp_master {
  void (__cdecl *prepare_for_pass) (j_compress_ptr cinfo);
  void (__cdecl *pass_startup) (j_compress_ptr cinfo);
  void (__cdecl *finish_pass) (j_compress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean call_pass_startup;    /* True if pass_startup must be called */
  boolean is_last_pass;         /* True during last pass */
  boolean lossless;             /* True if lossless mode is enabled */

  /* SIMD-specific variables */
  unsigned int simd_support;
  unsigned int simd_huffman;

#ifdef WITH_PROFILE
  double start, total_start, total_elapsed;
  double cconvert_elapsed, cconvert_mpixels;
  double downsample_elapsed, downsample_msamples;
  double convsamp_elapsed, convsamp_msamples;
  double fdct_elapsed, fdct_mcoeffs;
  double quantize_elapsed, quantize_mcoeffs;
  double entropy_elapsed, entropy_mcoeffs;
#endif
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_c_main_controller {
  void (__cdecl *start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (__cdecl *process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                        JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail);
  void (__cdecl *process_data_12) (j_compress_ptr cinfo, J12SAMPARRAY input_buf,
                           JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail);
#ifdef C_LOSSLESS_SUPPORTED
  void (__cdecl *process_data_16) (j_compress_ptr cinfo, J16SAMPARRAY input_buf,
                           JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail);
#endif
};

/* Compression preprocessing (downsampling input buffer control) */
struct jpeg_c_prep_controller {
  void (__cdecl *start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (__cdecl *pre_process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                            JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail,
                            JSAMPIMAGE output_buf,
                            JDIMENSION *out_row_group_ctr,
                            JDIMENSION out_row_groups_avail);
  void (__cdecl *pre_process_data_12) (j_compress_ptr cinfo, J12SAMPARRAY input_buf,
                               JDIMENSION *in_row_ctr,
                               JDIMENSION in_rows_avail,
                               J12SAMPIMAGE output_buf,
                               JDIMENSION *out_row_group_ctr,
                               JDIMENSION out_row_groups_avail);
#ifdef C_LOSSLESS_SUPPORTED
  void (__cdecl *pre_process_data_16) (j_compress_ptr cinfo, J16SAMPARRAY input_buf,
                               JDIMENSION *in_row_ctr,
                               JDIMENSION in_rows_avail,
                               J16SAMPIMAGE output_buf,
                               JDIMENSION *out_row_group_ctr,
                               JDIMENSION out_row_groups_avail);
#endif
};

struct jpeg_c_coef_controller {
  void (__cdecl *start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  boolean (__cdecl *compress_data) (j_compress_ptr cinfo, JSAMPIMAGE input_buf);
  boolean (__cdecl *compress_data_12) (j_compress_ptr cinfo, J12SAMPIMAGE input_buf);
#ifdef C_LOSSLESS_SUPPORTED
  boolean (__cdecl *compress_data_16) (j_compress_ptr cinfo, J16SAMPIMAGE input_buf);
#endif
};

/* Colorspace conversion */
struct jpeg_color_converter {
  void (__cdecl *start_pass) (j_compress_ptr cinfo);
  void (__cdecl *color_convert) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                         JSAMPIMAGE output_buf, JDIMENSION output_row,
                         int num_rows);
  void (__cdecl *color_convert_12) (j_compress_ptr cinfo, J12SAMPARRAY input_buf,
                            J12SAMPIMAGE output_buf, JDIMENSION output_row,
                            int num_rows);
#ifdef C_LOSSLESS_SUPPORTED
  void (__cdecl *color_convert_16) (j_compress_ptr cinfo, J16SAMPARRAY input_buf,
                            J16SAMPIMAGE output_buf, JDIMENSION output_row,
                            int num_rows);
#endif
  void (__cdecl *color_convert_simd) (JDIMENSION img_width, JSAMPARRAY input_buf,
                              JSAMPIMAGE output_buf, JDIMENSION output_row,
                              int num_rows);
};

/* Downsampling */
struct jpeg_downsampler {
  void (__cdecl *start_pass) (j_compress_ptr cinfo);
  void (__cdecl *downsample) (j_compress_ptr cinfo, JSAMPIMAGE input_buf,
                      JDIMENSION in_row_index, JSAMPIMAGE output_buf,
                      JDIMENSION out_row_group_index);
  void (__cdecl *downsample_12) (j_compress_ptr cinfo, J12SAMPIMAGE input_buf,
                         JDIMENSION in_row_index, J12SAMPIMAGE output_buf,
                         JDIMENSION out_row_group_index);
#ifdef C_LOSSLESS_SUPPORTED
  void (__cdecl *downsample_16) (j_compress_ptr cinfo, J16SAMPIMAGE input_buf,
                         JDIMENSION in_row_index, J16SAMPIMAGE output_buf,
                         JDIMENSION out_row_group_index);
#endif
  void (__cdecl *h2v1_downsample_simd) (JDIMENSION image_width, int max_v_samp_factor,
                                JDIMENSION v_samp_factor,
                                JDIMENSION width_blocks, JSAMPARRAY input_data,
                                JSAMPARRAY output_data);
  void (__cdecl *h2v2_downsample_simd) (JDIMENSION image_width, int max_v_samp_factor,
                                JDIMENSION v_samp_factor,
                                JDIMENSION width_blocks, JSAMPARRAY input_data,
                                JSAMPARRAY output_data);

  boolean need_context_rows;    /* TRUE if need rows above & below */
};

struct jpeg_forward_dct {
  void (__cdecl *start_pass) (j_compress_ptr cinfo);

  void (__cdecl *forward_DCT) (j_compress_ptr cinfo, jpeg_component_info *compptr,
                       JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
                       JDIMENSION start_row, JDIMENSION start_col,
                       JDIMENSION num_blocks);
  void (__cdecl *forward_DCT_12) (j_compress_ptr cinfo, jpeg_component_info *compptr,
                          J12SAMPARRAY sample_data, JBLOCKROW coef_blocks,
                          JDIMENSION start_row, JDIMENSION start_col,
                          JDIMENSION num_blocks);
};

/* Entropy encoding */
struct jpeg_entropy_encoder {
  void (__cdecl *start_pass) (j_compress_ptr cinfo, boolean gather_statistics);

  /* Lossy mode */
  boolean (__cdecl *encode_mcu) (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
  /* Lossless mode */
  JDIMENSION (__cdecl *encode_mcus) (j_compress_ptr cinfo, JDIFFIMAGE diff_buf,
                             JDIMENSION MCU_row_num, JDIMENSION MCU_col_num,
                             JDIMENSION nMCU);
  JOCTET *(__cdecl *huff_encode_one_block_simd) (void *state, JOCTET *buffer,
                                         JCOEFPTR block, int last_dc_val,
                                         void *dctbl, void *actbl);

  void (__cdecl *finish_pass) (j_compress_ptr cinfo);
};

/* Marker writing */
struct jpeg_marker_writer {
  void (__cdecl *write_file_header) (j_compress_ptr cinfo);
  void (__cdecl *write_frame_header) (j_compress_ptr cinfo);
  void (__cdecl *write_scan_header) (j_compress_ptr cinfo);
  void (__cdecl *write_file_trailer) (j_compress_ptr cinfo);
  void (__cdecl *write_tables_only) (j_compress_ptr cinfo);
  void (__cdecl *write_marker_header) (j_compress_ptr cinfo, int marker,
                               unsigned int datalen);
  void (__cdecl *write_marker_byte) (j_compress_ptr cinfo, int val);
};

/* Master control module */
struct jpeg_decomp_master {
  void (__cdecl *prepare_for_output_pass) (j_decompress_ptr cinfo);
  void (__cdecl *finish_output_pass) (j_decompress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean is_dummy_pass;        /* True during 1st pass for 2-pass quant */
  boolean lossless;             /* True if decompressing a lossless image */

  /* Partial decompression variables */
  JDIMENSION first_iMCU_col;
  JDIMENSION last_iMCU_col;
  JDIMENSION first_MCU_col[MAX_COMPONENTS];
  JDIMENSION last_MCU_col[MAX_COMPONENTS];
  boolean jinit_upsampler_no_alloc;

  /* Last iMCU row that was successfully decoded */
  JDIMENSION last_good_iMCU_row;

  /* Tail of list of saved markers */
  jpeg_saved_marker_ptr marker_list_end;

  /* SIMD-specific variables */
  unsigned int simd_support;
  unsigned int simd_huffman;

#ifdef WITH_PROFILE
  double start, total_start, total_elapsed;
  double entropy_elapsed, entropy_mcoeffs;
  double idct_elapsed, idct_mcoeffs;
  double merged_upsample_elapsed, merged_upsample_mpixels;
  double upsample_elapsed, upsample_msamples;
  double cconvert_elapsed, cconvert_mpixels;
#endif

  int jpeg_data_precision;
};

/* Input control module */
struct jpeg_input_controller {
  int (__cdecl *consume_input) (j_decompress_ptr cinfo);
  void (__cdecl *reset_input_controller) (j_decompress_ptr cinfo);
  void (__cdecl *start_input_pass) (j_decompress_ptr cinfo);
  void (__cdecl *finish_input_pass) (j_decompress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean has_multiple_scans;   /* True if file has multiple scans */
  boolean eoi_reached;          /* True when EOI has been consumed */
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_d_main_controller {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (__cdecl *process_data) (j_decompress_ptr cinfo, JSAMPARRAY output_buf,
                        JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
  void (__cdecl *process_data_12) (j_decompress_ptr cinfo, J12SAMPARRAY output_buf,
                           JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
#ifdef D_LOSSLESS_SUPPORTED
  void (__cdecl *process_data_16) (j_decompress_ptr cinfo, J16SAMPARRAY output_buf,
                           JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
#endif
};

struct jpeg_d_coef_controller {
  void (__cdecl *start_input_pass) (j_decompress_ptr cinfo);
  int (__cdecl *consume_data) (j_decompress_ptr cinfo);
  void (__cdecl *start_output_pass) (j_decompress_ptr cinfo);
  int (__cdecl *decompress_data) (j_decompress_ptr cinfo, JSAMPIMAGE output_buf);
  int (__cdecl *decompress_data_12) (j_decompress_ptr cinfo, J12SAMPIMAGE output_buf);
#ifdef D_LOSSLESS_SUPPORTED
  int (__cdecl *decompress_data_16) (j_decompress_ptr cinfo, J16SAMPIMAGE output_buf);
#endif

  JDIMENSION MCU_ctr;           /* counts MCUs processed in current row */
  int MCU_vert_offset;          /* counts MCU rows within iMCU row */
  int MCU_rows_per_iMCU_row;    /* number of such rows needed */

  jvirt_barray_ptr *coef_arrays;
};

/* Decompression postprocessing (color quantization buffer control) */
struct jpeg_d_post_controller {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (__cdecl *post_process_data) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                             JDIMENSION *in_row_group_ctr,
                             JDIMENSION in_row_groups_avail,
                             JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
                             JDIMENSION out_rows_avail);
  void (__cdecl *post_process_data_12) (j_decompress_ptr cinfo, J12SAMPIMAGE input_buf,
                                JDIMENSION *in_row_group_ctr,
                                JDIMENSION in_row_groups_avail,
                                J12SAMPARRAY output_buf,
                                JDIMENSION *out_row_ctr,
                                JDIMENSION out_rows_avail);
#ifdef D_LOSSLESS_SUPPORTED
  void (__cdecl *post_process_data_16) (j_decompress_ptr cinfo, J16SAMPIMAGE input_buf,
                                JDIMENSION *in_row_group_ctr,
                                JDIMENSION in_row_groups_avail,
                                J16SAMPARRAY output_buf,
                                JDIMENSION *out_row_ctr,
                                JDIMENSION out_rows_avail);
#endif
};

/* Marker reading & parsing */
struct jpeg_marker_reader {
  void (__cdecl *reset_marker_reader) (j_decompress_ptr cinfo);
  int (__cdecl *read_markers) (j_decompress_ptr cinfo);
  jpeg_marker_parser_method read_restart_marker;

  boolean saw_SOI;              /* found SOI? */
  boolean saw_SOF;              /* found SOF? */
  int next_restart_num;         /* next restart number expected (0-7) */
  unsigned int discarded_bytes; /* # of bytes skipped looking for a marker */
};

/* Entropy decoding */
struct jpeg_entropy_decoder {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo);

  /* Lossy mode */
  boolean (__cdecl *decode_mcu) (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);
  /* Lossless mode */
  JDIMENSION (__cdecl *decode_mcus) (j_decompress_ptr cinfo, JDIFFIMAGE diff_buf,
                             JDIMENSION MCU_row_num, JDIMENSION MCU_col_num,
                             JDIMENSION nMCU);
  boolean (__cdecl *process_restart) (j_decompress_ptr cinfo);

  boolean insufficient_data;    /* set TRUE after emitting warning */
};

typedef void (__cdecl *inverse_DCT_method_ptr) (j_decompress_ptr cinfo,
                                        jpeg_component_info *compptr,
                                        JCOEFPTR coef_block,
                                        JSAMPARRAY output_buf,
                                        JDIMENSION output_col);
typedef void (__cdecl *inverse_DCT_12_method_ptr) (j_decompress_ptr cinfo,
                                           jpeg_component_info *compptr,
                                           JCOEFPTR coef_block,
                                           J12SAMPARRAY output_buf,
                                           JDIMENSION output_col);

struct jpeg_inverse_dct {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo);

  inverse_DCT_method_ptr inverse_DCT[MAX_COMPONENTS];
  inverse_DCT_12_method_ptr inverse_DCT_12[MAX_COMPONENTS];

  void (__cdecl *idct_simd) (void *dct_table, JCOEFPTR coef_block,
                     JSAMPARRAY output_buf, JDIMENSION output_col);
  void (__cdecl *idct_2x2_simd) (void *dct_table, JCOEFPTR coef_block,
                         JSAMPARRAY output_buf, JDIMENSION output_col);
  void (__cdecl *idct_4x4_simd) (void *dct_table, JCOEFPTR coef_block,
                         JSAMPARRAY output_buf, JDIMENSION output_col);
};

struct jpeg_upsampler {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo);
  void (__cdecl *upsample) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                    JDIMENSION *in_row_group_ctr,
                    JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf,
                    JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
  void (__cdecl *upsample_12) (j_decompress_ptr cinfo, J12SAMPIMAGE input_buf,
                       JDIMENSION *in_row_group_ctr,
                       JDIMENSION in_row_groups_avail, J12SAMPARRAY output_buf,
                       JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
#ifdef D_LOSSLESS_SUPPORTED
  void (__cdecl *upsample_16) (j_decompress_ptr cinfo, J16SAMPIMAGE input_buf,
                       JDIMENSION *in_row_group_ctr,
                       JDIMENSION in_row_groups_avail, J16SAMPARRAY output_buf,
                       JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
#endif
  void (__cdecl *h2v1_upsample_simd) (int max_v_samp_factor, JDIMENSION output_width,
                              JSAMPARRAY input_data,
                              JSAMPARRAY *output_data_ptr);
  void (__cdecl *h2v2_upsample_simd) (int max_v_samp_factor, JDIMENSION output_width,
                              JSAMPARRAY input_data,
                              JSAMPARRAY *output_data_ptr);
  void (__cdecl *h1v2_upsample_simd) (int max_v_samp_factor, JDIMENSION output_width,
                              JSAMPARRAY input_data,
                              JSAMPARRAY *output_data_ptr);
  void (__cdecl *merged_upsample_simd) (JDIMENSION output_width, JSAMPIMAGE input_buf,
                                JDIMENSION in_row_group_ctr,
                                JSAMPARRAY output_buf);

  boolean need_context_rows;
};

/* Colorspace conversion */
struct jpeg_color_deconverter {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo);
  void (__cdecl *color_convert) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                         JDIMENSION input_row, JSAMPARRAY output_buf,
                         int num_rows);
  void (__cdecl *color_convert_12) (j_decompress_ptr cinfo, J12SAMPIMAGE input_buf,
                            JDIMENSION input_row, J12SAMPARRAY output_buf,
                            int num_rows);
#ifdef D_LOSSLESS_SUPPORTED
  void (__cdecl *color_convert_16) (j_decompress_ptr cinfo, J16SAMPIMAGE input_buf,
                            JDIMENSION input_row, J16SAMPARRAY output_buf,
                            int num_rows);
#endif
  void (__cdecl *color_convert_simd) (JDIMENSION out_width, JSAMPIMAGE input_buf,
                              JDIMENSION input_row, JSAMPARRAY output_buf,
                              int num_rows);
};

/* Color quantization or color precision reduction */
struct jpeg_color_quantizer {
  void (__cdecl *start_pass) (j_decompress_ptr cinfo, boolean is_pre_scan);
  void (__cdecl *color_quantize) (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
                          JSAMPARRAY output_buf, int num_rows);
  void (__cdecl *color_quantize_12) (j_decompress_ptr cinfo, J12SAMPARRAY input_buf,
                             J12SAMPARRAY output_buf, int num_rows);
  void (__cdecl *finish_pass) (j_decompress_ptr cinfo);
  void (__cdecl *new_color_map) (j_decompress_ptr cinfo);
};

/* Miscellaneous useful macros */
#undef MAX
#define MAX(a, b)       ((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a, b)       ((a) < (b) ? (a) : (b))

#ifdef ZERO_BUFFERS
#define MALLOC(size)  calloc(1, size)
#else
#define MALLOC(size)  malloc(size)
#endif

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS     JLONG shift_temp;
#define RIGHT_SHIFT(x, shft) \
  ((shift_temp = (x)) < 0 ? \
   (shift_temp >> (shft)) | ((~((JLONG)0)) << (32 - (shft))) : \
   (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x, shft)    ((x) >> (shft))
#endif

/* Compression module initialization routines */
EXTERN(void) jinit_compress_master(j_compress_ptr cinfo);
EXTERN(void) jinit_c_master_control(j_compress_ptr cinfo,
                                    boolean transcode_only);
EXTERN(void) jinit_c_main_controller(j_compress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_c_main_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_c_prep_controller(j_compress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_c_prep_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_c_coef_controller(j_compress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_c_coef_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_color_converter(j_compress_ptr cinfo);
EXTERN(void) j12init_color_converter(j_compress_ptr cinfo);
EXTERN(void) jinit_downsampler(j_compress_ptr cinfo);
EXTERN(void) j12init_downsampler(j_compress_ptr cinfo);
EXTERN(void) jinit_forward_dct(j_compress_ptr cinfo);
EXTERN(void) j12init_forward_dct(j_compress_ptr cinfo);
EXTERN(void) jinit_huff_encoder(j_compress_ptr cinfo);
EXTERN(void) jinit_phuff_encoder(j_compress_ptr cinfo);
EXTERN(void) jinit_arith_encoder(j_compress_ptr cinfo);
EXTERN(void) jinit_marker_writer(j_compress_ptr cinfo);
#ifdef C_LOSSLESS_SUPPORTED
EXTERN(void) j16init_c_main_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_c_prep_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_color_converter(j_compress_ptr cinfo);
EXTERN(void) j16init_downsampler(j_compress_ptr cinfo);
EXTERN(void) jinit_c_diff_controller(j_compress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_c_diff_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_c_diff_controller(j_compress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_lhuff_encoder(j_compress_ptr cinfo);
EXTERN(void) jinit_lossless_compressor(j_compress_ptr cinfo);
EXTERN(void) j12init_lossless_compressor(j_compress_ptr cinfo);
EXTERN(void) j16init_lossless_compressor(j_compress_ptr cinfo);
#endif

/* Decompression module initialization routines */
EXTERN(void) jinit_master_decompress(j_decompress_ptr cinfo);
EXTERN(void) jinit_d_main_controller(j_decompress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_d_main_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_d_coef_controller(j_decompress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_d_coef_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_d_post_controller(j_decompress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_d_post_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_input_controller(j_decompress_ptr cinfo);
EXTERN(void) jinit_marker_reader(j_decompress_ptr cinfo);
EXTERN(void) jinit_huff_decoder(j_decompress_ptr cinfo);
EXTERN(void) jinit_phuff_decoder(j_decompress_ptr cinfo);
EXTERN(void) jinit_arith_decoder(j_decompress_ptr cinfo);
EXTERN(void) jinit_inverse_dct(j_decompress_ptr cinfo);
EXTERN(void) j12init_inverse_dct(j_decompress_ptr cinfo);
EXTERN(void) jinit_upsampler(j_decompress_ptr cinfo);
EXTERN(void) j12init_upsampler(j_decompress_ptr cinfo);
EXTERN(void) jinit_color_deconverter(j_decompress_ptr cinfo);
EXTERN(void) j12init_color_deconverter(j_decompress_ptr cinfo);
EXTERN(void) jinit_1pass_quantizer(j_decompress_ptr cinfo);
EXTERN(void) j12init_1pass_quantizer(j_decompress_ptr cinfo);
EXTERN(void) jinit_2pass_quantizer(j_decompress_ptr cinfo);
EXTERN(void) j12init_2pass_quantizer(j_decompress_ptr cinfo);
EXTERN(void) jinit_merged_upsampler(j_decompress_ptr cinfo);
EXTERN(void) j12init_merged_upsampler(j_decompress_ptr cinfo);
#ifdef D_LOSSLESS_SUPPORTED
EXTERN(void) j16init_d_main_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_d_post_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_upsampler(j_decompress_ptr cinfo);
EXTERN(void) j16init_color_deconverter(j_decompress_ptr cinfo);
EXTERN(void) jinit_d_diff_controller(j_decompress_ptr cinfo,
                                     boolean need_full_buffer);
EXTERN(void) j12init_d_diff_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) j16init_d_diff_controller(j_decompress_ptr cinfo,
                                       boolean need_full_buffer);
EXTERN(void) jinit_lhuff_decoder(j_decompress_ptr cinfo);
EXTERN(void) jinit_lossless_decompressor(j_decompress_ptr cinfo);
EXTERN(void) j12init_lossless_decompressor(j_decompress_ptr cinfo);
EXTERN(void) j16init_lossless_decompressor(j_decompress_ptr cinfo);
#endif

/* Memory manager initialization */
EXTERN(void) jinit_memory_mgr(j_common_ptr cinfo);

/* Utility routines in jutils.c */
EXTERN(long) jdiv_round_up(long a, long b);
EXTERN(long) jround_up(long a, long b);
EXTERN(void) jcopy_sample_rows(JSAMPARRAY input_array, int source_row,
                               JSAMPARRAY output_array, int dest_row,
                               int num_rows, JDIMENSION num_cols);
EXTERN(void) j12copy_sample_rows(J12SAMPARRAY input_array, int source_row,
                                 J12SAMPARRAY output_array, int dest_row,
                                 int num_rows, JDIMENSION num_cols);
#if defined(C_LOSSLESS_SUPPORTED) || defined(D_LOSSLESS_SUPPORTED)
EXTERN(void) j16copy_sample_rows(J16SAMPARRAY input_array, int source_row,
                                 J16SAMPARRAY output_array, int dest_row,
                                 int num_rows, JDIMENSION num_cols);
#endif
EXTERN(void) jcopy_block_row(JBLOCKROW input_row, JBLOCKROW output_row,
                             JDIMENSION num_blocks);
EXTERN(void) jzero_far(void *target, size_t bytestozero);
/* Constant tables in jutils.c */
#if 0
extern const int jpeg_zigzag_order[];
#endif
extern const int jpeg_natural_order[];

/* Arithmetic coding probability estimation tables in jaricom.c */
extern const JLONG jpeg_aritab[];
