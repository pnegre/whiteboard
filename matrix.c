/* Copyright (C) 2008 Pere Negre
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "matrix.h"

matrix_t *matrixNew(int h, int v)
{
	matrix_t *m;
	m = malloc(sizeof(matrix_t));
	m->d = malloc(h*v*sizeof(float));
	m->h = h;
	m->v = v;
	return m;
}

void matrixFree(matrix_t *m)
{
	free(m->d);
	free(m);
}

float matrixGetElement(matrix_t *m, int i, int j)
{
	return (*(m->d + i + j*m->h));
}

void matrixSetElement(matrix_t *m, float e, int i, int j)
{
	*(m->d + i + j*m->h) = e;
}

void matrixPrint(matrix_t *m)
{
	int i,j;
	for (j=0; j<m->v; j++)
	{
		for (i=0; i<m->h; i++)
			printf("%03.2f ",matrixGetElement(m,i,j));
		printf("\n");
	}
}


matrix_t *matrixCof1(matrix_t *m, int x, int y)
{
	int i,j;
	int xx = 0;
	int yy = 0;

	matrix_t *c;

	c = matrixNew(m->h-1, m->v-1);

	for (i=0; i<m->h; i++)
	{
		if (i==x) continue;
		yy = 0;
		for (j=0; j<m->v; j++)
		{
			if (j==y) continue;
			matrixSetElement(c, matrixGetElement(m,i,j), xx, yy);
			yy++;
		}
		xx++;
	}
	return c;
}



float matrixDeterminant(matrix_t *m)
{
	int i;
	float r;
	int p;
	matrix_t *cofactor;

	if ((m->h == 2) && (m->v == 2))
		return (matrixGetElement(m,0,0)*matrixGetElement(m,1,1) -
			matrixGetElement(m,0,1)*matrixGetElement(m,1,0));
	
	// Determinant calculation: laplace formula

	r = 0.0f;
	for (i=0; i<m->h; i++)
	{
		cofactor = matrixCof1(m,i,0);
		if (i % 2) p=-1; else p=1;
		r = r + matrixGetElement(m,i,0) * p * matrixDeterminant(cofactor);
		matrixFree(cofactor);
	}

	return r;

}


matrix_t *matrixCofactor(matrix_t *m)
{
	int i,j;
	float f;
	int p;
	matrix_t *cc, *c;

	c = matrixNew(m->h,m->v);

	for (i=0; i<m->h; i++)
		for (j=0; j<m->v; j++)
		{
			p = ((i+j) % 2) ? -1: 1;
			cc = matrixCof1(m,i,j);
			f = p*matrixDeterminant(cc);
			matrixSetElement(c,f,i,j);
			matrixFree(cc);
		}
	
	return c;

}


matrix_t *matrixCopy(matrix_t *source)
{
	int i,j;
	matrix_t *r;
	r = matrixNew(source->h,source->v);
	
	for(i=0; i<source->h; i++)
		for(j=0; j<source->v; j++)
			matrixSetElement(r, matrixGetElement(source,i,j), i, j);

	return r;
}


void matrixTranspose(matrix_t *m)
{
	int i,j;
	matrix_t *c;
	c = matrixCopy(m);

	for (i=0; i<c->h; i++)
		for(j=0; j<c->v; j++)
			matrixSetElement(m, matrixGetElement(c,i,j), j, i);

	matrixFree(c);
}


void matrixInverse(matrix_t *m)
{
	float f, det;
	int i,j;
	matrix_t *temp;

	det = matrixDeterminant(m);
	temp = matrixCofactor(m);
	matrixTranspose(temp);

	for(i=0; i<temp->h; i++)
		for(j=0; j<temp->v; j++)
		{
			f = matrixGetElement(temp,i,j);
			f = f / det;
			matrixSetElement(m,f,i,j);
		}
	
	matrixFree(temp);

}

matrix_t *matrixMultiply(matrix_t *m, matrix_t *n)
{
	matrix_t *r;
	int i,j,k;
	float f;
	
	r = matrixNew(n->h,m->v);

	for(j=0; j<r->v; j++)
	{
		for(k=0; k<n->h; k++)
		{
			f=0.0f;
			for(i=0; i<m->h; i++)
			{
				f = f + matrixGetElement(m,i,j) * matrixGetElement(n,k,i);
			}
			matrixSetElement(r,f,k,j);
		}
	}

	return r;
}

/*

int main()
{
	int i,j;
	matrix_t *m, *n, *r;

	// Testing: solve the equations
	//
	//  2x + 3y - z = 5
	//  8x - 7y = 3
	//  x + 4y - 6z = 8
	//
	// Solutions: x=1.04, y=0.76, z=-0.66

	m = matrixNew(3,3);

	matrixSetElement(m,2.0f,0,0);
	matrixSetElement(m,3.0f,1,0);
	matrixSetElement(m,-1.0f,2,0);
	
	matrixSetElement(m,8.0f,0,1);
	matrixSetElement(m,-7.0f,1,1);
	matrixSetElement(m,0.0f,2,1);

	matrixSetElement(m,1.0f,0,2);
	matrixSetElement(m,4.0f,1,2);
	matrixSetElement(m,-6.0f,2,2);

	n = matrixNew(1,3);
	matrixSetElement(n,5.0f,0,0);
	matrixSetElement(n,3.0f,0,1);
	matrixSetElement(n,8.0f,0,2);

	matrixInverse(m);
	r = matrixMultiply(m,n);

	matrixPrint(r);

	matrixFree(m);
	matrixFree(r);
	matrixFree(n);

	exit(0);	
}


*/


