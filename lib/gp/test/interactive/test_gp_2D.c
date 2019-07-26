#include "l/l_incl.h"
#include "p/p_incl.h"
#include "sample/sample_incl.h"
#include "gp/gp_incl.h"

#define PLOT_POINTS 1
#define PLOT_LINES 2

void generate_indices(Vector_vector** X, int n, double t_0)
{
    int i;

    get_target_vector_vector(X, n);
    for(i = 0; i < n; i++)
    {
        get_initialized_vector(&(*X)->elements[i], 1, t_0 + i);
    }
}

void generate_data(Vector_vector** y, Vector_vector** f, const Vector_vector* X, double sigma)
{
    double u = 0.1;
    int n = X->length;
    int i;

    get_target_vector_vector(y, n);
    get_target_vector_vector(f, n);
    for(i = 0; i < n; i++)
    {
        get_target_vector(&(*y)->elements[i], 2);
        get_target_vector(&(*f)->elements[i], 2);

        (*f)->elements[i]->elements[0] = u * X->elements[i]->elements[0] * sin(u * X->elements[i]->elements[0]);
        (*f)->elements[i]->elements[1] = u * X->elements[i]->elements[0] * cos(u * X->elements[i]->elements[0]);

        gaussian_rand(&(*y)->elements[i]->elements[0], (*f)->elements[i]->elements[0], sigma);
        gaussian_rand(&(*y)->elements[i]->elements[1], (*f)->elements[i]->elements[1], sigma);
    }
}

void plot(int plot_id, const Vector_vector* xs, int type, const char* title)
{
    Vector_vector* x = NULL;

    get_vector_vector_transpose(&x, xs);

    if(type == PLOT_POINTS)
    {
        plot_points(plot_id, x->elements[0], x->elements[1], title);
    }
    else
    {
        plot_curve(plot_id, x->elements[0], x->elements[1], title);
    }

    free_vector_vector(x);
}

int main(int argc, char** argv)
{
    int n = 100;
    double l = 7.0;
    double sigma = 0.1;
    int plot_id;

    Vector_vector* X = NULL;
    Vector_vector* X_s = NULL;
    Vector_vector* y = NULL;
    Vector_vector* f = NULL;
    Vector_vector* f_prior = NULL;
    Vector_vector* f_s = NULL;
    Vector_vector* f_post = NULL;
    Vector* mu = NULL;

    kjb_init();

    if (! is_interactive()) 
    {
        p_stderr("This test program either needs to be adjusted for batch testing or moved.\n");
        p_stderr("Forcing failure to help increase the chances this gets done.\n"); 
        return EXIT_CANNOT_TEST;
    }
    
    kjb_seed_rand_with_tod();

    /*-----------------------------------------------------*/
    /*               Generate data and indices             */
    /*             ||       ||       ||       ||           */
    /*             \/       \/       \/       \/           */
    /*-----------------------------------------------------*/

    generate_indices(&X, n, 1.0);
    generate_indices(&X_s, n, 1.5);
    generate_data(&y, &f, X, sigma);


    /*-----------------------------------------------------*/
    /*                      Do GP stuff                    */
    /*             ||       ||       ||       ||           */
    /*             \/       \/       \/       \/           */
    /*-----------------------------------------------------*/

    sample_from_gaussian_process_prior(&f_prior, X_s, zero_mean_function, squared_exponential_covariance_function, &l, 2);
    sample_from_gaussian_process_predictive(&f_s, X, y, sigma, X_s, squared_exponential_covariance_function, &l);
    get_gaussian_process_posterior_distribution(&mu, NULL, X, y, sigma, squared_exponential_covariance_function, &l);
    vector_vector_from_vector(&f_post, mu, 2);


    /*-----------------------------------------------------*/
    /*                      Plot stuff                     */
    /*             ||       ||       ||       ||           */
    /*             \/       \/       \/       \/           */
    /*-----------------------------------------------------*/

    plot_id = plot_open();

    plot(plot_id, f, PLOT_LINES, "f");
    plot(plot_id, y, PLOT_POINTS, "y");
    plot(plot_id, f_prior, PLOT_LINES, "prior");
    plot(plot_id, f_s, PLOT_POINTS, "f*");
    plot(plot_id, f_post, PLOT_POINTS, "post");

    prompt_to_continue();

    plot_close(plot_id);

    free_vector_vector(X);
    free_vector_vector(X_s);
    free_vector_vector(y);
    free_vector_vector(f);
    free_vector_vector(f_prior);
    free_vector_vector(f_s);
    free_vector_vector(f_post);
    free_vector(mu);

    return EXIT_SUCCESS;
}

