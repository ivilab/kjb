:""
:set viminfo=
:set ul=0
:set ttyfast
:""
:""
:1s/.*/\/\*        ---     Machine generated file --- DO NOT EDIT ---   \*\/&/e
:""
:%s/\<double\>/float/ge
:%s/\<0\.0\>/FLT_ZERO/ge
:%s/\<1\.0\>/FLT_ONE/ge
:%s/DBL/FLT/ge
:%s/double_array/float_array/ge
:""
:""
:%s/\<M_VEC\(.*\)_INCLUDED\>/M_FLT_VEC\1/ge
:""
:%s/\<Vector\>/Flt_vector/ge
:%s/\<Vector_vector\>/Flt_vector_vector/ge
:%s/\<V_v_v\>/Flt_v_v_v/ge
:%s/\<V_v_v_v\>/Flt_v_v_v_v/ge
:%s/\<Indexed_vector_element\>/Flt_indexed_vector_element/ge
:%s/\<Indexed_vector\>/Flt_indexed_vector/ge
:""
:%s/vp_array/flt_vp_array/ge
:""
:%s/ vector\./ float vector./ge
:%s/ vector / float vector /ge
:%s/ vector$/ float vector/ge
:""
:%s/ matrices\./ float matrices./ge
:%s/ matrices / float matrices /ge
:%s/ matrices$/ float matrices/ge
:""
:%s/\<Double\>/Float/ge
:""
:%s/\<\([0-9a-z_]*\)_vector_\([0-9a-z_]*\)\>/\1_flt_vector_\2/ge
:%s/\<vector_\([0-9a-z_]*\)\>/flt_vector_\1/ge
:%s/\<\([0-9a-z_]*\)_vector\>/\1_flt_vector/ge
:""
:%s/\<\([0-9a-z_]*\)_1D_\([0-9a-z_]*\)_vector\(s*\)\>/\1_1D_\2_flt_vector\3/ge
:%s/\<\([0-9a-z_]*\)_2D_\([0-9a-z_]*\)_vector\(s*\)\>/\1_2D_\2_flt_vector\3/ge
:""
:%s/\<\([0-9a-z_]*\)_v3\>/\1_flt_v3/ge
:%s/\<\([0-9a-z_]*\)_v4\>/\1_flt_v4/ge
:""
:%s/\<DEBUG_\([0-9a-z_]*\)_vector_\([0-9a-z_]*\)\>/DEBUG_\1_flt_vector_\2/ge
:%s/\<DEBUG_vector_\([0-9a-z_]*\)\>/DEBUG_flt_vector_\1/ge
:%s/\<DEBUG_\([0-9a-z_]*\)_vector\>/DEBUG_\1_flt_vector/ge
:""
:" Special case where identify has two occurances of vector."
:" The second one will have been changed already."
:%s/\<\([0-9a-z_]*\)_vector\([0-9a-z_]*\)_flt_vector_\([0-9a-z_]*\)\>/\1_flt_vector\2_flt_vector_\3/ge
:%s/\<vector\([0-9a-z_]*\)_flt_vector_\([0-9a-z_]*\)\>/flt_vector\1_flt_vector_\2/ge
:%s/\<\([0-9a-z_]*\)_vector\([0-9a-z_]*\)_flt_vector\>/\1_flt_vector\2_flt_vector/ge
:""
:%s/\<DEBUG_\([0-9a-z_]*\)_vector\([0-9a-z_]*\)_flt_vector_\([0-9a-z_]*\)\>/DEBUG_\1_flt_vector\2_flt_vector_\3/ge
:%s/\<DEBUG_vector\([0-9a-z_]*\)_flt_vector_\([0-9a-z_]*\)\>/DEBUG_flt_vector\1_flt_vector_\2/ge
:%s/\<DEBUG_\([0-9a-z_]*\)_vector\([0-9a-z_]*\)_flt_vector\>/DEBUG_\1_flt_vector\2_flt_vector/ge
:""
:""
:""
:""
:%s/\<M_MAT\(.*\)_INCLUDED\>/M_FLT_MAT\1/ge
:""
:%s/\<Matrix\>/Flt_matrix/ge
:%s/\<Matrix_vector\>/Flt_matrix_vector/ge
:%s/\<Matrix_vector_vector\>/Flt_matrix_vector_vector/ge
:""
:%s/mp_array/flt_mp_array/ge
:""
:%s/ matrix\./ float matrix./ge
:%s/ matrix / float matrix /ge
:%s/ matrix$/ float matrix/ge
:""
:%s/ matrices\./ float matrices./ge
:%s/ matrices / float matrices /ge
:%s/ matrices$/ float matrices/ge
:""
:%s/\<Double\>/Float/ge
:""
:%s/\<\([0-9a-z_]*\)_matrices_\([0-9a-z_]*\)\>/\1_flt_matrices_\2/ge
:%s/\<\([0-9a-z_]*\)_matrix_\([0-9a-z_]*\)\>/\1_flt_matrix_\2/ge
:%s/\<matrix_\([0-9a-z_]*\)\>/flt_matrix_\1/ge
:%s/\<\([0-9a-z_]*\)_matrix\>/\1_flt_matrix/ge
:""
:""
:%s/\<DEBUG_\([0-9a-z_]*\)_matrix_\([0-9a-z_]*\)\>/DEBUG_\1_flt_matrix_\2/ge
:%s/\<DEBUG_matrix_\([0-9a-z_]*\)\>/DEBUG_flt_matrix_\1/ge
:%s/\<DEBUG_\([0-9a-z_]*\)_matrix\>/DEBUG_\1_flt_matrix/ge
:""
:" Special case where identify has two occurances of matrix."
:" The second one will have been changed already."
:%s/\<\([0-9a-z_]*\)_matrix\([0-9a-z_]*\)_flt_matrix_\([0-9a-z_]*\)\>/\1_flt_matrix\2_flt_matrix_\3/ge
:%s/\<matrix\([0-9a-z_]*\)_flt_matrix_\([0-9a-z_]*\)\>/flt_matrix\1_flt_matrix_\2/ge
:%s/\<\([0-9a-z_]*\)_matrix\([0-9a-z_]*\)_flt_matrix\>/\1_flt_matrix\2_flt_matrix/ge
:""
:%s/\<DEBUG_\([0-9a-z_]*\)_matrix\([0-9a-z_]*\)_flt_matrix_\([0-9a-z_]*\)\>/DEBUG_\1_flt_matrix\2_flt_matrix_\3/ge
:%s/\<DEBUG_matrix\([0-9a-z_]*\)_flt_matrix_\([0-9a-z_]*\)\>/DEBUG_flt_matrix\1_flt_matrix_\2/ge
:%s/\<DEBUG_\([0-9a-z_]*\)_matrix\([0-9a-z_]*\)_flt_matrix\>/DEBUG_\1_flt_matrix\2_flt_matrix/ge
:""
:""
:wq

