/**
 * @file
 * @brief test HTML color (as hex triplet) method of Pixel, using HSV grid
 * @author Andrew Predoehl
 */
/*
 * $Id: test_hsv_html.cpp 11667 2012-02-10 00:16:20Z predoehl $
 */

#include <i_cpp/i_cpp_incl.h>

#include <iostream>
#include <string>

std::string body()
{
    const std::string   Td1 = "<td style=\"background-color: ",
                        Td2 = ";\">&nbsp;<br></td>\n";
    const int           MAX = 20;

    std::string output = "<table><tbody>\n";
        for( int sat = 0; sat <= MAX; ++sat ) {
            output += "<tr>";
            for( int hue = 0; hue <= MAX; ++hue ) {
                float hue01 = hue / float(MAX), sat01 = sat / float(MAX);
                kjb::PixelHSVA pix1( hue01, sat01, 1.0f );
                kjb::PixelRGBA pix2( pix1 );
                output += Td1 + pix2.as_hex_triplet() + Td2;
            }
            output += "</tr>\n";
        }
    output += "</tbody></table>\n";

    return output;
}



int main()
{

    std::cout
        << "<html><head>"   
            "<meta content=\"text/html; charset=ISO-8859-1\" "
            "http-equiv=\"Content-Type\"><title>Test HSV Html Output</title>\n"
            "</head>\n<body>"
        << body()
        << "</body></html>\n";
    return EXIT_SUCCESS;
}

