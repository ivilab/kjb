/*
omap_computer.h

@author - Josh Bowdish - original Matlab code by David C. Lee

omap_computer is the base for all the functions under compute_omap.m

function tree for compute_omap:

compute_orientationmap
    orient_from_lines
        sample_line
        extend_line
        move_line_towards_vp
        sample_line
*/

#ifndef omap_computer
#define omap_computer

//includes!
#include <edge_cpp/line_segment.h>
#include "i_cpp/i_image.h"
#include "m_cpp/m_vector.h"
#include "m_cpp/m_matrix.h"
#include "g_cpp/g_util.h"
#include "g/g_geometry.h"
#include <edge_cpp/features_manager.h>

namespace kjb
{

class Omap_segment{ 

    private:

        //variables
        double n_sample;//the "length" of the line
        std::vector<kjb::Vector> sample;//A vector of spacings on the line (linspace)
        int lineclass;//which vanishing point the line is associated with (0-2)
        Vanishing_point vp;

        Vector startpoint;
        Vector endpoint;
        
    public:
        //constructors
        Omap_segment();
        Omap_segment(Manhattan_segment seg);
        Omap_segment(Vector startpoint,Vector endpoint);
        Omap_segment(Vector startpoint,Vector endpoint,int lineclass);
        Omap_segment(double x1,double y1, double x2,double y2);
        Omap_segment(double x1,double y1, double x2,double y2,int lineclass);
        
        //methods that do
        void develop_sample();

        //setters
        void set_lineclass(int lc);
        void set_vp(const Vanishing_point & vanishingp);
        //getters
        double get_n_sample(){ return n_sample; }
        std::vector<kjb::Vector> get_sample(){ return sample; } 
        int get_lineclass(){ return lineclass; } 
        kjb::Vector get_start_point(){ return startpoint; }
        kjb::Vector get_end_point(){ return endpoint; }
        double get_start_x();
        double get_start_y();
        double get_end_x();
        double get_end_y();
        Vanishing_point get_vp(){ return vp; }

}; //end class Omap_segment

class Omap_computer
{
public:
    enum Orientation_map_type {
          OMAP_LOOSE = 0,
          OMAP_STRICT,
          OMAP_VERY_STRICT,
          OMAP_COUNT
    };
    private:
    
    //variables
    bool using_featman_lines;
    bool delete_features;
    
    std::string imagename;   
    std::string dname;
    Features_manager * featman;
    std::vector<Vanishing_point> vpts;
    double focal_length;
    const Manhattan_world * mw;
    double imgsize;
    int imageheight;
    int imagewidth;
    unsigned numlines;
    std::vector<Omap_segment> lines;    
    std::vector<std::vector<std::vector<Matrix> > > lineextimg; 

    std::vector<Omap_segment> lines_at_1;
    std::vector<Omap_segment> lines_at_2;
    std::vector<Omap_segment> lines_at_3;

    //for extend_line
    //  std::vector<kjb::Vector> get_sample(){ return sample; } 
    std::vector<kjb::Vector> mastersamples;
    std::vector<int> mastersamplelc;
    std::vector<kjb::Vector> samples_at_0; //samples of lines associated with vp 0
    std::vector<kjb::Vector> samples_at_1;
    std::vector<kjb::Vector> samples_at_2;
    std::vector<kjb::Vector> samples_at_3;

    unsigned sample_rate;

    //methods

    /*
        IN PROGRESS
        Takes the lines and associates them with the vanishing points it thinks are appropriate
        Sets up the lines variable (of Omap_segments)/
    */
    void taglinesvp(){

    }
    /*
        orient_from_lines() - Only called once there is a set of lines to orient_from. 
        This function fills in the lineextimg variable. It goes through each line and 
        tries to see how much area it can describe the orientation of using extend_line().
        It adds that to a mask, which is layered on top of the appropriate section of 
        lineextimg. 
    */
    std::vector<std::vector<std::vector<Matrix> > > orient_from_lines(); //[][][] where each [] is a pair
                                  //[][][] of  imgheight,imgwidth
                                  //[][][] matrix of line overlaps
                                  //(its [3][3][2])

    /*
        sample_line() - This method is only called once. Goes through each of the lines
        found in the image and develops the "std::vector<Omap_segment> lines" variable
        For each line attached to an actual VP, it calculates 
        lines(i).n_sample - length
        lines(i).sample - linspace(startpoint,endpoint)
        lines(i).lineclass - which vp it's attached to (0-2)
    */
    void sample_line();

    /*
        extend_line() - Called twice for every line. Takes the line and attempts to
        extend its area (creating a trapezoid) toward the vanishing point. It stops
        if the line does one of several things:
        1) if the extension goes beyond the edge of the image
        2) if the extension hits the vanishing point
        3) if the extension intersects (approximately) any of the lines that point 
        toward the other vanishing point (The one that isn't being extended to).
 
    */  
    Matrix extend_line
    (
        Omap_segment line,
        Vanishing_point vp,
        int toward_or_away,
        std::vector<kjb::Vector> sample,
        int imgheight,
        int imgwidth
    );
    
    /* private copy constructor */
    Omap_computer(const Omap_computer & src)
    {
    	(*this) = src;
    }

    /** Assignment operator is private so that we cannot assign this class */
    Omap_computer & operator=(const Omap_computer & /* src */)
    {
        return (*this);
    }

    public:

    ~Omap_computer()
    {
    	if(delete_features)
    	{
    		delete featman;
    	}
    }

    //constructors

    /*
        Takes a filepath to an already computed feature file for the image. It will
        then compute from the features. It has no reference to the actual image.
        Note: Preferred/fastest constructor 
    
        @param - filepath to the pre-computed features
    */
    Omap_computer
    (
    	const char* featurepath,
    	const char* filename
    );
    
    /*
        Takes the file with lines, the file with the vp and the image file (mostly to
        make this constructor different from the 2 (const char*) constructor). It will
        then compute from the lines and vps given to it.

        Edit: included featurepath because it was giving me an error for invoking 
        features_manager's blank constructor... even though I didn't
        Note: takes the files I built from matlab, need to be of specific construction
    */
    Omap_computer
    (
    	const char* linepath,
    	const char* vppath,
    	const char* featurepath,
    	const char* filename
    );

    /*
        Calls the const char* one when the input is a little different
        WARNING: Doesn't work with bestfeatures.txt files yet. (or anything else for that matter)
        Out of commission until I understand why it doesn't work.
    */
    Omap_computer(const std::string featurepath);
    
    /*
        Takes the image object (not the filepath) and constructs a featuremanager from
        it.
        Note: takes a while to extract lines and things.

        @param - Image object to analyse
    */

    Omap_computer(const Image & img);

    Omap_computer(Features_manager * ifm);

    //methods that do calculations 

    /*
        @param - point1 - start point of the line
             point2 - end point of the line
             vp - the vanishing point you want to extend toward
             amount - the distance to attempt
    */
    std::vector<double> move_line_towards_vp
    ( // public for now
        Vector point1,
        Vector point2,
        Vanishing_point vp,
        double amount
    );

    /*
        Initialises lineextimg...
        calls orient_from_lines()...
        once lineextimg is filled out, computes the actual orientation maps by
        and-ing/or-ing variations of lineextimg's parts.
    */  
    void compute_omap
    (
		Image & out_omap,
	    Orientation_map_type omap_type
    );

    //getters
    Edge_segment_set get_edge_segments();
    std::vector<Omap_segment> get_lines();
    std::vector<Vanishing_point>  get_vpts();
    double get_imgsize();
    kjb::Vector get_n_samples();
    std::vector<kjb::Vector> get_samples();
    std::vector<int> get_lineclasses();

    void set_lines_at_1(std::vector<Omap_segment> lineset);
    void set_lines_at_2(std::vector<Omap_segment> lineset);
    void set_lines_at_3(std::vector<Omap_segment> lineset);

    std::vector<Omap_segment> get_lines_at_vp(int vp);
    std::vector<Omap_segment> get_lines_at_1();
    std::vector<Omap_segment> get_lines_at_2();
    std::vector<Omap_segment> get_lines_at_3();

    std::vector<kjb::Vector> get_samples_at_vp(int vp);
    std::vector<kjb::Vector> get_samples_at_0();
    std::vector<kjb::Vector> get_samples_at_1();
    std::vector<kjb::Vector> get_samples_at_2();
    std::vector<kjb::Vector> get_samples_at_3();

    //constructor asisstants
    std::vector<Omap_segment> read_matlab_line_file(const char* linepath);
    std::vector<Vanishing_point> read_matlab_vp_file(const char* vppath);
    void initialize_lines_and_samples();

    /*
        prints a file with all the lines that have vanishing points associated with
        them in the following format, (per line)
        "startx starty endx endy vp"
    */
    void print_lines(std::string filename);
    void print_lines(const char* filename);
    
    //I suppose I should probably move these to m_cpp/m_matrix
    void print_matrix_as_image(std::string filename,Matrix printme,int red,int green, int blue);
    void print_matrix_as_image_gray(std::string filename,Matrix printme);
    void compute_tricolor_image
    (
    	Image & tricolor_image,
    	const Matrix & red,
    	const Matrix & green,
    	const Matrix & blue
    );

    double compare_images(std::string source, std::string check); 
    double compare_sample_files(std::string source, std::string check,std::string name);


}; //end class Omap_computer

}

#endif
