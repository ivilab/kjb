/////////////////////////////////////////////////////////////////////////////
// kmeans.cpp - K-Means clustering class
// Author: Doron Tal
// Date created: March, 2000
// Optimization changes: David Martin, Sept 4, 2000 -- changed layout of
// inputs (in memory) so that the cache is used more efficiently, gave a
// 1.2x speedup on pentium and 5.7x speedup on sparc.

#include "l/l_sys_debug.h"  /* For ASSERT. */
#include <algorithm>
#include <list>
#include "wrap_dtlib_cpp/kmeans.h"
#include "wrap_dtlib_cpp/reflect.h"

/*
 * Kobus: We have run into trouble with 32 bit centric code in this
 * distribution. I have changed some long's to kjb_int32's.  The immediate
 * problem is that the segmentation maps can get written out as 64 bit integers. 
*/
#include "l/l_sys_def.h"
#include <iostream>

using namespace DTLib;
using namespace kjb_c;

/////////////////////////////////////////////////////////////////////////////

CKMeans::CKMeans(const int P, const int K, const int D, float** ppData, 
                 bool bTransposeData)
{
    m_nP = P;
    m_nK = K;
    m_nOrigK = K;
    m_nD = D;
    m_pPtClusters = new kjb_int32[P];
    if (bTransposeData) {
        // if we transpose the data, we need to store a copy of (the 
        // transposed) data and copy over from ppData
        m_ppPtsDims = new float*[P];
        for (int p = 0; p < m_nP; p++) {
            m_ppPtsDims[p] = new float [D];
        }
        // Transpose ppData into m_ppPtsDims (for cache-friendly access).
        for (int p = 0; p < P; p++) {
            for (int d = 0; d < D; d++) {
                m_ppPtsDims[p][d] = ppData[d][p];
            } // for d
        } // for p
        m_bDeallocatePtsDims = true; // we'll need to delete this data
    }
    else { 
        // no transpose, so we don't even need to copy things, or store
        // them internally -- just make m_ppPtsDims point to the data
        m_ppPtsDims = ppData;
        m_bDeallocatePtsDims = false;
    }
    m_pClusterNumpts = new int[m_nK];
    m_ppClusterDimSums = new float*[m_nK];
    m_ppClusterDimMeans = new float*[m_nK];

    for (int k = 0; k < m_nK; k++) {
        m_ppClusterDimSums[k] = new float[m_nD];
        m_ppClusterDimMeans[k] = new float[m_nD];
    } // for (int i = 0; i < m_nK; i++) {

    m_pClusterNumpts2 = new int[m_nK];
    m_ppClusterDimSums2 = new float*[m_nK];
    m_ppClusterDimMeans2 = new float*[m_nK];
}

/////////////////////////////////////////////////////////////////////////////

CKMeans::~CKMeans()
{
    zap(m_pPtClusters);
    if (m_bDeallocatePtsDims) { 
        for (int p = 0; p < m_nP; p++) {
            zap(m_ppPtsDims[p]);
        }
        zap(m_ppPtsDims);
    }

    zap(m_pClusterNumpts);
    for (int i = 0; i < m_nOrigK; i++) {
        zap(m_ppClusterDimSums[i]);
        zap(m_ppClusterDimMeans[i]);
    }
    zap(m_ppClusterDimSums);
    zap(m_ppClusterDimMeans);

    zap(m_pClusterNumpts2);
    zap(m_ppClusterDimSums2);
    zap(m_ppClusterDimMeans2);
}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::Init()
{
    m_nChanges = 0;
    // assign the k cluster means to a random picking (with no
    // repetitions) from all data points, copying over the means to
    // m_ppCluterDimMeans
    const float RandFactor = (float)m_nP/((float)RAND_MAX+1.0f);
    int *aRandAssign = new int[m_nK];
    for (int k = 0; k < m_nK; k++) {
        // get a *unique* random sample point index
        int RandP =  (int)((float)rand()*RandFactor);
        int* end = aRandAssign+k;
        while (find(aRandAssign, end, RandP) != end) {
            RandP =  (int)((float)rand()*RandFactor);
        }
        aRandAssign[k] = RandP;
        // copy over sample point's data and make it a cluster mean
        for (int d = 0; d < m_nD; d++) {
            m_ppClusterDimMeans[k][d] = m_ppPtsDims[RandP][d];
        }
    } // for k
    zap(aRandAssign);
    AssignClustersToPoints();
}

/////////////////////////////////////////////////////////////////////////////
// the idea: 
// 1. run K-means J times with subsampling, get J sets of mean vectors
// 2. take data from K x J runs and initialize with each of the J sets
//    re-run k-means on this new data 

void CKMeans::RefinedInit(const int& SubsampleSize, const int& J)
{
    // loop indices: d=dimensions, j=kmeans runs,p=data-points,k=#clusters
    int d, j, p, k; 

    // allocate memory for subsampled data
    float** ppPtsDims = new float*[SubsampleSize];
    for (p = 0; p < SubsampleSize; p++) {
        ppPtsDims[p] = new float[m_nD];
    }

    // accumulating results of each of the J k-means runs in KMList
    list<CKMeans*> KMList;
    // subsample 'SubsampleSize' points randomly and run k-means 'J' times
    const float RandFactor = (float)m_nP/((float)RAND_MAX+1.0f);
    for (j = 0; j < J; j++) {
        // subsample 'SubsampleSize' points randomly 
        for (p = 0; p < SubsampleSize; p++) {
            const int iRandP =  (int)((float)rand()*RandFactor);
            for (d = 0; d < m_nD; d++) {
                ppPtsDims[p][d] = m_ppPtsDims[iRandP][d];
            } // for d
        } // for p
        
        CKMeans* pKM = new CKMeans(SubsampleSize, m_nK,m_nD, ppPtsDims,false); 
        pKM->Init();
        pKM->IterateMod();
        KMList.push_back(pKM);
    } // for j
    ASSERT((int)(KMList.size()) == J);
    // deallocate memory for subsampled data
    for (p = 0; p < SubsampleSize; p++) zap( ppPtsDims[p]);
    zap(ppPtsDims);
    
    // ---------------
    // 2nd k-means run
    // ---------------

    // new # of samples
    const int JKSize = J*m_nK; 

    // allocate memory for 2nd k-means run's data
    ppPtsDims = new float*[JKSize];
    for (int p = 0; p < JKSize; p++) {
        ppPtsDims[p] = new float[m_nD];
    }

    // copy over data into ppDimsPts (reusing ppDimsPts here)
    // each of the J K-Means objects in the list contributes K points
    // to our new data in ppDimsPts
    list<CKMeans*>::iterator ipKM;
    ipKM = KMList.begin();
    for (j = 0, p = 0; j < J; j++, ipKM++) {
        for (k = 0; k < m_nK; k++, p++) {
            for (int d = 0; d < m_nD; d++) {
                // (*ipKM)->m_ppClusterDimMeans is a KxD array, K 
                // pointers to arrays of size D each.
                ppPtsDims[p][d] = (*ipKM)->m_ppClusterDimMeans[k][d];
            } // for d
        } // for k
    } // for j
    // done setting up ppDimsPts

    // 
    // for each of the k-means objects in our list, use its mean vector
    // to initialize a k-means object, keeping track which of these new
    // k-means objects has the smallest distortion
    //

    // here we store the min score winner
    float **ppMindistClusterDimMeans = NULL; 

    // here we keep the k-means objects that are initialized with the
    // means from the previous list's k-means objects

    list<CKMeans*> KMList2;
    float MinScore = CONST_MAX_FLOAT;
    ipKM = KMList.begin();
    for (j = 0; j < J; j++, ipKM++) {
        CKMeans* pKM = new CKMeans(JKSize, m_nK, m_nD, ppPtsDims, false);
        pKM->InitWithMeanVectors((*ipKM)->m_ppClusterDimMeans);
        pKM->Iterate(-1, -1);
        KMList2.push_back(pKM);
        float Distortion = pKM->ComputeTotalError();
        if (Distortion < MinScore) {
            MinScore = Distortion;
            ppMindistClusterDimMeans = pKM->m_ppClusterDimMeans;
        }
    } // for (int j = 0; j < J; j++, ipKM++) {

    // now iMinKM has the KM object that got the lowest score
    // reinitialize, using the mean vector of iMinKM
    InitWithMeanVectors(ppMindistClusterDimMeans);
  
    // deallocate memory for subsampled data
    for (p = 0; p < JKSize; p++) zap( ppPtsDims[p]);
    zap(ppPtsDims);
#if 0
    ipKM = KMList.begin();
    list<CKMeans*>::iterator ipKM2 = KMList2.begin();
    for (j = 0; j < J; j++, ipKM++, ipKM2++) {
        zap(*ipKM);
        zap(*ipKM2);
    }
#endif
    KMList.clear();
    KMList2.clear();

}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::InitWithMeanVectors(float** ppMeanVectors)
{
    m_nChanges = 0;
    memset(m_pClusterNumpts, 0, m_nK*sizeof(int));
    // copy over the means to m_ppCluterDimMeans
    for (int k = 0; k < m_nK; k++) {
        memset(m_ppClusterDimSums[k], 0, m_nD*sizeof(float));
        for (int d = 0; d < m_nD; d++) {
            m_ppClusterDimMeans[k][d] = ppMeanVectors[k][d];
        }
    }
    AssignClustersToPoints();
}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::IterateMod()
{
    int k, d, p;
    bool bSomeClustersHaveNoMembers = true;
    while (bSomeClustersHaveNoMembers) {
        Iterate(-1, -1); // classic k-means, run till converges
        // check for membership
        bSomeClustersHaveNoMembers = false; // loop below tries disproving this
        int *pClusterNumpts = m_pClusterNumpts;
        for (k = 0; k < m_nK; k++, pClusterNumpts++) {
            if (m_pClusterNumpts[k] == 0) {
                // designate that we'll have to run k-means again:
                bSomeClustersHaveNoMembers = true;

                // the cluster k has no memberships

                // 'MaxDist' is max distance of any point to cluster k
                float MaxDist = 0.0f; 
                //
                // loop to find furthest point 'iFarthest_p' from cluster k
                //
                int iFarthest_p = -1; // index of pt whose dist from k= MaxDist
                for (p = 0; p < m_nP; p++) {
                    const float SquareDist = ComputeSquareDistance(k, p);
                    if (SquareDist > MaxDist) {
                        MaxDist = SquareDist;
                        iFarthest_p = p;
                    } // if (SquareDist > MaxDist) {
                } // for (int p = 0; i < m_nP; p++) {
                ASSERT(iFarthest_p != -1);

                // now we found fartherst point from cluster center, make this
                // pt the center of the current cluster (which has no members)
                for (d = 0; d < m_nD; d++) {
                    m_ppClusterDimMeans[k][d] = m_ppPtsDims[p][d];
                }
            } // if 
        } // for k
        if (bSomeClustersHaveNoMembers) {
            // prepare for calling iterate
            m_nChanges = 0;
            for (k = 0; k < m_nK; k++) {
                memset(m_ppClusterDimSums[k], 0, m_nD*sizeof(float));
            } // for k
            memset(m_pClusterNumpts, 0, m_nK*sizeof(int));
        }
    } // while (bSomeClustersHaveNoMembers) {
}

/////////////////////////////////////////////////////////////////////////////

int CKMeans::Iterate(const int& nMaxIters, const int& nMaxChanges,
                     const int& SmallestClusterSize,
                     const int& Width, const int& Height)
{
    for (int i = 0; i != nMaxIters; i++) {
        const int nPrevChanges = m_nChanges; // to see if there've been changes
        for (int p = 0; p < m_nP; p++) {
            int iNearestK = FindNearestClusterToPoint(p);
            const int iParentK = m_pPtClusters[p];
            if (iParentK != iNearestK)  {
                SwapParents(p, iNearestK, iParentK);
            } // if (iParentK != iNearestK) {
        } // for (int p = 0; p < size; p++) {
        if ((Width != -1) && (Height != -1)) {
            CleanupMemberships2D(Width, Height);
        }
        RemoveSmallClusters(SmallestClusterSize);
        ComputeClusterMeans();
        if (i && (m_nChanges == nPrevChanges)) { 
            // k-means convrged!  return total # of iterations
            return i+1; 
        }
        if ((nMaxChanges > 0) && (m_nChanges >= nMaxChanges)) {
            // has not converged, seen too many changes
            return -2;
        }
    } // for (i = 0; i < nMaxIters; i++) {
    return -1; // to signal that nMaxIters was reached w/out converging
}

/////////////////////////////////////////////////////////////////////////////
// 1. absolute error delta, not relative
// 2. after removing a cluster, iterate k-means for 1-5 steps or so

void CKMeans::Prune(const float& StoppingFactor, const int& StoppingK)
{
    // compute total squared error and target 'Final' error:
    const float FinalError = StoppingFactor*ComputeTotalError();
    float NewError = 0.0f;
    float* aKErrors = new float[m_nK];
    while ((m_nK > 1) &&
           (NewError < FinalError) &&
           ((StoppingK > 0) && (m_nK > StoppingK))) {
        // first loop over p over k to estimate worst k (k to remove)
        memset(aKErrors, 0, m_nK*sizeof(float));
        for (int p = 0; p < m_nP; p++) {
            // loop initialization: pick the first two cluster centers,
            // compute their errors, and find the minimum error and
            // 2nd minimum -- this picks first two smallest-error clusters,
            // we'll continue walking the list in the loop below, and 
            // replace the first two smallest with what we see..
            //             const float K0dist = ComputeSquareDistance(0, p);
            const float K0dist = ComputeSquareDistance(0, p);
            const float K1dist = ComputeSquareDistance(1, p);
            int iNearestK1; // index of nearest K to current point
            // find the distance from each point to all cluster centers,
            // keeping track of closest, and second closest cluster centers
            float minDist1, minDist2;
            if (K0dist < K1dist) {
                iNearestK1 = 0;
                minDist1 = K0dist;
                minDist2 = K1dist;
            }
            else {
                iNearestK1 = 1;
                minDist1 = K1dist;
                minDist2 = K0dist;
            }
            // loop inv: the first smallest-error cluster has error
            // 'minDist1' and index 'iNearestK1', and the
            // second-smallest-error cluster has error 'minDist2'
            for (int k = 2; k < m_nK; k++) {
                const float SquareDist = ComputeSquareDistance(k, p);
                if (SquareDist < minDist1) {
                    minDist2 = minDist1;
                    minDist1 = SquareDist;
                    iNearestK1 = k;
                } // if (SquareDist < minDist1) {
            }
            ASSERT(minDist1 < minDist2);
            // update the error for the cluster nearest to current point:
            aKErrors[iNearestK1] += minDist2-minDist1;
        } // for (p = 0; p < m_nP; p++) {
        // find k to remove s.t. removal increases error the least
        const int iminKError = iMin(aKErrors, m_nK);
        ASSERT(iminKError >= 0);
        ASSERT(iminKError < m_nK);
        // removes cluster indexed by iminKError by swapping it with
        // the last cluster, decrementing m_nK, and reassigning
        // clusters to points
        Swap(aKErrors[iminKError], aKErrors[m_nK-1]); 
        Swap(m_ppClusterDimMeans[iminKError], m_ppClusterDimMeans[m_nK-1]);
        Swap(m_ppClusterDimSums[iminKError], m_ppClusterDimSums[m_nK-1]);
        Swap(m_pClusterNumpts[iminKError], m_pClusterNumpts[m_nK-1]);
        m_nK--;
        // this next call will ensure no pixel is assigned to the prunned k
        AssignClustersToPoints();
        // but now we need to recompute means
        ComputeClusterMeans();
        // clean up a bit by iterating
        Iterate(3, -1);
        // recompute total error
        NewError = ComputeTotalError();
    } // while (NewError < 1.1f*MSError) {
    zap(aKErrors);
}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::SwapParents(const int& p, const int& iNearestK,
                          const int& iParentK)
{
    // increment # of point swaps
    m_nChanges++;
    // assign parent class to point
    m_pPtClusters[p] = iNearestK;
    // increment # of points belonging to iNearestK'th cluster
    m_pClusterNumpts[iNearestK]++;
    // add point to iNearestK'th cluster's sum
    for (int d = 0; d < m_nD; d++) {
        m_ppClusterDimSums[iNearestK][d] += m_ppPtsDims[p][d];
    } // for d
    if (m_pClusterNumpts[iParentK] > 0) {
        // decrement # of points belonging to cluster iParentK
        m_pClusterNumpts[iParentK]--;
        // subtract point out of iParentK'th cluster's sum 
        for (int d = 0; d < m_nD; d++) {
            m_ppClusterDimSums[iParentK][d] -= m_ppPtsDims[p][d];
        }
    } // for d
}

/////////////////////////////////////////////////////////////////////////////
// reset m_pPtClusters, m_pClusterNumpts, and m_ppClusterDimSums
// to their correct value, depending on the current cluster means
// and on the input data

void CKMeans::AssignClustersToPoints()
{
    // reset the sum and numpts info for each cluster
    memset(m_pClusterNumpts, 0, m_nK*sizeof(int));
    for (int k = 0; k < m_nK; k++) {
        memset(m_ppClusterDimSums[k], 0, m_nD*sizeof(float));
    }
    for (int p = 0; p < m_nP; p++) {
        int iNearestK = FindNearestClusterToPoint(p);
        // assign parent class to point
        m_pPtClusters[p] = iNearestK;
        // increment # of points belonging to iNearestK'th cluster
        m_pClusterNumpts[iNearestK]++;
        // add point to iNearestK'th cluster's sum
        for (int d = 0; d < m_nD; d++) {
            m_ppClusterDimSums[iNearestK][d] += m_ppPtsDims[p][d];
        } // for (int d = 0; d < m_nD; d++) {
    } // for (int p = 0; p < m_nP; p++) {
}

/////////////////////////////////////////////////////////////////////////////
// Go through all K clusters and for each one, compute the mean Value along
// each of it's D dimensions (iterations == K*D).  

void CKMeans::ComputeClusterMeans()
{
    for (int k = 0; k < m_nK; k++) {
        float* pClusterMean = m_ppClusterDimMeans[k];
        float* pClusterSum = m_ppClusterDimSums[k];
        if (m_pClusterNumpts[k] != 0) {
            const float Factor = 1.0f/(float)m_pClusterNumpts[k];
            for (int d = 0; d < m_nD; d++) {
                pClusterMean[d] = pClusterSum[d]*Factor;
            }
        } // if (nPts != 0) {
    } // for (int k = 0; k < m_nK; k++) {
}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::RemoveSmallClusters(const int& SmallestSizeAllowed)
{
    if (SmallestSizeAllowed > 0) {
        int n2Remove = 0, nRemaining = 0, i = -1;
        for (int k = 0; k < m_nK; k++) {
            const int ClusterNumpts = m_pClusterNumpts[k];
            if (m_pClusterNumpts[k] < SmallestSizeAllowed) {
                i = m_nK-1-n2Remove;
                n2Remove++;
            }
            else {
                i = nRemaining;
                nRemaining++;
            }
            m_pClusterNumpts2[i] = ClusterNumpts;
            m_ppClusterDimSums2[i] = m_ppClusterDimSums[k];
            m_ppClusterDimMeans2[i] = m_ppClusterDimMeans[k];
        } // for (int k = 0; k < m_nK; k++) {
        if (n2Remove != 0) {
            cerr << "KM::::: removing " << n2Remove << " CLUSTERS!!!!!!!\n";
            for (int k = 0; k < m_nK; k++) {
                m_pClusterNumpts[k] = m_pClusterNumpts2[k];
                m_ppClusterDimSums[k] = m_ppClusterDimSums2[k];
                m_ppClusterDimMeans[k] = m_ppClusterDimMeans2[k];
            }
            m_nK -= n2Remove;
        } // if (n2Remove > 0) {
    } // if (SmallestSizeAllowed > 0) {
}

/////////////////////////////////////////////////////////////////////////////

float CKMeans::ComputeTotalError() 
{
    float Error = 0.0f;
    for (int p = 0; p < m_nP; p++)
        Error += ComputeSquareDistance(m_pPtClusters[p], p);
    return (Error);
}

/////////////////////////////////////////////////////////////////////////////

int CKMeans::FindNearestClusterToPoint(const int p) 
{
    int iNearestK = -1;
    float minDist = CONST_MAX_FLOAT;
    for (int k = 0; k < m_nK; k++) {
        const float SquareDist = ComputeSquareDistance(k, p);
        if (SquareDist < minDist) {
            minDist = SquareDist;
            iNearestK = k;
        } // if (SquareDist < minDist) {
    } // for (int k = 0; k < m_nK; k++) {
    ASSERT(iNearestK >= 0);
    return(iNearestK);
}

/////////////////////////////////////////////////////////////////////////////

float CKMeans::ComputeSquareDistance(const int k, const int p) 
{
    float SquareDistance = 0.0f;
    for (register int d = 0; d < m_nD; d++) {
        const float Diff = m_ppClusterDimMeans[k][d]-m_ppPtsDims[p][d];
        SquareDistance += Diff*Diff;
    }
    return SquareDistance;
}

/////////////////////////////////////////////////////////////////////////////

void CKMeans::CleanupMemberships2D(const int& Width, const int& Height)
{
    CImg<kjb_int32> PtClustersImg(m_pPtClusters, Width, Height, false);
    CImg<kjb_int32> TmpImg(Width+2, Height+2);
    Reflect(PtClustersImg, TmpImg);
    const int StartX = TmpImg.ROIStartX(), 
        StartY = TmpImg.ROIStartY(), 
        EndX = TmpImg.ROIEndX(),
        EndY = TmpImg.ROIEndY();
    ASSERT(EndX-StartX == Width);
    ASSERT(EndY-StartY == Height);
    kjb_int32* pCluster = TmpImg.pROI();
    kjb_int32* pNewCluster = PtClustersImg.pBuffer();
    const int sk = TmpImg.ROISkipCols();
    kjb_int32* aKhisto = new kjb_int32[m_nK];
    const int SizeKhisto = m_nK*sizeof(kjb_int32);
    for (int y = StartY, p = 0; y < EndY; y++, pCluster += sk) {
        for (int x = StartX; x < EndX; x++, pCluster++, pNewCluster++, p++) {
            const int iParent = *pCluster;
            ASSERT(pNewCluster == m_pPtClusters+p);
            ASSERT(*pCluster == *pNewCluster);

            memset(aKhisto, 0, SizeKhisto);
            aKhisto[*(pCluster-1)]++;
            aKhisto[*(pCluster+1)]++;
            aKhisto[*(pCluster-Width-1)]++;
            aKhisto[*(pCluster-Width)]++;
            aKhisto[*(pCluster-Width+1)]++;
            aKhisto[*(pCluster+Width-1)]++;
            aKhisto[*(pCluster+Width)]++;
            aKhisto[*(pCluster+Width+1)]++;
            int iNewParent = -1;
            kjb_int32 maxNewParent = 0;
            for (int k = 0; k < m_nK; k++) {
                if (aKhisto[k] > 4) {
                    // if (aKhisto[k] > maxNewParent) {
                    maxNewParent = aKhisto[k];
                    iNewParent = k;
                    break;
                }
            }
            if ((iNewParent != -1) && (iNewParent != iParent)) {
                SwapParents(p, iNewParent, *pCluster);
            }

        } // for x
    } // for y
    zap(aKhisto);
}
