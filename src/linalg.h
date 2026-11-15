#ifndef LINALG_H
#define LINALG_H

typedef struct matrix {
	F64* Data;
	U64 NRows, NCols;
} matrix;
#define mat_at(A, i, j) ((A)->data[(U64)(i)*(A)->NCols + (j)])

internal void MatMul(matrix* C, const matrix* A, const matrix* B);

#endif // LINALG_H
