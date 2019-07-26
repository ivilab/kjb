/* =============================================================================
 *                              g_camera_matrix
 *
 * Provides functions for generating camera matrices given known values.
 *
 * Summary:
 *      get_camera_matrix_from_line_correspondences - Given a set of six line
 * correspondences, (arranged as two matrices), usees a simple psuedo-matrix
 * strategy to find the camera matrix that would produce the correspondences.
 *
 *      get_camera_matrix_from_point_and_line_correspondences - a flexible
 * function that will find a camera matrix given any mix of point and line
 * correspondences. Takes all correspondences as arrays of Vectors, with an
 * additional value denoting the number of correspondences. 3d lines are given
 * as pairs of points on the line. This function will ahve some degree of error,
 * but can take any number of total correspondences provided there are at least
 * 6 total.
 *
 * Remaining function in this file are helpers to the above.
 *
 * Author: Andrew Emmott (aemmott)
 *
 * -------------------------------------------------------------------------- */

#include <g/g_camera_matrix.h>
#include <g/g_plucker.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =============================================================================
 *                                 get_camera_matrix_from_point_and_line_correspondences
 *
 * Given at least six line or point correspondences, will approximate the camera matrix
 * among them using a least-squares strategy.
 *
 * Sets the given matrix to contain an approximated camera matrix given enough correspondences.
 * Takes the points correspondences as arrays of Vectors, With the 2d points being in 3-vector
 * form and the 3d points being 4-vectors. The lines are given in three arrays, with the 
 * 2d lines being 3-vectors and hte 3d lines being two seperate points in 3d, (4-vectors).
 * The number of point and line correspondences needs to be given. The function requires
 * at least 6 corespondences total, but has no maximum.
 *
 * Returns:
 *    ERROR if Vectors are the wrong size. NO_ERROR otherwise. Can still fail if too correspondences
 *    are linearly dependent.
 *
 * See also:
 *    get_camera_matrix_from_line_correspondences
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: camera, correspondences
 * -------------------------------------------------------------------------- */
int get_camera_matrix_from_point_and_line_correspondences
(
    Matrix** camera,        /*Target location for camera-matrix*/
    const Vector_vector* points2d,      /*2d points as an array of 3-vectors*/
    const Vector_vector* points3d,      /*3d points as an array of 4-vectors*/
    int numPoints,          /*The number of point correspondences*/
    const Vector_vector* lines2d,       /*2d lines as an array of 3-vectors*/
    const Vector_vector* lines3da,      /*3d points as an array of 4 vectors*/
    const Vector_vector* lines3db,      /*3d points as an array of 4-vectors*/
    int numLines            /*The number of line correspondences*/
)
{
    int i,j,k;
    Matrix* fill=NULL; /*Will be filled with 'linear equations'*/
    Matrix* fillT=NULL; /*Will be the transpose*/
    Matrix* prod=NULL; /*The psuedo-inverse of the above*/
    Matrix* leastSquares=NULL; /*The solution to the psuedo-inverse*/
    Vector* diagonals=NULL; /*Not actually used*/
    Vector* v2out=0; /*homogenized placeholder*/
    Vector* v3aout=0;/*homogenized placeholder*/
    Vector* v3bout=0;/*homogenized placeholder*/

    /*Standard error*/
    if (numPoints + numLines < 6) {
        add_error("get_camera_matrix_from_point_and_line_correspondences passed fewer than 6 correspondences.");
        return ERROR;
    }

    /*Set the size of the fill matrix*/
    get_target_matrix(&fill, 2*(numPoints+numLines),12);
    for(i = 0; i < numPoints; i++) {
        homogenize_vector(&v2out,points2d->elements[i]);
        homogenize_vector(&v3aout,points3d->elements[i]);
        put_point_corrs(fill, i, v2out, v3aout);/*And fill it with two equations*/
    }
    for(i = 0; i < numLines; i++) {
        normalize_vector(&v2out,lines2d->elements[i],NORMALIZE_BY_MAGNITUDE);
        homogenize_vector(&v3aout,lines3da->elements[i]);
        homogenize_vector(&v3bout,lines3db->elements[i]);
        put_line_corrs(fill, numPoints+i, v2out, v3aout, v3bout);/*ditto for lines*/
    }

    get_matrix_transpose(&fillT,fill);/*Transpose ...*/

    multiply_matrices(&prod,fillT,fill);/*Psuedo-invert ...*/

    ERE(diagonalize(prod, &leastSquares, &diagonals));/*Solve ...*/

    ERE(get_target_matrix(camera, 3,4));
    k=0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 4; j++) {
            (*camera)->elements[i][j] = leastSquares->elements[k][11];/*The last column has the values we want.*/
            k++;
        }
    }
    
    /*Clean-up*/
    free_matrix(fill);
    free_matrix(fillT);
    free_matrix(prod);
    free_matrix(leastSquares);
    free_vector(diagonals);
    free_vector(v2out);
    free_vector(v3aout);
    free_vector(v3bout);
    return NO_ERROR;
}

/* =============================================================================
 *                                 put_point_corrs
 *
 * A helper to get_camera_matrix_from_point_and_line_correspondences()
 *
 * Given a point correspondence, this makes two equations and puts them in the
 * target matrix at the specified index.
 *
 * Returns:
 *    NO_ERROR
 *
 * See also:
 *    get_camera_matrix_from_point_and_line_correspondences
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: get_camera_matrix_from_point_and_line_correspondences
 * -------------------------------------------------------------------------- */
int put_point_corrs
(
    Matrix* fill,       /*The target matrix*/
    int rowOffset,      /*The starting row index*/
    const Vector* point2d,    /*A 2d point*/
    const Vector* point3d     /*A 3d point*/
)
{
    int u,u1,u2,u3,u4;
    int v,v1,v2,v3,v4;
    int off1,off2;
    int i;

    /*Generate 2 12-variable 'equations' for the fill matrix*/
    off1=2*rowOffset;
    off2=off1+1;
    u=point2d->elements[0]/point2d->elements[2];
    v=point2d->elements[1]/point2d->elements[2];
    u1=u*point3d->elements[0];
    u2=u*point3d->elements[1];
    u3=u*point3d->elements[2];
    u4=u*point3d->elements[3];
    v1=v*point3d->elements[0];
    v2=v*point3d->elements[1];
    v3=v*point3d->elements[2];
    v4=v*point3d->elements[3];
    for (i = 0; i < 4; i++) {
        fill->elements[off1][i]=-point3d->elements[i];
        fill->elements[off1][i+4]=0;
        fill->elements[off2][i]=0;
        fill->elements[off2][i+4]=-point3d->elements[i];
    }
    fill->elements[off1][8]=u1;
    fill->elements[off2][8]=v1;
    fill->elements[off1][9]=u2;
    fill->elements[off2][9]=v2;
    fill->elements[off1][10]=u3;
    fill->elements[off2][10]=v3;
    fill->elements[off1][11]=u4;
    fill->elements[off2][11]=v4;

    return NO_ERROR;
}

/* =============================================================================
 *                                 put_line_corrs
 *
 * A helper to get_camera_matrix_from_point_and_line_correspondences()
 *
 * Given a line correspondence, this makes two equations and puts them in the
 * target matrix at the specified index.
 *
 * Returns:
 *    NO_ERROR
 *
 * See also:
 *    get_camera_matrix_from_point_and_line_correspondences
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: get_camera_matrix_from_point_and_line_correspondences
 * -------------------------------------------------------------------------- */
int put_line_corrs
(
    Matrix* fill,       /*The target matrix*/
    int rowOffset,      /*the starting row index*/
    const Vector* line2d,     /*A 2d line*/
    const Vector* line3da,    /*A 3d point*/
    const Vector* line3db     /*Another 3d point*/
)
{
    int off1,off2;
    int i;

    /*Generate 2 12-variable 'equations' for the fill matrix*/
    off1 = 2*rowOffset;
    off2 = off1+1;
    for (i = 0; i < 12; i++) {
        fill->elements[off1][i]=line2d->elements[i/4]*line3da->elements[i%4];
        fill->elements[off2][i]=line2d->elements[i/4]*line3db->elements[i%4];
    }
    return NO_ERROR;
}

/* =============================================================================
 *                                 ransac_calibrate_camera_from_corrs
 *
 * Using ransac, tries to guess the correct correspondences from the unordered
 * correspondences.
 *
 * Given some number of point and line correspondences, (unordered), this function
 * attempts to guess which correspondences belong to each other by randomly
 * pairing them and checking for error.
 *
 * Returns:
 *    NO_ERROR
 *
 * See also:
 *    get_camera_matrix_from_point_and_line_correspondences
 *
 * Author:
 *    Andrew Emmott (aemmott)
 *
 * Documenter:
 *    Andrew Emmott (aemmott)
 *
 * Index: ransac, camera calibration, correspondences
 * -------------------------------------------------------------------------- */
int ransac_calibrate_camera_from_corrs
(
    Matrix** camera, /*Location of the best resulting camera.*/
    double* error, /*Will store the error of that camera.*/
    const Vector_vector* points2din, /*The 2D Points*/
    const Vector_vector* points3din, /*Corresponding 3D Points*/
    int numPoints, /*The number of them to use.*/
    const Vector_vector* lines2din, /*2D Lines*/
    const Vector_vector* lines3dain, /*Corresponding 3D Lines as points.*/
    const Vector_vector* lines3dbin, /**/
    int numLines, /*The number of them to use.*/
    int iterations, /*The number of random guesses to try.*/
    double tolerance
)
{
    /*The homogenized input Vectors*/
    Vector_vector* points2d=0;
    Vector_vector* points3d=0;
    Vector_vector* lines2d=0;
    Vector_vector* lines3da=0;
    Vector_vector* lines3db=0;

    /*Some loop variables.*/
    int i,j;
    int iter;
    /*
    int bestIter;
    */

    /*Outbound vectors sent to get_camera_from... */
    int corrs_out=6;
    Vector_vector* p2Dout=0;
    Vector_vector* p3Dout=0;
    int nPointsOut;
    Vector_vector* l2Dout=0;
    Vector_vector* l3DAout=0;
    Vector_vector* l3DBout=0;
    int nLinesOut;

    /*Used to error check lines.*/
    Matrix* line_proj=0;
    Vector_vector* pluckers=0;
    Vector_vector* l_projs=0;
    Vector_vector* p_projs=0;

    /*Used to contain the randomly generated guess corespondences.*/
    Int_vector* rand2dSeq=0;
    Int_vector* rand3dPointSeq=0;
    Int_vector* rand3dLineSeq=0;
    /*
    Int_vector* bestSeq2d=0;
    Int_vector* bestSeq3d=0;
    */
    int toExclude;

    /*Variables to track the current and best validation efforts.*/
    Matrix* nextCamera=0;
    double err=0, curErr=0;
    double val=0, curVal=0;

    /*double d, denominator;*/


    /*Initialize our local lists.*/
    ERE(get_target_vector_vector(&p2Dout,numPoints));
    ERE(get_target_vector_vector(&p3Dout,numPoints));
    ERE(get_target_vector_vector(&l2Dout,numLines));
    ERE(get_target_vector_vector(&l3DAout,numLines));
    ERE(get_target_vector_vector(&l3DBout,numLines));

    ERE(get_target_vector_vector(&points2d,numPoints));
    ERE(get_target_vector_vector(&points3d,numPoints));
    ERE(get_target_vector_vector(&lines2d,numLines));
    ERE(get_target_vector_vector(&lines3da,numLines));
    ERE(get_target_vector_vector(&lines3db,numLines));

    /*Homogenize the input vectors.*/
    for (i=0; i<numPoints; i++)
    {
        ERE(homogenize_vector(&points2d->elements[i],points2din->elements[i]));
        ERE(homogenize_vector(&points3d->elements[i],points3din->elements[i]));
    }
    for (i=0; i<numLines; i++)
    {
        if (lines2din->elements[i]->elements[1]!=0.0)
        {
            ERE(multiply_vector_by_scalar(&(lines2d->elements[i]),lines2din->elements[i],1.0/lines2din->elements[i]->elements[1]));
        }
        else
        {
            ERE(copy_vector(&(lines2d->elements[i]),lines2din->elements[i]));
        }
        ERE(homogenize_vector(&(lines3da->elements[i]),lines3dain->elements[i]));
        ERE(homogenize_vector(&(lines3db->elements[i]),lines3dbin->elements[i]));
    }

    /*Create plucker representations of each 3D line.*/
    ERE(get_target_vector_vector(&pluckers,numLines));
    ERE(get_target_vector_vector(&l_projs,numLines));
    ERE(get_target_vector_vector(&p_projs,numPoints));
    for (i=0; i<numLines; i++)
    {
        ERE(plucker_line_from_points(&(pluckers->elements[i]),lines3da->elements[i],lines3db->elements[i]));
    }

    /*The Main Loop -- Randomly guess and keep the one with the lowest error.*/
    val = 1.0/DBL_EPSILON;
    for (iter=0;iter<iterations;iter++)
    {

        /*Begin setting up outgoing guess*/
        /*Pick randomly among all 2D inputs, (both points and lines).*/
        ERE(pick_m_from_n(&rand2dSeq,numPoints+numLines,numPoints+numLines));
        nPointsOut=0;
        nLinesOut=0;
        for (j=0;j<corrs_out;j++)
        {
            if (rand2dSeq->elements[j]<numPoints)
            {
                ERE(copy_vector(&(p2Dout->elements[nPointsOut]),points2d->elements[rand2dSeq->elements[j]]));
                nPointsOut++;
            }
            else
            {
                ERE(copy_vector(&(l2Dout->elements[nLinesOut]),lines2d->elements[rand2dSeq->elements[j]-numPoints]));
                nLinesOut++;
            }
        }
        /*Pick a number of 3D points equal to the number 2D points picked.*/
        ERE(pick_m_from_n(&rand3dPointSeq,numPoints,numPoints));
        for (j=0;j<nPointsOut;j++)
        {
            ERE(copy_vector(&(p3Dout->elements[j]),points3d->elements[rand3dPointSeq->elements[j]]));
        }
        /*Pick a number of 3D lines equal to the number 2D lines picked.*/
        ERE(pick_m_from_n(&rand3dLineSeq,numLines,numLines));
        for (j=0;j<nLinesOut;j++)
        {
            ERE(copy_vector(&(l3DAout->elements[j]),lines3da->elements[rand3dLineSeq->elements[j]]));
            ERE(copy_vector(&(l3DBout->elements[j]),lines3db->elements[rand3dLineSeq->elements[j]]));
        }

        /*End setting up outgoing guess*/

        /*The get_camera_... call*/
        ERE(get_camera_matrix_from_point_and_line_correspondences
        (
            &nextCamera,
            p2Dout,
            p3Dout,
            nPointsOut,
            l2Dout,
            l3DAout,
            l3DBout,
            nLinesOut
        ));

        /*Create a matching line-projection matrix.*/
        ERE(line_projection_matrix_from_point_project_matrix(&line_proj,nextCamera));
        for (i=0; i<numLines; i++)
        {
            ERE(multiply_matrix_and_vector(&(l_projs->elements[i]),line_proj,pluckers->elements[i]));
        }
        for (i=0; i<numPoints; i++)
        {
            ERE(multiply_matrix_and_vector(&(p_projs->elements[i]),nextCamera,points3d->elements[i]));
        }

        /*First check the error of the assumed correspondences.*/
        curVal = 0.0;
        for (i=0;i<nPointsOut;i++)
        {
            curVal += error_check_vectors(p_projs->elements[rand3dPointSeq->elements[i]],p2Dout->elements[i],2);
        }
        for (i=0; i<nLinesOut; i++)
        {
            curVal += error_check_vectors(l_projs->elements[rand3dLineSeq->elements[i]],l2Dout->elements[i],1);
        }

        /*Next check the unused correspondences.*/
        for (i=corrs_out; i<numPoints+numLines; i++)
        {
            err = 1.0/DBL_EPSILON;
            /*If the 2D value is less then the number of input points, then we are dealing with a point ...*/
            if (rand2dSeq->elements[i]<numPoints)
            {
                for (j=nPointsOut;j<numPoints;j++)
                {
                    if (rand3dPointSeq->elements[j]>=0) {
                        curVal += error_check_vectors(p_projs->elements[rand3dPointSeq->elements[j]],lines2d->elements[rand2dSeq->elements[i]],2);
                        if (curErr<err)
                        {
                            err=curErr;
                            toExclude = j;
                        }
                    }
                }
                rand3dPointSeq->elements[toExclude] = -1;
            }
            /*... Otherwise we are dealing with a line.*/
            else
            {
                for (j=nLinesOut;j<numLines;j++)
                {
                    if (rand3dLineSeq->elements[j]>=0) {
                        curErr = error_check_vectors(l_projs->elements[rand3dLineSeq->elements[j]],lines2d->elements[rand2dSeq->elements[i]-numPoints],1);
                        if (curErr<err)
                        {
                            err=curErr;
                            toExclude = j;
                        }
                    }
                }
                rand3dLineSeq->elements[toExclude] = -1;
            }
            curVal += err;
        }
        curVal/=(numPoints+numLines);
        /*Keep the minimum.*/
        if (curVal<val) {
            val=curVal;
            *error=val;
            ERE(copy_matrix(camera, nextCamera));
            if (val < tolerance)
            {
                break;
            }
        }
    }/*End guess loop.*/


    /*frees*/
    free_int_vector(rand2dSeq);
    free_int_vector(rand3dPointSeq);
    free_int_vector(rand3dLineSeq);
    /*
    free_int_vector(bestSeq2d);
    free_int_vector(bestSeq3d);
    */
    free_matrix(nextCamera);
    free_vector_vector(p_projs);
    free_vector_vector(l_projs);
    free_matrix(line_proj);

    free_vector_vector(points2d);
    free_vector_vector(lines2d);
    free_vector_vector(points3d);
    free_vector_vector(lines3da);
    free_vector_vector(lines3db);

    free_vector_vector(p2Dout);
    free_vector_vector(l2Dout);
    free_vector_vector(p3Dout);
    free_vector_vector(l3DBout);
    free_vector_vector(l3DAout);

    free_vector_vector(pluckers);

    return NO_ERROR;
}

double error_check_vectors
(
    const Vector* vp1,
    const Vector* vp2,
    int i
)
{
    Vector* dupe1=0;
    Vector* dupe2=0;
    double rVal;
    if (vp1->elements[i]==0.0 || vp1->elements[i]==0.0)
    {
        if (vp2->elements[i]==0.0 && vp1->elements[i]==0.0)
        {
            subtract_vectors(&dupe1,vp1,vp2);
        }
        else
        {
            return 1.0/DBL_EPSILON;
        }
    }
    else
    {
        homogenize_vector_at_index(&dupe1,vp1,i);
        homogenize_vector_at_index(&dupe2,vp2,i);
        ow_subtract_vectors(dupe1,dupe2);
    }
    rVal = vector_magnitude(dupe1);
    free_vector(dupe1);
    free_vector(dupe2);
    return rVal;
}
#ifdef __cplusplus
}
#endif
