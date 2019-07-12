/***
*
*Zewei Jiang
*
******/
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "n_cpp/n_eig.h"


using namespace kjb;
using namespace std;

class PCA
{
    Matrix data;//this is the matrix that hold the original data
    Matrix pca;//this is the matrix that hold the eig vector after "sorting"
    Vector eig_value;//vector that contain the eig value
    Vector training_mean;//this is the mean of the variance_modified original data
    Vector training_variance;//this is the 
    Matrix scale_data;//this is the matrix that hold the after scale data

    public:
    //this is the constructor that receive the data matrix and do the pca stuff
    PCA(const Matrix mat)
    {
        data=mat;
        scale_data=data;
        int dimension=data.get_num_cols();
        int numofdata=data.get_num_rows(); 

        //scale the data, also remember the  mean and variance in case we want to restore them    
        Vector tmp_ev=sum_matrix_rows(data)/numofdata;
        training_mean=tmp_ev;
        training_variance=tmp_ev;
        for(int i=0;i<dimension;i++)
        {
            double variance=0.0;
            for(int j=0;j<numofdata;j++)
            {
                variance+=(data(j,i)-tmp_ev(i))*(data(j,i)-tmp_ev(i));
            }
            training_variance(i)=sqrt(variance/numofdata);
        }
        
        for(int i=0;i<dimension;i++)
        {
            for(int j=0;j<numofdata;j++)
            {
                scale_data(j,i)=data(j,i)/training_variance(i);
            }
        }
          
        training_mean=sum_matrix_rows(scale_data)/numofdata;
 
        for(int i=0;i<dimension;i++)
        {
            for(int j=0;j<numofdata;j++)
            {
                scale_data(j,i)=scale_data(j,i)-training_mean(i);
            }
        }

        Matrix covmat(dimension,dimension);

        Matrix tmpmatone(1,numofdata);
        Matrix tmpmattwo(numofdata,1);
        Matrix tmpmatthree(1,1);

        Vector ey=sum_matrix_rows(scale_data)/numofdata;

        for (int i=0;i<dimension;i++)
        {
            for(int j=0;j<dimension;j++)
            {
                for(int k=0;k<numofdata;k++)
                {
                    tmpmatone(0,k)=scale_data(k,i)-ey(i);
                    tmpmattwo(k,0)=scale_data(k,j)-ey(j);
                }
                tmpmatthree=tmpmatone*tmpmattwo;
                covmat(i,j)=tmpmatthree(0,0)/(numofdata-1);//here use n-1 istead of n because Bessel's correction
            }
        }

        Matrix eigvector(dimension,dimension);
        Vector eigvalue(dimension);
        diagonalize(covmat,eigvector,eigvalue,true);

        pca=eigvector;
        eig_value=eigvalue;        
    }
    
    //return first num_components part of the dimension-reduced data
    Matrix get_data_after_PCA(int num_components)
    {
        if (num_components>pca.get_num_cols()||num_components<=0)
        {
            return scale_data*pca;
        }
        else
        {
            return scale_data*pca.submatrix(0,0,pca.get_num_rows(),num_components);
        }         
    }
    
    //return scene given the value of the first component
    Vector get_scence_with_one_dimension_value(int value)
    {
        Vector return_vector = pca.get_col(0)*value+training_mean;
        for(int i=0;i<training_mean.get_length();i++)
        {
            return_vector(i)=return_vector(i)*training_variance(i);   
        }
        return return_vector;
    }
};
