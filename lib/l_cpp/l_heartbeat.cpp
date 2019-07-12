/**
 * @file
 * @brief Implementation of output methods of the Heartbeat class.
 * @author Andrew Predoehl
 */
/*
 * $Id: l_heartbeat.cpp 21596 2017-07-30 23:33:36Z kobus $
 *
 * Recommended tab width:  4
 */

#include "l/l_sys_debug.h"   /* For ASSERT */
#include "l/l_sys_io.h"
#include "l/l_sys_lib.h"
#include "l_cpp/l_heartbeat.h"
#include "l_cpp/l_util.h"

#include <iostream>
#include <sstream>

namespace {

std::string two_units( int units1, char suffix1, int units2, char suffix2 )
{
    ASSERT( 0 < units1 );
    ASSERT( 0 <= units2 );

    std::ostringstream os;
    os << units1 << suffix1;
    if ( 0 < units2 )
    {
        os << ' ' << units2 << suffix2;
    }

    return os.str();
}


/**
 * @brief Convert a number of seconds into a human-readable description
 * @param duration Number of seconds to describe
 *
 * Example:  input is 12345. Output is "3h 25m"
 * Example:  input is 12345678.  Output is "months"
 */
std::string timestring( time_t duration )
{
    if ( duration <= 0 )
    {
        return "0 s";                       // special case
    }

    const time_t    DAYS_PER_YEARS = 730,   // min. # days to count as "years"
                    DAYS_PER_MONTHS = 60,   // min. # days to count as "months"
                    SEC_PER_MIN = 60,
                    MIN_PER_HOUR = 60,
                    HOUR_PER_DAY = 24,
                    SEC_PER_HOUR = SEC_PER_MIN * MIN_PER_HOUR,
                    SEC_PER_DAY = SEC_PER_HOUR * HOUR_PER_DAY;

    int days = duration / SEC_PER_DAY;  // min. # of days the duration spans

    // If the duration is very long, the exact numbers are irrelevant
    if ( days > DAYS_PER_YEARS )
    {
        return "years";                 // very, very long duration
    }
    else if ( days > DAYS_PER_MONTHS )
    {
        return "months";                // very long duration
    }

    duration -= days * SEC_PER_DAY;
    int hours = duration / SEC_PER_HOUR;
    duration -= hours * SEC_PER_HOUR;
    int minutes = duration / SEC_PER_MIN;
    int seconds = duration - minutes * SEC_PER_MIN;

    if ( days > 0 )
    {
        return two_units( days, 'd', hours, 'h' );
    }
    else if ( hours > 0 )
    {
        return two_units( hours, 'h', minutes, 'm' );
    }
    else if ( minutes > 0 )
    {
        return two_units( minutes, 'm', seconds, 's' );
    }
    else
    {
        return two_units( seconds, 's', 0, 0 );
    }
}


}


namespace kjb {


Heartbeat::Heartbeat(
    const std::string& message,
    unsigned total,
    unsigned masklen
)
:   m_msg( message ),
    m_mask( ( 1 << masklen ) - 1 ), // a Mersenne number
    m_total( total ),
    m_denominator( 100.0 / m_total ),
    m_start_time( time( 00 ) ),
    m_progress_counter( 0 ),
    m_is_output_to_tty( kjb_c::kjb_isatty( STDIN_FILENO ) ),
    m_lastlen( 0 )
{
}


std::string Heartbeat::thump( bool finalize ) const
{
    /**
     * @brief the delay time before which we won't extrapolate remaining time.
     *
     * Units are seconds.  Extrapolation could be further delayed beyond this
     * time; this is only a lower bound.
     */
    const time_t AWHILE = 15;

    std::ostringstream buf;

    /* If we are writing to a TTY then we plan to recycle the current line
     * on screen, over and over again.  The line length in general changes each
     * time.  If we were to print a short line in the same space where a longer
     * line was, that would be messy, illegible, confusing or misleading!
     * Thus when we generate a short line, we might also want to append some
     * trailing blanks so that we can obliterate the remnants of the previous
     * line.  So we want to know how long the previous line was.  But actually
     * we do not necessarily care about the entire previous line:  we care
     * about its "isgraph" characters, but not its trailing blanks.  And (you
     * can see where this is going?) if we want to know how many isgraph
     * characters were in the previous line, then humility obliges us to count
     * how many isgraph characters we insert into THIS line.  That's what this
     * accumulator tracks.
     */
    int isgraph_width = 0;

    /*
     * Later we will just add the length of buf to isgraph_width, but here
     * if we insert a carriage return into buf, we preemptively ding
     * isgraph_width by 1 because it is not a graphical character.
     */
    if ( m_is_output_to_tty )
    {
        buf << '\r';
        --isgraph_width;
    }

    int perct = static_cast<int>( m_progress_counter * m_denominator + 0.5f );
    buf << m_msg << ' '
            << m_progress_counter << " / " << m_total << ", " << perct << '%';

    /* Now we (possibly) generate a string with timing status:  either an
     * estimated time of completion or a report of the total time consumed.
     */
    time_t sofar = time( 0 ) - m_start_time;
    if ( finalize )
    {
        buf << " total time: " << timestring( sofar );
    }
    else if ( m_is_output_to_tty && sofar >= AWHILE && perct )
    {
        buf << " Estimated time until completion: "
            << timestring( time_t( sofar
                        * ( float( m_total ) / m_progress_counter - 1.0f ) ) );
    }

    isgraph_width += buf.str().size();

    // Append zero or more spaces to "erase" the remnants of the previous line
    for( int iii = 0; iii < m_lastlen - isgraph_width; ++iii )
    {
        buf << ' ';
    }

    if ( finalize )
    {
        buf << '\n';
    }

    std::cerr << buf.str();
    m_lastlen = isgraph_width;
    return buf.str();
}

}
