/*
 *@BEGIN LICENSE
 *
 * PSI4: an ab initio quantum chemistry software package
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
 *@END LICENSE
 */

/** Standard library includes */
#include <libqt/qt.h>
#include "defines.h"
#include "dfocc.h"

using namespace boost;
using namespace psi;
using namespace std;


namespace psi{ namespace dfoccwave{
  
void DFOCC::separable_tpdm()
{   

    timer_on("sep_tpdm");
if (reference_ == "RESTRICTED") {
    // Build J_Q
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = 0.0;
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              value += bQooA->get(Q,ii);
         }
         Jc->set(Q, 2.0*value);
    }

    // G_Q = \sum_{m,n} b_mn^Q G1c_mn
    g1Qc->gemv(false, nQ_ref, noccA * noccA, bQooA, G1c_oo, 1.0, 0.0);

    //=========================
    // Reference TPDM
    //=========================
    // G_ij (ref) = 
    G2c_oo = SharedTensor2d(new Tensor2d("Reference 3-Index TPDM (Q|OO)", nQ_ref, noccA * noccA));
    G2c_oo->copy(bQooA);
    bQooA.reset();
    G2c_oo->scale(-1.0);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = Jc->get(Q);
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              G2c_oo->add(Q, ii, value);
         }
    }
    G2c_oo->scale(2.0);
    G2c_oo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_oo->print();
    G2c_oo.reset();

    //=========================
    // Reference TPDM Done.
    //=========================

    // Gt_Q = \sum_{e,f} b_ef^Q G1c_ef
    bQvvA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|VV)", nQ_ref, nvirA * nvirA));
    bQvvA->read(psio_, PSIF_DFOCC_INTS);
    g1Qt->gemv(false, nQ_ref, nvirA * nvirA, bQvvA, G1c_vv, 1.0, 0.0);
    bQvvA.reset();

    //=========================
    // Separable Part
    //=========================
    // G_ij^Q
    G2c_oo = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|OO)", nQ_ref, noccA * noccA));
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = 2.0 * ( g1Qc->get(Q) + g1Qt->get(Q) );
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              G2c_oo->set(Q, ii, value);
         }
    }

    // G_ij^Q += J_Q G_ij
    G2c_oo->dirprd123(Jc, G1c_oo, oo_idxAA, 1.0, 1.0); 

    // G_ij^Q += - \sum_{m} b_im^Q G_mj - \sum_{m} b_jm^Q G_im 
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    G2c_oo->contract(false, false, nQ_ref * noccA, noccA, noccA, bQooA, G1c_oo, -1.0, 1.0);
    G2c_oo->contract233(false, false, noccA, noccA, G1c_oo, bQooA, -1.0, 1.0);
    bQooA.reset();
    G2c_oo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_oo->print();
    G2c_oo.reset();

    // G_ia^Q += - \sum_{e} b_ie^Q G_ea 
    G2c_ov = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|OV)", nQ_ref, noccA, nvirA));
    bQovA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OV)", nQ_ref, noccA * nvirA));
    bQovA->read(psio_, PSIF_DFOCC_INTS);
    G2c_ov->contract(false, false, nQ_ref * noccA, nvirA, nvirA, bQovA, G1c_vv, -1.0, 0.0);
    bQovA.reset();
    G2c_ov->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_ov->print();

    // Form G_vo^Q
    G2c_vo = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|VO)", nQ_ref, nvirA, noccA));
    G2c_vo->swap_3index_col(G2c_ov);
    G2c_ov.reset();
    G2c_vo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vo->print();
    G2c_vo.reset();

    // G_ab^Q = J_Q G_ab
    G2c_vv = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|VV)", nQ_ref, nvirA * nvirA));
    G2c_vv->dirprd123(Jc, G1c_vv, vv_idxAA, 1.0, 0.0); 
    G2c_vv->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vv->print();
    G2c_vv.reset();

}// end if (reference_ == "RESTRICTED")

else if (reference_ == "UNRESTRICTED") {
    // Build J_Q
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = 0.0;
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              value += bQooA->get(Q,ii);
         }
         Jc->set(Q, value);
    }
    bQooA.reset();

    // beta
    bQooB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|oo)", nQ_ref, noccB * noccB));
    bQooB->read(psio_, PSIF_DFOCC_INTS);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = 0.0;
         for (int i = 0; i < noccB; i++) {
              int ii = oo_idxBB->get(i,i);
              value += bQooB->get(Q,ii);
         }
         Jc->add(Q, value);
    }
    bQooA.reset();

    //=========================
    // Reference TPDM
    //=========================
    // G_IJ (ref) = 
    G2c_oo = SharedTensor2d(new Tensor2d("Reference 3-Index TPDM (Q|OO)", nQ_ref, noccA * noccA));
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    G2c_oo->copy(bQooA);
    bQooA.reset();
    G2c_oo->scale(-1.0);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = Jc->get(Q);
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              G2c_oo->add(Q, ii, value);
         }
    }
    G2c_oo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_oo->print();
    G2c_oo.reset();

    // G_ij (ref) = 
    G2c_oo = SharedTensor2d(new Tensor2d("Reference 3-Index TPDM (Q|oo)", nQ_ref, noccB * noccB));
    bQooB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|oo)", nQ_ref, noccB * noccB));
    bQooB->read(psio_, PSIF_DFOCC_INTS);
    G2c_oo->copy(bQooB);
    bQooB.reset();
    G2c_oo->scale(-1.0);
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = Jc->get(Q);
         for (int i = 0; i < noccB; i++) {
              int ii = oo_idxBB->get(i,i);
              G2c_oo->add(Q, ii, value);
         }
    }
    G2c_oo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_oo->print();
    G2c_oo.reset();

    //=========================
    // Reference TPDM Done.
    //=========================

    // G_Q = \sum_{m,n} b_mn^Q G1c_mn
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    g1Qc->gemv(false, nQ_ref, noccA * noccA, bQooA, G1c_ooA, 1.0, 0.0);
    bQooA.reset();

    // G_Q = \sum_{m,n} b_mn^Q G1c_mn
    bQooB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|oo)", nQ_ref, noccB * noccB));
    bQooB->read(psio_, PSIF_DFOCC_INTS);
    g1Qc->gemv(false, nQ_ref, noccB * noccB, bQooB, G1c_ooB, 1.0, 1.0);
    bQooB.reset();

    // Gt_Q = \sum_{e,f} b_ef^Q G1c_ef
    bQvvA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|VV)", nQ_ref, nvirA * nvirA));
    bQvvA->read(psio_, PSIF_DFOCC_INTS);
    g1Qt->gemv(false, nQ_ref, nvirA * nvirA, bQvvA, G1c_vvA, 1.0, 0.0);
    bQvvA.reset();

    // Gt_Q = \sum_{e,f} b_ef^Q G1c_ef
    bQvvB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|vv)", nQ_ref, nvirB * nvirB));
    bQvvB->read(psio_, PSIF_DFOCC_INTS);
    g1Qt->gemv(false, nQ_ref, nvirB * nvirB, bQvvB, G1c_vvB, 1.0, 1.0);
    bQvvB.reset();

    //=========================
    // Separable Part
    //=========================

    // G_IJ^Q
    G2c_ooA = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|OO)", nQ_ref, noccA * noccA));
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = g1Qc->get(Q) + g1Qt->get(Q) ;
         for (int i = 0; i < noccA; i++) {
              int ii = oo_idxAA->get(i,i);
              G2c_ooA->set(Q, ii, value);
         }
    }

    // G_IJ^Q += J_Q G_IJ
    G2c_ooA->dirprd123(Jc, G1c_ooA, oo_idxAA, 1.0, 1.0); 

    // G_IJ^Q += - \sum_{M} b_IM^Q G_MJ - \sum_{M} b_JM^Q G_IM
    bQooA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OO)", nQ_ref, noccA * noccA));
    bQooA->read(psio_, PSIF_DFOCC_INTS);
    G2c_ooA->contract(false, false, nQ_ref * noccA, noccA, noccA, bQooA, G1c_ooA, -1.0, 1.0);
    G2c_ooA->contract233(false, false, noccA, noccA, G1c_ooA, bQooA, -1.0, 1.0);
    bQooA.reset();
    G2c_ooA->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_ooA->print();
    G2c_ooA.reset();

    // G_ij^Q
    G2c_ooB = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|oo)", nQ_ref, noccB * noccB));
    for (int Q = 0; Q < nQ_ref; Q++) {
         double value = g1Qc->get(Q) + g1Qt->get(Q) ;
         for (int i = 0; i < noccB; i++) {
              int ii = oo_idxBB->get(i,i);
              G2c_ooB->set(Q, ii, value);
         }
    }

    // G_ij^Q += J_Q G_ij
    G2c_ooB->dirprd123(Jc, G1c_ooB, oo_idxBB, 1.0, 1.0); 

    // G_ij^Q += - \sum_{m} b_im^Q G_mj - \sum_{m} b_jm^Q G_im 
    bQooB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|oo)", nQ_ref, noccB * noccB));
    bQooB->read(psio_, PSIF_DFOCC_INTS);
    G2c_ooB->contract(false, false, nQ_ref * noccB, noccB, noccB, bQooB, G1c_ooB, -1.0, 1.0);
    G2c_ooB->contract233(false, false, noccB, noccB, G1c_ooB, bQooB, -1.0, 1.0);
    bQooB.reset();
    G2c_ooB->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_ooB->print();
    G2c_ooB.reset();

    // G_IA^Q += - \sum_{E} b_IE^Q G_EA
    G2c_ov = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|OV)", nQ_ref, noccA, nvirA));
    bQovA = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|OV)", nQ_ref, noccA * nvirA));
    bQovA->read(psio_, PSIF_DFOCC_INTS);
    G2c_ov->contract(false, false, nQ_ref * noccA, nvirA, nvirA, bQovA, G1c_vvA, -1.0, 0.0);
    bQovA.reset();
    G2c_ov->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_ov->print();

    // Form G_VO^Q
    G2c_vo = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|VO)", nQ_ref, nvirA, noccA));
    G2c_vo->swap_3index_col(G2c_ov);
    G2c_ov.reset();
    G2c_vo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vo->print();
    G2c_vo.reset();

    // G_ia^Q += - \sum_{e} b_ie^Q G_ea 
    G2c_ov = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|ov)", nQ_ref, noccB, nvirB));
    bQovB = SharedTensor2d(new Tensor2d("DF_BASIS_SCF B (Q|ov)", nQ_ref, noccB * nvirB));
    bQovB->read(psio_, PSIF_DFOCC_INTS);
    G2c_ov->contract(false, false, nQ_ref * noccB, nvirB, nvirB, bQovB, G1c_vvB, -1.0, 0.0);
    bQovB.reset();
    G2c_ov->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_ov->print();

    // Form G_vo^Q
    G2c_vo = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|vo)", nQ_ref, nvirB, noccB));
    G2c_vo->swap_3index_col(G2c_ov);
    G2c_ov.reset();
    G2c_vo->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vo->print();
    G2c_vo.reset();

    // G_AB^Q = J_Q G_ab
    G2c_vv = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|VV)", nQ_ref, nvirA * nvirA));
    G2c_vv->dirprd123(Jc, G1c_vvA, vv_idxAA, 1.0, 0.0); 
    G2c_vv->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vv->print();
    G2c_vv.reset();

    // G_ab^Q = J_Q G_ab
    G2c_vv = SharedTensor2d(new Tensor2d("3-Index Separable TPDM (Q|vv)", nQ_ref, nvirB * nvirB));
    G2c_vv->dirprd123(Jc, G1c_vvB, vv_idxBB, 1.0, 0.0); 
    G2c_vv->write(psio_, PSIF_DFOCC_DENS);
    if(print_ > 3) G2c_vv->print();
    G2c_vv.reset();

}// else if (reference_ == "UNRESTRICTED")
    timer_off("sep_tpdm");
} // end seprable_tpdm

}} // End Namespaces

