/**
 * @file demo for multithreaded convolution of images
 * @author Andrew Predoehl
 */
/*
 * $Id: test_mt_fft.cpp 15752 2013-10-20 23:26:00Z predoehl $
 */

#include <i/i_convolve.h>
#include <l_mt_cpp/l_mt_mutexlock.h>
#include <i_cpp/i_collage.h>
#include <l_cpp/l_word_list.h>
#include <i_cpp/i_mt_convo.h>
#include <i_cpp/i_hsv.h>

namespace 
{

const int MASK_SIZE = 100;
const int IMAGE_SIZE = 500;

const double sigma = 20;

kjb::Fftw_image_convolution* pcon = 0;

kjb_c::kjb_pthread_mutex_t mtx = KJB_PTHREAD_MUTEX_INITIALIZER;

// This is what each worker thread needs:
struct Convolution_context
{
    const kjb::Image *input;                 // const pointer to input image
    kjb::Image *output;                      // pointer to output image
};


// This is the function each worker thread enters
void* thread_workbench(void *v)
{
    using namespace kjb;
    using namespace kjb_c;

    Convolution_context* cc = static_cast<Convolution_context*>(v);
    NRN(pcon);
    NRN(cc);

    // sequentially get work buffer
    Fftw_convolution_2d::Work_buffer wb;
    if(1) { Mutex_lock l(&mtx); wb = pcon -> allocate_work_buffer(); } 

    // convolve -- the slowest step (except maybe for the print statements)
    pcon -> reflect_and_convolve(*(cc -> input), *(cc -> output), wb);

    // sequentially kill the work buffer by clobbering its contents
    if (!work_buffer_is_unique(wb)) {
        set_error("wb not unique in thread %d", get_kjb_pthread_number());
        return NULL;
    }
    if(1) { Mutex_lock l(&mtx); wb = Fftw_convolution_2d::Work_buffer(); }

    return v;
}


int work()
{
    std::vector<kjb::Image> queue, blur_1, blur_2;

    // Expecting working directory to be .../src/lib/i_cpp/test/interactive
    kjb::Word_list wlj("../input/*.jpeg"), wlt("../input/*.tiff"), *w[2];
    w[0] = &wlj;
    w[1] = &wlt;

    // read dorky sample images from input directory; force them to be one size
    for (size_t j = 0; j < 2; ++j)
    {
        for (size_t i = 0; i < w[j] -> size(); ++i)
        {
            kjb::Image f(w[j] -> operator[](i));
            // random background color
            const kjb_c::Pixel p(kjb::PixelHSVA(kjb_c::kjb_rand(), .8, .8));
            queue.push_back(
                kjb::Image::create_initialized_image(IMAGE_SIZE, IMAGE_SIZE,
                        p.r, p.g, p.b));
            queue.back().draw_image(f, 0, 0);
        }
    }

    // blur them the old fashioned way, for reference
    for (size_t i = 0; i < queue.size(); ++i)
    {
        kjb_c::KJB_image *b = NULL;
        KJB(EPETE(gauss_convolve_image(&b, queue[i].c_ptr(), sigma)));
        blur_1.push_back(kjb::Image(b));
    }

    // blur them fast way . . .
    blur_2.resize(queue.size());

    kjb::Fftw_image_convolution con(IMAGE_SIZE, IMAGE_SIZE,
                                                        1+6*sigma, 1+6*sigma);
    con.set_gaussian_mask(sigma);
    pcon = &con;

    std::vector <kjb_c::kjb_pthread_t> vt(queue.size());
    std::vector < Convolution_context > vb(queue.size());

    // allocate a work buffer for each thread, plus a few pointers 
    for (size_t i = 0; i < queue.size(); ++i)
    {
        vb[i].input = &queue[i];
        vb[i].output = &blur_2[i];
    }

    // launch threads
    for (size_t i = 0; i < queue.size(); ++i)
    {
        KJB(ERE(kjb_pthread_create(&vt[i], NULL, thread_workbench, &vb[i])));
    }

    // wait for them to finish
    for (size_t i = 0; i < queue.size(); ++i)
    {
        void *v;
        KJB(ERE(kjb_pthread_join(vt[i], &v)));
        KJB(NRN(v));
    }
    pcon = 0;

    // display the blurred image
    std::vector<kjb::Image*> sharp, bc1, bc2;
    for (size_t i = 0; i < queue.size(); ++i)
    {
        sharp.push_back(&queue[i]);
        bc1.push_back(&blur_1[i]);
        bc2.push_back(&blur_2[i]);
    }
    const kjb::Image sc(kjb::make_collage(&sharp.front(), 1, queue.size())),
                    gc1(kjb::make_collage(&bc1.front(), 1, queue.size())),
                    gc2(kjb::make_collage(&bc2.front(), 1, queue.size()));

    sc.display("input images");
    gc1.display("reference blur");
    gc2.display("fast blur");
    while(1) kjb_c::nap(1000);

    /* NOTREACHED */
    return kjb_c::NO_ERROR;
}

}

int main(int argc, char *argv[])
{
    try
    {
        KJB(EPETE(work()));
    }
    catch (const kjb::Exception& e)
    {
        e.print_details_exit();
    }
}

