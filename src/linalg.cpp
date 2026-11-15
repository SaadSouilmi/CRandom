#include <math.h>

internal void
_MatMul_mm(matrix* C, const matrix* A, const matrix* B)
{
}

internal void
_MatMul_mt(matrix* C, const matrix* A, const matrix* B)
{
}

internal void
_MatMul_tm(matrix* C, const matrix* A, const matrix* B)
{
}

internal void
_MatMul_tt(matrix* C, const matrix* A, const matrix* B)
{
}

internal I32
MatMul(matrix* C, const matrix* A, const matrix* B, I32 At, I32 Bt)
{
	U64 ARows = (At) ? A->NCols : A->NRows;
	U64 ACols = (At) ? A->NRows : A->NCols;
	U64 BRows = (Bt) ? B->NCols : B->NRows;
	U64 BCols = (Bt) ? B->NRows : B->NCols;

	if (ARows != C->NRows || BCols != C->NCols || ACols != BRows) { return 1; }
	
	switch ((At<<1)|Bt)
	{
		case 0b00: { _MatMul_mm(C, A, B); }
		case 0b01: { _MatMul_mt(C, A, B); }
		case 0b10: { _MatMul_tm(C, A, B); }
		case 0b11: { _MatMul_tt(C, A, B); }
	}

	return 0;
}

