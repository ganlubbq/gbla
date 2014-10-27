#include "gb.h"

#define __GB_CLI_DEBUG  0

void print_help() {
  printf("\n");
  printf("NAME\n");
  printf("    gb - computes a Groebner basis\n");
  printf("\n");
  printf("SYNOPSIS\n");
  printf("    gb [options] [file..]\n");
  printf("\n");
  printf("DESCRIPTION\n");
  printf("    Computes Gaussian Elimination of a structured hybrid sparse-\n");
  printf("    dense matrix coming from Groebner basis computations.\n");
  printf("\n");
  printf("OPTIONS\n");
  printf("    -b          Dimensions of a block in submatrices.\n");
  printf("                Default: 256.\n");
  printf("                NOTE: The block dimension must be a multiple of __GB_LOOP_UNROLL_BIG\n");
  printf("                      which is by default 64. You can reset __GB_LOOP_UNROLL_BIG in \n");
  printf("                      src/gb_config.h.in (you need to rebuild the library afterwards).\n");
  printf("    -c          Result checked resp. validated with structured Gaussian\n");
  printf("                Elimination. By default there is no validation.\n");
  printf("    -h          Print help.\n");
  printf("    -f          Free memory on the go.\n");
  printf("                Default: 1, memory is freed on the go.\n");
  printf("                Use 0 for keeping complete memory until the end.\n");
  printf("    -m          Number of rows per multiline.\n");
  printf("                Default: 1.\n");
  printf("    -p          Writes intermediate matrices in pbm format.\n");
  printf("    -s          Splicing options:\n");
  printf("                0: standard Faugère-Lachartre block method,\n");
  printf("                1: A is multiline, all other submatrices are blocks,\n");
  printf("                2: A and C are multiline, B and D are blocks.\n");
  printf("                Default: 0.\n");
  printf("    -t THRDS    Number of threads used.\n");
  printf("                Default: 1.\n");
  printf("    -v LEVEL    Level of verbosity:\n");
  printf("                1 -> only error messages printed\n");
  printf("                2 -> some meta information printed\n");
  printf("                3 -> additionally meta information printed\n");
  printf("                Note: Everything >2 is time consuming and slows down\n");
  printf("                      the overall computations.\n");
  printf("\n");

  return; 
}


int main(int argc, char *argv[]) {  
  const char *fn        = NULL;
  int free_mem          = 1;
  int validate_results  = 0;
  int verbose           = 0;
  int method            = 0;
  int nrows_multiline   = 1;
  int block_dimension   = 256;
  int write_pbm         = 0;
  int nthreads          = 1;
  int splicing          = 0;

  int index;
  int opt;

  // timing structs
  struct timeval t_load_start;

  opterr  = 0;

  while ((opt = getopt(argc, argv, "b:cf:hm:t:v:ps:")) != -1) {
    switch (opt) {
      case 'b': 
        block_dimension = atoi(optarg);
        if (block_dimension < 1)
          block_dimension = 256;
        break;
      case 'c': 
        validate_results  = 1;
        break;
      case 'f':
        free_mem = atoi(optarg);
        if (free_mem < 0)
          free_mem  = 1;
        break;
      case 'h':
        print_help();
        return 0;
      case 'm': 
        nrows_multiline = atoi(optarg);
        if (nrows_multiline < 1)
          nrows_multiline = 1;
        break;
      case 'p': 
        write_pbm = 1;
        break;
      case 't': 
        nthreads  = atoi(optarg);
        break;
      case 's': 
        splicing = atoi(optarg);
        if (splicing<0)
          splicing  = 0;
        if (splicing>2)
          splicing  = 2;
        break;
      case 'v': 
        verbose = atoi(optarg);
        if (verbose > 3)
          verbose = 2;
        break;
      case '?':
        if (optopt == 'f')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
              "Unknown option character `\\x%x'.\n",
              optopt);
        return 1;
      default:
        abort ();
    }
  }
  for (index = optind; index < argc; index++)
    fn = argv[index];

  if (fn == NULL) {
    fprintf(stderr, "File name is required.\nSee help using '-h' option.\n");
    return 1;
  }

  sm_t *M = NULL;

  if (verbose > 0) {
    printf("---------------------------------------------------------------------\n");
    printf("------------- Computing an FL-style Gaussian Elimination ------------\n");
    printf("------------------ with the following options set -------------------\n");
    printf("---------------------------------------------------------------------\n");
    printf("number of threads           %4d\n", nthreads);
    printf("splicing option             %4d\n", splicing);
    printf("dimension of blocks         %4d\n", block_dimension);
    printf("free memory on the go       %4d\n", free_mem);
    printf("write PBM file              %4d\n", write_pbm);
    printf("---------------------------------------------------------------------\n");
  }

  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART loading JCF matrix ...\n");
  }
  // load JCF matrix
  M = load_jcf_matrix(fn, verbose);
  if (verbose > 1) {
    printf("<<<<\tDONE  loading JCF matrix.\n");
    // print walltime
    printf("TIME\t%.3f sec (%.3f %s/sec)\n",
        walltime(t_load_start) / (1000000),
        M->fs / (walltime(t_load_start) / (1000000)), M->fsu);
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
    printf("---------------------------------------------------------------------\n");
    printf("Data for %s\n", fn);
    printf("---------------------------------------------------------------------\n");
    printf("modulus                     %14d\n", M->mod);
    printf("number of rows              %14d\n", M->nrows);
    printf("number of columns           %14d\n", M->ncols);
    printf("number of nonzero elements  %14ld\n", M->nnz);
    printf("density                     %14.2f %%\n", M->density);
    printf("size                        %14.2f %s\n", M->fs, M->fsu);
    printf("---------------------------------------------------------------------\n");
  }

  // write JCF matrix to PBM file
  if (write_pbm) {
    if (verbose > 1) {
      gettimeofday(&t_load_start, NULL);
      printf("---------------------------------------------------------------------\n");
      printf(">>>>\tSTART writing input matrix to PBM file ...\n");
    }
    const char *pbm_fn  = "input-matrix.pbm";
    write_jcf_matrix_to_pbm(M, pbm_fn, verbose);
    if (verbose > 1) {
      printf("<<<<\tDONE writing input matrix to PBM file.\n");
      // print walltime
      printf("TIME\t%.3f sec (%.3f %s/sec)\n",
          walltime(t_load_start) / (1000000),
          M->fs / (walltime(t_load_start) / (1000000)), M->fsu);
      print_mem_usage();
      printf("---------------------------------------------------------------------\n");
      printf("\n");
    }
  }

  switch (splicing) {
    // all submatrices are of block type
    case 0:
      if (fl_block(M, block_dimension, nrows_multiline, nthreads, free_mem, verbose)) {
        printf("Error while trying to eliminate matrix from file '%s' in all block type mode.\n",fn);
      }
      break;
    // submatrix A of multiline type, other submatrices are of block type
    case 1:
      if (fl_ml_A(M, block_dimension, nrows_multiline, nthreads, free_mem, verbose)) {
        printf("Error while trying to eliminate matrix from file '%s' in A multiline,\n",fn);
        printf("all others block type mode.\n");
      }
      break;
    // submatrices A and C of multiline type, submatrices B and D are of block type
    case 2:
      if (fl_ml_A_C(M, block_dimension, nrows_multiline, nthreads, free_mem, verbose)) {
        printf("Error while trying to eliminate matrix from file '%s' in A and C multiline,\n",fn);
        printf("B and D block type mode.\n");
      }
      break;
  }

  free(M);
  return 0;
}

int fl_block(sm_t *M, int block_dimension, int nrows_multiline, int nthreads, int free_mem, int verbose) {
  struct timeval t_load_start;
  // all submatrices of block type
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART splicing and mapping of input matrix ...\n");
  }
  // construct splicing of matrix M into A, B, C and D
  sbm_fl_t *A   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sbm_fl_t *B   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sbm_fl_t *C   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sbm_fl_t *D   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  map_fl_t *map = (map_fl_t *)malloc(sizeof(map_fl_t)); // stores mappings from M <-> ABCD
  splice_fl_matrix(M, A, B, C, D, map, block_dimension, nrows_multiline, nthreads,
      free_mem, verbose);

  if (verbose > 1) {
    printf("<<<<\tDONE  splicing and mapping of input matrix.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
    printf("Number of pivots found: %d\n", map->npiv);
    printf("---------------------------------------------------------------------\n");
    if (verbose > 2) {
      compute_density_block_submatrix(A);
      compute_density_block_submatrix(B);
      compute_density_block_submatrix(C);
      compute_density_block_submatrix(D);
      printf("A [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          A->nrows, A->ncols, A->nnz, A->density);
      printf("B [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          B->nrows, B->ncols, B->nnz, B->density);
      printf("C [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          C->nrows, C->ncols, C->nnz, C->density);
      printf("D [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          D->nrows, D->ncols, D->nnz, D->density);
    } else {
      printf("A [%d x %d]\n",
          A->nrows, A->ncols);
      printf("B [%d x %d]\n",
          B->nrows, B->ncols);
      printf("C [%d x %d]\n",
          C->nrows, C->ncols);
      printf("D [%d x %d]\n",
          D->nrows, D->ncols);
    }
    printf("---------------------------------------------------------------------\n");
  }
#if __GB_CLI_DEBUG
  // column loops 
  const uint32_t clA  = (uint32_t) ceil((float)A->ncols / A->bwidth);
  const uint32_t clB  = (uint32_t) ceil((float)B->ncols / B->bwidth);
  const uint32_t clC  = (uint32_t) ceil((float)C->ncols / C->bwidth);
  const uint32_t clD  = (uint32_t) ceil((float)D->ncols / D->bwidth);
  // row loops 
  const uint32_t rlA  = (uint32_t) ceil((float)A->nrows / A->bheight);
  const uint32_t rlB  = (uint32_t) ceil((float)B->nrows / B->bheight);
  const uint32_t rlC  = (uint32_t) ceil((float)C->nrows / C->bheight);
  const uint32_t rlD  = (uint32_t) ceil((float)D->nrows / D->bheight);

  int ii,jj,kk,ll;
  for (ii=0; ii<rlA; ++ii) {
    for (jj=0; jj<clA; ++jj) {
      if (A->blocks[ii][jj] != NULL) {
        for (kk=0; kk<block_dimension/2; ++kk) {
          printf("%d .. %d .. %d\n",ii,jj,kk);
          printf("%d | %d\n", A->blocks[ii][jj][kk].sz, A->blocks[ii][jj][kk].dense);
        }
      } else {
        for (kk=0; kk<block_dimension/2; ++kk) {
          printf("%d .. %d .. %d\n",ii,jj,kk);
          printf("0 | 0\n");
        }
      }
    }
  }
  printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
  for (ii=0; ii<rlB; ++ii) {
    for (jj=0; jj<clB; ++jj) {
      if (B->blocks[ii][jj] != NULL) {
        for (kk=0; kk<block_dimension/2; ++kk) {
          printf("%d .. %d .. %d\n",ii,jj,kk);
          printf("%d | %d\n", B->blocks[ii][jj][kk].sz, B->blocks[ii][jj][kk].dense);
        }
      }
    }
  }
  // column loops 
  const uint32_t clA  = (uint32_t) ceil((float)A->ncols / A->bwidth);
  const uint32_t clB  = (uint32_t) ceil((float)B->ncols / B->bwidth);
  const uint32_t clC  = (uint32_t) ceil((float)C->ncols / C->bwidth);
  const uint32_t clD  = (uint32_t) ceil((float)D->ncols / D->bwidth);
  // row loops 
  const uint32_t rlA  = (uint32_t) ceil((float)A->nrows / A->bheight);
  const uint32_t rlB  = (uint32_t) ceil((float)B->nrows / B->bheight);
  const uint32_t rlC  = (uint32_t) ceil((float)C->nrows / C->bheight);
  const uint32_t rlD  = (uint32_t) ceil((float)D->nrows / D->bheight);

  int ii,jj,kk,ll;
  for (ii=0; ii<rlA; ++ii) {
    for (jj=0; jj<clA; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (A->blocks[ii][jj] != NULL) {
          for (ll=0; ll<A->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", A->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", A->blocks[ii][jj][kk].val[2*ll], A->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  printf("NNZ %ld\n\n",ctr_nnz);
  for (ii=0; ii<rlB; ++ii) {
    for (jj=0; jj<clB; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (B->blocks[ii][jj] != NULL) {
          for (ll=0; ll<B->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", B->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", B->blocks[ii][jj][kk].val[2*ll], B->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  for (ii=0; ii<rlC; ++ii) {
    for (jj=0; jj<clC; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (C->blocks[ii][jj] != NULL) {
          for (ll=0; ll<C->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", C->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", C->blocks[ii][jj][kk].val[2*ll], C->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  for (ii=0; ii<rlD; ++ii) {
    for (jj=0; jj<clD; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (D->blocks[ii][jj] != NULL) {
          for (ll=0; ll<D->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", D->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", D->blocks[ii][jj][kk].val[2*ll], D->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
#endif

  // reducing submatrix A using methods of Faugère & Lachartre
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART reducing A ...\n");
  }
  if (elim_fl_A_block(A, B, M->mod, nthreads)) {
    printf("Error while reducing A.\n");
    return 1;
  }
  if (verbose > 1) {
    printf("<<<<\tDONE  reducing A.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
  }

  // reducing submatrix C to zero using methods of Faugère & Lachartre
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART reducing C to zero ...\n");
  }
  if (elim_fl_C_block(B, C, D, M->mod, nthreads)) {
    printf("Error while reducing C.\n");
    return 1;
  }
  if (verbose > 1) {
    printf("<<<<\tDONE  reducing C to zero.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
  }

  // echelonize D using methods of Faugère & Lachartre

  // We need to do at least two rounds on D, possibly even reducing B further
  // with the reduced D later on. For this purpose we use a multiline version of
  // D as output of the Gaussian elimination.
  sm_fl_ml_t *D_red = (sm_fl_ml_t *)malloc(sizeof(sm_fl_ml_t));
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART reducing D to upper triangular matrix ...\n");
  }
  if (elim_fl_D_block(D, D_red, M->mod, nthreads)) {
    printf("Error while reducing D to upper triangular matrix.\n");
    return 1;
  }
  if (verbose > 1) {
    printf("<<<<\tDONE  reducing D to upper triangular matrix.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
  }
  return 0;
}

int fl_ml_A(sm_t *M, int block_dimension, int nrows_multiline, int nthreads, int free_mem, int verbose) {
  struct timeval t_load_start;
  // submatrix A of multiline type, rest of block type
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART splicing and mapping of input matrix ...\n");
  }
  // construct splicing of matrix M into A, B, C and D
  sm_fl_ml_t *A = (sm_fl_ml_t *)malloc(sizeof(sm_fl_ml_t));
  sbm_fl_t *B   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sbm_fl_t *C   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sbm_fl_t *D   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  map_fl_t *map = (map_fl_t *)malloc(sizeof(map_fl_t)); // stores mappings from M <-> ABCD
  splice_fl_matrix_ml_A(M, A, B, C, D, map, block_dimension, nrows_multiline, nthreads,
      free_mem, verbose);

  if (verbose > 1) {
    printf("<<<<\tDONE  splicing and mapping of input matrix.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
    printf("Number of pivots found: %d\n", map->npiv);
    printf("---------------------------------------------------------------------\n");
    if (verbose > 2) {
      compute_density_ml_submatrix(A);
      compute_density_block_submatrix(B);
      compute_density_block_submatrix(C);
      compute_density_block_submatrix(D);
      printf("A [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          A->nrows, A->ncols, A->nnz, A->density);
      printf("B [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          B->nrows, B->ncols, B->nnz, B->density);
      printf("C [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          C->nrows, C->ncols, C->nnz, C->density);
      printf("D [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          D->nrows, D->ncols, D->nnz, D->density);
    } else {
      printf("A [%d x %d]\n",
          A->nrows, A->ncols);
      printf("B [%d x %d]\n",
          B->nrows, B->ncols);
      printf("C [%d x %d]\n",
          C->nrows, C->ncols);
      printf("D [%d x %d]\n",
          D->nrows, D->ncols);
    }
    printf("---------------------------------------------------------------------\n");
  }

#if __GB_CLI_DEBUG
  // column loops 
  const uint32_t clB  = (uint32_t) ceil((float)B->ncols / B->bwidth);
  const uint32_t clC  = (uint32_t) ceil((float)C->ncols / C->bwidth);
  const uint32_t clD  = (uint32_t) ceil((float)D->ncols / D->bwidth);
  // row loops 
  const uint32_t rlA  = (uint32_t) ceil((float)A->nrows / __GB_NROWS_MULTILINE);
  const uint32_t rlB  = (uint32_t) ceil((float)B->nrows / B->bheight);
  const uint32_t rlC  = (uint32_t) ceil((float)C->nrows / C->bheight);
  const uint32_t rlD  = (uint32_t) ceil((float)D->nrows / D->bheight);

  int ii,jj,kk,ll;
  for (ii=0; ii<rlA; ++ii) {
    printf("%d .. \n",ii);
    if (A->ml[ii].sz>0) {
      for (ll=0; ll<A->ml[ii].sz; ++ll) {
        printf("%d %d ", A->ml[ii].val[2*ll], A->ml[ii].val[2*ll+1]);
      }
      printf("\n");
    }
  }
  for (ii=0; ii<rlB; ++ii) {
    for (jj=0; jj<clB; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (B->blocks[ii][jj] != NULL) {
          for (ll=0; ll<B->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d %d ", B->blocks[ii][jj][kk].val[2*ll], B->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  for (ii=0; ii<rlC; ++ii) {
    for (jj=0; jj<clC; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (C->blocks[ii][jj] != NULL) {
          for (ll=0; ll<C->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d %d ", C->blocks[ii][jj][kk].val[2*ll], C->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  for (ii=0; ii<rlD; ++ii) {
    for (jj=0; jj<clD; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (D->blocks[ii][jj] != NULL) {
          for (ll=0; ll<D->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d %d ", D->blocks[ii][jj][kk].val[2*ll], D->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
#endif

  // reducing submatrix A using methods of Faugère & Lachartre
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART reducing A ...\n");
  }
  if (elim_fl_A_ml_block(A, B, nthreads)) {
    printf("Error while reducing A.\n");
    return 1;
  }
  if (verbose > 1) {
    printf("<<<<\tDONE  reducing A.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
  }
  return 0;
}

int fl_ml_A_C(sm_t *M, int block_dimension, int nrows_multiline, int nthreads, int free_mem, int verbose) {
  struct timeval t_load_start;
  // submatrices A and C of multiline type, B and D of block type
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART splicing and mapping of input matrix ...\n");
  }
  // construct splicing of matrix M into A, B, C and D
  sm_fl_ml_t *A = (sm_fl_ml_t *)malloc(sizeof(sm_fl_ml_t));
  sbm_fl_t *B   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  sm_fl_ml_t *C = (sm_fl_ml_t *)malloc(sizeof(sm_fl_ml_t));
  sbm_fl_t *D   = (sbm_fl_t *)malloc(sizeof(sbm_fl_t));
  map_fl_t *map = (map_fl_t *)malloc(sizeof(map_fl_t)); // stores mappings from M <-> ABCD
  splice_fl_matrix_ml_A_C(M, A, B, C, D, map, block_dimension, nrows_multiline, nthreads,
      free_mem, verbose);

  if (verbose > 1) {
    printf("<<<<\tDONE  splicing and mapping of input matrix.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
    printf("Number of pivots found: %d\n", map->npiv);
    printf("---------------------------------------------------------------------\n");
    if (verbose > 2) {
      compute_density_ml_submatrix(A);
      compute_density_block_submatrix(B);
      compute_density_ml_submatrix(C);
      compute_density_block_submatrix(D);
      printf("A [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          A->nrows, A->ncols, A->nnz, A->density);
      printf("B [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          B->nrows, B->ncols, B->nnz, B->density);
      printf("C [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          C->nrows, C->ncols, C->nnz, C->density);
      printf("D [%d x %d]\t - %10ld nze --> %7.3f %% density\n",
          D->nrows, D->ncols, D->nnz, D->density);
    } else {
      printf("A [%d x %d]\n",
          A->nrows, A->ncols);
      printf("B [%d x %d]\n",
          B->nrows, B->ncols);
      printf("C [%d x %d]\n",
          C->nrows, C->ncols);
      printf("D [%d x %d]\n",
          D->nrows, D->ncols);
    }
    printf("---------------------------------------------------------------------\n");
  }

#if __GB_CLI_DEBUG
  // column loops 
  const uint32_t clB  = (uint32_t) ceil((float)B->ncols / B->bwidth);
  const uint32_t clD  = (uint32_t) ceil((float)D->ncols / D->bwidth);
  // row loops 
  const uint32_t rlA  = (uint32_t) ceil((float)A->nrows / __GB_NROWS_MULTILINE);
  const uint32_t rlB  = (uint32_t) ceil((float)B->nrows / B->bheight);
  const uint32_t rlC  = (uint32_t) ceil((float)C->nrows / __GB_NROWS_MULTILINE);
  const uint32_t rlD  = (uint32_t) ceil((float)D->nrows / D->bheight);

  int ii,jj,kk,ll;
  for (ii=0; ii<rlA; ++ii) {
    printf("%d .. \n",ii);
    printf("size %d\n", A->ml[ii].sz * 2);
    if (A->ml[ii].sz>0) {
      for (ll=0; ll<A->ml[ii].sz; ++ll) {
        printf("%d -- ", A->ml[ii].idx[ll]);
        printf("%d %d ", A->ml[ii].val[2*ll], A->ml[ii].val[2*ll+1]);
      }
      printf("\n");
    }
  }
  for (ii=0; ii<rlB; ++ii) {
    for (jj=0; jj<clB; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (B->blocks[ii][jj] != NULL) {
          for (ll=0; ll<B->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", B->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", B->blocks[ii][jj][kk].val[2*ll], B->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
  for (ii=0; ii<rlC; ++ii) {
    printf("%d .. \n",ii);
    printf("size %d\n", C->ml[ii].sz * 2);
    if (C->ml[ii].sz>0) {
      for (ll=0; ll<C->ml[ii].sz; ++ll) {
        printf("%d -- ", C->ml[ii].idx[ll]);
        printf("%d %d ", C->ml[ii].val[2*ll], C->ml[ii].val[2*ll+1]);
      }
      printf("\n");
    }
  }
  for (ii=0; ii<rlD; ++ii) {
    for (jj=0; jj<clD; ++jj) {
      for (kk=0; kk<block_dimension/2; ++kk) {
        printf("%d .. %d .. %d\n",ii,jj,kk);
        if (D->blocks[ii][jj] != NULL) {
          for (ll=0; ll<D->blocks[ii][jj][kk].sz; ++ll) {
            printf("%d -- ", D->blocks[ii][jj][kk].idx[ll]);
            printf("%d %d ", D->blocks[ii][jj][kk].val[2*ll], D->blocks[ii][jj][kk].val[2*ll+1]);
          }
          printf("\n");
        }
      }
    }
  }
#endif

  // reducing submatrix A using methods of Faugère & Lachartre
  if (verbose > 1) {
    gettimeofday(&t_load_start, NULL);
    printf("---------------------------------------------------------------------\n");
    printf(">>>>\tSTART reducing A ...\n");
  }
  if (elim_fl_A_ml_block(A, B, nthreads)) {
    printf("Error while reducing A.\n");
    return 1;
  }
  if (verbose > 1) {
    printf("<<<<\tDONE  reducing A.\n");
    printf("TIME\t%.3f sec\n",
        walltime(t_load_start) / (1000000));
    print_mem_usage();
    printf("---------------------------------------------------------------------\n");
    printf("\n");
  }
  return 0;
}

