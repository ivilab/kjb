/* 
omap_computer.cpp

@author - Josh Bowdish - original matlab code by David C. Lee

omap_computer is the base for all the functions under compute_omap.m


*/

#include "edge_cpp/omap_computer.h"
#include <cmath>
#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

using namespace kjb;
using namespace std;

Omap_computer::Omap_computer
(
    const char* linepath,
    const char* vppath,
    const char* featurepath,
    const char* filename
)
{
    featman = new Features_manager(featurepath);
    delete_features = true;
    mw = &(featman->get_manhattan_world());
    cout << "Constructing omap_computer using matlab derived lines.\n";
    using_featman_lines = false;    
    imagename = filename;
    dname = "omap_test_images/matlines/";
    sample_rate = 1;
    
    imageheight = featman->get_edges().get_num_rows();
    imagewidth = featman->get_edges().get_num_cols();
    cout << "num_rows: " << imageheight << endl;
    cout << "num_cols: " << imagewidth << endl;
    vpts = read_matlab_vp_file(vppath);
    lines = read_matlab_line_file(linepath);

    initialize_lines_and_samples();
    cout << "Reached the end of the \"from matlab\" constructor!\n";
}
/*
======================================================================================
Constructor for when you already have the feature manager computed
Reads the feature manager file in 
note: filename shouldn't have an extesion. Filename is pretty much for ease of
printing the outputfiles 
note also: need to add 1 to the lineclasses and vp numbers to keep it consistent
with the matlab version of the stuffs
======================================================================================
*/
Omap_computer::Omap_computer
(
    const char* featurepath,
    const char* filename
)
{
    featman = new Features_manager(featurepath);
    delete_features = true;
    mw = &(featman->get_manhattan_world());
    cout << "Constructing omap_computer using features_magager.\n";
    imagename = filename;
    using_featman_lines = true;     
    dname = "omap_test_images/featlines/";
    numlines = featman->get_edge_segments().size();
    sample_rate = 1;

    imageheight = featman->get_edges().get_num_rows();
    imagewidth = featman->get_edges().get_num_cols();

    std::vector<Vanishing_point> temp= mw->get_vanishing_points();
    vpts.resize(4);
    vpts[1] = temp[0];
    vpts[2] = temp[1];
    vpts[3] = temp[2];

    std::vector < std::vector<Manhattan_segment> > demsegs = mw->get_assignments();
    
    for(int i=0;i<3;i++){
        std::vector<Manhattan_segment> desesegs = demsegs[i];
        for(unsigned j=0;j<desesegs.size();j++){
            Omap_segment newseg(desesegs[j]);
            newseg.set_lineclass(i+1);
            lines.push_back(newseg);
        }
    }
    initialize_lines_and_samples();
}

/*
======================================================================================
Constructor for when you don't have the feature manager already computed
Warning: takes forfuckingever.
======================================================================================
*/
Omap_computer::Omap_computer(const kjb::Image & img)
{
    featman = new Features_manager(img);
    delete_features = true;
    using_featman_lines = true;
    mw = &(featman->get_manhattan_world());
    cout<< "Construction from Image successful" << endl;
    numlines = featman->get_edge_segments().size();
    sample_rate = 1;

    imageheight = featman->get_edges().get_num_rows();
    imagewidth = featman->get_edges().get_num_cols();

    std::vector<Vanishing_point> temp= mw->get_vanishing_points();
    vpts.resize(4);
    vpts[1] = temp[0];
    vpts[2] = temp[1];
    vpts[3] = temp[2];

    std::vector < std::vector<Manhattan_segment> > demsegs = mw->get_assignments();

    for(int i=0;i<3;i++){
        std::vector<Manhattan_segment> desesegs = demsegs[i];
        for(unsigned j=0;j<desesegs.size();j++){
            Omap_segment newseg(desesegs[j]);
            newseg.set_lineclass(i+1);
            lines.push_back(newseg);
        }
    }
    initialize_lines_and_samples();
} 

Omap_computer::Omap_computer
(
    Features_manager * ifm
) : featman(ifm)
{
    delete_features = false;
    using_featman_lines = true;
    mw = &(featman->get_manhattan_world());
    numlines = featman->get_edge_segments().size();
    sample_rate = 1;

    imageheight = featman->get_edges().get_num_rows();
    imagewidth = featman->get_edges().get_num_cols();

    std::vector<Vanishing_point> temp= mw->get_vanishing_points();
    vpts.resize(4);
    vpts[1] = temp[0];
    vpts[2] = temp[1];
    vpts[3] = temp[2];

    std::vector < std::vector<Manhattan_segment> > demsegs = mw->get_assignments();

    for(int i=0;i<3;i++){
        std::vector<Manhattan_segment> desesegs = demsegs[i];
        for(unsigned j=0;j<desesegs.size();j++){
            Omap_segment newseg(desesegs[j]);
            newseg.set_lineclass(i+1);
            lines.push_back(newseg);
        }
    }
    initialize_lines_and_samples();
}


/*
======================================================================================
This sort of matches the compute_orientationmap function in the matlab code
Return type will change when I figure out what it's supposed to be.
======================================================================================
*/
void Omap_computer::compute_omap
(
    Image & out_omap,
    Orientation_map_type omap_type
)
{
    cout << "Starting compute_omap()...\n";
    int height=3;
    int width=3;
    int depth=2;
    
    lineextimg.resize(height);
    for(int i = 0; i< height; i++){
        lineextimg[i].resize(width);
        for(int j=0;j<width;j++){
            lineextimg[i][j].resize(depth);
            for(int k=0;k<depth;k++){
                lineextimg[i][j][k] = Matrix(imageheight,imagewidth,0.0);
            }
        }
    }

/***************************************************************************/
    orient_from_lines();
/***************************************************************************/

    Matrix ao23 = logical_or(lineextimg[1][2][0],lineextimg[1][2][1]);
    Matrix ao32 = logical_or(lineextimg[2][1][0],lineextimg[2][1][1]);
    Matrix ao13 = logical_or(lineextimg[0][2][0],lineextimg[0][2][1]);
    Matrix ao31 = logical_or(lineextimg[2][0][0],lineextimg[2][0][1]);
    Matrix ao12 = logical_or(lineextimg[0][1][0],lineextimg[0][1][1]);
    Matrix ao21 = logical_or(lineextimg[1][0][0],lineextimg[1][0][1]);

    Matrix aa23 = logical_and(lineextimg[1][2][0],lineextimg[1][2][1]);
    Matrix aa32 = logical_and(lineextimg[2][1][0],lineextimg[2][1][1]);
    Matrix aa13 = logical_and(lineextimg[0][2][0],lineextimg[0][2][1]);
    Matrix aa31 = logical_and(lineextimg[2][0][0],lineextimg[2][0][1]);
    Matrix aa12 = logical_and(lineextimg[0][1][0],lineextimg[0][1][1]);
    Matrix aa21 = logical_and(lineextimg[1][0][0],lineextimg[1][0][1]);

    std::vector<Matrix> a;
    a.push_back(logical_and(ao23,ao32));
    a.push_back(logical_and(ao13,ao31));
    a.push_back(logical_and(ao12,ao21));

    std::vector<Matrix> b;

    cout << " creating b...\n";
    b.push_back(logical_and(a[0],logical_and(logical_not(a[1]),logical_not(a[2]))));    
    b.push_back(logical_and(logical_not(a[0]),logical_and(a[1],logical_not(a[2]))));    
    b.push_back(logical_and(logical_not(a[0]),logical_and(logical_not(a[1]),a[2])));    

    cout << " making the omaps\n ";
    if(omap_type == OMAP_LOOSE)
    {
        if(using_featman_lines == true)
        {
            compute_tricolor_image(out_omap, b[2], b[1], b[0]);
        }
        else
        {
            compute_tricolor_image(out_omap, b[0], b[1], b[2]);
        }
    }

    //omapstrict1
    
    a[0] = logical_and(aa23,aa32);
    a[1] = logical_and(aa13,aa31);
    a[2] = logical_and(aa12,aa21);

    b[0] = logical_and(a[0],logical_and(logical_not(a[1]),logical_not(a[2])));
    b[1] = logical_and(logical_not(a[0]),logical_and(a[1],logical_not(a[2])));
    b[2] = logical_and(logical_not(a[0]),logical_and(logical_not(a[1]),a[2]));

    if(omap_type == OMAP_VERY_STRICT)
    {
        if(using_featman_lines == true)
        {
            compute_tricolor_image(out_omap, b[2], b[0], b[1]);
        }
        else
        {
            compute_tricolor_image(out_omap, b[0], b[1], b[2]);
        }
    }

    //omapstrict2

    a[0] = logical_or(logical_and(ao23,aa32),logical_and(aa23,ao32));
    a[1] = logical_or(logical_and(ao13,aa31),logical_and(aa13,ao31));
    a[2] = logical_or(logical_and(ao12,aa32),logical_and(aa12,ao21));
    
    b[0] = logical_and(a[0],logical_and(logical_not(a[1]),logical_not(a[2])));
    b[1] = logical_and(logical_not(a[0]),logical_and(a[1],logical_not(a[2])));
    b[2] = logical_and(logical_not(a[0]),logical_and(logical_not(a[1]),a[2]));

    if(omap_type == OMAP_STRICT)
    {
        if(using_featman_lines == true)
        {
            compute_tricolor_image(out_omap, b[2], b[0], b[1]);
        }
        else
        {
            compute_tricolor_image(out_omap, b[0], b[1], b[2]);
        }
    }
}
/*
======================================================================================
Description goes here
nope - goes in the .h
======================================================================================
*/
vector<vector<vector<Matrix> > > Omap_computer::orient_from_lines(){

    cout << "Extending " << lines.size() << " lines...\n";
    /*****************************************************************************/
    Image tehim(imageheight,imagewidth,0,0,0);
    Matrix mask = create_zero_matrix(imageheight,imagewidth);

    for(unsigned i = 0; i<lines.size(); i++){
        cout << "line #" << i << ": " << lines[i].get_lineclass() << ", " ;
        cout.flush();

        int lc = lines[i].get_lineclass();
        //cout << "- " << lc << ", ";
        std::vector<int> extdir;
        //extdir = setdiff(1:3, lc)
        if(lc == 1){ extdir.push_back(2); extdir.push_back(3); }
        if(lc == 2){ extdir.push_back(1); extdir.push_back(3); }
        if(lc == 3){ extdir.push_back(1); extdir.push_back(2); }

        for(unsigned j = 0; j<extdir.size(); j++){

            int targetdir;
            int cextdir;// current extdir
            if(j==0){ targetdir = extdir[0]; cextdir = extdir[1]; }
            else{     targetdir = extdir[1]; cextdir = extdir[0]; }  

            Matrix poly = extend_line(lines[i],vpts[cextdir],1,get_samples_at_vp(targetdir), imageheight,imagewidth); 
            mask = geometry::polygon_to_mask(poly,imageheight,imagewidth);
            lineextimg[lc-1][cextdir-1][0] += mask; 

            poly = extend_line(lines[i],vpts[cextdir], -1,get_samples_at_vp(targetdir), imageheight,imagewidth); 
            mask = geometry::polygon_to_mask(poly,imageheight,imagewidth);
        
            lineextimg[lc-1][cextdir-1][1] +=  mask; 
        }
    }
    cout << "Exiting orient_from_lines\n";  
    return lineextimg;
}//end of orient_from_lines()


/*
======================================================================================
function ls = sample_line(lines)
the matlab function returns an array with 3 values per index: n_sample, sample and lineclass.
n_sample - double
sample - [linspace(something),linspace(something)]
lineclass - repmat(lines(i).lineclass,n_sample,1);
======================================================================================
*/
/****************************************************************************************/

/****************************************************************************************/

/****************************************************************************************/
std::vector<double>  Omap_computer::move_line_towards_vp(kjb::Vector p1, kjb::Vector p2, Vanishing_point vp, double amount){

    //things to return
    double p1x;
    double p1y;
    double p2x;
    double p2y;
    double atvp;

    /** TODO HERE */
    Vector vp_point(vp.get_x(),vp.get_y());
    if(vp.is_at_infinity())
    {
        if(vp.get_type() == Vanishing_point::INFINITY_UP)
        {
            vp_point(0) = imagewidth/2.0;
            vp_point(1) = -100000000;
        }
        else if(vp.get_type() == Vanishing_point::INFINITY_DOWN)
        {
            vp_point(0) = imagewidth/2.0;
            vp_point(1) = 100000000;
        }
        else if(vp.get_type() == Vanishing_point::INFINITY_RIGHT)
        {
            vp_point(1) = imageheight/2.0;
            vp_point(0) = 100000000;
        }
        else if(vp.get_type() == Vanishing_point::INFINITY_LEFT)
        {
            vp_point(1) = imageheight/2.0;
            vp_point(0) = -100000000;
        }
    }

    double n1 = norm2(vp_point-p1);
    double n2 = norm2(vp_point-p2);
    double ratio21; 
    Vector dir1;
    Vector dir2;
    if(n1 == 0){
        n1 = 1;
    }   
    if(n2 == 0){
        n2 = 1;
    }
        dir1 = (vp_point-p1)/n1;
        dir2 = (vp_point-p2)/n2;
        ratio21 = n2/n1;

    if(n1 <= amount){
        p1x = p1(0);
        p1y = p1(1);    
        p2x = p2(0);
        p2y = p2(1);    
        atvp = 1;
    }
    else{
            dir1 *= amount;
        Vector newp1 = p1 + dir1;
        p1x = newp1(0);
        p1y = newp1(1);
            dir2 *= amount;
            dir2 *= ratio21;
        Vector newp2 = p2 + dir2;
        p2x = newp2(0);
        p2y = newp2(1);
        atvp = 0;
    }

    std::vector<double> toreturn(0);
    toreturn.push_back(p1x);
    toreturn.push_back(p1y);
    toreturn.push_back(p2x);
    toreturn.push_back(p2y);
    toreturn.push_back(atvp);
    
    return toreturn;
}//end of  move_line_towards_vp()
/****************************************************************************************/

/****************************************************************************************/
Matrix Omap_computer::extend_line(Omap_segment line, Vanishing_point vp, int toward_or_away, std::vector<kjb::Vector> samples, int imgheight, int imgwidth){

    Vector startpt = line.get_start_point();
    Vector endpt = line.get_end_point();
    //cout <<"line: (" << startpt(0) << "," << startpt(1) << "),(" << endpt(0) << "," << endpt(1) << ") " ; 
    //cout << "toward (" << vp.get_x() << "," << vp.get_y() << ")" << endl;

    int move_amount = 128;
    int failcount = 0;
    int startchecking = 0; //so that its slightly more efficient going through the potentially very large sample set.

    while(move_amount>=1){
        
    //  cout << "Move_amount: " << move_amount << endl; 
        std::vector<double> movedline = move_line_towards_vp(startpt,endpt,vp,toward_or_away * move_amount);
    //  cout << "managed to move the line\n";   
        int failed = 0; 
        if(movedline[4] ==1){//if atvp ==1
            //hit the vp
            failed = 1;
        }

        else if(movedline[0]>imgwidth || movedline[0]<1 ||
            movedline[1]>imgheight|| movedline[1]<1 ||
            movedline[2]>imgwidth || movedline[2]<1 ||
            movedline[3]>imgheight|| movedline[3]<1){
            
            // moved beyond the image
            failed = 1;

            //cout << "moved beyond the boundaries of the image\n";
        }
        else{
            //isstop = inpolygon(stoppinglines_samples(:,1),stoppinglines_sample(:...
            //stoppinglines_samples(:,1) = linesamples(linesamplesclass==targetdir,:)
            //linesamples = cat(1,ls(:).samples)
            //targetdir = an int 0-2

            //cout << "hit this else thing\n";
            Matrix region(4,2);

            //p1,2
            region(0,0) = line.get_start_x();
            region(0,1) = line.get_start_y(); 
            region(1,0) = line.get_end_x(); 
            region(1,1) = line.get_end_y(); 
            //newp1,2
            region(2,0) = movedline[2];
            region(2,1) = movedline[3];
            region(3,0) = movedline[0];
            region(3,1) = movedline[1];

            
            //cout << "checking if the samples are in the polygon\n";   
            for(unsigned i=startchecking;i<samples.size();i++){
            //  cout << "sample " << i << ":\n";
                Vector ref = samples[i];
                if(kjb::geometry::is_point_in_polygon_new(region,ref)){
                    //failed polygon check 
                //  cout << "found a point in the polygon\n";
                    failed = 1;//exit loop
                    startchecking = i;
                    i+=samples.size();
                }
            }   
            //cout << "finished checking if the samples are in the polygon\n";
        }
        
        if(failed == 1){
            failcount++;
            move_amount = move_amount/2;
        }
        else{
            startpt = Vector(movedline[0],movedline[1]);
            endpt = Vector(movedline[2],movedline[3]);
        }
    }//end while(move_amount>=1

    //returns - poly = [p1(:)'; p2(:)'; curp2(:)'; curp1(:)'];
    //where p1 and p2 are the original points and curp1 and 2 are the moved points.
    Matrix poly = create_zero_matrix(4,2);  
            poly(0,0) = line.get_start_x();
            poly(0,1) = line.get_start_y();
            poly(1,0) = line.get_end_x();
            poly(1,1) = line.get_end_y(); 
            poly(2,0) = endpt(0);
            poly(2,1) = endpt(1); 
            poly(3,0) = startpt(0);
            poly(3,1) = startpt(1);

    //cout << "done!\n";
    return poly;
}//end extend_line()

std::vector<Omap_segment> Omap_computer::read_matlab_line_file(const char* linepath){

    std::vector<Omap_segment> toreturn;
    string line;
    ifstream linefile(linepath);
    double sx;
    double sy;
    double ex;
    double ey;
    double lc;
    if(linefile.is_open()){
        while(linefile.good()){
            getline(linefile,line);
            string number;
            int index = 0;
            for(unsigned i=0;i<line.size();i++){
                if(line.at(i) == ' '){
                    int numnum = atoi(number.c_str());
                
                    switch(index){
                        case 0:
                            sx = numnum;
                            number = "";
                            
                        break;
                        case 1:
                            sy = numnum;
                            number = "";
                        break;
                        case 2:
                            ex = numnum;
                            number = "";
                        break;
                        case 3:
                            ey = numnum;
                            number = "";
                        break;
                        default:
                            cout << "You really shouldn't be here\n";
                        break;
                    }
                    index++;
                }
                number += line.at(i);
            }
            lc = atoi(number.c_str());
            Omap_segment addme(sx,sy,ex,ey,lc);
            toreturn.push_back(addme);
            number = "";
        }
    }
    linefile.close();

    return toreturn;
}

void Omap_computer::initialize_lines_and_samples(){

    int mastersamplesize = 0;
    int sample1size = 0;
    int sample2size = 0;
    int sample3size = 0;
    for(unsigned i=0;i<lines.size();i++){
        lines[i].set_vp(vpts[lines[i].get_lineclass()]);
        int samplesize = lines[i].get_sample().size();
        mastersamplesize += samplesize;
        if(lines[i].get_lineclass() == 1){ sample1size += samplesize; }
        else if(lines[i].get_lineclass() == 2){ sample2size += samplesize; }
        else if(lines[i].get_lineclass() == 3){ sample3size += samplesize; }
    }

    mastersamples.resize(mastersamplesize);
    mastersamplelc.resize(mastersamplesize);
    samples_at_1.resize(sample1size);
    samples_at_2.resize(sample2size);
    samples_at_3.resize(sample3size);
    
    ofstream masterout;
    ofstream masterlcout;
    masterout.open("samplesfiles/mastersamples.txt");
    masterlcout.open("samplesfiles/mastersampleslc.txt");
/*
    string samples1 = imagename + "_samples1.txt";  
    string samples2 = imagename + "_samples2.txt";  
    string samples3 = imagename + "_samples3.txt";  

    ofstream samfile1;
    ofstream samfile2;
    ofstream samfile3;
    samfile1.open(samples1.c_str());
    samfile2.open(samples2.c_str());
    samfile3.open(samples3.c_str());
*/
    cout << "Creating sample lists\n";
    int masterindex = 0;
    int index1 = 0;
    int index2 = 0;
    int index3 = 0;
    for(unsigned i=0;i<lines.size();i++){
        std::vector<kjb::Vector> csam = lines[i].get_sample();
        for(unsigned j=0;j<csam.size();j++){
            kjb::Vector cvec = csam[j];
            mastersamples[masterindex] = cvec;
            masterout << cvec(0) << " " << cvec(1) << endl;         
            masterlcout << lines[i].get_lineclass() << endl;
            if(lines[i].get_lineclass() == 1){
                samples_at_1[index1] = cvec;    
                index1++;
                mastersamplelc[masterindex] = 1;
                //samfile1 << cvec(0) << " " << cvec(1) << endl;
            }   
            else if(lines[i].get_lineclass() == 2){
                samples_at_2[index2] = cvec;    
                index2++;
                mastersamplelc[masterindex] = 2;
                //samfile2 << cvec(0) << " " << cvec(1) << endl;        
            }   
            else if(lines[i].get_lineclass() == 3){
                samples_at_3[index3] = cvec;    
                index3++;
                mastersamplelc[masterindex] = 3;
                //samfile3 << cvec(0) << " " << cvec(1) << endl;
            }   
            masterindex++;  
        }       
    }
    
    cout << "samples_at_1.size(): " << samples_at_1.size() << endl;
    cout << "samples_at_2.size(): " << samples_at_2.size() << endl;
    cout << "samples_at_3.size(): " << samples_at_3.size() << endl;

    masterout.close();
    masterlcout.close();

    /*
    samfile1.close();
    samfile2.close();
    samfile3.close();
    */
}

std::vector<Vanishing_point> Omap_computer::read_matlab_vp_file(const char* vppath){

    std::vector<Vanishing_point> toreturn;  
    string line;
    ifstream vpfile(vppath);

    toreturn.push_back(Vanishing_point(0,0));
    
    if(vpfile.is_open()){
        int index = 1;
        double point1;
    
        while(vpfile.good()){
            getline(vpfile,line);
            switch(index){
                default:
                    cout << "You really shouldn't be here\n";
                break;
                case 1:
                    point1 = atoi(line.c_str());
                break;
                case 2:
                    Vanishing_point mademe(point1,atoi(line.c_str()));
                    toreturn.push_back(mademe); 
                    index = 0;
                break;
            }   
            index++;
        }
    }

    return toreturn;

}
/****************************************************************************************/

/****************************************************************************************/
void Omap_computer::print_matrix_as_image(string filename,Matrix printme,int red,int green,int blue)
{ 

    Image tehim(printme.get_num_rows(),printme.get_num_cols(),0,0,0);

    for(int i=0;i<printme.get_num_rows();i++){
        for(int j=0;j<printme.get_num_cols();j++){
            if(printme(i,j)!=0){
                tehim(i,j,0) = red;
                tehim(i,j,1) = green;
                tehim(i,j,2) = blue;
            }               
        }
    }

    tehim.write(filename);

}
/****************************************************************************************/

/****************************************************************************************/
void Omap_computer::print_matrix_as_image_gray(string filename,Matrix printme)
{ 
    cout << "Gray print\n";

    Image tehim(printme.get_num_rows(),printme.get_num_cols(),0,0,0);
    double min = INT_MAX;
    double max = INT_MIN;

    for(int i=0;i<printme.get_num_rows();i++){
        for(int j=0;j<printme.get_num_cols();j++){
            if(printme(i,j)<min){ min = printme(i,j); }
            if(printme(i,j)>max){ max = printme(i,j); }
        }
    }

    int sf = 1; 
    if(max!=min){
        sf = std::floor(255.0/(max-min));//scaling factor
    }

    for(int i=0;i<printme.get_num_rows();i++){
        for(int j=0;j<printme.get_num_cols();j++){
            if(printme(i,j)!=0){
                tehim(i,j,0) = printme(i,j)*sf;
                tehim(i,j,1) = printme(i,j)*sf;
                tehim(i,j,2) = printme(i,j)*sf;
            }               
        }
    }
    tehim.write(filename);

}
/****************************************************************************************/

/****************************************************************************************/
void Omap_computer::compute_tricolor_image
(
    Image & img,
    const Matrix & red,
    const Matrix & green,
    const Matrix & blue
)
{ 
    if((red.get_num_rows() != green.get_num_rows()) ||
       (red.get_num_rows() != blue.get_num_rows()) ||
       (red.get_num_cols() != green.get_num_cols()) ||
       (red.get_num_cols() != blue.get_num_cols())){
        cout << "Tried to print tricolor matrix with unequal matricies\n";
    }
    else{

        //scaling
        double rmin = INT_MAX;
        double rmax = INT_MIN;
        double gmin = INT_MAX;
        double gmax = INT_MIN;
        double bmin = INT_MAX;
        double bmax = INT_MIN;

        for(int i=0;i<red.get_num_rows();i++){
            for(int j=0;j<red.get_num_cols();j++){
                if(red(i,j)<rmin){ rmin = red(i,j); }
                if(red(i,j)>rmax){ rmax = red(i,j); }
                if(green(i,j)<gmin){ gmin = green(i,j); }
                if(green(i,j)>gmax){ gmax = green(i,j); }
                if(blue(i,j)<bmin){ bmin = blue(i,j); }
                if(blue(i,j)>bmax){ bmax = blue(i,j); }
            }
        }

        int rsf = 1;    
        int gsf = 1;    
        int bsf = 1;    
        if(rmax!=rmin){
            rsf = std::floor(255.0/(rmax-rmin));//scaling factor
        }
    
        if(gmax!=gmin){
            gsf = std::floor(255.0/(gmax-gmin));//scaling factor
        }
        
        if(bmax!=bmin){
            bsf = std::floor(255.0/(bmax-bmin));//scaling factor
        }

        img = kjb::Image(red.get_num_rows(),red.get_num_cols(),0,0,0);

        for(int i = 0; i < red.get_num_rows(); i++)
        {
            for(int j = 0; j < red.get_num_cols(); j++)
            {
                img(i,j,0) = red(i,j) * rsf;
                img(i,j,1) = green(i,j) * gsf;
                img(i,j,2) = blue(i,j) * bsf;
            }
        }
    }

}
/****************************************************************************************/

/****************************************************************************************/
void Omap_computer::print_lines(string filename){
    print_lines(filename.c_str());
}

void Omap_computer::print_lines(const char* filename){

    ofstream tehfile;
    tehfile.open(filename);
    int totallines = 0;
    for(unsigned i=0;i<3;i++){
        totallines += mw->num_lines_assigned_to_vp(i);
    }
    
    tehfile << totallines << " " << totallines << " " << totallines << " " << totallines << " " << totallines << endl; 
    
    for(unsigned i=0;i<3; i++){
        //n_sample = ceil(norm(lines(i).point1-lines(i).point2) /sample_rate);
        unsigned linesinvp = mw->num_lines_assigned_to_vp(i);

        for(unsigned j=0; j<linesinvp;j++){

            Manhattan_segment tehmanseg = featman->get_manhattan_world().get_manhattan_segment(i,j);
            Edge_segment segment = tehmanseg.get_edge_segment();
            double sx = segment.get_start_x(); //current start x
            double sy = segment.get_start_y(); //current start y
            double ex = segment.get_end_x(); //current end x
            double ey = segment.get_end_y(); //current end y
            
            tehfile << sx <<  " " << sy << " " << ex << " " <<  ey << " " << i << endl;
        }
    }
    tehfile.close();
        
    
}
/****************************************************************************************/

//getters!
std::vector<Omap_segment> Omap_computer::get_lines(){ return lines; }
Edge_segment_set Omap_computer::get_edge_segments(){ return featman->get_edge_segments(); }


std::vector<Vanishing_point> Omap_computer::get_vpts(){ return vpts; }

std::vector<Omap_segment> Omap_computer::get_lines_at_vp(int vp){
    switch(vp){
        case 1: return get_lines_at_1(); break;
        case 2: return get_lines_at_2(); break;
        case 3: return get_lines_at_3(); break;
        default: cout << "You really shouldn't be here\n"; break;
    }
    return get_lines_at_1();
}

std::vector<Omap_segment> Omap_computer::get_lines_at_1(){ return lines_at_1; }
std::vector<Omap_segment> Omap_computer::get_lines_at_2(){ return lines_at_2; }
std::vector<Omap_segment> Omap_computer::get_lines_at_3(){ return lines_at_3; }

double Omap_computer::get_imgsize(){ return imgsize; }

std::vector<kjb::Vector> Omap_computer::get_samples_at_0(){ return samples_at_0; }
std::vector<kjb::Vector> Omap_computer::get_samples_at_1(){ return samples_at_1; }
std::vector<kjb::Vector> Omap_computer::get_samples_at_2(){ return samples_at_2; }
std::vector<kjb::Vector> Omap_computer::get_samples_at_3(){ return samples_at_3; }
    
std::vector<kjb::Vector> Omap_computer::get_samples_at_vp(int vp){
    if(vp==0){return get_samples_at_0();}
    else if(vp==1){return get_samples_at_1();}
    else if(vp==2){return get_samples_at_2();}
    else if(vp==3){return get_samples_at_3();}
    else{
        cout << "Attempted to get samples at out of index vp, vps are 0-2, returning 0's" << endl; 
        return get_samples_at_0();
    } 
}

double Omap_computer::compare_sample_files(string source,string check,string name){

    double tolerance = 1;

    cout << "streaming \"" << source << "\"\n";

    ifstream matlab(source.c_str());
    cout << "streaming \"" << check << "\"\n";
    ifstream cpp(check.c_str());
    string matline;
    string cppline;

    int total_match = 0;
    int total = 0;  

    ofstream outfile;

    string outname = "compare_"+name;   
    outfile.open(outname.c_str());

    int linecount = 0;
    int linesright = 0;
    if(matlab.is_open() && cpp.is_open()){
        while(matlab.good() && cpp.good()){
            outfile << "line #" << linecount << endl;
            getline(matlab,matline);
            outfile << "matline: " << matline << endl;
            getline(cpp,cppline);
            outfile << "cppline: " << cppline<< endl;
            string number;
            std::vector<double> matnums;
            std::vector<double> cppnums;
            for(unsigned i=0;i<matline.size();i++){
                if(matline.at(i) == ' '){
                    matnums.push_back((double) atoi(number.c_str()));
                    number = "";
                }
                number += matline.at(i);
            }//end linebyline loop
            matnums.push_back((double) atoi(number.c_str()));
            number = "";
            for(unsigned i=0;i<cppline.size();i++){
                if(cppline.at(i) == ' '){
                    cppnums.push_back((double) atoi(number.c_str()));
                    number = "";
                }
                number += cppline.at(i);
            }
            cppnums.push_back((double) atoi(number.c_str()));
            total++;
            //outfile << matnum1 << " vs " << cppnum1 << endl;
            //outfile << matnum2 << " vs " << cppnum2 << endl;
            int stilltrue= 1;      
            if(matnums.size() != cppnums.size()){
                stilltrue = 0;
                outfile << "WRONG! too many numbers in one\n";
            }
            for(unsigned i=0;i<matnums.size();i++){
                if(std::abs(matnums[i] - cppnums[i]) <= tolerance){
                    //good, don't do anything
                }
                else{
                    stilltrue = 0;
                    outfile << "WRONG!\n";
                    i += matnums.size();
                }
            }
            if(stilltrue == 1){
                linesright++;
                outfile << "Match!\n";
                total_match++;
            }
            linecount++;
        }//end while
        
    }//end if files are open
    matlab.close();
    cpp.close();
    

    cout << "total_matching: " << total_match << " out of " << total << " ";
    outfile << "total_matching: " << total_match << " out of " << total << " ";
    double percent_match = (double) 100*total_match/total;
    outfile << "- " << percent_match << "%\n";
    outfile << linesright << " lines correct out of " << linecount << ", ";
    outfile << linecount-linesright << " lines wrong\n";
    cout << "- " << percent_match << "%\n";
    cout << linecount-linesright << " lines wrong\n";

    return percent_match;
}


double Omap_computer::compare_images(string source,string check){

    int tolerance = 20;

    ifstream ifilesource(source.c_str());
    ifstream ifilecheck(check.c_str());

    Image matlab;
    Image mine;

    if(ifilesource){
        Image temp(source);
        matlab = temp;
    }
    else{ return 0; } 
    if(ifilecheck){
        Image temp(check);
        mine = temp;
    }
    else{ return 0; }

    int total_pixels = matlab.get_num_rows()*matlab.get_num_cols();
    int total_matching = 0;
    int straight_match = 0;
    int swap_match = 0;

    for(int i =0; i< matlab.get_num_rows(); i++){
        for(int j=0; j<matlab.get_num_cols();j++){
            double source_r = matlab(i,j,0);
            double check_r = mine(i,j,0);
            double source_g = matlab(i,j,0);
            double check_g = mine(i,j,0);
            double source_b = matlab(i,j,0);
            double check_b = mine(i,j,0);
    
            if((std::abs(source_r - check_r) <= tolerance) &&
               (std::abs(source_g - check_g) <= tolerance) &&
               (std::abs(source_b - check_b) <= tolerance)){
                straight_match++;   
            }
            else if((std::abs(source_r - check_r) <= tolerance) &&
                    (std::abs(source_g - check_b) <= tolerance) &&
                    (std::abs(source_b - check_g) <= tolerance)){
                swap_match++;   
            }
        }
    }

    if(swap_match > straight_match){
        total_matching = swap_match;
    }
    else{ total_matching = straight_match; }

    cout << "total_matching: " << total_matching << " out of " << total_pixels << " ";
    double percent_match = (double) 100*total_matching/total_pixels;
    cout << "- " << percent_match << "%\n";

    return percent_match;
}

/* 

Methods for class Omap_segment

*/
//constructors
Omap_segment::Omap_segment(){
    Omap_segment(0,0,0,0,0);
}


Omap_segment::Omap_segment(Manhattan_segment seg){

    double sx = seg.get_edge_segment().get_start_x();
    double sy = seg.get_edge_segment().get_start_y();
    double ex = seg.get_edge_segment().get_end_x();
    double ey = seg.get_edge_segment().get_end_y();

    kjb::Vector sp(sx,sy);
    kjb::Vector ep(ex,ey);

    startpoint = sp;
    endpoint = ep;
    vp = (*(seg.get_vanishing_point()));
    develop_sample();


    
}

Omap_segment::Omap_segment(Vector sp,Vector ep){
    startpoint = sp;
    endpoint = ep;  
    develop_sample();   
}

Omap_segment::Omap_segment(Vector sp,Vector ep,int lc){
    startpoint = sp;
    endpoint = ep;  
    set_lineclass(lc);
    develop_sample();   
}

Omap_segment::Omap_segment(double x1,double y1,double x2, double y2){
    Vector sp(x1,y1);
    Vector ep(x2,y2);
    startpoint = sp;
    endpoint = ep;
    develop_sample();   
}

Omap_segment::Omap_segment(double x1,double y1,double x2, double y2,int lc){
    Vector sp(x1,y1);
    Vector ep(x2,y2);
    startpoint = sp;
    endpoint = ep;
    set_lineclass(lc);
    develop_sample();   
}

void Omap_segment::set_lineclass(int lc){
    lineclass = lc; 
}

void Omap_segment::set_vp(const Vanishing_point & setme){
    vp = setme;
}

void Omap_segment::develop_sample(){//might rename to "get_samples()"

    int sample_rate = 1;

    Vector tocompute = startpoint-endpoint;

    n_sample = ceil(norm2(tocompute)/sample_rate);

    Vector xsamples = create_uniformly_spaced_vector(startpoint(0),endpoint(0),n_sample);
    Vector ysamples = create_uniformly_spaced_vector(startpoint(1),endpoint(1),n_sample);
    std::vector<kjb::Vector> tempsample;
        
    for(int k=0; k< xsamples.size(); k++){
        Vector thispoint(xsamples[k],ysamples[k]);
        tempsample.push_back(thispoint);        
    } 
    sample = tempsample;

}//end of sample_line

double Omap_segment::get_start_x()
{
    return startpoint(0);
}

double Omap_segment::get_start_y()
{
    return startpoint(1);
}

double Omap_segment::get_end_x()
{
    return endpoint(0);
}

double Omap_segment::get_end_y()
{
    return endpoint(1);
}
