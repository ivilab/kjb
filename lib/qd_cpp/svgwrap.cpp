/**
 * @file
 * @brief Implementation of the PixPath class
 * @author Andrew Predoehl
 */

/*
 * $Id: svgwrap.cpp 21596 2017-07-30 23:33:36Z kobus $
 */


#include "l/l_sys_io.h"
#include "qd_cpp/pixpath.h"
#include "qd_cpp/svgwrap.h"


namespace {


// predicate:  is a character in an XML string bad (as in, troublesome)?
inline
bool bad_xml_text_char( const char& ccc )
{
    switch( ccc ) {
    case '<':
    case '>':
    case '&':
    case '\\':
        return true; // truly bad
    default:
        return ! std::isprint( ccc );   // bad if not printable
    }
    /* NOTREACHED */
}


} // end anonymous ns


namespace kjb {
namespace qd {


/// @brief Get fill attribute for paths (if in use; else empty).
std::string SvgWrap::generate_fill_attr() const
{
    // respond to the "experts-only" flag that trumps other settings
    if ( m_totally_omit_fill_attribute ) return "";

    if ( m_fill_path )
    {
        // Iff the flag is set and m_fill_color has positive size, use it.
        if ( m_fill_color.size() )
        {
            // Output must start with a space!
            return " fill=\"" + m_fill_color + "\"";
        }
        // If the flag is set but fill_color is empty, try to use default.
        if ( m_color.size() )
        {
            // Output must start with a space!
            return " fill=\"" + m_color + "\"";
        }
    }
    // SVG default behavior is to fill, unless told otherwise.
    return " fill=\"none\"";
}

/// @brief get stroke attribute for paths, if in use; else empty string.
std::string SvgWrap::generate_stroke_attr() const
{
    if ( 0 == m_color.size() || m_totally_omit_stroke_attribute )
    {
        return ""; // Empty output does not need to start with a space.
    }
    // Output must start with a space!
    return " stroke=\"" + m_color + "\"";
}

/// @brief render the path in a rectangular box using SVG 1.1
std::string SvgWrap::operator()() const
{
    if (m_width <= 0) KJB_THROW_2(Illegal_argument, "m_width must be positive");
    if (m_height <= 0) KJB_THROW_2(Illegal_argument, "m_height must be positive");
    if (m_magnify <= 0) KJB_THROW_2(Illegal_argument, "m_magnify must be positive");

    std::ostringstream ss;
    if ( m_gen_xml_header )
    {
        ss << xml_header();
    }

    if ( m_open_svg )
    {
        ss << svg_open_tag();
    }

    if ( m_id.size() )
    {
        ss << "<g id=\"" << m_id << "\">\n";
    }

    /* could open a global transform tag now, e.g.,
        ss << "<g transform=\"scale(0.4)\">\n";
     */

    /* could add an image reference now, e.g.,
       ss << "<image x=\"0\" y=\"0\" width=\"2000\" height=\"2000\" "
                "xlink:href=\"./doqimage.png\" />\n";
     */

    if ( m_show_segments && 1 < m_pixpath.size() )
    {
        PixPath::const_iterator ppp = m_pixpath.begin();
        ss << "<path d=\"M " << ppp -> str(",") << "\nL ";
        for( ppp++; ppp != m_pixpath.end(); ppp++ )
        {
            ss << ppp -> str(",") << '\n';
        }
        ss << "\" "<< m_extra << generate_fill_attr() << generate_stroke_attr()
           << "/>\n";
    }

    if ( m_show_text && m_pixpath.size() )
    {
        ss  << "<g fill=\"" << m_color << "\" font-size=\"15\">\n";
        size_t index = 0;
        for( PixPath::const_iterator ppp = m_pixpath.begin();
                                                ppp != m_pixpath.end(); ++ppp )
        {
            ss  << "<text x=\"" << ppp -> x << "\" y=\"" << ppp -> y
                    << "\">" << index++ << "</text>\n";
        }
        ss << "</g>\n";
    }

    /* close global transform tag:
        ss << "</g>\n";
     */

    if ( m_id.size() )
    {
        ss << "</g>\n"; // close id tag
    }

    if ( m_close_svg )
    {
        ss << "</svg>\n";
    }

    return ss.str();
}

/**
 * @brief try to set the id string for the path
 * @throws kjb::Illegal_argument if the ID string contains a bad character.
 * @return reference to self (for easy chaining of these setters).
 *
 * Disallowed characters include nonprintable characters and the characters
 * in the string "<&>" -- no support (yet) for escape sequences like "&amp;"
 * Also the double-quote character and backslash are disallowed.
 */
SvgWrap& SvgWrap::set_id( const std::string& id )
{
    if  (   std::find_if(id.begin(), id.end(), bad_xml_text_char) < id.end()
        ||  id.find('"') < id.size()
        )
    {
        KJB_THROW_2( kjb::Illegal_argument, "Invalid character in ID" );
    }
    m_id = id;
    return *this;
}

/**
 * @brief set an extra attribute to add to each path node in SVG
 * @throws kjb::Illegal_argument if the string contains a bad character.
 *
 * The purpose is to set one or more attributes to be applied to each path node
 * to support the many SVG features that we don't want to explicitly add to the
 * API.  Use as many attributes as you like, just obey XML syntax.
 * Some sample attributes that you might want to use are below.
 * - stroke-width="17"
 * - stroke-linecap="round"  (also try "square" or "butt")
 * - stroke-linejoin="round" (also try "miter" or "bevel")
 * - fill-rule="evenodd"
 * - opacity="0.5"
 *
 * The attribute must not, accidentally or on purpose, close a tag or break XML
 * syntax, so we do not allow the characters '<' '>' '\' or '&' which all
 * require special treatment.  The above restriction is neither totally
 * necessary nor sufficient to preserve the XML rules, so, apologies for the
 * crudeness but be careful anyway.  You can mess up your XML easily with this
 * method!
 */
SvgWrap& SvgWrap::set_path_extra( const std::string& e )
{
    if ( std::find_if(e.begin(), e.end(), bad_xml_text_char) < e.end() )
    {
        KJB_THROW_2( kjb::Illegal_argument, "Invalid character in extra" );
    }
    m_extra = e;
    return *this;
}




} // end namespace qd
} // end namespace kjb

