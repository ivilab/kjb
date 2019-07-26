/**
 * @file
 * @brief Contains definition of class Heartbeat.
 * @author Andrew Predoehl
 *
 * The Heartbeat class is useful for making a slow, interactive, console-based
 * program a little friendlier, by extrapolating how much longer it might run.
 */
/*
 * $Id: l_heartbeat.h 14984 2013-07-22 23:46:33Z predoehl $
 *
 * Recommended tab width:  4
 */


#ifndef HEARTBEAT_H_PREDOEHL_UOFARIZONAVISION
#define HEARTBEAT_H_PREDOEHL_UOFARIZONAVISION

#include <string>

namespace kjb {


/**
 * @brief A class for for indicating the status of slow-moving loops.
 *
 * @ingroup MiscellaneousHelpers
 *
 * This class is really only appropriate when you know in advance how many
 * iterations will occur for your loop.  Assuming you know that number, then
 * you would use this class something like as shown
 *
 * @code
 * Heartbeat heart( "Eating green eggs and ham", BAZILLION ); // ctor
 * for( int iteration = 0; iteration < BAZILLION; ++iteration )
 * {
 *      heart.beat();   // call at every single iteration
 *      Grn_eggs_ham.eat( iteration );
 * }
 * heart.stop(); // call stop() one time, at the end of the code
 * @endcode
 *
 * @todo:  show some sample output here
 */
class Heartbeat {
    const std::string m_msg;        ///< string description of this heartbeat
    const unsigned m_mask;          ///< log_2 of thumps-per-update
    const unsigned m_total;         ///< total number of thumps
    const float m_denominator;      ///< "percent complete" measure of a thump
    const time_t m_start_time;      ///< absolute time of obj. instantiation
    unsigned m_progress_counter;    ///< number of thumps so far
    const bool m_is_output_to_tty;  ///< flag: is the output to a terminal?

    /// @brief how long (not counting any suffix of blanks) was the last line?
    mutable int m_lastlen;

    /**
     * @brief send a progress string to standard output (or a final report)
     * @param finalize  optional flag; usually omitted or true, indicating this
     *                  thump is not the last one.  Set it to false when the
     *                  final call takes place, to dispense with the
     *                  extrapolation (if any) and see the total time that
     *                  was required.
     * @post output string is sent to stdout (regardless of m_is_output_to_tty)
     * @return string output that was also sent to stdout, in case you want it.
     */
    std::string thump( bool finalize = false ) const;

public:

    /**
     * @brief Make a "heart" which will beat while your slow algorithm runs.
     * @param message   Name or short description of the algorithm
     * @param total     Total number of beats, that is, the number of steps the
     *                  algorithm will run.
     * @param masklen   Indicates the period of the heartbeat, i.e., a big
     *                  value means (exponentially) slower beat.
     *                  Fraction of calls to beat() that produce output is
     *                  two raised to minus this power (hence default is that
     *                  out of 1024 calls to beat(), only one produces output).
     *
     * The idea is that you call the beat() method for this object inside the
     * outermost loop of your code, and a fraction, such as one in every 1024
     * calls to beat(), causes timing output to be emitted on standard output.
     *
     * This ctor calls isatty(3) to determine if it can recycle the line it is
     * printing to; if standard input is a TTY (not a file) then this uses the
     * carriage return character to re-use the line.  But if it is a file, then
     * this emits a newline character and each output string thus shows up on a
     * separate line.  A kluge.
     */
    Heartbeat(
        const std::string& message,
        unsigned total = 1,
        unsigned masklen = 10
    );


    /**
     * @brief Indicate one tiny step of your algorithm's execution.
     *
     * This indicates one small step of your algorithm or loop is occurring,
     * which usually passes silently, but on a deliberately small fraction of
     * calls to this method, some timing output will be  generated and sent to
     * console standard error so that the user of the program does not despair.
     *
     * However, that's only if standard input is attached TO the console, i.e.,
     * you are running the program "interactively."  If standard input is not
     * coming from a TTY, then this object assumes the program is running in
     * batch mode, in which case this method emits nothing to standard
     * output.
     *
     * Also the same string is returned, in case you want it.
     * (For maximum speed, just ignore the string that is returned.)
     * @return the progress string that was emitted via stdout.
     */
    std::string beat()
    {
        std::string retval;
        if ( m_is_output_to_tty && 0 == (m_progress_counter & m_mask) )
        {
            retval = thump();   // finalize default value is false
        }
        ++m_progress_counter;
        return retval;
    }

    /**
     * @brief Call this when your algorithm is finished, to see total time.
     *
     * This is an optional cleanup method; if you call it (ONCE) at the end of
     * your algorithm or loop, then you can show the user the total amount of
     * time expended during the loop.
     *
     * This method sends its output to standard output regardless of whether
     * it thinks the program is interactive.
     * @return output string that was sent to stdout, in case you want a copy
     */
    std::string stop() const
    {
        return thump( /* set finalize to... */ true );
    }
};

}

#endif
