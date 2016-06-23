/*
 * @BEGIN LICENSE
 *
 * Psi4: an open-source quantum chemistry software package
 *
 * Copyright (c) 2007-2016 The Psi4 Developers.
 *
 * The copyrights for code used from other parties are included in
 * the corresponding files.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * @END LICENSE
 */

/*! \file
    \ingroup DPD
    \brief Enter brief description of file here
*/
#include <cstdio>
#include "psi4/src/lib/libciomr/libciomr.h"
#include "dpd.h"

namespace psi {

int DPD::file4_mat_irrep_row_zero(dpdfile4 *File, int irrep, int row)
{
    int coltot, my_irrep;

    if(File->incore) return 0;  /* Don't do this if the file is in core */

    my_irrep = File->my_irrep;

    coltot = File->params->coltot[irrep^my_irrep];

    if(coltot)
        zero_arr(File->matrix[irrep][0], coltot);

    return 0;

}

}