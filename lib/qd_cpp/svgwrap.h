/**
 * @file
 * @brief Contains definition for class SvgWrap
 * @author Andrew Predoehl
 */
/*
 * $Id: svgwrap.h 20160 2015-12-08 23:36:20Z predoehl $
 */

#ifndef SVGWRAP_H_UOFARIZONAVISION
#define SVGWRAP_H_UOFARIZONAVISION 1

#include <i_cpp/i_pixel.h>
#include <qd_cpp/pixpath.h>

#include <string>
#include <sstream>


namespace kjb
{
namespace qd
{


/**
 * @brief class used to render a PixPath as an SVG polygonal path picture
 *
 * The many "set_xxx" functions all return a reference to *this, so that
 * they can be chained.
 *
 * Expert users might want to avoid the automated logic that controls how the
 * stroke and fill attributes are computed, perhaps because the path will
 * be placed within a group tag.  The "m_totally_omit_*" booleans will turn off
 * those attributes, irrespective of any other flags.  Under some circumstances
 * that is the only way to get the desired output; however, because they are
 * specialty settings and they trump other state settings, we don't provide
 * set functions for them and the user has to write out their deliberately long
 * and clumsy names, to make clear that they are for experts only.
 */
class SvgWrap
{
    PixPath m_pixpath;      ///< pixpath to show
    std::string m_id;       ///< optional identity string (a name)
    std::string m_extra;    ///< extra stuff to add to each stroke

public:

    // public attributes the user may twiddle at will
    bool        m_show_segments,    ///< show line segments between points?
                m_show_text,        ///< show numbered sequence of points?
                m_gen_xml_header,   ///< print the special XML header tag?
                m_open_svg,         ///< print an opening SVG tag?
                m_close_svg,        ///< print the close tag of the SVG node?
                m_fill_path;        ///< should the path be filled?
    float       m_width,            ///< width in world units of the SVG image
                m_height,           ///< height in world units of SVG image
                m_magnify,          ///< magnification from world to image
                m_origin_bias_x,    ///< world-unit shift image origin, x axis
                m_origin_bias_y;    ///< world-uint shift image origin, y axis
    std::string m_color,            ///< color, as text string, of objects
                m_fill_color;       ///< color of fill, can override default

    // "experts-only" booleans -- usually not indicated
    bool    m_totally_omit_stroke_attribute, ///< no mention of stroke at all
            m_totally_omit_fill_attribute;   ///< no mention of fill at all

    /// @brief ctor take a path and sets fields to sensible default values
    SvgWrap( const PixPath& pp = PixPath::reserve() )
    :   m_pixpath( pp ),
        m_show_segments( true ),
        m_show_text( true ),
        m_gen_xml_header( true ),
        m_open_svg( true ),
        m_close_svg( true ),
        m_fill_path( false ),
        m_width( 2000 ),
        m_height( 2000 ),
        m_magnify( 1 ),
        m_origin_bias_x( 0 ),
        m_origin_bias_y( 0 ),
        m_color( "black" ),
        m_fill_color( "" ),
        m_totally_omit_stroke_attribute( false ),
        m_totally_omit_fill_attribute( false )
    {}

    /// @brief can opt in/out of showing line segments between points of path
    SvgWrap& set_segs( bool seg = true )
    {
        m_show_segments = seg;
        return *this;
    }

    /// @brief can opt in/out of emitting XML tag at start of output
    SvgWrap& set_xml( bool xml = true )
    {
        m_gen_xml_header = xml;
        return *this;
    }

    /// @brief can opt in/out of emitting opening SVG tag at start of output
    SvgWrap& set_svg( bool svg = true )
    {
        m_open_svg = m_close_svg = svg;
        return *this;
    }

    /// @brief can opt in/out of emitting SVG open, close tags (you pick which)
    SvgWrap& set_svg( bool svg_open, bool svg_close )
    {
        m_open_svg = svg_open;
        m_close_svg = svg_close;
        return *this;
    }

    /// @brief can opt in/out of showing point sequence index
    SvgWrap& set_text( bool txt = true )
    {
        m_show_text = txt;
        return *this;
    }

    /**
     * @brief set the color of the path and text
     * @param color is a string specifying output color.  See
     * http://www.w3.org/Graphics/SVG/IG/resources/svgprimer.html#colors
     * for a presentation of valid color.  In brief, you can use standard
     * named colors like "red," hex values like "#cbacca," or other things.
     * @returns reference to self for easy chaining of setters.
     *
     * We call this the "default color" because it controls the color of lines,
     * text, and polygon fill unless the user provides an overriding value for
     * polygon fill color.
     */
    SvgWrap& set_color( const std::string& color )
    {
        m_color = color;
        return *this;
    }

    /// @brief see overload of same name, but this takes a struct kjb_c::Pixel.
    SvgWrap& set_color( const kjb_c::Pixel& color )
    {
        return set_color( kjb::pixel_as_hex_triplet_string( color ) );
    }

    /// @brief can opt in/out of making this a filled polygon
    SvgWrap& set_fill( bool fill = false )
    {
        m_fill_path = fill;
        return *this;
    }

    /**
     * @brief set the color used for filling the polygon interior (if any).
     *
     * @see set_color() for string format; also "" and "none" are valid.
     *
     * The ctor-time default value is the empty string, which has the semantics
     * of "use the default color."
     *
     * The value here controls the color used to fill the interior points of
     * the polygon defined by the path vertices.  It does not affect stroke
     * color or text color, both of which are set by the default color.
     * If you want the polygons filled, you must turn on the fill flag; if you
     * also want the interior color different from the outline, you must also
     * set the fill color to the name/description of your desired color.
     *
     * Technical points:
     * - If the segments flag or fill flag is off, the fill color is ignored.
     * - Otherwise, if this string is set to the empty string,
     *   then the polygon will be filled with the default color.
     * - If the fill and segments flags are on yet the fill color is "none"
     *   then the polygon will not be filled, i.e., "filled with nothing."
     * - This color is not used for text fill, just polygon fill.
     */
    SvgWrap& set_fill_color( const std::string& color )
    {
        m_fill_color = color;
        return *this;
    }

    /// @brief see overload of same name, but this takes a struct kjb_c::Pixel.
    SvgWrap& set_fill_color( const kjb_c::Pixel& color )
    {
        return set_fill_color( kjb::pixel_as_hex_triplet_string( color ) );
    }

    SvgWrap& set_id( const std::string& );

    /// @brief retrieve the identifier for this rendering
    /// @see set_id
    const std::string& get_id() const
    {
        return m_id;
    }

    SvgWrap& set_path_extra( const std::string& );

    /// @brief retrieve the optional extra-path-attribute string.
    /// @see set_path_extra
    const std::string& get_path_extra() const
    {
        return m_extra;
    }

    /// @brief string for top of a standalone XML file
    static std::string xml_header()
    {
        return "<?xml version='1.0' ?>\n"
        "<!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN'\n"
        "'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n";
    }

    std::string operator()() const;

private:
    /// @brief String for an opening SVG tag.
    std::string svg_open_tag() const
    {
        std::ostringstream tag;
        tag << "<svg xmlns='http://www.w3.org/2000/svg' "
                "xmlns:xlink='http://www.w3.org/1999/xlink' "
                "width='" << m_width * m_magnify << "px' "
                "height='" << m_height * m_magnify << "px' "
                "viewBox='" << m_origin_bias_x << ' ' << m_origin_bias_y
            << ' ' << m_width << ' ' << m_height << "'>\n";
        return tag.str();
    }

    std::string generate_fill_attr() const;
    std::string generate_stroke_attr() const;
};




}
}


#endif
