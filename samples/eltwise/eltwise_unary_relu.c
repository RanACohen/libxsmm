/******************************************************************************
* Copyright (c) Intel Corporation - All rights reserved.                      *
* This file is part of the LIBXSMM library.                                   *
*                                                                             *
* For information on the license, see the LICENSE file.                       *
* Further information: https://github.com/hfp/libxsmm/                        *
* SPDX-License-Identifier: BSD-3-Clause                                       *
******************************************************************************/
/* Alexander Heinecke (Intel Corp.)
******************************************************************************/
#include <libxsmm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

void relu_fwd_f32_f32_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, float *in, float *out, unsigned char *out_mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      out[(j*ldo) + i] = ( in[(j*ldi) + i] < 0.0f ) ? 0.0f : in[(j*ldi) + i];
      out_mask[(j*ldo/8) + i/8] |= (unsigned char)(( in[(j*ldi) + i] < 0.0f ) ? 0x0 : (1 << (i%8)) );
    }
  }
}

void relu_fwd_bf16_bf16_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, libxsmm_bfloat16 *in, libxsmm_bfloat16 *out, unsigned char *out_mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      out[(j*ldo) + i] = ( (in[(j*ldi) + i] & 0x8000) == 0x8000 ) ? 0 : in[(j*ldi) + i];
      out_mask[(j*ldo/8) + i/8] |= (unsigned char)(( (in[(j*ldi) + i] & 0x8000) == 0x8000 ) ? 0x0 : (1 << (i%8)) );
    }
  }
}

void relu_fwd_f32_bf16_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, float *in, libxsmm_bfloat16 *out, unsigned char *out_mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      union libxsmm_bfloat16_hp bf16_hp;
      bf16_hp.f = in[(j*ldi) + i];
      out[(j*ldo) + i] = ( in[(j*ldi) + i] < 0.0f ) ? 0 : bf16_hp.i[1];
      out_mask[(j*ldo/8) + i/8] |= (unsigned char)(( in[(j*ldi) + i] < 0.0f ) ? 0x0 : (1 << (i%8)) );
    }
  }
}

void relu_fwd_bf16_f32_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, libxsmm_bfloat16 *in, float *out, unsigned char *out_mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      union libxsmm_bfloat16_hp bf16_hp;
      bf16_hp.i[1] = in[(j*ldi) + i];
      bf16_hp.i[0] = 0;
      out[(j*ldo) + i] = ( bf16_hp.f < 0.0f ) ? 0 : bf16_hp.f;
      out_mask[(j*ldo/8) + i/8] |= (unsigned char)(( bf16_hp.f < 0.0f ) ? 0x0 : (1 << (i%8)) );
    }
  }
}

void relu_bwd_f32_f32_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, float *in, float *out, unsigned char *mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      out[(j*ldo) + i] = ( (mask[(j*ldi/8) + i/8] & (1 << (i%8))) > 0 ) ? in[(j*ldi) + i] : 0.0f;
    }
  }
}

void relu_bwd_bf16_bf16_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, libxsmm_bfloat16 *in, libxsmm_bfloat16 *out, unsigned char *mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      out[(j*ldo) + i] = ( (mask[(j*ldi/8) + i/8] & (1 << (i%8))) > 0 ) ? in[(j*ldi) + i] : 0;
    }
  }
}

void relu_bwd_f32_bf16_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, float *in, libxsmm_bfloat16 *out, unsigned char *mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      union libxsmm_bfloat16_hp bf16_hp;
      bf16_hp.f = in[(j*ldi) + i];
      out[(j*ldo) + i] = ( (mask[(j*ldi/8) + i/8] & (1 << (i%8))) > 0 ) ? bf16_hp.i[1] : 0;
    }
  }
}

void relu_bwd_bf16_f32_gold(libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo, libxsmm_bfloat16 *in, float *out, unsigned char *mask) {
  libxsmm_blasint i, j;
  for ( j = 0; j < N; ++j ) {
    for ( i = 0; i < M; ++i ) {
      union libxsmm_bfloat16_hp bf16_hp;
      bf16_hp.i[1] = ( (mask[(j*ldi/8) + i/8] & (1 << (i%8))) > 0 ) ? in[(j*ldi) + i] : 0;
      bf16_hp.i[0] = 0;
      out[(j*ldo) + i] = bf16_hp.f;
    }
  }
}

void test_relu_f32_f32_fwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  float *in;
  float *out, *out_gold;
  unsigned char *mask, *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldo : ldo/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_f32_f32_fwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_f32_f32_fwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldi,   64);
  out       = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  out_gold  = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  mask      = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      in[(i*ldi)+j] = (float)libxsmm_rng_f64();
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    mask[i] = 0;
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_fwd_f32_f32_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldo)/8] );
  }

  unary_param.in.primary  = (void*)in;
  unary_param.out.primary = (void*)out;
  unary_param.out.secondary = (bitm == 0) ? NULL : (void*)mask;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }
  if ( bitm != 0 ) {
    for ( i = 0; i < N; ++i ) {
      for ( j = 0; j < M/8; ++j ) {
        if ( mask_gold[(i*mask_ld)+j] != mask[(i*mask_ld)+j] ) {
          printf("error at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
          s = 1;
        }
#if 0
        else {
          printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
        }
#endif
      }
    }
    if ( s == 0 ) {
      printf("SUCCESS mask\n");
    } else {
      printf("FAILURE mask\n");
    }
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_bf16_bf16_fwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  libxsmm_bfloat16 *in;
  libxsmm_bfloat16 *out, *out_gold;
  unsigned char *mask, *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldo : ldo/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_bf16_bf16_fwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_bf16_bf16_fwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldi,   64);
  out       = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  out_gold  = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  mask      = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      in[(i*ldi)+j] = bf16_hp.i[1];
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    mask[i] = 0;
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_fwd_bf16_bf16_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldo)/8] );
  }

  unary_param.in.primary  = (void*)in;
  unary_param.out.primary = (void*)out;
  unary_param.out.secondary = (bitm == 0) ? NULL : (void*)mask;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_BF16, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );


  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }
  if ( bitm != 0 ) {
    for ( i = 0; i < N; ++i ) {
      for ( j = 0; j < M/8; ++j ) {
        if ( mask_gold[(i*mask_ld)+j] != mask[(i*mask_ld)+j] ) {
          printf("error at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
          s = 1;
        }
#if 0
        else {
          printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
        }
#endif
      }
    }
    if ( s == 0 ) {
      printf("SUCCESS mask\n");
    } else {
      printf("FAILURE mask\n");
    }
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_f32_bf16_fwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  float *in;
  libxsmm_bfloat16 *out, *out_gold;
  unsigned char *mask, *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldo : ldo/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_f32_bf16_fwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_f32_bf16_fwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldi,   64);
  out       = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  out_gold  = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  mask      = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      bf16_hp.i[0] = 0;
      in[(i*ldi)+j] = bf16_hp.f;
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    mask[i] = 0;
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_fwd_f32_bf16_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldo)/8] );
  }

  unary_param.in.primary  = (void*)in;
  unary_param.out.primary = (void*)out;
  unary_param.out.secondary = (bitm == 0) ? NULL : (void*)mask;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_BF16, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }
  if ( bitm != 0 ) {
    for ( i = 0; i < N; ++i ) {
      for ( j = 0; j < M/8; ++j ) {
        if ( mask_gold[(i*mask_ld)+j] != mask[(i*mask_ld)+j] ) {
          printf("error at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
          s = 1;
        }
#if 0
        else {
          printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
        }
#endif
      }
    }
    if ( s == 0 ) {
      printf("SUCCESS mask\n");
    } else {
      printf("FAILURE mask\n");
    }
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_bf16_f32_fwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  libxsmm_bfloat16 *in;
  float *out, *out_gold;
  unsigned char *mask, *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldo : ldo/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_bf16_f32_fwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_bf16_f32_fwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldi,   64);
  out       = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  out_gold  = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  mask      = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      in[(i*ldi)+j] = bf16_hp.i[1];
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0.0f;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0.0f;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    mask[i] = 0;
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_fwd_bf16_f32_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldo)/8] );
  }

  /* use jited relu */
  unary_param.in.primary  = (void*)in;
  unary_param.out.primary = (void*)out;
  unary_param.out.secondary = (bitm == 0) ? NULL : (void*)mask;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }
  if ( bitm != 0 ) {
    for ( i = 0; i < N; ++i ) {
      for ( j = 0; j < M/8; ++j ) {
        if ( mask_gold[(i*mask_ld)+j] != mask[(i*mask_ld)+j] ) {
          printf("error at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
          s = 1;
        }
#if 0
        else {
          printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, mask[(i*mask_ld)+j], mask_gold[(i*mask_ld)+j]);
        }
#endif
      }
    }
    if ( s == 0 ) {
      printf("SUCCESS mask\n");
    } else {
      printf("FAILURE mask\n");
    }
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_f32_f32_bwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  float *in;
  float *out, *out_gold;
  unsigned int *mask;
  unsigned char *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldi : ldi/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_f32_f32_bwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_f32_f32_bwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldi,   64);
  out       = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  out_gold  = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  mask      = (unsigned int*) libxsmm_aligned_malloc( sizeof(unsigned int)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      in[(i*ldi)+j] = (float)libxsmm_rng_f64();
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    if ( bitm == 0 ) {
      mask[i] = ( i % 2 == 1) ? 0x3f800000 : 0x0;
    } else {
      mask[i] = 0xaaaaaaaa;
    }
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0xaa;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_bwd_f32_f32_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldi)/8] );
  }

  /* use jited relu */
  unary_param.in.primary    = (void*)in;
  unary_param.in.secondary  = (void*)mask;
  unary_param.out.primary   = (void*)out;

  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU_INV);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_bf16_bf16_bwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  libxsmm_bfloat16 *in;
  libxsmm_bfloat16 *out, *out_gold;
  unsigned short *mask;
  unsigned char *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldi : ldi/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_bf16_bf16_bwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_bf16_bf16_bwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldi,   64);
  out       = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  out_gold  = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  mask      = (unsigned short*) libxsmm_aligned_malloc( sizeof(unsigned short)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      in[(i*ldi)+j] = bf16_hp.i[1];
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    if ( bitm == 0 ) {
      mask[i] = ( i % 2 == 1) ? 0x3f80 : 0x0;
    } else {
      mask[i] = 0xaaaa;
    }
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0xaa;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_bwd_bf16_bf16_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldi)/8] );
  }

  /* use jited relu */
  unary_param.in.primary    = (void*)in;
  unary_param.in.secondary  = (void*)mask;
  unary_param.out.primary   = (void*)out;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_BF16, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU_INV);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_f32_bf16_bwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  float *in;
  libxsmm_bfloat16 *out, *out_gold;
  unsigned int *mask;
  unsigned char *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldi : ldi/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_f32_bf16_bwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_f32_bf16_bwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldi,   64);
  out       = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  out_gold  = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldo,   64);
  mask      = (unsigned int*) libxsmm_aligned_malloc( sizeof(unsigned int)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      bf16_hp.i[0] = 0;
      in[(i*ldi)+j] = bf16_hp.f;
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    if ( bitm == 0 ) {
      mask[i] = ( i % 2 == 1) ? 0x3f800000 : 0x0;
    } else {
      mask[i] = 0xaaaaaaaa;
    }
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0xaa;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_bwd_f32_bf16_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldi)/8] );
  }

  /* use jited relu */
  unary_param.in.primary    = (void*)in;
  unary_param.in.secondary  = (void*)mask;
  unary_param.out.primary   = (void*)out;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_BF16, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU_INV);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );

  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %u, %u\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

void test_relu_bf16_f32_bwd( libxsmm_blasint bitm, libxsmm_blasint M, libxsmm_blasint N, libxsmm_blasint ldi, libxsmm_blasint ldo ) {
  libxsmm_bfloat16 *in;
  float *out, *out_gold;
  unsigned short *mask;
  unsigned char *mask_gold;
  unsigned int i, j;
  unsigned int s;
  libxsmm_meltw_unary_param unary_param;
  libxsmm_meltw_unary_flags unary_flags;
  union libxsmm_bfloat16_hp bf16_hp;
  libxsmm_blasint mask_ld = (bitm == 0) ? ldi : ldi/8;

  if ( M > ldi ) {
    fprintf( stderr, "test_relu_bf16_f32_bwd: ldi needs to be equal to or bigger than M\n");
    exit(-1);
  }
  if (M > ldo ) {
    fprintf( stderr, "test_relu_bf16_f32_bwd: ldo needs to be equal to or bigger than N\n");
    exit(-1);
  }

  libxsmm_rng_set_seed(1);

  in        = (libxsmm_bfloat16*) libxsmm_aligned_malloc( sizeof(libxsmm_bfloat16)*N*ldi,   64);
  out       = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  out_gold  = (float*) libxsmm_aligned_malloc( sizeof(float)*N*ldo,   64);
  mask      = (unsigned short*) libxsmm_aligned_malloc( sizeof(unsigned short)*N*mask_ld, 64);
  mask_gold = (unsigned char*) libxsmm_aligned_malloc( sizeof(unsigned char)*N*(mask_ld+1), 64);

  /* init in */
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < ldi; ++j ) {
      bf16_hp.f = (float)libxsmm_rng_f64();
      in[(i*ldi)+j] = bf16_hp.i[1];
    }
  }

  /* init out */
  for ( i = 0; i < N*ldo; ++i ) {
    out[i] = 0.0f;
  }
  for ( i = 0; i < N*ldo; ++i ) {
    out_gold[i] = 0.0f;
  }
  for ( i = 0; i < N*mask_ld; ++i ) {
    if ( bitm == 0 ) {
      mask[i] = ( i % 2 == 1) ? 0x3f80 : 0x0;
    } else {
      mask[i] = 0xaaaa;
    }
  }
  for ( i = 0; i < N*(mask_ld+1); ++i ) {
    mask_gold[i] = 0xaa;
  }

  /* compute out_gold */
  for ( i = 0; i < N; ++i ) {
    relu_bwd_bf16_f32_gold( M, 1, ldi, ldo, &in[(i*ldi)], &out_gold[(i*ldo)], &mask_gold[(i*ldi)/8] );
  }

  /* use jited relu */
  unary_param.in.primary    = (void*)in;
  unary_param.in.secondary  = (void*)mask;
  unary_param.out.primary   = (void*)out;
  unary_flags = (bitm == 0) ? LIBXSMM_MELTW_FLAG_UNARY_NONE : LIBXSMM_MELTW_FLAG_UNARY_BITMASK;
  libxsmm_meltwfunction_unary unary_kernel = libxsmm_dispatch_meltw_unary(M, N, &ldi, &ldo, LIBXSMM_DATATYPE_BF16, LIBXSMM_DATATYPE_F32, LIBXSMM_DATATYPE_F32, unary_flags, LIBXSMM_MELTW_TYPE_UNARY_RELU_INV);
  if ( unary_kernel == NULL ) {
    fprintf( stderr, "JIT for UNARY TPP. Bailing...!\n");
    exit(-1);
  }
  unary_kernel( &unary_param );
  /* compare result */
  s = 0;
  for ( i = 0; i < N; ++i ) {
    for ( j = 0; j < M; ++j ) {
      if ( out_gold[(i*ldo)+j] != out[(i*ldo)+j] ) {
        printf("error at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
        s = 1;
      }
#if 0
      else {
        printf("correct at possition i=%i, j=%i, %f, %f\n", i, j, out[(i*ldo)+j], out_gold[(i*ldo)+j]);
      }
#endif
    }
  }
  if ( s == 0 ) {
    printf("SUCCESS output\n");
  } else {
    printf("FAILURE output\n");
  }

  libxsmm_free( out_gold );
  libxsmm_free( out );
  libxsmm_free( in );
  libxsmm_free( mask );
  libxsmm_free( mask_gold );
}

int main( int argc, char* argv[] ) {
  libxsmm_blasint dtype_in;
  libxsmm_blasint dtype_out;
  char op;
  libxsmm_blasint bitm;
  libxsmm_blasint M;
  libxsmm_blasint N;
  libxsmm_blasint ldi;
  libxsmm_blasint ldo;

  if ( argc != 9 ) {
    printf(" Error! Usage: %s [F/B] [bitmask: 0/1] [prec_in: 4/2] [prec_out: 4/2] [M] [N] [ldi] [ldo]\n", argv[0] );
    exit(-1);
  }

  op        = *(argv[1]);
  bitm      = atoi(argv[2]);
  dtype_in  = atoi(argv[3]);
  dtype_out = atoi(argv[4]);
  M         = atoi(argv[5]);
  N         = atoi(argv[6]);
  ldi       = atoi(argv[7]);
  ldo       = atoi(argv[8]);

  if ( op == 'F' && dtype_in == 4 && dtype_out == 4  ) {
    printf("Testing F32 F32 forward relu\n");
    test_relu_f32_f32_fwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'F' && dtype_in == 2  && dtype_out == 2 ) {
    printf("Testing BF16 BF16 forward relu\n");
    test_relu_bf16_bf16_fwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'F' && dtype_in == 4  && dtype_out == 2 ) {
    printf("Testing F32 BF16 forward relu\n");
    test_relu_f32_bf16_fwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'F' && dtype_in == 2  && dtype_out == 4 ) {
    printf("Testing BF16 F32 forward relu\n");
    test_relu_bf16_f32_fwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'B' && dtype_in == 4 && dtype_out == 4 ) {
    printf("Testing F32 F32 backward relu\n");
    test_relu_f32_f32_bwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'B' && dtype_in == 2 && dtype_out == 2 ) {
    printf("Testing BF16 BF16 backward relu\n");
    test_relu_bf16_bf16_bwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'B' && dtype_in == 4 && dtype_out == 2 ) {
    printf("Testing F32 BF16 backward relu\n");
    test_relu_f32_bf16_bwd( bitm, M, N, ldi, ldo );
  } else if ( op == 'B' && dtype_in == 2 && dtype_out == 4 ) {
    printf("Testing BF16 F32 backward relu\n");
    test_relu_bf16_f32_bwd( bitm, M, N, ldi, ldo );
  } else {
    printf(" Not implemented case! Usage: %s [F/B] [bitmask: 0/1] [prec_in: 4/2] [prec_out: 4/2] [M] [N] [ldi] [ldo]\n", argv[0] );
    exit(-1);
  }
}
