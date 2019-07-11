
# This file needs to be sourced by bash (e.g., from .bashrc), or it contents
# should be copied for use by bash as appropriate. Copying runs the risk of
# missing out on updates. 

# This script assumes that KJB_SRC_PATH points to a directory which has Make as a
# subirectory that has the init_compile script in it, and also scripts as a
# sub-sub directory. (The standard layout in SVN of src/Make). Since the default
# location of this script is ${KJB_SRC_PATH}/Make/scripts, it seems safe to assume
# that KJB_SRC_PATH can be set as it is implicit in calling this script anyway. 

# These should be updated in sync with the tcsh versions in ~kobus/.cshrc2. They are created
# from the tcsh versions by simple replacements:
#   s# '#='# 
#   s#\$MACROS/init_compile_full#$KJB_SRC_PATH/Make/scripts/init_compile.sh#
#   s#setenv  *\([^ ;'][^ ;']*\) #export \1=#g
#   s#unsetenv  *\([^ ;'][^ ;']*\) *;#export \1=;#g
#
alias set_default='export KJB_CC=; export KJB_CXX=; export KJB_F77=; export KJB_LD=; export PRODUCTION=; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_gnu='if [[ -z "${FORCE_GCC_VERSION}" ]]; then suffix=""; else suffix="-${FORCE_GCC_VERSION}"; fi; export KJB_CC=gcc${suffix}; export KJB_CXX=g++${suffix}; export KJB_F77=gfortran${suffix}; export KJB_LD=${KJB_CC}${suffix}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_gcc='set_gnu'
alias set_g++='set_gnu'

alias set_clang='export KJB_CC=clang; export KJB_CXX=clang++; export KJB_F77=clang; export KJB_LD=${KJB_CC}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_clang++='export KJB_CC=clang++; export KJB_CXX=clang++; export KJB_F77=clang; export KJB_LD=${KJB_CC}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_icc='export KJB_CC=icc; export KJB_CXX=icpc; export KJB_F77=ifort; export KJB_LD=${KJB_CC}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

# Probably obsolete.
# alias set_cc='export KJB_CC=cc; export KJB_CXX=CC; export KJB_LD=${KJB_CC}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
# alias set_CC='export KJB_CC=CC; export KJB_CXX=CC; export KJB_LD=R{KJB_CC}; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_dev='export PRODUCTION=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_prod='export PRODUCTION=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_no_libs='export NO_LIBS=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_libs='export NO_LIBS=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_no_boost='export NO_BOOST=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_boost='export NO_BOOST=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

# Asks for preferred CXX compiler on this system. 
alias set_cxx='export KJB_PREFER_CXX=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
# Asks for preferred C compiler on this system. 
alias set_c='export KJB_PREFER_CXX=0; if ($?KJB_CC != 0) unexport KJB_CC;=source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_pch='export PREFER_PCH=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh' 
alias set_no_pch='unexport PREFER_PCH;=source $KJB_SRC_PATH/Make/scripts/init_compile.sh' 

# alias set_after='export FORCE_NO_AFTER=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
# alias set_no_after='export FORCE_NO_AFTER=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_no_opt='export FORCE_OPTIMIZE=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_opt='export FORCE_OPTIMIZE=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_no_test='export FORCE_TEST=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_test='export FORCE_TEST=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_debug='export FORCE_DEBUG=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_no_debug='export FORCE_DEBUG=0; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_tma='export FORCE_TRACK_MEMORY_ALLOCATION=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'

alias set_mesa='export FORCE_OSMESA=1; export USE_OFFSCREEN_RENDERING=1; source $KJB_SRC_PATH/Make/scripts/init_compile.sh'
alias set_no_mesa='unexport FORCE_OSMESA;=unexport USE_OFFSCREEN_RENDERING;=source $KJB_SRC_PATH/Make/scripts/init_compile.sh'


