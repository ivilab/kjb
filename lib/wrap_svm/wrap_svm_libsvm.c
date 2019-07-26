
/* $Id: wrap_svm_libsvm.c 22174 2018-07-01 21:49:18Z kobus $ */

/* =========================================================================== *
|                                                                              |
|  Copyright (c) 2005-2008 by members of University of Arizona Computer Vision|
|  group (the authors) including                                               |
|        Ranjini Swaminathan
|        Kobus Barnard.                                                        |
|                                                                              |
|  (Copyright only applies to the wrapping code, not the wrapped code).        |
|                                                                              |
|  Personal and educational use of this code is granted, provided that this    |
|  header is kept intact, and that the authorship is not misrepresented, that  |
|  its use is acknowledged in publications, and relevant papers are cited.      |
|                                                                              |
|  For other use contact the author (kobus AT cs DOT arizona DOT edu).         |
|                                                                              |
|  Please note that the code in this file has not necessarily been adequately  |
|  tested. Naturally, there is no guarantee of performance, support, or fitness|
|  for any particular task. Nonetheless, I am interested in hearing about      |
|  problems that you encounter.                                                |
|                                                                              |
* =========================================================================== */

/* -------------------------------------------------------------------------- */

/*
 * Kobus, 06-03-11:
 *     This file was cobbled together largely be Ranjini Swaminathan from code
 *     in the libsvm example files svm-predict.c, svm-train., and svm-scale.c.
 *
 *     Some of the code has been modfied to follow KJB library conventions.
 *     However, it is a confusing blend of what libsvm does and what we do.
*/

/* -------------------------------------------------------------------------- */

#include "n/n_gen.h"      /*  Only safe if first #include in a ".c" file  */
#include "wrap_svm/wrap_svm.h"
#include "wrap_svm/wrap_svm_libsvm.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_LIBSVM
#    include "svm.h"
#endif

/* -------------------------------------------------------------------------- */

/*
 *  Defines that libsvm use, sanitized.
*/
#define LIBSVM_MALLOC(type,n) (type *)malloc((n)*sizeof(type))
#define libsvm_max(x,y) ((x>y)?x:y)
#define libsvm_min(x,y) ((x<y)?x:y)

/* -------------------------------------------------------------------------- */

#ifdef KJB_HAVE_LIBSVM
/* -----------------------------------------------------------------------------
|                              KJB_HAVE_LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/
static int fs_libsvm_svm_type                      = 0;
static int fs_libsvm_kernel_type                   = 0;
static int fs_libsvm_kernel_degree                 = 3;
static int fs_libsvm_kernel_gamma                     ;
static int fs_libsvm_kernel_coeff                  = 0;
static int fs_libsvm_cost                          = 1;
static double fs_libsvm_nu                         = 0.5;
static double fs_libsvm_epsilon_loss_fn            = 0.1;
static int fs_libsvm_cache_size                     = 40;
static double fs_libsvm_epsilon_tolerance          = 0.001;
static int fs_libsvm_shrink_heuristic              = 1;
static int fs_libsvm_prob_estimate                 = 1;
static int fs_libsvm_cross_validate                = FALSE;
static int fs_libsvm_scale_data                    = TRUE;
static double fs_libsvm_lower                      = 0.0;
static double fs_libsvm_upper                      = 1.0;
static int fs_libsvm_scale_target                  = FALSE;  /*scaling y - not implemented currently*/
static double fs_libsvm_y_lower                    = 0.0;
static double fs_libsvm_y_upper                    = 1.0;
static double fs_libsvm_y_max                      = -DBL_MAX;
static double fs_libsvm_y_min                      = DBL_MAX;
static int fs_libsvm_y_scaling                     = 0;

static int fs_libsvm_max_line_len = 1024;
static int fs_libsvm_max_nr_attr = 64;

static char *fs_libsvm_scale_line = NULL;
/* -------------------------------------------------------------------------- */

static int libsvm_read_problem(struct svm_problem *,struct svm_parameter *,struct svm_node *,const char *);
static int libsvm_predict(FILE *, FILE *, struct svm_model *,struct svm_node **);
static int  libsvm_scale_data(const char *, const char *, const char *,int);
static int libsvm_output_target(double, FILE *);
static void libsvm_output(int, double, const double *, const double  *, FILE *);
static char *libsvm_readline( FILE *);
/* -------------------------------------------------------------------------- */

#endif

int set_libsvm_options(const char* option, const char* value)
{
    int  result         = NOT_FOUND;
#ifdef KJB_HAVE_LIBSVM
    char lc_option[ 100 ];
    int  temp_int_value;
    double temp_double_value;

    EXTENDED_LC_BUFF_CPY(lc_option, option);

    if (    (lc_option[ 0 ] == '\0')
            || match_pattern(lc_option, "svm-type")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The svm type is %d by default.\n",
                    fs_libsvm_svm_type));

        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("svm-type = %d\n", fs_libsvm_svm_type));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_libsvm_svm_type = temp_int_value;
        }
        result = NO_ERROR;
    }
    if (    (lc_option[ 0 ] == '\0')
            || match_pattern(lc_option, "svm-kernel-type")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The kernel type is %d .\n",
                    fs_libsvm_kernel_type));

        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("kernel-type = %d\n", fs_libsvm_kernel_type));
        }
        else
        {
            ERE(ss1pi(value, &temp_int_value));
            fs_libsvm_kernel_type = temp_int_value;
        }
        result = NO_ERROR;
    }
    if (    (lc_option[ 0 ] == '\0')
            || match_pattern(lc_option, "svm-epsilon-tolerance")
       )
    {
        if (value == NULL)
        {
            return NO_ERROR;
        }
        else if (value[ 0 ] == '\0')
        {
            ERE(pso("The svm epsilon tolerance is %.4g .\n",
                    fs_libsvm_svm_type));

        }
        else if (value[ 0 ] == '?')
        {
            ERE(pso("svm-epsilon-tolerance = %d\n", fs_libsvm_epsilon_tolerance));
        }
        else
        {
            ERE(ss1d(value, &temp_double_value));
            fs_libsvm_epsilon_tolerance = temp_double_value;
        }
        result = NO_ERROR;
    }
#endif

    return result;
}


#ifndef KJB_HAVE_LIBSVM
/* -----------------------------------------------------------------------------
|                              no LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/
static void set_dont_have_libsvm_error(void)
{
    set_error("Operation failed because the program was built without ");
    cat_error("the LIBSVM interface.");
    add_error("(Appropriate re-compiling is needed to fix this.)");
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no LIBSVM
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int have_libsvm(void)
{
#ifdef KJB_HAVE_LIBSVM
    return TRUE;
#else
    return FALSE;
#endif
}
/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/* =============================================================================
 *                             do_libsvm_svm_computation
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

#ifdef KJB_HAVE_LIBSVM
/* -----------------------------------------------------------------------------
|                              KJB_HAVE_LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/

int do_libsvm_svm_computation
(
    const char* training_file_name,
    const char* model_file_name,
    const char* scale_path
)
{
    int                  result  = NO_ERROR;
    struct svm_parameter param;
    struct svm_problem   prob;
    struct svm_node*     x_space = NULL;
    struct svm_model*    model   = NULL;
    char                 scaled_training_file[ MAX_FILE_NAME_SIZE];
    const char*          error_msg = NULL;

    param.svm_type = fs_libsvm_svm_type;
    param.kernel_type = fs_libsvm_kernel_type;
    param.degree = fs_libsvm_kernel_degree;
    param.gamma =  fs_libsvm_kernel_gamma ;    /* 1/k*/
    param.coef0 = fs_libsvm_kernel_coeff;
    param.nu = fs_libsvm_nu ;
    param.cache_size = fs_libsvm_cache_size ;
    param.C = fs_libsvm_cost;
    param.eps = fs_libsvm_epsilon_tolerance;
    param.p = fs_libsvm_epsilon_loss_fn       ;
    param.shrinking = fs_libsvm_shrink_heuristic;
    param.probability = fs_libsvm_prob_estimate;

    /*Ranjini : deal with weight labels and validation */
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;


    if(fs_libsvm_scale_data == TRUE)
    {

        ERE(kjb_sprintf(scaled_training_file, sizeof(scaled_training_file),
                        "%s%s", training_file_name,"_scaled"));



        result =  libsvm_scale_data(training_file_name, scaled_training_file,scale_path ,1);
        if(result != NO_ERROR)
        {
            return result;
        }
    }

    libsvm_read_problem(&prob,&param,x_space,scaled_training_file);

    error_msg = svm_check_parameter(&prob,&param);

    if(error_msg)
    {
        pso("Error: %s\n",error_msg);
        svm_destroy_param(&param);
        free(prob.y);
        free(prob.x);
        free(x_space);

        exit(1);
    }
    model = svm_train(&prob,&param);
    svm_save_model(model_file_name,model);
    svm_destroy_model(model);
    svm_destroy_param(&param);

    free(prob.y);
    free(prob.x);
    free(x_space);

    return result;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              KJB_HAVE_LIBSVM
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/

int do_libsvm_svm_computation
(
    const char* __attribute__((unused)) dummy_training_file_name,
    const char* __attribute__((unused)) dummy_model_file_name,
    const char* __attribute__((unused)) dummy_scale_path
)
{


    set_dont_have_libsvm_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no LIBSVM
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


/* =============================================================================
 *                             do_libsvm_svm_prediction
 *
 *
 *
 * -----------------------------------------------------------------------------
*/

#ifdef KJB_HAVE_LIBSVM
/* -----------------------------------------------------------------------------
|                              KJB_HAVE_LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/

int do_libsvm_svm_prediction
(
    const char* input_file_name,
    const char* model_file_name,
    const char* prediction_file_name,
    const char* scale_path
)
{
    int result = NO_ERROR;
    int predict_probability=0;
    char* line;
    FILE *input, *output;
    char scaled_input_file[ MAX_FILE_NAME_SIZE];
    struct svm_model *model;
    struct svm_node *x;

    predict_probability = fs_libsvm_prob_estimate;

    if(fs_libsvm_scale_data == TRUE)
    {

        ERE(kjb_sprintf(scaled_input_file, sizeof(scaled_input_file),"%s%s", input_file_name,"_scaled"));

        result =  libsvm_scale_data(input_file_name, scaled_input_file,scale_path, 0);
        if(result != NO_ERROR)
        {
            return result;
        }
        NRE(input = kjb_fopen(scaled_input_file,"r"));
    }
    else
    {
        NRE(input = kjb_fopen(input_file_name,"r"));
    }
    if(input == NULL)
    {
        pso("can't open input file %s\n",input_file_name);
        exit(1);
    }

    NRE(output = kjb_fopen(prediction_file_name,"w"));
    if(output == NULL)
    {
        pso("can't open output file %s\n",prediction_file_name);
        exit(1);
    }

    if((model=svm_load_model(model_file_name))==0)
    {
        pso("can't open model file %s\n",model_file_name);
        exit(1);
    }

    NRE(line = (char *) kjb_malloc(fs_libsvm_max_line_len*sizeof(char)));
    x = (struct svm_node *) malloc(fs_libsvm_max_nr_attr*sizeof(struct svm_node));

    if(predict_probability)
        if(svm_check_probability_model(model)==0)
        {
            pso("model does not support probabiliy estimates\n");
            predict_probability=0;
        }
    libsvm_predict(input,output,model,&x);
    svm_destroy_model(model);
    kjb_free(line);

    free(x);

    ERE(kjb_fclose(input));
    ERE(kjb_fclose(output));

    return result;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              KJB_HAVE_LIBSVM
----------------------------------------------------------------------------- */
#else
/* -----------------------------------------------------------------------------
|                              no LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/

int do_libsvm_svm_prediction
(
    const char* __attribute__((unused)) dummy_input_file_name,
    const char* __attribute__((unused)) dummy_model_file_name,
    const char* __attribute__((unused)) dummy_prediction_file_name,
    const char* __attribute__((unused)) dummy_scale_path
)
{


    set_dont_have_libsvm_error();
    return ERROR;
}
/*
|                                  /\
|                                 /||\
|                                  ||
|                              no LIBSVM
----------------------------------------------------------------------------- */
#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef KJB_HAVE_LIBSVM
/* -----------------------------------------------------------------------------
|                              KJB_HAVE_LIBSVM
|                                  ||
|                                 \||/
|                                  \/
*/

static int libsvm_read_problem
(
  struct svm_problem *prob,
  struct svm_parameter *param,
  struct svm_node *x_space,
  const char *filename
)
{
    int elements, max_index, i, j;
    FILE *fp ;

    NRE(fp= kjb_fopen(filename,"r"));

    if(fp == NULL)
    {
        fprintf(stderr,"can't open input file %s\n",filename);
        exit(1);
    }

    prob->l = 0;
    elements = 0;
    while(1)
    {
        int c = fgetc(fp);
        switch(c)
        {
            case '\n':
                ++prob->l;
                /*fall through,
                  count the '-1' element*/
            case ':':
                ++elements;
                break;
            case EOF:
                goto out;
            default:
                ;
        }
    }
out:
    rewind(fp);

    prob->y = LIBSVM_MALLOC(double,prob->l);
    prob->x = LIBSVM_MALLOC(struct svm_node *,prob->l);
    x_space = LIBSVM_MALLOC(struct svm_node,elements);

    max_index = 0;
    j=0;
    for(i=0;i<prob->l;i++)
    {
        double label;
        prob->x[i] = &x_space[j];
        fscanf(fp,"%lf",&label);
        prob->y[i] = label;
        while(1)
        {
            int c;
            do {
                c = getc(fp);
                if(c=='\n') goto out2;
            } while(isspace(c));
            ungetc(c,fp);
            fscanf(fp,"%d:%lf",&(x_space[j].index),&(x_space[j].value));
            ++j;
        }   
out2:
        if(j>=1 && x_space[j-1].index > max_index)
            max_index = x_space[j-1].index;
        x_space[j++].index = -1;
    }

    if(param->gamma == 0)
        param->gamma = 1.0/max_index;

    ERE(kjb_fclose(fp));

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int libsvm_predict
(
    FILE*             input,
    FILE*             output,
    struct svm_model* model,
    struct svm_node** x_node_pp 
)
{
    int correct = 0;
    int total = 0;
    double error = 0;
    double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;

    int svm_type=svm_get_svm_type(model);
    int nr_class=svm_get_nr_class(model);
    int *labels;
    double *prob_estimates=NULL;
    int j;
    struct svm_node *x = *(x_node_pp);


    NRE(labels=(int *) kjb_malloc(nr_class*sizeof(int)));
    if(fs_libsvm_prob_estimate)
    {
        if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
            pso("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(model));
        else
        {
            svm_get_labels(model,labels);
            NRE(prob_estimates = (double *) kjb_malloc(nr_class*sizeof(double)));
            /* Ranjini :
            // Commented this line out so that the predictions file has only blob labels
            // and no other text
             */
            /* kjb_fprintf(output,"labels");      
               for(j=0;j<nr_class;j++)
               kjb_fprintf(output," %d",labels[j]);
               kjb_fprintf(output,"\n");*/
        }
    }
    while(1)
    {
        int i = 0;
        int c;
        double target,v;

        if (fscanf(input,"%lf",&target)==EOF)
            break;

        while(1)
        {
            if(i>=fs_libsvm_max_nr_attr-1)    /*need one more for index = -1*/
            {
                fs_libsvm_max_nr_attr *= 2;

                x = (struct svm_node *) realloc(x,fs_libsvm_max_nr_attr*sizeof(struct svm_node));
            }

            do {
                c = getc(input);
                if(c=='\n' || c==EOF) goto out2;
            } while(isspace(c));
            ungetc(c,input);
            fscanf(input,"%d:%lf",&x[i].index,&x[i].value);
            ++i;
        }   

out2:
        x[i++].index = -1;

        if (fs_libsvm_prob_estimate && (svm_type==C_SVC || svm_type==NU_SVC))
        {
            v = svm_predict_probability(model,x,prob_estimates);
            ERE(kjb_fprintf(output,"%g ",v));
            ERE(kjb_fflush(output));
            for(j=0;j<nr_class;j++)
            {
                ERE(kjb_fprintf(output,"%g ",prob_estimates[j]));
                ERE(kjb_fflush(output));
            }
            ERE(kjb_fprintf(output,"\n"));
            ERE(kjb_fflush(output));
        }
        else
        {
            v = svm_predict(model,x);
            ERE(kjb_fprintf(output,"%g\n",v));
        }

        if(v == target)
            ++correct;
        error += (v-target)*(v-target);
        sumv += v;
        sumy += target;
        sumvv += v*v;
        sumyy += target*target;
        sumvy += v*target;
        ++total;
    }
    pso("Accuracy = %g%% (%d/%d) (classification)\n",
        (double)correct/total*100,correct,total);
    pso("Mean squared error = %g (regression)\n",error/total);
    pso("Squared correlation coefficient = %g (regression)\n",
        ((total*sumvy-sumv*sumy)*(total*sumvy-sumv*sumy))/
        ((total*sumvv-sumv*sumv)*(total*sumyy-sumy*sumy))
       );

    if(fs_libsvm_prob_estimate)
    {
        kjb_free(prob_estimates);
        kjb_free(labels);
    }

    *(x_node_pp) = x;

    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int libsvm_scale_data
(
    const char* input_file_name,
    const char* scaled_input_file_name,
    const char* scale_path,
    int         write_restore_file 
)
{
    int result = NO_ERROR;

    int i;


    char *p;
    char scale_restore_file [  MAX_FILE_NAME_SIZE ];
    char dir_str[  MAX_FILE_NAME_SIZE ];
    char base_name [  MAX_FILE_NAME_SIZE ];
    char suffix [  MAX_FILE_NAME_SIZE ];
    static const char* suffixes[ ] = {"dat"};

    int next_index=1;
    double target;
    double value;

    FILE *fp ;
    int idx, c;
    double fmin, fmax;

    int index;

    double *feature_max = NULL;
    double *feature_min = NULL;

    int max_index;
    FILE *input_fp;
    FILE *output_fp;

    NRE(input_fp = kjb_fopen(input_file_name,"r"));

    if(input_fp == NULL)
    {
        pso("Cannot open train/test file\n");
        return ERROR;
    }

    ERE(kjb_sprintf(scale_restore_file, sizeof(scale_restore_file),"%s%s%s", scale_path,DIR_STR,"scale_file"));
    /*  ERE(get_base_name(input_file_name,dir_str,sizeof(dir_str),base_name,sizeof(base_name),suffix,sizeof(suffix),suffixes));

        ERE(kjb_sprintf(scale_restore_file, sizeof(scale_restore_file),
        "%s%s%s",dir_str,DIR_STR,"scale_file"));
     */

    fs_libsvm_scale_line = (char *) malloc(fs_libsvm_max_line_len*sizeof(char));

#define SKIP_TARGET\
    while(isspace(*p)) ++p;\
        while(!isspace(*p)) ++p;

#define SKIP_ELEMENT\
    while(*p!=':') ++p;\
        ++p;\
            while(isspace(*p)) ++p;\
                while(*p && !isspace(*p)) ++p;

    /* assumption: min index of attributes is 1 */
    /* pass 1: find out max index of attributes */

    max_index = 0;

    while(libsvm_readline(input_fp) != NULL)
    {
        p = fs_libsvm_scale_line;

        SKIP_TARGET

            while(sscanf(p,"%d:%*f",&index)==1)
            {
                max_index = libsvm_max(max_index, index);
                SKIP_ELEMENT
            }       

    }

    feature_max = (double *)kjb_malloc((max_index+1)* sizeof(double));
    feature_min = (double *)kjb_malloc((max_index+1)* sizeof(double));


    for(i = 0; i <= max_index; i++)
    {
        feature_max[i] = -DBL_MAX;
        feature_min[i] = DBL_MAX;
    }

    rewind(input_fp);

    /* pass 2: find out min/max value */

    while(libsvm_readline(input_fp) != NULL)
    {
        p = fs_libsvm_scale_line;
        next_index = 1;


        if(sscanf(p,"%lf",&target)==EOF)
        {
            pso("ERROR Reading input file while in routine libsvm_scale_data\n");
        }
        fs_libsvm_y_max = libsvm_max(fs_libsvm_y_max,target);
        fs_libsvm_y_min = libsvm_min(fs_libsvm_y_min,target);

        SKIP_TARGET

            while(sscanf(p,"%d:%lf",&index,&value) == 2)
            {
                for(i = next_index;i < index;i++)
                {
                    feature_max[i] = libsvm_max(feature_max[i],0);
                    feature_min[i] = libsvm_min(feature_min[i],0);
                }

                feature_max[index] = libsvm_max(feature_max[index],value);
                feature_min[index] = libsvm_min(feature_min[index],value);

                SKIP_ELEMENT
                    next_index = index + 1;
            }       

        for(i = next_index; i <= max_index; i++)
        {
            feature_max[i] = libsvm_max(feature_max[i],0);
            feature_min[i] = libsvm_min(feature_min[i],0);
        }   

    }

    rewind(input_fp);

    /* pass 2.5: save/restore feature_min/feature_max */

    if(!write_restore_file)
    {

        NRE(fp = kjb_fopen(scale_restore_file,"r"));


        if(fp == NULL)
        {
            warn_pso("Cannot open file with scale values\n");

        }
        else
        {
            if((c = fgetc(fp)) == 'y')
            {
                fscanf(fp, "%lf %lf\n", &fs_libsvm_y_lower, &fs_libsvm_y_upper);
                fscanf(fp, "%lf %lf\n", &fs_libsvm_y_min, &fs_libsvm_y_max);
                fs_libsvm_y_scaling = 1;
            }
            else
                ungetc(c, fp);

            if (fgetc(fp) == 'x') {
                fscanf(fp, "%lf %lf\n", &fs_libsvm_lower, &fs_libsvm_upper);
                while(fscanf(fp,"%d %lf %lf\n",&idx,&fmin,&fmax)==3)
                {
                    if(idx<=max_index)
                    {
                        feature_min[idx] = fmin;
                        feature_max[idx] = fmax;
                    }
                }
            }
            ERE(kjb_fclose(fp));
        }
    }

    if(write_restore_file)
    {
        NRE(fp = kjb_fopen(scale_restore_file,"w"));
        if(fp==NULL)
        {
            pso("Cannot open file( save_file for scales)\n");
            return ERROR;
        }
        if(fs_libsvm_y_scaling)
        {
            ERE(kjb_fprintf(fp, "y\n"));
            ERE( kjb_fprintf(fp, "%.16g %.16g\n", fs_libsvm_y_lower, fs_libsvm_y_upper));
            ERE(kjb_fprintf(fp, "%.16g %.16g\n", fs_libsvm_y_min, fs_libsvm_y_max));
        }
        ERE(kjb_fprintf(fp, "x\n"));
        ERE(kjb_fprintf(fp, "%.16g %.16g\n", fs_libsvm_lower, fs_libsvm_upper));

        for(i = 1; i <= max_index; i++)
        {
            if(feature_min[i] != feature_max[i])
            {
                ERE(kjb_fprintf(fp,"%d %.16g %.16g\n",i,feature_min[i],feature_max[i]));
            }
        }
        ERE(kjb_fclose(fp));
    }

    /* pass 3: scale */

    NRE(output_fp = kjb_fopen(scaled_input_file_name,"w"));

    if(output_fp == NULL)
    {
        warn_pso("Cannot open file for scaling train/test data. Possible bug\n");

    }

    while(libsvm_readline(input_fp) != NULL)
    {
        p = fs_libsvm_scale_line;
        next_index=1;



        if(sscanf(p,"%lf",&target) == EOF)
        {
            pso("ERROR : Scanning scale file\n");
            exit(1);
        }
        libsvm_output_target(target,output_fp);

        SKIP_TARGET

            while(sscanf(p,"%d:%lf",&index,&value)==2)
            {
                for(i = next_index; i < index; i++)
                {
                    libsvm_output(i,0, feature_max, feature_min, output_fp);
                }

                libsvm_output(index,value, feature_max, feature_min, output_fp);

                SKIP_ELEMENT
                    next_index=index+1;
            }


        for(i = next_index; i <= max_index; i++)
        {
            libsvm_output(i,0, feature_max, feature_min, output_fp);
        }

        ERE(kjb_fprintf(output_fp,"\n"));
    }

    free(fs_libsvm_scale_line);
    kjb_free(feature_max);
    kjb_free(feature_min);
    kjb_fclose(input_fp);
    kjb_fclose(output_fp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static char * libsvm_readline
(
 FILE *input_fp
)
{
  int len;
    
  if(fgets(fs_libsvm_scale_line,fs_libsvm_max_line_len,input_fp) == NULL)
  {
    return NULL;
  }

  while(strrchr(fs_libsvm_scale_line,'\n') == NULL)
  {
    fs_libsvm_max_line_len *= 2;
    fs_libsvm_scale_line = (char *) realloc(fs_libsvm_scale_line,fs_libsvm_max_line_len);
    len = strlen(fs_libsvm_scale_line);
    if(fgets(fs_libsvm_scale_line + len,fs_libsvm_max_line_len - len,input_fp) == NULL)
      break;
  }
  return fs_libsvm_scale_line;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int libsvm_output_target(double value, FILE* fp)
{
    if(fs_libsvm_y_scaling)
    {
        if(value == fs_libsvm_y_min)
            value = fs_libsvm_y_lower;
        else if(value == fs_libsvm_y_max)
            value = fs_libsvm_y_upper;
        else value = fs_libsvm_y_lower + (fs_libsvm_y_upper - fs_libsvm_y_lower) *
            (value - fs_libsvm_y_min)/(fs_libsvm_y_max - fs_libsvm_y_min);
    }
    
    ERE( kjb_fprintf(fp,"%g ",value));
    
    return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static void libsvm_output
(
    int           index,
    double        value,
    const double* feature_max,
    const double* feature_min,
    FILE*         fp 
)
{
    /* skip single-valued attribute */
    if(feature_max[index] == feature_min[index])
    {
        return;
    }

    if(value == feature_min[index])
    {
        value = fs_libsvm_lower;
    }
    else if(value == feature_max[index])
    {
        value = fs_libsvm_upper;
    }
    else
    {
        value = fs_libsvm_lower + (((fs_libsvm_upper - fs_libsvm_lower) * (value-feature_min[index]))/
                                   ((feature_max[index]-feature_min[index])));
    }

    if(value != 0)
    {
        if(kjb_fprintf(fp, "%d:%g ",index, value)== ERROR)
        {
            pso("Error encountered in file wrap_svm_libsvm.c ,routine libsvm_output\n");
            exit(1);
        }
    }

    return;
}

#endif

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


