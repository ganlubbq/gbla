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


#ifndef __GBLA_field_ops_H
#define __GBLA_field_ops_H

#include <math.h>

/* double */

double fmod_double(double a, double p)
{
	double t = fmod(a,p);
	if (t<0.) t += p ;
	return t;
}

/* int32_t */

int32_t fmod_int32_t(int32_t a, int32_t p)
{
	return a%p;
}


#endif /* __GBLA_field_ops_H */
