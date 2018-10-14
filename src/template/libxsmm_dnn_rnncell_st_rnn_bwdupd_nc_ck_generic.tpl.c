/******************************************************************************
** Copyright (c) 2017-2018, Intel Corporation                                **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              **
******************************************************************************/
/* Alexander Heinecke (Intel Corp.)
******************************************************************************/

/* helper variables */
libxsmm_blasint i, ik, in, ic, jk, jn, jc, ek, en, ec;
/* tensor dimensions */
libxsmm_blasint K = handle->desc.K;
libxsmm_blasint N = handle->desc.N;
libxsmm_blasint C = handle->desc.C;
libxsmm_blasint t = handle->desc.t;
libxsmm_blasint bk = handle->bk;
libxsmm_blasint bn = handle->bn;
libxsmm_blasint bc = handle->bc;
/* tensor raw pointers */
element_output_type *djdht = (element_output_type*)handle->djdht->data;
element_output_type *deltat = (element_output_type*)handle->scratch_deltat;
element_filter_type *uD = (element_filter_type*)handle->u->data;
element_input_type *xt = (element_input_type*)handle->xt->data;
element_output_type *ht = (element_output_type*)handle->ht->data;
element_filter_type *wD = (element_filter_type*)handle->w->data;
element_filter_type *djduD = (element_filter_type*)handle->djdu->data;
element_filter_type *djdwD = (element_filter_type*)handle->djdw->data;
element_output_type *djdb = (element_output_type*)handle->djdb->data;
element_input_type *djdxt = (element_input_type*)handle->djdxt->data;
element_filter_type *scratch_wT = (element_filter_type*)handle->scratch_wT;
element_filter_type *scratch_uT = (element_filter_type*)handle->scratch_uT;
element_input_type *scratch_xT = (element_input_type*)handle->scratch_xT;
element_output_type *scratch_hT = (element_output_type*)handle->scratch_hT;
/* multidimensional arrays */
LIBXSMM_VLA_DECL(2, element_filter_type, u, uD, K);
LIBXSMM_VLA_DECL(2, element_filter_type, w, wD, K);
LIBXSMM_VLA_DECL(2, element_filter_type, djdu, djduD, K);
LIBXSMM_VLA_DECL(2, element_filter_type, djdw, djdwD, K);
LIBXSMM_VLA_DECL(3, element_output_type, djdh, djdht, N, K);
LIBXSMM_VLA_DECL(3, element_output_type, delta, deltat, N, K);
LIBXSMM_VLA_DECL(3, element_input_type, x, xt, N, C);
LIBXSMM_VLA_DECL(3, element_output_type, h, ht, N, K);
LIBXSMM_VLA_DECL(3, element_input_type, djdx, djdxt, N, C);
LIBXSMM_VLA_DECL(2, element_filter_type, wT, scratch_wT, C);
LIBXSMM_VLA_DECL(2, element_filter_type, uT, scratch_uT, K);
LIBXSMM_VLA_DECL(2, element_input_type, xT, scratch_xT, N);
LIBXSMM_VLA_DECL(2, element_output_type, hT, scratch_hT, N);
#if defined(LIBXSMM_DNN_RNN_RELU_BWDUPD) || defined(LIBXSMM_DNN_RNN_SIGMOID_BWDUPD) || defined(LIBXSMM_DNN_RNN_TANH_BWDUPD)
element_output_type *zt = (element_output_type*)handle->internal_z;
LIBXSMM_VLA_DECL(3, element_output_type, z, zt, N, K);
#endif

/* computing first logical thread */
const libxsmm_blasint ltid = (libxsmm_blasint)tid - (libxsmm_blasint)start_thread;

/* lazy barrier init */
libxsmm_barrier_init(handle->barrier, ltid);

/* initialization is done at the beginning */
if ( (LIBXSMM_DNN_COMPUTE_KIND_BWD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
  libxsmm_internal_matrix_zero(N*C*t, djdxt, start_thread, tid, handle->desc.threads);
}
if ( (LIBXSMM_DNN_COMPUTE_KIND_UPD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
  libxsmm_internal_matrix_zero(C*K,   djdwD, start_thread, tid, handle->desc.threads);
  libxsmm_internal_matrix_zero(K*K,   djduD, start_thread, tid, handle->desc.threads);
  libxsmm_internal_matrix_zero(K,     djdb,  start_thread, tid, handle->desc.threads);
}

/* transpose W */
for (ik = 0; ik < K; ik += bk) {
  for (ic = 0; ic < C; ic += bc) {
    for (jk = 0; jk < bk; ++jk) {
      for (jc = 0; jc < bc; ++jc) {
        ek = ik + jk;
        ec = ic + jc;
        LIBXSMM_VLA_ACCESS(2, wT, ek, ec, C) =  LIBXSMM_VLA_ACCESS(2, w, ec, ek, K);
      }
    }
  }
}

/* transpose U */
for (ik = 0; ik < K; ik += bk) {
  for (ic = 0; ic < K; ic += bc) {
    for (jk = 0; jk < bk; ++jk) {
      for (jc = 0; jc < bk; ++jc) {
        ek = ik + jk;
        ec = ic + jc;
        LIBXSMM_VLA_ACCESS(2, uT, ek, ec, K) =  LIBXSMM_VLA_ACCESS(2, u, ec, ek, K);
      }
    }
  }
}

/* transpose xt for current timestep */
for (ic = 0; ic < C; ic += bc) {
  for (in = 0; in < N; in += bn) {
    for (jc = 0; jc < bc; ++jc) {
      for (jn = 0; jn < bn; ++jn) {
        en = in + jn;
        ec = ic + jc;
        LIBXSMM_VLA_ACCESS(2, xT, ec, en, N) =  LIBXSMM_VLA_ACCESS(3, x, t-1, en, ec, N, C);
      }
    }
  }
}

/* transpose ht for current timestep */
for (ik = 0; ik < K; ik += bk) {
  for (in = 0; in < N; in += bn) {
    for (jk = 0; jk < bk; ++jk) {
      for (jn = 0; jn < bn; ++jn) {
        en = in + jn;
        ek = ik + jk;
        LIBXSMM_VLA_ACCESS(2, hT, ek, en, N) =  LIBXSMM_VLA_ACCESS(3, h, t-1, en, ek, N, K);
      }
    }
  }
}

libxsmm_barrier_wait(handle->barrier, ltid);

/* The following code is for time step t-1 */
for (in = 0; in < N; in += bn) {
  for (ik = 0; ik < K; ik += bk) {
#if defined(LIBXSMM_DNN_RNN_RELU_BWDUPD)
    libxsmm_internal_matrix_relu_inverse_ld(    bk, bn, K, &LIBXSMM_VLA_ACCESS(3, z, t-1, in, ik, N, K), &LIBXSMM_VLA_ACCESS(3, delta, t-1, in, ik, N, K) );
#endif
#if defined(LIBXSMM_DNN_RNN_SIGMOID_BWDUPD)
    libxsmm_internal_matrix_sigmoid_inverse_ld( bk, bn, K, &LIBXSMM_VLA_ACCESS(3, z, t-1, in, ik, N, K), &LIBXSMM_VLA_ACCESS(3, delta, t-1, in, ik, N, K) );
#endif
#if defined(LIBXSMM_DNN_RNN_TANH_BWDUPD)
    libxsmm_internal_matrix_tanh_inverse_ld(    bk, bn, K, &LIBXSMM_VLA_ACCESS(3, z, t-1, in, ik, N, K), &LIBXSMM_VLA_ACCESS(3, delta, t-1, in, ik, N, K) );
#endif

    libxsmm_internal_matrix_inplace_eltwise_mult_ld( bk, bn, K, &LIBXSMM_VLA_ACCESS(3, djdh,  t-1, in, ik, N, K),
                                                                &LIBXSMM_VLA_ACCESS(3, delta, t-1, in, ik, N, K) );

    if ( (LIBXSMM_DNN_COMPUTE_KIND_BWD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
      for (jn = 0; jn < bn; jn++) {
        for (jk = 0; jk < bk; jk++) {
          en = in + jn;
          ek = ik + jk;
          /* djdx = W^T * delta */
          for (ic = 0; ic < C; ic += bc) {
            for (jc = 0; jc < bc; jc++) {
              ec = ic + jc;
              LIBXSMM_VLA_ACCESS(3, djdx, t-1, en, ec, N, C) += LIBXSMM_VLA_ACCESS(3, delta, t-1, en, ek, N, K) * LIBXSMM_VLA_ACCESS(2, wT, ek, ec, C);
            }
          }
        }
      }
    }
    if ( (LIBXSMM_DNN_COMPUTE_KIND_UPD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
      for (jn = 0; jn < bn; jn++) {
        for (jk = 0; jk < bk; jk++) {
          en = in + jn;
          ek = ik + jk;
          /* djdu = delta * h^T */
          for (ic = 0; ic < K; ic += bk) {
            for (jc = 0; jc < bk; jc++) {
              ec = ic + jc;
              LIBXSMM_VLA_ACCESS(2, djdu, ec, ek, K) += LIBXSMM_VLA_ACCESS(2, hT, ec, en, N) * LIBXSMM_VLA_ACCESS(3, delta, t-1, en, ek, N, K);
            }
          }
          /* djdw = delta * x^T */
          for (ic = 0; ic < C; ic += bc) {
            for (jc = 0; jc < bc; jc++) {
              ec = ic + jc;
              LIBXSMM_VLA_ACCESS(2, djdw, ec, ek, K) += LIBXSMM_VLA_ACCESS(2, xT, ec, en, N) * LIBXSMM_VLA_ACCESS(3, delta, t-1, en, ek, N, K);
            }
          }
          djdb[ek] += LIBXSMM_VLA_ACCESS(3, delta, t-1, en, ek, N, K);
        }
      }
    }
  }
}

libxsmm_barrier_wait(handle->barrier, ltid);

for (i = t-2; i >= 0; --i) {
  /* transpose xt for current timestep */
  for (ic = 0; ic < C; ic += bc) {
    for (in = 0; in < N; in += bn) {
      for (jc = 0; jc < bc; ++jc) {
        for (jn = 0; jn < bn; ++jn) {
          en = in + jn;
          ec = ic + jc;
          LIBXSMM_VLA_ACCESS(2, xT, ec, en, N) =  LIBXSMM_VLA_ACCESS(3, x, i, en, ec, N, C);
        }
      }
    }
  }

  /* transpose ht for current timestep */
  for (ik = 0; ik < K; ik += bk) {
    for (in = 0; in < N; in += bn) {
      for (jk = 0; jk < bk; ++jk) {
        for (jn = 0; jn < bn; ++jn) {
          en = in + jn;
          ek = ik + jk;
          LIBXSMM_VLA_ACCESS(2, hT, ek, en, N) =  LIBXSMM_VLA_ACCESS(3, h, i, en, ek, N, K);
        }
      }
    }
  }

  /* let's run the cell in blocks for good locality */
  for (in = 0; in < N; in += bn) {
    for (ik = 0; ik < K; ik += bk) {
      /* di1 = U^T * delta */
      for (jn = 0; jn < bn; jn++) {
        for (jk = 0; jk < bk; jk++) {
          element_output_type tmp = (element_output_type)0;
          en = in + jn;
          ek = ik + jk;
          LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K) = (element_output_type)0;
          for (ic = 0; ic < K; ic += bk) {
            for (jc = 0; jc < bk; jc++) {
              ec = ic + jc;
              LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K) += LIBXSMM_VLA_ACCESS(3, delta, i+1, en, ec, N, K) * LIBXSMM_VLA_ACCESS(2, uT, ec, ek, K);
            }
          }
          LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K) += LIBXSMM_VLA_ACCESS(3, djdh, i, en, ek, N, K);
#if defined(LIBXSMM_DNN_RNN_RELU_BWDUPD)
          libxsmm_internal_matrix_relu_inverse_ld( 1, 1, K, &LIBXSMM_VLA_ACCESS(3, z, i, en, ek, N, K), &tmp );
#endif
#if defined(LIBXSMM_DNN_RNN_SIGMOID_BWDUPD)
          libxsmm_internal_matrix_sigmoid_inverse_ld( 1, 1, K, &LIBXSMM_VLA_ACCESS(3, z, i, en, ek, N, K), &tmp );
#endif
#if defined(LIBXSMM_DNN_RNN_TANH_BWDUPD)
          libxsmm_internal_matrix_tanh_inverse_ld( 1, 1, K, &LIBXSMM_VLA_ACCESS(3, z, i, en, ek, N, K), &tmp );
#endif
          LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K) *= tmp;

          if ( (LIBXSMM_DNN_COMPUTE_KIND_BWD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
            /* djdx = W^T * delta */
            for (ic = 0; ic < C; ic += bc) {
              for (jc = 0; jc < bc; jc++) {
                ec = ic + jc;
                LIBXSMM_VLA_ACCESS(3, djdx, i, en, ec, N, C) += LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K) * LIBXSMM_VLA_ACCESS(2, wT, ek, ec, C);
              }
            }
          }
          if ( (LIBXSMM_DNN_COMPUTE_KIND_UPD == kind) || (LIBXSMM_DNN_COMPUTE_KIND_BWDUPD == kind) ) {
            /* djdu = delta * h^T */
            for (ic = 0; ic < K; ic += bk) {
              for (jc = 0; jc < bk; jc++) {
                ec = ic + jc;
                LIBXSMM_VLA_ACCESS(2, djdu, ec, ek, K) += LIBXSMM_VLA_ACCESS(2, hT, ec, en, N) * LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K);
              }
            }
            /* djdw = delta * x^T */
            for (ic = 0; ic < C; ic += bc) {
              for (jc = 0; jc < bc; jc++) {
                ec = ic + jc;
                LIBXSMM_VLA_ACCESS(2, djdw, ec, ek, K) += LIBXSMM_VLA_ACCESS(2, xT, ec, en, N) * LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K);
              }
            }
            djdb[ek] += LIBXSMM_VLA_ACCESS(3, delta, i, en, ek, N, K);
          }
        }
      }
    }
  }
}

libxsmm_barrier_wait(handle->barrier, ltid);