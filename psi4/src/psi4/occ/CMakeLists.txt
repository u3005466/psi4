list(APPEND sources
  arrays.cc
  cc_energy.cc
  ccl_energy.cc
  cepa_iterations.cc
  coord_grad.cc
  corr_tpdm.cc
  diis.cc
  dpd.cc
  ekt_ea.cc
  ekt_ip.cc
  ep2_ip.cc
  fock_alpha.cc
  fock_beta.cc
  get_moinfo.cc
  gfock.cc
  gfock_diag.cc
  gfock_ea.cc
  idp.cc
  idp2.cc
  kappa_msd.cc
  kappa_orb_resp.cc
  kappa_orb_resp_iter.cc
  main.cc
  manager.cc
  mograd.cc
  occ_iterations.cc
  occwave.cc
  ocepa_g_int.cc
  ocepa_response_pdms.cc
  ocepa_t2_1st_sc.cc
  omp2_ip_poles.cc
  omp2_response_pdms.cc
  omp2_t2_1st.cc
  omp3_g_int.cc
  omp3_ip_poles.cc
  omp3_response_pdms.cc
  omp3_t2_1st_general.cc
  omp3_t2_1st_sc.cc
  semi_canonic.cc
  t1_1st.cc
  t2_2nd_general.cc
  t2_2nd_sc.cc
  t2_amps.cc
  tei_sort_iabc.cc
  tpdm_ref_corr_opdm.cc
  trans_ints_rhf.cc
  trans_ints_rmp2.cc
  trans_ints_uhf.cc
  trans_ints_ump2.cc
  update_mo.cc
  v_2nd_order.cc
  v_int.cc
  w_1st_order.cc
  w_int.cc
  z_vector.cc
  )
psi4_add_module(bin occ sources)
