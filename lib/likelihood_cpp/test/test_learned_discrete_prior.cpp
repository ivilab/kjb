/* $Id: test_learned_discrete_prior.cpp 21596 2017-07-30 23:33:36Z kobus $ */
/*=========================================================================== *
  |
  | Copyright (c) 1994-2010 by Kobus Barnard (author)
  |
  | Personal and educational use of this code is granted, provided that this
  | header is kept intact, and that the authorship is not misrepresented, that
  | its use is acknowledged in publications, and relevant papers are cited.
  |
  | For other use contact the author (kobus AT cs DOT arizona DOT edu).
  |
  | Please note that the code in this file has not necessarily been adequately
  | tested. Naturally, there is no guarantee of performance, support, or fitness
  | for any particular task. Nonetheless, I am interested in hearing about
  | problems that you encounter.
  |
  | Author:  Emily Hartley
 *=========================================================================== */

#include <iostream>
#include <fstream>
#include <string.h>

#include "likelihood_cpp/learned_discrete_prior.h"

using namespace kjb;


void print_histo(Learned_discrete_prior& dp, std::ofstream& outFile)
{
    int i;
    Vector bins;

    outFile << "Num_bins: " << dp.get_num_bins() << std::endl;
    outFile << "Maximum:  " << dp.get_histo_max() << std::endl;
    outFile << "Minimum:  " << dp.get_histo_min() << std::endl;

    bins = dp.get_histo_bins();

    outFile << "Histogram: (";
    for(i = 0; i < dp.get_num_bins() - 1; i++)
    {
        outFile << bins(i) << ", ";
    }
    outFile << bins(i) << ")\n\n";
}


// Takes list of objects to create histograms for as arguments.
// Objects must be one of the following:
//     bed, couch, table, chair, cabinet, door, window, picture_frame, 
//     subcategories
int main(int argc, char* argv[])
{
    int i;
    Vector bins;
    Learned_discrete_prior dp(0, 0, 0, argv[1]);
    char* nameStart = "histo_results";
    char* nameEnd = ".txt";
    char fileName[56];

    strcpy(fileName, nameStart);

    int bed = 0;
    int couch = 0;
    int table = 0;
    int chair = 0;
    int cabinet = 0;
    int door = 0;
    int window = 0;
    int picture_frame = 0;
    int sub = 0;

    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "bed") == 0)
        {
            bed = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "couch") == 0)
        {
            couch = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "table") == 0)
        {
            table = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "chair") == 0)
        {
            chair = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "cabinet") == 0)
        {
            cabinet = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "door") == 0)
        {
            door = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "window") == 0)
        {
            window = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "picture_frame") == 0)
        {
            picture_frame = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else if(strcmp(argv[i], "subcategories") == 0)
        {
            sub = 1;
            strcat(fileName, "_");
            strcat(fileName, argv[i]);
        }
        else
        {
            std::cout << "INVALID ARG: " << argv[i] << std::endl;
        }
    }
    strcat(fileName, nameEnd);

    std::ofstream outFile(fileName);

    if(!outFile.is_open())
    {
        KJB_THROW_2(IO_error, "Could not open output file");
        std::cout << "File is not open!\n";
        return 1;
    }

    //////////////////////////////////////////////////////////
    // Beds //
    //////////////////////////////////////////////////////////
    if(bed)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/bedNoBunk_maxOverHeight.txt");
        outFile << "BEDS (no bunks) - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("bed_noBunk_maxOverHeight_histogram.ps");
        //////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/bedNoBunk_minOverMax.txt");
        outFile << "BEDS (no bunks) - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("bed_noBunk_minOverMax_histogram.ps");
    }
    ///////////////////////////////////////////////////////
    // Couches //
    //////////////////////////////////////////////////////////
    if(couch)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0,  
            "/data/elh/room/histograms/inputFiles/couchNoArmchair_maxOverHeight.txt");
        outFile << "COUCHES (no armchairs) - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("couch_noArmchair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/couchNoArmchair_minOverMax.txt");
        outFile << "COUCHES (no armchairs) - min(length,width) / "
                << "max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("couch_noArmchair_minOverMax_histogram.ps");
    }
        ///////////////////////////////////////////////////////
    if(sub) // couch subcategories
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/couchNoLounge_maxOverHeight.txt");
        outFile << "COUCHES (no armchairs or lounges) - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("couch_noArmchairNoLounge_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/couchNoLounge_minOverMax.txt");
        outFile << "COUCHES (no armchairs or lounges) - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("couch_noArmchairNoLounge_minOverMax_histogram.ps");
    }
    //////////////////////////////////////////////////////////
    // Cabinets //
    //////////////////////////////////////////////////////////
    if(cabinet)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/cabinet_maxOverHeight.txt");
        outFile << "CABINET - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("cabinet_maxOverHeight_histogram.ps");
        //////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/cabinet_minOverMax.txt");
        outFile << "CABINET - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("cabinet_minOverMax_histogram.ps");
    }
    ///////////////////////////////////////////////////////
    // Tables //
    //////////////////////////////////////////////////////////
    if(table)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/table_maxOverHeight.txt");
        outFile << "TABLE - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("table_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/table_minOverMax.txt");
        outFile << "TABLE - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("table_minOverMax_histogram.ps");
    }
        ///////////////////////////////////////////////////////
    if(sub) // table subcategories
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/tableNoRound_maxOverHeight.txt");
        outFile << "TABLE (no round) - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("table_noRound_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/tableNoRound_minOverMax.txt");
        outFile << "TABLE (no round) - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("table_noRound_minOverMax_histogram.ps");
    }
    ///////////////////////////////////////////////////////////
    // Windows //
    ///////////////////////////////////////////////////////////
    if(window)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/window_widthOverHeight.txt");
        outFile << "WINDOW - width / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("window_widthOverHeight_histogram.ps");
    }
    //////////////////////////////////////////////////////////
    // Chairs //
    ///////////////////////////////////////////////////////////
    if(chair)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/allChairs_maxOverHeight.txt");
        outFile << "ALL CHAIRS - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("allChairs_maxOverHeight_histogram.ps");
        //////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/allChairs_minOverMax.txt");
        outFile << "ALL CHAIRS - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("allChairs_minOverMax_histogram.ps");
    }
        ///////////////////////////////////////////////////////
    if(sub) // chair subcategories
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/barStool_maxOverHeight.txt");
        outFile << "BAR STOOL - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("barStool_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/barStool_minOverMax.txt");
        outFile << "BAR STOOL - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("barStool_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/chair_maxOverHeight.txt");
        outFile << "CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("chair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/chair_minOverMax.txt");   
        outFile << "CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("chair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/childChair_maxOverHeight.txt");
        outFile << "CHILD CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("childChair_maxOverHeight_histogram.ps");
        //////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/childChair_minOverMax.txt");
        outFile << "CHILD CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("childChair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/foldingChair_maxOverHeight.txt");
        outFile << "FOLDING CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("foldingChair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/foldingChair_minOverMax.txt");
        outFile << "FOLDING CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("foldingChair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/highChair_maxOverHeight.txt");
        outFile << "HIGH CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("highChair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/highChair_minOverMax.txt");   
        outFile << "HIGH CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("highChair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/recliningChair_maxOverHeight.txt");
        outFile << "RECLINING CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("recliningChair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/recliningChair_minOverMax.txt");   
        outFile << "RECLINING CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("recliningChair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/rockingChair_maxOverHeight.txt");
        outFile << "ROCKING CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("rockingChair_maxOverHeight_histogram.ps");
        //////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/rockingChair_minOverMax.txt");
        outFile << "ROCKING CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("rockingChair_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/stool_maxOverHeight.txt");
        outFile << "STOOL - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("stool_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/stool_minOverMax.txt");
        outFile << "STOOL - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("stool_minOverMax_histogram.ps");
        ///////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/swivelChair_maxOverHeight.txt");
        outFile << "SWIVEL CHAIR - max(length,width) / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("swivelChair_maxOverHeight_histogram.ps");
        ///////////////////////////////////////////////////////////
        dp = Learned_discrete_prior(20, 1.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/swivelChair_minOverMax.txt");   
        outFile << "SWIVEL CHAIR - min(length,width) / max(length,width)\n";
        print_histo(dp, outFile);
        dp.plot_histogram("swivelChair_minOverMax_histogram.ps");
    }
    ///////////////////////////////////////////////////////////
    // Doors //
    ///////////////////////////////////////////////////////////
    if(door)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/doors_widthOverHeight.txt");
        outFile << "DOOR - width / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("doors_widthOverHeight_histogram.ps");
    }
    ///////////////////////////////////////////////////////////
    // Picture Frames //
    ///////////////////////////////////////////////////////////
    if(picture_frame)
    {
        dp = Learned_discrete_prior(20, 8.0, 0.0, 
            "/data/elh/room/histograms/inputFiles/pictureFrames_widthOverHeight.txt");
        outFile << "PICTURE FRAME - width / height\n";
        print_histo(dp, outFile);
        dp.plot_histogram("pictureFrames_widthOverHeight_histogram.ps");
    }

    outFile.close();
}
