#include <fflas-ffpack/fflas-ffpack.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t RowReduce_int32_t ( int32_t p, int32_t * A, uint32_t m, uint32_t n, uint32_t lda) ;
uint32_t RowReduce_double  ( double p, double * A, uint32_t m, uint32_t n, uint32_t lda, uint32_t nt) ;
void     Freduce_double(double p, double * A, uint64_t n);
void     Finit_double  (double p, const double * A, double * B, uint64_t n);

// #include <cblas.h>

#ifdef __cplusplus
}
#endif


uint32_t RowReduce_uint32_t( uint32_t p, uint32_t * A, uint32_t m, uint32_t n, uint32_t lda)
{
	size_t * P  = FFLAS::fflas_new<size_t>(std::max(m,n)) ;
	size_t * Qt = FFLAS::fflas_new<size_t>(std::max(m,n)) ;

	Givaro::Modular<uint32_t> F(p);
	// uint32_t r = FFPACK::LUdivine(F,FFLAS::FflasUnit,FFLAS::FflasNoTrans,m,n,A,lda,P,Qt);
	uint32_t r = FFPACK::RowEchelonForm(F,m,n,A,lda,P,Qt);

	FFLAS::fflas_delete(P);
	FFLAS::fflas_delete(Qt);
	return r ;
}

uint32_t RowReduce_double( double p, double * A, uint32_t m, uint32_t n, uint32_t lda, uint32_t nt)
{
	size_t * P  = FFLAS::fflas_new<size_t>(std::max(m,n)) ;
	size_t * Qt = FFLAS::fflas_new<size_t>(std::max(m,n)) ;

	Givaro::Modular<double> F(p);
	uint32_t r ; // = FFPACK::LUdivine(F,FFLAS::FflasUnit,FFLAS::FflasNoTrans,m,n,A,lda,P,Qt);
#ifndef _OPENMP
	r = FFPACK::RowEchelonForm(F,m,n,A,lda,P,Qt);
#else
	PAR_REGION{
		r = FFPACK::pPLUQ(F,FFLAS::FflasUnit,m,n,A,lda,P,Qt,nt);
	}
#endif

	FFLAS::fflas_delete(P);
	FFLAS::fflas_delete(Qt);
	return r ;

}

void Freduce_double(double p, double * A, uint64_t n)
{
	Givaro::Modular<double> F(p);
	FFLAS::freduce(F,n,A,1);
}

void Finit_double(double p, const double * A, double * B, uint64_t n)
{
	Givaro::Modular<double> F(p);
	FFLAS::finit(F,n,A,1,B,1);
}
