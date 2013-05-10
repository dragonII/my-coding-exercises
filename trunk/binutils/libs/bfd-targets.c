/* Generic target-file-type support for the BFD library */

#include "include/bfd_.h"


/* All knonw xvecs (even those that don't compile on all systems).
   Alphabetized for easy reference.
   They are listed a second time below, since
   we can't intermix extern's and initializers */

extern const bfd_target a_out_adobe_vec;

static const bfd_target * const _bfd_target_vector[] =
{
    /* This list is alphabetized to make it easy to compare
       with other vector lists -- the decls above and
       the case statement in configure.in.
       Vectors that don't compile on all systems, or aren't finished,
       should have an entry here with #if 0 around it, to show that
       it wasn't omitted by mistake */
    &a_out_adobe_vec,
    &aout0_big_vec,
    &aout_mips_little_vec,
    &arm_epoc_pe_big_vec,
    &arm_epoc_pe_little_vec,
    &arm_epoc_pei_big_vec,
    &arm_epoc_pei_little_vec,
    &armcoff_big_vec,
    &armcoff_little_vec,
    &armnetbsd_vec,
    &armpe_big_vec,
    &armpe_little_vec,
    &armpei_big_vec,
    &armpei_little_vec,
    &b_out_vec_big_host,
    &b_out_vec_little_host,
    &bfd_efi_app_ia32_vec,
    &bfd_elf32_avr_vec,
    &bfd_elf32_bfin_vec,
    &bfd_elf32_bfinfdpic_vec,
    /* This, and other vectors, may not be used in any *.mt configuration.
       But that does not mean they are unnecessary.  If configured with
       --enable-targets=all, objdump or gdb should be able to examine
       the file even if we don't recognize the machine type.  */
    &bfd_elf32_big_generic_vec,
    &bfd_elf32_bigarc_vec,
    &bfd_elf32_bigarm_vec,
    &bfd_elf32_bigarm_symbian_vec,
    &bfd_elf32_bigarm_vxworks_vec,
    &bfd_elf32_bigmips_vec,
    &bfd_elf32_bigmips_vxworks_vec,
    &bfd_elf32_cr16c_vec,
    &bfd_elf32_cris_vec,
    &bfd_elf32_crx_vec,
    &bfd_elf32_d10v_vec,
    &bfd_elf32_d30v_vec,
    &bfd_elf32_dlx_big_vec,
    &bfd_elf32_fr30_vec,
    &bfd_elf32_frv_vec,
    &bfd_elf32_frvfdpic_vec,
    &bfd_elf32_h8300_vec,
    &bfd_elf32_hppa_linux_vec,
    &bfd_elf32_hppa_nbsd_vec,
    &bfd_elf32_hppa_vec,
    &bfd_elf32_i370_vec,
    &bfd_elf32_i386_freebsd_vec,
    &bfd_elf32_i386_vxworks_vec,
    &bfd_elf32_i386_vec,
    &bfd_elf32_i860_little_vec,
    &bfd_elf32_i860_vec,
    &bfd_elf32_i960_vec,
    &bfd_elf32_ip2k_vec,
    &bfd_elf32_iq2000_vec,
    &bfd_elf32_little_generic_vec,
    &bfd_elf32_littlearc_vec,
    &bfd_elf32_littlearm_vec,
    &bfd_elf32_littlearm_symbian_vec,
    &bfd_elf32_littlearm_vxworks_vec,
    &bfd_elf32_littlemips_vec,
    &bfd_elf32_littlemips_vxworks_vec,
    &bfd_elf32_m32c_vec,
    &bfd_elf32_m32r_vec,
    &bfd_elf32_m32rle_vec,
    &bfd_elf32_m32rlin_vec,
    &bfd_elf32_m32rlelin_vec,
    &bfd_elf32_m68hc11_vec,
    &bfd_elf32_m68hc12_vec,
    &bfd_elf32_m68k_vec,
    &bfd_elf32_m88k_vec,
    &bfd_elf32_mcore_big_vec,
    &bfd_elf32_mcore_little_vec,
    &bfd_elf32_mn10200_vec,
    &bfd_elf32_mn10300_vec,
    &bfd_elf32_mt_vec,
    &bfd_elf32_msp430_vec,
    &bfd_elf32_openrisc_vec,
    &bfd_elf32_or32_big_vec,
    &bfd_elf32_pj_vec,
    &bfd_elf32_pjl_vec,
    &bfd_elf32_powerpc_vec,
    &bfd_elf32_powerpc_vxworks_vec,
    &bfd_elf32_powerpcle_vec,
    &bfd_elf32_s390_vec,
    &bfd_elf32_sh_vec,
    &bfd_elf32_shblin_vec,
    &bfd_elf32_shl_vec,
    &bfd_elf32_shl_symbian_vec,
    &bfd_elf32_shlin_vec,
    &bfd_elf32_shlnbsd_vec,
    &bfd_elf32_shnbsd_vec,
    &bfd_elf32_sparc_vec,
    &bfd_elf32_sparc_vxworks_vec,
    &bfd_elf32_tradbigmips_vec,
    &bfd_elf32_tradlittlemips_vec,
    &bfd_elf32_us_cris_vec,
    &bfd_elf32_v850_vec,
    &bfd_elf32_vax_vec,
    &bfd_elf32_xc16x_vec,
    &bfd_elf32_xstormy16_vec,
    &bfd_elf32_xtensa_be_vec,
    &bfd_elf32_xtensa_le_vec,
    &bfd_powerpc_pe_vec,
    &bfd_powerpc_pei_vec,
    &bfd_powerpcle_pe_vec,
    &bfd_powerpcle_pei_vec,
    &cris_aout_vec,
    &ecoff_big_vec,
    &ecoff_biglittle_vec,
    &ecoff_little_vec,
    &go32coff_vec,
    &go32stubbedcoff_vec,
    &h8300coff_vec,
    &h8500coff_vec,
    &hp300hpux_vec,
    &i386aout_vec,
    &i386bsd_vec,
    &i386coff_vec,
    &i386freebsd_vec,
    &i386lynx_aout_vec,
    &i386lynx_coff_vec,
    &i386msdos_vec,
    &i386netbsd_vec,
    &i386os9k_vec,
    &i386pe_vec,
    &i386pei_vec,
    &i860coff_vec,
    &icoff_big_vec,
    &icoff_little_vec,
    &ieee_vec,
    &m68kcoff_vec,
    &m68kcoffun_vec,
    &m68knetbsd_vec,
    &m68ksysvcoff_vec,
    &m88kbcs_vec,
    &m88kmach3_vec,
    &m88kopenbsd_vec,
    &mach_o_be_vec,
    &mach_o_le_vec,
    &mach_o_fat_vec,
    &maxqcoff_vec,
    &mcore_pe_big_vec,
    &mcore_pe_little_vec,
    &mcore_pei_big_vec,
    &mcore_pei_little_vec,
    &mipslpe_vec,
    &mipslpei_vec,
    &newsos3_vec,
    &nlm32_i386_vec,
    &nlm32_powerpc_vec,
    &nlm32_sparc_vec,
    /* Entry for the OpenRISC family.  */
    &or32coff_big_vec,
    &pc532machaout_vec,
    &pc532netbsd_vec,
    &pdp11_aout_vec,
    &pef_vec,
    &pef_xlib_vec,
    &ppcboot_vec,
    &rs6000coff_vec,
    &shcoff_small_vec,
    &shcoff_vec,
    &shlcoff_small_vec,
    &shlcoff_vec,
    &shlpe_vec,
    &shlpei_vec,
#if defined (HOST_HPPAHPUX) || defined (HOST_HPPABSD) || defined (HOST_HPPAOSF)
    &som_vec,
#endif
    &sparccoff_vec,
    &sparcle_aout_vec,
    &sparclinux_vec,
    &sparclynx_aout_vec,
    &sparclynx_coff_vec,
    &sparcnetbsd_vec,
    &sunos_big_vec,
    &sym_vec,
    &tic30_aout_vec,
    &tic30_coff_vec,
    &tic54x_coff0_beh_vec,
    &tic54x_coff0_vec,
    &tic54x_coff1_beh_vec,
    &tic54x_coff1_vec,
    &tic54x_coff2_beh_vec,
    &tic54x_coff2_vec,
    &tic80coff_vec,
    &vaxbsd_vec,
    &vaxnetbsd_vec,
    &vax1knetbsd_vec,
    &versados_vec,
    &vms_vax_vec,
    &w65_vec,
    &we32kcoff_vec,
    &z80coff_vec,
    &z8kcoff_vec,
    &bfd_elf32_am33lin_vec,
    /* Always support S-records, for convenience.  */
    &srec_vec,
    &symbolsrec_vec,
    /* And tekhex */
    &tekhex_vec,
    /* Likewise for binary output.  */
    &binary_vec,
    /* Likewise for ihex.  */
    &ihex_vec,
    
    /* Add any required traditional-core-file-handler.  */
    
#ifdef AIX386_CORE
    &aix386_core_vec,
#endif
#if 0
    /* We don't include cisco_core_*_vec.  Although it has a magic number,
       the magic number isn't at the beginning of the file, and thus
       might spuriously match other kinds of files.  */
    &cisco_core_big_vec,
    &cisco_core_little_vec,
#endif
#ifdef HPPABSD_CORE
    &hppabsd_core_vec,
#endif
#ifdef HPUX_CORE
    &hpux_core_vec,
#endif
#ifdef IRIX_CORE
    &irix_core_vec,
#endif
#ifdef NETBSD_CORE
    &netbsd_core_vec,
#endif
#ifdef OSF_CORE
    &osf_core_vec,
#endif
#ifdef PTRACE_CORE
    &ptrace_core_vec,
#endif
#ifdef SCO5_CORE
    &sco5_core_vec,
#endif
#ifdef TRAD_CORE
    &trad_core_vec,
#endif
    
    NULL /* end of list marker */
};



const bfd_target * const * bfd_target_vector = _bfd_target_vector;


/* Return a freshly malloced NULL-terminated
   vector of the names of all the valid BFD targets.
 */
const char **bfd_target_list(void)
{
    int vec_length = 0;
    bfd_size_type amt;

    const bfd_target * const *target;
    const char **name_list, **name_ptr;

    for(target = &bfd_target_vector[0]; *target != NULL; target++)
        vec_length++;

    amt = (vec_length + 1) * sizeof (char **);
    name_ptr = name_list = bfd_malloc(amt);

    if(name_list == NULL)
        return NULL;

    for(target = &bfd_target_vector[0]; *target != NULL; target++)
        if(target == &bfd_target_vector[0]
            || *target != bfd_target_vector[0])
            *name_ptr++ = (*target)->name;

    *name_ptr = NULL;
    return name_list;
}
