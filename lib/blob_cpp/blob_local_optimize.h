/**
 * @file
 * @author Ernesto Brau
 */
/*
 * $Id$
 */

#ifndef LOCAL_OPTIMIZE_H_INCLUDED_
#define LOCAL_OPTIMIZE_H_INCLUDED_

/**
 * @brief   Find the local optima of a 1-dimensional discrete function.
 *
 * The function here is represented by an one-dimensional array, with
 * the [] operator defined.
 */
template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_1D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
);

/**
 * @brief   Find the local optima of a 2-dimensional discrete function.
 *
 * The function here is represented by an two-dimensional array, with
 * the [][] operator defined.
 */
template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_2D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
);

/**
 * @brief   Find the local optima of a 3-dimensional discrete function.
 *
 * The function here is represented by an three-dimensional array, with
 * the [][][] operator defined.
 */
template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_3D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
);


//helper functions

template<class DiscreteFunction>
bool less_than_neighbors_2D(int i, int j, const DiscreteFunction& f);

template<class DiscreteFunction>
bool greater_than_neighbors_2D(int i, int j, const DiscreteFunction& f);

template<class DiscreteFunction>
bool less_than_neighbors_3D(int i, int j, int k, const DiscreteFunction& f);

template<class DiscreteFunction>
bool greater_than_neighbors_3D(int i, int j, int k, const DiscreteFunction& f);


//============================================================================
// Definition of template and inline functions
//============================================================================

/************************************************
 *                  ONE DIMENSION               *
 ************************************************/
template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_1D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
)
{
    const int FSZ = f.size() - 1;
    for (int i = 1; i < FSZ; i++)
    {
        if (f[i] < f[i - 1] && f[i] < f[i + 1] && f[i] <= -thresh)
        {
            *result++ = convert_index(i);
        }

        if (f[i] > f[i - 1] && f[i] > f[i + 1] && f[i] >= thresh)
        {
            *result++ = convert_index(i);
        }
    }

    return result;
}


/************************************************
 *                  TWO DIMENSIONS              *
 ************************************************/

template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_2D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
)
{
    const int FSZ = f.size() - 1;
    for (int i = 1; i < FSZ; ++i)
    {
        const int FI_SZ = f[i].size() - 1;
        for (int j = 1; j < FI_SZ; ++j)
        {
            if (less_than_neighbors_2D(i, j, f) && f[i][j] <= -thresh)
            {
                *result++ = convert_index(i, j);
            }

            if (greater_than_neighbors_2D(i, j, f) && f[i][j] >= thresh)
            {
                *result++ = convert_index(i, j);
            }
        }
    }

    return result;
}

template<class DiscreteFunction>
bool less_than_neighbors_2D(int i, int j, const DiscreteFunction& f)
{
    for (int l_i = i - 1; l_i <= i + 1; ++l_i)
    {
        for (int l_j = j - 1; l_j <= j + 1; ++l_j)
        {
            if (f[l_i][l_j] <= f[i][j])
            {
                if (i != l_i || j != l_j)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

template<class DiscreteFunction>
bool greater_than_neighbors_2D(int i, int j, const DiscreteFunction& f)
{
    for (int l_i = i - 1; l_i <= i + 1; ++l_i)
    {
        for (int l_j = j - 1; l_j <= j + 1; ++l_j)
        {
            if (f[l_i][l_j] >= f[i][j])
            {
                if (i != l_i || j != l_j)
                {
                    return false;
                }
            }
        }
    }

    return true;
}


/************************************************
 *              THREE DIMENSIONS                *
 ************************************************/

template<class DiscreteFunction, class OutputIterator, class ConvertIndex>
OutputIterator local_argoptima_3D(
    const DiscreteFunction& f,
    double thresh,
    OutputIterator result,
    ConvertIndex convert_index
)
{
    const int FSZ = f.size() - 1;
    for (int i = 1; i < FSZ; ++i)
    {
        const int FI_SZ = f[i].size() - 1;
        for (int j = 1; j < FI_SZ; ++j)
        {
            const int FIJ_SZ = f[i][j].size() - 1;
            for (int k = 1; k < FIJ_SZ; ++k)
            {
                if (less_than_neighbors_3D(i,j,k,f) && f[i][j][k] <= -thresh)
                {
                    *result++ = convert_index(i, j, k);
                }

                if (greater_than_neighbors_3D(i,j,k,f) && f[i][j][k] >= thresh)
                {
                    *result++ = convert_index(i, j, k);
                }
            }
        }
    }

    return result;
}

template<class DiscreteFunction>
bool less_than_neighbors_3D(int i, int j, int k, const DiscreteFunction& f)
{
    for (int l_i = i - 1; l_i <= i + 1; ++l_i)
    {
        for (int l_j = j - 1; l_j <= j + 1; ++l_j)
        {
            for (int l_k = k - 1; l_k <= k + 1; ++l_k)
            {
                if (f[l_i][l_j][l_k] <= f[i][j][k])
                {
                    if (i != l_i || j != l_j || k != l_k)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

template<class DiscreteFunction>
bool greater_than_neighbors_3D(int i, int j, int k, const DiscreteFunction& f)
{
    for (int l_i = i - 1; l_i <= i + 1; ++l_i)
    {
        for (int l_j = j - 1; l_j <= j + 1; ++l_j)
        {
            for (int l_k = k - 1; l_k <= k + 1; ++l_k)
            {
                if (f[l_i][l_j][l_k] >= f[i][j][k])
                {
                    if (i != l_i || j != l_j || k != l_k)
                    {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

#endif /*LOCAL_OPTIMIZE_H_INCLUDED_ */

