#!/source_this_file

###############################################################################
#
# Identification:
#    Provides the KJB build environment variables to bash users
#
# Description:
#    This script provides the KJB build environment variables to bash users. It
#    is used by the bash versions of the build aliases (e.g., set_prod). It also
#    adjust your PATH variable as the build changes.  Specifically, it also
#    pre-pends the LD_OBJ_DIR to PATH. In this way, it is like the tcsh script
#    init_compile_full. 
#
#    This script must be sourced in bash (or its contents copied for use by
#    bash). 
#
#    This script assumes that KJB_SRC_PATH points to a directory which has Make
#    as a subirectory that has the init_compile script in it, and also scripts
#    as a sub-sub directory. (The standard layout in SVN of src/Make). Since the
#    default location of this script is ${KJB_SRC_PATH}/Make/scripts, it seems
#    safe to assume that KJB_SRC_PATH can be set as it is implicit in calling
#    this script anyway. 
#
###############################################################################

export TMPDIR=/tmp

# Make tcsh source init_compile. This might not be the most readible way, but it
# works. 
echo source ${KJB_SRC_PATH}/Make/init_compile | tcsh -s 

if [[ $? == 0 ]] 
then
    source ${TMPDIR}/${USER}/kjb_env.sh 
    if [[ $? != 0 ]] 
    then
        echo "Sourcing ${TMPDIR}/${USER}/kjb_env.sh failed."
        return 1
    fi 
else 
    echo "Sourcing  ${KJB_SRC_PATH}/Make/init_compile within tcsh failed."
    return 1
fi 

# TODO. Prepend LD_OBJ_DIR to path. 

# echo PATH: $PATH
strip_path=`echo $PATH | sed "s#^\./${MACHINE}//*[^/][^/]*//*[^/][^/]*/*:##"`
# echo strip_path: $strip_path

export PATH="./${LD_OBJ_DIR}:$PATH"
# echo PATH: $PATH



