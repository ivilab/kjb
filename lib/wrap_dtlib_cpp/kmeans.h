/////////////////////////////////////////////////////////////////////////////
// kmeans.h - K-Means clustering class
// Author: Doron Tal
// Date created: March, 2000

#ifndef _KMEANS_H
#define _KMEANS_H

#include "wrap_dtlib_cpp/img.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"

#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace std;
#warning "[Code police] Do not put 'using namespace' in global scope of header."
using namespace kjb_c;


namespace DTLib {

    // K-Means class - vector quantization on data in Euclidean space
    // --------------------------------------------------------------
    //
    // Usage example:
    //      CKMeans ck(P, K, D, ppData);
    //      ck.Init();
    //      ck.Iterate(20);
    //      ck.Prune(1.1f);
    //      ... and the answer is in ck.pPtClusters()
    //
    class CKMeans
    {
    public:
        // Constructor doesn't initialize, just allocates memory and
        // sets variables.  To initialize, use one of the Init
        // functions.
        // 'P' - number of data points
        // 'K' - number of clusters
        // 'D' - number of dimensions
        // 'ppData' - a pointer to either:
        //            (1) D arrays of length P each, such that
        //                ppDimsPts[d][p] is the Value of the
        //                d_th dimension of the p_th point.  If this is
        //                the data format, then we need to make
        //                the next argument, 'bTransposeData' true.
        //            (2) P arrays of length D each, in which case,
        //                we must make 'bTransposeData' false.
        // 'bTransposeData' - this arg. describes how the data is supplied
        //                    in 'ppData'
        // NOTE: if 'bTransposeData' is true, the constructor makes a
        // local copy of the data that gets destroyed only when this
        // kmeans object itself gets destroyed.
        CKMeans(const int P, const int K, const int D, float** ppData,
                bool bTransposeData = true);

        ~CKMeans();

        // Classic initialization: assign each cluster center to a
        // data point chosen randomly.  NOTE: the cluster centers
        // could end up just about on top of each other, or near a
        // part of the space that gets very few data points,
        // i.e. anything can happen with this type of initialization.
        void Init();

        // Usama Fayad's refined K-Means initialization procedure
        void RefinedInit(const int& SubsampleSize, const int& J);

        // used in Usama Fayad's algorithm
        void InitWithMeanVectors(float** ppMeanVectors);

        // "safer" Iterate(), for Usama Fayad's RefinedInit() above
        void IterateMod();

        // Cluster a la K-Means.  Termination: function terminates
        // when either of two conditions are met: (a) terminate after
        // fewer than 'nChanges' points change clusters; (b) terminate
        // after 'nIterations' iterations.  The resulting cluster
        // assignment to each point is left in m_pClusterAssignments;
        // RETURN VALUE: if the algorithm converged, returns the # of
        // iterations; otherwise if 'nMaxIters' was reached, returns
        // -1; otheriwse if 'nMaxChanges' was reached, returns -2.
        int Iterate(const int& nMaxIters = -1, const int& nMaxChanges = -1,
                    const int& SmallestClusterSize = -1,
                    const int& Width = -1, const int& Height = -1);

        // at the start of Prune(), we compute the initial
        // distortion, e*, we stop when the distortion becomes
        // (StoppingFactor)*(e*).
        void Prune(const float& StoppingFactor,
                   const int& StoppingK = -1);

        //////////////////////////////////////////////////////////////////
        // DATA ACCESS FUNCTIONS
        //////////////////////////////////////////////////////////////////

        int nK() { return m_nK; }

        // THIS IS THE OUTPUT OF K-MEANS
        // assignments of clusters to the p data points, ZERO-INDEXED
        // (i.e. first cluster center is indexed by 0, next index is 1, ..)
        kjb_int32* pPtClusters() { return m_pPtClusters; }

        // as above, but detaches buffer from object (to avoid copying)
        // NB: you must delete this pointer yourself eventually!
        kjb_int32* pPtClustersDetach() {
            kjb_int32* pRes = m_pPtClusters; m_pPtClusters = NULL; return pRes;
        }

    private:
        inline void SwapParents(const int& p, const int& iNearestK,
                                const int& iParentK);

        void CleanupMemberships2D(const int& Width, const int& Height);

        // (re)assigns a cluster to each data point by picking the closest one
        // also recomputes the the number of points assigned to each cluster
        // and the sum of all the points of each cluster
        void AssignClustersToPoints();

        // Compute the mean for each cluster, using m_ppClusterDimSums
        // and m_pClusterNumpts
        void ComputeClusterMeans();

        void RemoveSmallClusters(const int& SmallestSizeAllowed = 1);

        // returns index of cluster nearest to data point indexed by 'p'
        inline int FindNearestClusterToPoint(const int p);

        // 'k' - index of cluster center
        // 'p' - index of data point
        // RETURN VALUE: the squared distance between data point 'p'
        // and cluster center 'k'
        inline float ComputeSquareDistance(const int k, const int p);

        // return the total distortion, or squared error
        float ComputeTotalError();

        //////////////////// MEMBER VARIABLES ////////////////////////////

        // true # of clusters currently used (can change after a call to
        // Prune(), or to Iterate())
        int m_nK;

        // original # of clusters class object was constructed with
        int m_nOrigK;

        int m_nChanges;

        // # of data points (# of Img points)
        int m_nP;

        // # of dimensions of each data point
        int m_nD;

        // The data points. PxD matrix - P pointers to arrays of D floats each.
        float** m_ppPtsDims;

        // if true, delete m_ppPtsDims, otherwise don't delete it
        // (if we supplied the transposed data in the constructor then we
        // haven't allocated the data internally and should not deallocate it,
        // otherwise, we'll allocate it).  This member var is used internally
        // only.
        bool m_bDeallocatePtsDims;

        // THIS IS THE OUTPUT OF K-MEANS
        // the assignment of clusters to points (size = P)
        kjb_int32* m_pPtClusters;

        // Array of K integers, each = # of points per cluster.

        int* m_pClusterNumpts;

        // KxD matrix - K pointers to arrays of D points each.
        // each of these arrays corresponds to a cluster's mean
        // but the mean is unnormalized (i.e. it's just the sum, not
        // divided by the number of elements).
        float** m_ppClusterDimSums;

        // KxD matrix - K pointers to arrays of D points each.
        // each of these arrays corresponds to a cluster's mean.
        float** m_ppClusterDimMeans;

        // temporary storage area used to avoid memory reallocation when
        // removing clusters
        int* m_pClusterNumpts2;
        float** m_ppClusterDimSums2;
        float** m_ppClusterDimMeans2;
    };

} // namespace DTLib {

#endif /* #ifndef _KMEANS_H */
