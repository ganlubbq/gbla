/* gbla: Gröbner Basis Linear Algebra
 * Copyright (C) 2015 Brice Boyer <brice.boyer@lip6.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "types.h"

/*
 * a b 0 0 0 0 c
 * a 0 0 0 b 0 c
 * 0 c 0 0 d 0 0
 * 0 0 c 0 0 d 0
 *
 * p_abc = 2 3 4
 * p_cd = 9 8
 *
 * becomes
 * a b 0 . 0 0 0 c
 * 0 c 0 . 0 d 0 0
 * 0 0 c . 0 0 d 0
 * -------------
 * a 0 0 . 0 b 0 c
 *
 * that is :
 *
 * 2 3 0 | 0 0 0 4
 * 0 9 0 | 0 8 0 0
 * 0 0 9 | 0 0 8 0
 * ---------------
 * 2 0 0 | 0 3 0 4

 *
 * becomes
 *
 * a 0 0 | 0 b 0 c
 * 0 d 0 | 0 c 0 0
 * 0 0 c | 0 0 d 0
 * ---------------
 * a b 0 | 0 0 0 c
 *
 * that is :
 *
 * 2 0 0 | 0 3 0 4
 * 0 8 0 | 0 9 0 0
 * 0 0 9 | 0 0 8 0
 * ---------------
 * 2 3 0 | 0 0 0 4
 *
 *
 * A^-1 . Bt =
 * 0 3/2 0   2
 * 0 9/8 0   0
 * 0 0   8/9 0
 *
 * 0 5 0 2
 * 0 2 0 0
 * 0 0 4 0
 *
 */

#include "tools.h"

#include "selecter.h"


int main()
{
	const uint32_t row = 4 ;
	const uint32_t col = 7 ;
	const uint64_t nnz = 10 ;


#ifdef _PROPOSED_FORMAT
	const elem_s mod = 11 ;
	FILE * toto =fopen("test_new.gb","wb");
	assert(toto);


	uint32_t un = Mjoin(select,elem_s)();



	const uint64_t start_zo[] = { 0 , 3, 6 , 8 ,10 };
	/* uint32_t colid_zo[nnz] = { 0, 1, 6, 0, 4, 6, 1, 4, 2, 5 }; */
	const uint32_t colid_zo[] = { 0 , 2 , 6 | NEGMASK, 0 | NEGMASK, 4 | NEGMASK, 6 | NEGMASK, 1 |NEGMASK, 4 | NEGMASK, 2 | NEGMASK, 5| NEGMASK};
	const uint64_t colid_size = 10 ;
	const uint32_t map_zo_pol[] = { 0, 0, 1, 1 };
	const uint32_t pol_nb = 2 ;
	const uint32_t pol_start [] = { 0 , 3 , 5 } ;
	const uint32_t pol_nnz = pol_start[pol_nb] ;
	const elem_s pol_data [] = { 2, 3, 4, 9, 8 };

	fwrite(&un,sizeof(uint32_t),1,toto);
	fwrite(&row,sizeof(uint32_t),1,toto);
	fwrite(&col,sizeof(uint32_t),1,toto);
	fwrite(&mod,sizeof(elem_s),1,toto);
	fwrite(&nnz,sizeof(uint64_t),1,toto);
	fwrite(start_zo,sizeof(uint64_t),row+1,toto);
	fwrite(map_zo_pol,sizeof(uint32_t),row,toto);
	fwrite(&colid_size,sizeof(uint64_t),1,toto);
	fwrite(colid_zo,sizeof(uint32_t),colid_size,toto);
	fwrite(&pol_nb,sizeof(uint32_t),1,toto);
	fwrite(pol_start,sizeof(uint32_t),pol_nb+1,toto);
	fwrite(pol_data,sizeof(elem_s),pol_nnz,toto);
#else
	const uint32_t mod = 7 ;
	FILE * toto =fopen("test_old.gb","wb");
	assert(toto);
	fwrite(&row,sizeof(uint32_t),1,toto);
	fwrite(&col,sizeof(uint32_t),1,toto);
	fwrite(&mod,sizeof(uint32_t),1,toto);
	fwrite(&nnz,sizeof(uint64_t),1,toto);
	const elem_s data[] = {  2 , 3 , 4, 2, 3, 4, 9 ,8 ,9 ,8 };
	fwrite(data,sizeof(elem_s),nnz,toto);
	const uint32_t cols[] = { 0, 1, 6, 0, 4, 6, 1, 4, 2, 5 } ;
	fwrite(cols,sizeof(uint32_t),nnz,toto);
	const uint32_t rows[] = { 3 , 3, 2 , 2 };
	fwrite(rows,sizeof(uint32_t),row,toto);
#endif


	fclose(toto);
	return 0;
}


