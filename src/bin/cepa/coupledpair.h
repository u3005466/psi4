#ifndef CEPA_H
#define CEPA_H

#include<libmints/wavefunction.h>

namespace psi{ namespace cepa{

class CoupledPair:public Wavefunction
{
public:

    CoupledPair(boost::shared_ptr<Wavefunction> reference_wavefunction, Options &options);
    ~CoupledPair();

    double compute_energy();
    virtual bool same_a_b_orbs() const { return true; }
    virtual bool same_a_b_dens() const { return true; }

protected:

    void common_init();
    void finalize();

    /// is t2 stored on disk or in core?  default false
    bool t2_on_disk;

    /**
      * Define CEPA Tasks.  most diagrams are designated as
      * independent tasks.  some will be tiled out as 
      * separate tasks either so we can do them with limited
      * memory or for distribution among many processors 
     */
    void DefineTasks();
    long int ncepatasks;
    /**
      * CEPA Tasks parameters.  for terms that are tiled, the
      * let the task know which tile it should be working on.
      * NOTE: the tiling is really meant for the parallel
      * code, not this one.
     */
    struct CepaTaskParams{
        int mtile,ntile,ktile;
    };
    CepaTaskParams*CepaParams,*CCSubParams1,*CCSubParams2;
    /**
      * CEPA Tasks. this struct contains a pointer to
      * the task function, and some other stupid info
      * like how many flops the task will take.
     */
    struct CepaTask{
        void(psi::cepa::CoupledPair::*func)(CepaTaskParams);
        double flopcount;
    };
    CepaTask*CepaTasklist,*CCSubTasklist1,*CCSubTasklist2;

    /**
      * this function solves the CEPA equations (requires a minimum of 3o^2v^2 memory)
     */
    PsiReturnType CEPAIterations();
  
    void WriteBanner();

    /**
      * allocate memory, define tiling of gigantic diragrams
     */
    void AllocateMemory(long int extramemory);

    /**
      * some CC diagrams.  these were "easy" ones that i used to
      * do on the cpu while the gpu worked.  maybe we'll do that
      * again eventually.
      */
    void CPU_t1_vmeai(CepaTaskParams params);
    void CPU_t1_vmeni(CepaTaskParams params);
    void CPU_t1_vmaef(CepaTaskParams params);

    /**
      * this diagram required ov^3 storage for an
      * intermediate in the Piecuch CPC.  this formulation
      * only requires o^3v storage...at the expense of
      * 4 extra O(N^5) terms.
      */
    void CPU_I2p_abci_refactored_term1(CepaTaskParams params);

    /// update t1
    void UpdateT1(long int iter);

    /// update t2
    void UpdateT2(long int iter);

    /// compute the current energy
    double CheckEnergy();

    /// which cepa level
    int cepa_level;
    /// string version of cepa_level
    char*cepa_type;

    /// construct an array of pair energies
    void PairEnergy();
    double*pair_energy;

    /**
      * the N^6 CEPA diagrams.
      */
    void I2ijkl(CepaTaskParams params);
    void I2piajk(CepaTaskParams params);
    void Vabcd_so(CepaTaskParams params);
    void Vabcd1(CepaTaskParams params);
    void Vabcd2(CepaTaskParams params);
    void I2iabj(CepaTaskParams params);
    void I2iajb(CepaTaskParams params);

    /**
      * DIIS stuff
      */
    void DIIS(double*c,long int nvec,long int n);
    void DIISOldVector(long int iter,int diis_iter,int replace_diis_iter);
    double DIISErrorVector(int diis_iter,int replace_diis_iter,int iter);
    void DIISNewAmplitudes(int diis_iter);
    long int maxdiis;
    double*diisvec;

    /**
      * basic parameters
      */
    long int ndoccact,ndocc,nvirt,nso,nmotemp,nmo,memory;
    int maxiter,nfzc,nfzv;
    double conv,*oei,*tei,*Fock,*eps;
    double escf,enuc,efzc,emp2,ecepa,et;

    /**
      * workspace buffers.
      */
    double*integrals,*tempt,*tempv;

    /**
      * t1 and t2
      */
    double*tb,*t1;

    /**
      * the singles residual and a couple of tiny intermediates
      */
    double *w1,*I1,*I1p;

    /**
      * define tiling.
      */
    void DefineTilingCPU(long int extra);
    long int ovtilesize,lastovtile,lastov2tile,ov2tilesize;
    long int tilesize,lasttile,maxelem;
    long int ntiles,novtiles,nov2tiles;

    /**
      *  SCS-MP2 function and variables
      */
    void SCS_MP2();
    double emp2_os,emp2_ss,emp2_os_fac,emp2_ss_fac;

    /**
      *  SCS-CEPA function and variables
      */
    void SCS_CEPA();
    double ecepa_os,ecepa_ss,ecepa_os_fac,ecepa_ss_fac;

    /**
      *  the CIM version of SCS_CEPA()
      */
    void Local_SCS_CEPA();

    /// build the opdm for 1-electron properties
    void OPDM();
};

}}

#endif