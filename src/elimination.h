/* gbla: Gröbner Basis Linear Algebra
 * Copyright (C) 2015 Christian Eder <ederc@mathematik.uni-kl.de>
 * This file is part of gbla.
 * gbla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * gbla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with gbla . If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file elimination.h
 * \brief Different Gaussian Elimination methods
 *
 * \author Christian Eder <ederc@mathematik.uni-kl.de>
 */

#ifndef GB_ELIMINATION_H
#define GB_ELIMINATION_H

#include "src/config.h"
#include <mapping.h>
#include <matrix.h>
#if GBLA_WITH_FFLAS
#include "../draft/echelon.h"
#endif

/**
 * \brief Structure of a waiting list element
 */
typedef struct wle_t {
	ri_t idx; /*!<  row index */
	ri_t lp;  /*!<  row index of last pivot that idx was reduced with */
} wle_t;

/**
 * \brief Structure defining the waiting list or queue for parallel dense
 * echelonization of the D part.
 *
 * \note We use a mutiline concept for the row index lists.
 */
typedef struct wl_t {
	wle_t *list;  /*!<  row indices of rows in waiting list */
	ri_t sidx;    /*!<  smallest row index in rows */
	ri_t slp;     /*!<  last pivot row index by which sidx is
									reduced by already */
	ri_t sz;      /*!<  size of waiting list */
} wl_t;

/**
 * \brief Comparison function for qsort for waiting list
 *
 * \param waiting list element a
 *
 * \param waiting list element b
 *
 * \return *b.idx - *a.idx
 */
int cmp_wle(const void *a, const void *b) ;

/**
 * \brief Adds a new row to end of waiting list
 *
 * \param waiting list wl
 *
 * \param row index to be added row_idx
 *
 * \param row index of last pivot row_idx was reduced by last_piv_reduced_by
 */
void push_row_to_waiting_list(wl_t *waiting_global, const ri_t row_idx,
		const ri_t last_piv_reduced_by) ;

/**
 * \brief Copies entry from sparse block sparse_block to dense block dense_block
 * for elimination purposes.
 *
 * \param sparse block sparse_block
 *
 * \param dense block dense_block
 *
 * \param block height bheight
 *
 * \param block width bwidth
 */
void copy_sparse_to_dense_block(
		mbl_t *sparse_block, re_l_t **dense_block, int bheight, int bwidth) ;

/**
 * \brief Copies entry from dense block to sparse rows
 * for elimination purposes.
 *
 * \param dense block dense_block
 *
 * \param sparse block sparse_block
 *
 * \param block height bheight
 *
 * \param block width bwidth
 *
 * \param modulus resp. field characteristic modulus
 */
void copy_dense_block_to_sparse(
		re_l_t **dense_block, mbl_t **sparse_block, int bheight, int bwidth,
		mod_t modulus);

/**
 * \brief Computes a dense AXPY for one row
 *
 * \param value 1 from A Av1_col1
 *
 * \param value 2 from A Av2_col1
 *
 * \param corresponding multiline block in B
 *
 * \param line index line_idx
 *
 * \param dense block width bwidth
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void dense_scal_mul_sub_1_row_vect_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const mbl_t multiline,
		const bi_t line_idx,
		const bi_t  bwidth,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a dense AXPY for one row using full multilines
 *
 * \param value 1 from A Av1_col1
 *
 * \param value 2 from A Av2_col1
 *
 * \param corresponding multiline
 *
 * \param line index line_idx
 *
 * \param dense block width bwidth
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 *
 * \param offset for first nonzero coefficient in multiline
 */
void dense_scal_mul_sub_1_row_vect_array_multiline_var_size(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const ml_t multiline,
		const bi_t line_idx,
		re_l_t *dense_val1,
		re_l_t *dense_val2,
		const ci_t offset) ;

/**
 * \brief Computes a dense AXPY for two rows for multilines
 *
 * \param value 1,1 from A Av1_col1
 *
 * \param value 2,1 from A Av2_col1
 *
 * \param value 1,2 from A Av1_col2
 *
 * \param value 2,2 from A Av2_col2
 *
 * \param corresponding multiline
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 *
 * \param offset for first nonzero coefficient in multiline
 */
void dense_scal_mul_sub_2_rows_vect_array_multiline_var_size(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const re_m_t Av1_col2,
		const re_m_t Av2_col2,
		const ml_t multiline,
		re_l_t *dense_val1,
		re_l_t *dense_val2,
		const ci_t offset1,
		const ci_t offset2) ;

/**
 * \brief Computes a sparse AXPY for one row for full multilines
 *
 * \param value 1 from A Av1_col1
 *
 * \param value 2 from A Av2_col1
 *
 * \param corresponding multiline
 *
 * \param line index line_idx
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void sparse_scal_mul_sub_1_row_vect_array_multiline(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const ml_t multiline,
		const bi_t line_idx,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a sparse AXPY for one row
 *
 * \param value 1 from A Av1_col1
 *
 * \param value 2 from A Av2_col1
 *
 * \param corresponding multiline block in B
 *
 * \param line index line_idx
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void sparse_scal_mul_sub_1_row_vect_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const mbl_t multiline,
		const bi_t line_idx,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a dense AXPY for two rows
 *
 * \param value 1,1 from A Av1_col1
 *
 * \param value 2,1 from A Av2_col1
 *
 * \param value 1,2 from A Av1_col2
 *
 * \param value 2,2 from A Av2_col2
 *
 * \param corresponding multiline block in B
 *
 * \param dense block width bwidth
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void dense_scal_mul_sub_2_rows_vect_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const re_m_t Av1_col2,
		const re_m_t Av2_col2,
		const mbl_t multiline,
		const bi_t  bwidth,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a sparse AXPY for two rows
 *
 * \param value 1,1 from A Av1_col1
 *
 * \param value 2,1 from A Av2_col1
 *
 * \param value 1,2 from A Av1_col2
 *
 * \param value 2,2 from A Av2_col2
 *
 * \param corresponding multiline block in B
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void sparse_scal_mul_sub_2_rows_vect_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const re_m_t Av1_col2,
		const re_m_t Av2_col2,
		const mbl_t multiline,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a sparse AXPY for two rows for multilines
 *
 * \param value 1,1 from A Av1_col1
 *
 * \param value 2,1 from A Av2_col1
 *
 * \param value 1,2 from A Av1_col2
 *
 * \param value 2,2 from A Av2_col2
 *
 * \param corresponding multiline
 *
 * \param dense value 1 holder for delayed modulus dense_val1
 *
 * \param dense value 2 holder for delayed modulus dense_val2
 */
void sparse_scal_mul_sub_2_rows_vect_array_multiline(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const re_m_t Av1_col2,
		const re_m_t Av2_col2,
		const ml_t multiline,
		re_l_t *dense_val1,
		re_l_t *dense_val2) ;

/**
 * \brief Computes a dense AXPY for one dense row (in triangular A^-1B situation)
 *
 * \param value 1 from A Av1_col1
 *
 * \param value 2 from A Av2_col1
 *
 * \param block width bwidth
 *
 * \param dense source array dense_array_source
 *
 * \param dense array 1 holder for delayed modulus dense_array1
 *
 * \param dense array 2 holder for delayed modulus dense_array2
 */
void dense_scal_mul_sub_1_row_array_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const bi_t bwidth,
		const re_l_t *dense_array_source,
		re_l_t *dense_array1,
		re_l_t *dense_array2) ;

/**
 * \brief Computes a dense AXPY for two dense rows (in triangular A^-1B situation)
 *
 * \param value 1,1 from A Av1_col1
 *
 * \param value 2,1 from A Av2_col1
 *
 * \param value 1,2 from A Av1_col2
 *
 * \param value 2,2 from A Av2_col2
 *
 * \param block width bwidth
 *
 * \param dense source array dense_array_source1
 *
 * \param dense source array dense_array_source2
 *
 * \param dense array 1 holder for delayed modulus dense_array1
 *
 * \param dense array 2 holder for delayed modulus dense_array2
 */
void dense_scal_mul_sub_2_rows_array_array(
		const re_m_t Av1_col1,
		const re_m_t Av2_col1,
		const re_m_t Av1_col2,
		const re_m_t Av2_col2,
		const bi_t bwidth,
		const re_l_t *dense_array_source1,
		const re_l_t *dense_array_source2,
		re_l_t *dense_array1,
		re_l_t *dense_array2);

/**
 * \brief Modular reduction of dense row array
 *
 * \param dense row array to be reduced dense_array
 *
 * \param block width bwidth
 *
 * \param modulus resp. field characteristic modulus
 */
void red_dense_array_modular(re_l_t *dense_array, const bi_t bwidth, const mod_t modulus);

/**
 * \brief Gets first nonzero entry in multiline m at line index line_idx and
 * stores it in h resp. h_idx.
 *
 * \param multiline m
 *
 * \param line index line_idx
 *
 * \param storage for "head" of line h
 *
 * \param storage for "head index" of line h_idx
 *
 * \return index value corresponding to h from input matrix M
 */
mli_t get_head_multiline(const ml_t *m, const bi_t line_idx, re_t *h, mli_t *h_idx) ;

/**
 * \brief Gets first nonzero entry in multiline m at line index line_idx and
 * stores it in h resp. h_idx. Use hybrid representation of multiline.
 *
 * \param multiline m
 *
 * \param line index line_idx
 *
 * \param storage for "head" of line h
 *
 * \param storage for "head index" of line h_idx
 *
 * \return index value corresponding to h from input matrix M
 */
mli_t get_head_multiline_hybrid(const ml_t *m,
		const bi_t line_idx, re_t *h, mli_t *h_idx, const ci_t coldim) ;

/**
 * \brief Gets first nonzero entry in dense array. Reduces to zero on the go if
 * possible.
 *
 * \param dense array dense_array
 *
 * \param holder for nonzero head value
 *
 * \param column dimension coldim
 *
 * \param field characteristic modulus
 *
 * \return index value corresponding to head value index in dense array
 */
int get_head_dense_array(re_l_t *dense_array,
		re_t *val, const ci_t coldim, const mod_t modulus) ;

/**
 * \brief Computes inverse value of x modulo y:
 * We compute the inverse using the extended GCD. So we are only interested in x.
 * Note that internally we need signed types, but we return only unsigned type
 * re_t for x.
 *
 * \param x
 *
 * \param y
 */
void inverse_val(re_t *x, const mod_t modulus) ;

/**
 * \brief Normalizes dense array and returns index of head element in array
 *
 * \param dense array dense_array
 *
 * \param column dimension coldim
 *
 * \param field characteristic modulus
 *
 * \return index value corresponding to head value index in dense array
 */
int normalize_dense_array(re_l_t *dense_array,
		const ci_t coldim, const mod_t modulus);

/**
 * \brief Normalizes multiline vector.
 *
 * \param multiline m
 *
 * \param field characteristic modulus
 */
void normalize_multiline(ml_t *m, const ci_t coldim, const mod_t modulus) ;

/**
 * \brief Copies two dense arrays to a dense multiline for further processing.
 * \note Multiline m must have already memory allocated correspondingly.
 * Moreover all entries of m->val must be initialized to zero beforehand.
 *
 * \param dense array dense_1
 *
 * \param dense array dense_2
 *
 * \param minimal first position of non-zero entries in dense_1 and dense_2
 * start_pos
 *
 * \param multiline m
 *
 * \param array size resp. column dimension coldim
 */
void copy_dense_arrays_to_zero_dense_multiline(const re_l_t *dense_1,
		const re_l_t *dense_2, const int start_pos, ml_t *m, const ci_t coldim,
		const mod_t modulus) ;

/**
 * \brief Copies two dense arrays to a multiline for further processing.
 *
 * \param dense array dense_1
 *
 * \param dense array dense_2
 *
 * \param minimal first position of non-zero entries in dense_1 and dense_2
 * start_pos
 *
 * \param multiline m
 *
 * \param array size resp. column dimension coldim
 */
void copy_dense_arrays_to_multiline(const re_l_t *dense_1,
		const re_l_t *dense_2, const int start_pos, ml_t *m, const ci_t coldim,
		const mod_t modulus) ;

/**
 * \brief Copies one dense array to a dense multiline for further processing.
 * \note Multiline m must have already memory allocated correspondingly.
 * Moreover all entries of m->val must be initialized to zero beforehand.
 *
 * \param dense array dense_1
 *
 * \param position of first non-zero entry in array dense_1 start_pos
 *
 * \param multiline m
 *
 * \param array size resp. column dimension coldim
 */
void copy_dense_array_to_zero_dense_multiline(const re_l_t *dense_1,
		const int start_pos, ml_t *m, const ci_t coldim, const mod_t modulus) ;

#if 0
void copy_dense_arrays_to_dense_multiline(const re_l_t *dense_1,
		const re_l_t *dense_2, ml_t *m, const ci_t coldim, const mod_t modulus) {

	ci_t i;
	re_l_t tmp_1, tmp_2;
	for (i=0; i<coldim; ++i) {
		tmp_1 = MODP(dense_1[i] , modulus);
		tmp_2 = MODP(dense_2[i] , modulus);
		m->val[2*i]   = (re_t)tmp_1;
		m->val[2*i+1] = (re_t)tmp_2;
	}
}
#endif

/**
 * \brief Copies multiline to two dense arrays for further processing.
 *
 * \param multiline m
 *
 * \param dense array dense_1
 *
 * \param dense array dense_2
 *
 * \param array size resp. column dimension coldim
 */
void copy_multiline_to_dense_array(const ml_t m, re_l_t *dense_1,
		re_l_t *dense_2, const ci_t coldim) ;

/**
 * \brief Returns smallest row index in waiting list for echelonization.
 *
 * \param waiting list waiting
 *
 * \return 1 if waiting list is not empty, 0 else
 */
int get_smallest_waiting_row(wl_t *waiting_global,
		ri_t *wl_idx, ri_t *wl_lp) ;

#if GBLA_WITH_FFLAS
/**
 * \brief Copies meta data from block matrix representation B to dense matrix
 * representation A. Needs modulus, too.
 *
 * \param dense matrix representation A
 *
 * \param block multiline matrix representation B
 *
 * \param characteristic of underlying field modulus
 */
void copyMetaData(DNS *A, sbm_fl_t *B, mod_t modulus) {
	A->row  = (dimen_t)B->nrows;
	A->col  = (dimen_t)B->ncols;
	A->ld   = ALIGN(A->col);
	A->mod  = (elemt_t)modulus;
	A->nnz  = (index_t)B->nnz;
}
#endif

/**
 * \brief Reduces dense block block_B with rectangular sparse block block_A.
 *
 * \param sparse block block_A
 *
 * \param sparse block block_B
 *
 * \param dense block dense_B
 *
 * \param block height bheight
 *
 * \param invert scalars? inv_scalars
 *
 * \param characteristic of underlying field modulus
 */
void red_with_rectangular_block(mbl_t *block_A, mbl_t *block_B, re_l_t **dense_B,
		const ri_t bheight, const int inv_scalars, const mod_t modulus);

/**
 * \brief Reduces dense block block_B with triangular sparse block block_A.
 *
 * \param sparse block block_A
 *
 * \param dense block dense_B
 *
 * \param block height bheight
 *
 * \param characteristic of underlying field modulus
 *
 * \param invert scalars? inv_scalars
 */
void red_with_triangular_block(mbl_t *block_A, re_l_t **dense_B,
		const ri_t bheight, int inv_scalars, const mod_t modulus);

/**
 * \brief Elimination procedure which reduces the block submatrix A to the unit
 * matrix. Corresponding changes in block submatrix B are carried out, too.
 *
 * \param block submatrix A (left upper side)
 *
 * \param block submatrix B (right upper side)
 *
 * \param characteristic of underlying field modulus
 *
 * \param number of threads nthrds
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_A_block(sbm_fl_t **A, sbm_fl_t *B, const mod_t modulus, int nthrds);

/**
 * \brief Different block tasks when reducing block submatrix A.
 *
 * \param block submatrix A (left upper side)
 *
 * \param block submatrix B (right upper side)
 *
 * \param column index of blocks in B block_col_idx_B
 *
 * \param number of block rows in A nblock_rows_A
 *
 * \param characteristic of underlying field modulus
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_A_blocks_task(sbm_fl_t *A, sbm_fl_t *B, const ci_t block_col_idx_B,
		const ri_t nbrows_A, const mod_t modulus);

/**
 * \brief Elimination procedure which reduces the block submatrix C to zero.
 * Corresponding changes in block submatrix D are carried out using B, too.
 *
 * \param block submatrix B (right upper side)
 *
 * \param block submatrix C (left lower side)
 *
 * \param block submatrix D (right lower side)
 *
 * \param inverse scalars? inv_scalars
 *
 * \param characteristic of underlying field modulus
 *
 * \param number of threads nthrds
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_C_block(sbm_fl_t *B, sbm_fl_t **C, sbm_fl_t *D, const int inv_scalars,
		const mod_t modulus, const int nthrds);

/**
 * \brief Different block tasks when reducing block submatrix C.
 *
 * \param block submatrix B (right upper side)
 *
 * \param block submatrix C (left lower side)
 *
 * \param block submatrix D (right lower side)
 *
 * \param column index of blocks in D block_col_idx_D
 *
 * \param number of block rows in C nblock_rows_C
 *
 * \param inverse scalars? inv_scalars
 *
 * \param characteristic of underlying field modulus
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_C_blocks_task(sbm_fl_t *B, sbm_fl_t *C, sbm_fl_t *D,
		const ci_t block_col_idx_D, const ri_t nbrows_C, const ci_t nbcols_C,
		const int inv_scalars, const mod_t modulus);

/**
 * \brief Elimination procedure which reduces the block submatrix D to an
 * upper triangular matrix. For this FFLAS-FFPACK procedures are used. Thus the
 * input matrix D has to be converted to DNS type first.
 *
 * \param block submatrix D (right lower side), input matrix
 *
 * \param characteristic of underlying field modulus
 *
 * \param number of threads nthrds
 *
 * \return rank of D_red
 */
ri_t elim_fl_D_fflas_ffpack(sbm_fl_t *D, const mod_t modulus, int nthrds);

/**
 * \brief Elimination procedure which reduces the block submatrix D to an
 * upper triangular matrix. Note that the input matrix D will be removed
 * later on and the output matrix D_red is in multiline format for further
 * reduction steps.
 *
 * \param block submatrix D (right lower side), input matrix
 *
 * \param multiline submatrix D_red, output matrix
 *
 * \param characteristic of underlying field modulus
 *
 * \param number of threads nthrds
 *
 * \return rank of D_red
 */
ri_t elim_fl_D_block(sbm_fl_t *D, sm_fl_ml_t *D_red, const mod_t modulus, int nthrds);

/**
 * \brief Elimination procedure which reduces the multiline matrix C to zero
 * carrying out corresponding computations by A and B
 *
 * \param multiline submatrix C (left lower side)
 *
 * \param multiline submatrix A (left upper side)
 *
 * \param characteristic of underlying field modulus
 *
 * \param number of threads nthrds
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_C_ml(sm_fl_ml_t *C, sm_fl_ml_t *A, mod_t modulus, int nthrds);

/**
 * \brief Executing corresponding multiples from A in multilines of C in order
 * to prepare C for the reduction of D by multiples of B
 *
 * \param multiline submatrix C (left lower side)
 *
 * \param multiline submatrix A (left upper side)
 *
 * \param characteristic of underlying field modulus
 *
 * \return 0 if success, 1 if failure
 */
int elim_fl_C_ml_task(sm_fl_ml_t *C, sm_fl_ml_t *A, ri_t row_idx, mod_t modulus);

/**
 * \brief Echelonizes the multiline rows of A from row index from up to row
 * index to sequential. This is done in order to prepare a basis for the
 * upcoming parallel structure echelonization
 *
 * \param multiline submatrix A (left upper side)
 *
 * \param start row index from
 *
 * \param end row index to
 *
 * \param field characteristic modulus
 *
 * \return number of real pivots found
 */
ri_t echelonize_rows_sequential(sm_fl_ml_t *A, const ri_t from, const ri_t to,
		const mod_t modulus);

/**
 * \brief Echelonizes the multiline rows of A from row index from up to row
 * index to sequential. This is done in order to prepare a basis for the
 * upcoming parallel structure echelonization
 *
 * \param multiline submatrix A (left upper side)
 *
 * \param field characteristic modulus
 *
 * \return 0 if success, 1 if failure
 */
int echelonize_rows_task(sm_fl_ml_t *A, const ri_t ml_ncols,
		/* ri_t global_next_row_to_reduce, ri_t global_last_piv, */
		/* wl_t *waiting_global, */
		const mod_t modulus
		/* , omp_lock_t echelonize_lock */
		);

/**
 * \brief Echelonizes one multiline row (represented by dense_array_1 and
 * dense_array_2) by the already known pivot rows A->ml[first_piv] up to
 * A->ml[last_piv].
 *
 * \param multiline submatrix A (left upper side)
 *
 * \param dense array for first part of multiline row dense_array_1
 *
 * \param dense array for second part of multiline row dense_array_2
 *
 * \param first known pivot first_piv
 *
 * \param last known pivot last_piv
 *
 * \param field characteristic modulus
 */
void echelonize_one_row(sm_fl_ml_t *A, re_l_t *dense_array_1,
		re_l_t *dense_array_2,
		const ri_t first_piv, const ri_t last_piv,
		const mod_t modulus);

/**
 * \brief Restores the two dense arrays in a multiline row in D after
 * echelonizing the corresponding multiline row
 *
 * \param multiline row in D ml
 *
 * \param dense array for first part of multiline row dense_array_1
 *
 * \param dense array for second part of multiline row dense_array_2
 *
 * \param columns dimension coldim
 *
 * \param field characteristic modulus
 *
 * \param reduce or just store in multiline row? reduce
 * if 1 then reduction is done, else we only store the multiline row
 *
 * \param current multiline row in D the two dense arrays represent,
 * i.e. ml = A->ml[curr_row_to_reduce] curr_row_to_reduce
 */
void save_back_and_reduce(ml_t *ml, re_l_t *dense_array_1,
		re_l_t *dense_array_2, const ci_t coldim, const mod_t modulus,
		const int reduce, const ri_t curr_row_to_reduce);

#endif /* GB_ELIMINATION_H */

/* vim:sts=2:sw=2:ts=2:
*/
