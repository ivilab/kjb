/*
 * $Id: test_md5.cpp 8966 2011-03-18 23:23:15Z predoehl $
 */

#include <l/l_incl.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cassert>

#include <wrap_openssl_cpp/wrap_openssl_md5.h>

#define FAIL(m) fail((m),__FILE__,__LINE__)

namespace {

// the following speech is of a length that is a multiple of four (important).
const char* EVD =

    "Yes, in good time we are going to sweep into power "
    "in this nation and throughout the world.  We are going to destroy "
    "all enslaving and degrading capitalist institutions "
    "and re-create them as free and humanizing institutions.  "
    "The world is daily changing before our eyes.  "
    "The sun of capitalism is setting; the sun of socialism is rising.  "
    "It is our duty to build the new nation and the free republic.  "

    "We need industrial and social builders.  "
    "We Socialists are the builders of the beautiful world that is to be. "
    "We are all pledged to do our part.  "

    "We are inviting -- aye challenging you this afternoon "
    "in the name of your own manhood and womanhood "
    "to join us and do your part."; //-- Eugene Victor Debs


const char* const evdsum = "bcd5e4038da5d17aafdf1a8757deb5a6";

const unsigned debs[] = {
        /* on a little-endian ascii system with 4-byte unsigned ints,
         * the numbers below are the same as the speech above.
         */
        745760089u,
        544106784u,
        1685024615u,
        1835627552u,
        1702305893u,
        1701994784u,
        1768908576u,
        1948280686u,
        2004033647u,
        544236901u,
        1869901417u,
        2003791904u,
        1763734117u,
        1752440942u,
        1847620457u,
        1869182049u,
        1851859054u,
        1752440932u,
        1735749490u,
        1953853288u,
        1701344288u,
        1919907616u,
        539911276u,
        543512352u,
        543519329u,
        1852403559u,
        1869881447u,
        1936024608u,
        2037346932u,
        1819042080u,
        1936614688u,
        1769365868u,
        1629513582u,
        1679844462u,
        1634887525u,
        1735289188u,
        1885430560u,
        1818326121u,
        544502633u,
        1953721961u,
        1953854569u,
        1936617321u,
        1684955424u,
        761623072u,
        1634038371u,
        1948280180u,
        544040296u,
        1713402721u,
        543516018u,
        543452769u,
        1634563432u,
        1769630062u,
        1763731310u,
        1769239406u,
        1769239924u,
        779316847u,
        1750343712u,
        1870078053u,
        543452274u,
        1679848297u,
        2037148001u,
        1634231072u,
        1852401518u,
        1700929639u,
        1701998438u,
        1920298784u,
        1702454560u,
        538979955u,
        543516756u,
        544109939u,
        1663067759u,
        1953067105u,
        1936288865u,
        1936269421u,
        1952805664u,
        1735289204u,
        1752440891u,
        1970479205u,
        1718558830u,
        1668248352u,
        1768710505u,
        1763732851u,
        1769087091u,
        1735289203u,
        1226842158u,
        1936269428u,
        1920298784u,
        1953850400u,
        1869881465u,
        1769300512u,
        1948279916u,
        1847616872u,
        1847621477u,
        1869182049u,
        1851859054u,
        1752440932u,
        1919295589u,
        1914725733u,
        1651863653u,
        778266988u,
        1700208672u,
        1701146144u,
        1852383332u,
        1953723748u,
        1818323314u,
        1684955424u,
        1668248352u,
        543973737u,
        1818850658u,
        1936876900u,
        1461723182u,
        1867718757u,
        1818323299u,
        1937011561u,
        1701994784u,
        1701344288u,
        1769300512u,
        1919247468u,
        1718558835u,
        1701344288u,
        1634034208u,
        1718187125u,
        1998613621u,
        1684828783u,
        1634235424u,
        1936269428u,
        544175136u,
        539911522u,
        1629513047u,
        1629513074u,
        1881173100u,
        1734632812u,
        1948279909u,
        1868832879u,
        1920298784u,
        1918988320u,
        538979956u,
        1629513047u,
        1763730802u,
        1953068654u,
        543649385u,
        1629498669u,
        1663067513u,
        1819042152u,
        1768386149u,
        2032166766u,
        1948284271u,
        544434536u,
        1702127201u,
        1869573746u,
        1852383342u,
        1701344288u,
        1835101728u,
        1718558821u,
        1970239776u,
        2003771506u,
        1634541678u,
        1869572206u,
        1851859044u,
        1870078052u,
        1752064365u,
        543453039u,
        1780510580u,
        544106863u,
        1629516661u,
        1679844462u,
        1870209135u,
        1881174645u,
        779383393u,
        0u /* sentinel */
};

// the following speech is of a length that is a multiple of four (important).
const char* FDR =

    "For twelve years this Nation was afflicted "
    "with hear-nothing, see-nothing, do-nothing Government.  "
    "The Nation looked to Government but the Government looked away.  "
    "Nine mocking years with the golden calf"
    " and three long years of the scourge!  "
    "Nine crazy years at the ticker"
    " and three long years in the breadlines!  "
    "Nine mad years of mirage and three long years of despair!  "
    "Powerful influences strive today to restore that kind of government "
    "with its doctrine that that Government is best"
    " which is most indifferent. . . .  "

    "We had to struggle with the old enemies of peace:  "
    "business and financial monopoly, speculation, reckless banking, "
    "class antagonism, sectionalism, war profiteering.  "

    "They had begun to consider the Government of the United States "
    "as a mere appendage to their own affairs.  "
    "We know now that Government by organized money "
    "is just as dangerous as Government by organized mob.  "

    "Never before in all our history have these forces "
    "been so united against one candidate as they stand today.  "
    "They are unanimous in their hate for me, "
    "and I welcome their hatred."; //-- Franklin Delano Roosevelt


const char* const fdrsum = "c6ed84d8b6fc1f2fab5b7b24d367d925";

const unsigned roos[] = {
        544370502u,
        1818589044u,
        2032166262u,
        1936875877u,
        1768453152u,
        1632510067u,
        1852795252u,
        1935767328u,
        1717985568u,
        1952672108u,
        1998611557u,
        543716457u,
        1918985576u,
        1953459757u,
        1735289192u,
        1702043692u,
        1869491557u,
        1852401780u,
        1679830119u,
        1869491567u,
        1852401780u,
        1866932327u,
        1852990838u,
        1953391981u,
        1411391534u,
        1310745960u,
        1869182049u,
        1869357166u,
        1684368239u,
        544175136u,
        1702260551u,
        1701670514u,
        1646294126u,
        1948284021u,
        1193305448u,
        1919252079u,
        1852140910u,
        1869357172u,
        1684368239u,
        1635213600u,
        538979961u,
        1701734734u,
        1668246816u,
        1735289195u,
        1634040096u,
        1998615410u,
        543716457u,
        543516788u,
        1684828007u,
        1663069797u,
        543583329u,
        543452769u,
        1701996660u,
        1869357157u,
        2032166766u,
        1936875877u,
        543584032u,
        543516788u,
        1970234227u,
        560293746u,
        1766727712u,
        1663067502u,
        2038063474u,
        1634040096u,
        1629516658u,
        1752440948u,
        1769218149u,
        1919249251u,
        1684955424u,
        1919448096u,
        1814062437u,
        543649391u,
        1918985593u,
        1852383347u,
        1701344288u,
        1701995040u,
        1768711265u,
        561210734u,
        1766727712u,
        1830839662u,
        2032165985u,
        1936875877u,
        543584032u,
        1634888045u,
        1629513063u,
        1948279918u,
        1701147240u,
        1852795936u,
        1702436967u,
        544436833u,
        1679844975u,
        1634759525u,
        539062889u,
        2003783712u,
        1969648229u,
        1852383340u,
        1702194278u,
        1936024430u,
        1920234272u,
        543520361u,
        1633972084u,
        1869881465u,
        1936028192u,
        1701998452u,
        1634235424u,
        1768628340u,
        1864393838u,
        1869029478u,
        1852990838u,
        1953391981u,
        1953068832u,
        1953046632u,
        1868832883u,
        1769108579u,
        1948280174u,
        544498024u,
        1952540788u,
        1987004192u,
        1835954789u,
        544501349u,
        1646293865u,
        544502629u,
        1667852407u,
        1936269416u,
        1936682272u,
        1852383348u,
        1717987684u,
        1852142181u,
        773860980u,
        773860896u,
        1700208672u,
        1684105248u,
        544175136u,
        1970435187u,
        1701603175u,
        1953068832u,
        1752440936u,
        1819222117u,
        1852121188u,
        1701408101u,
        1718558835u,
        1634037792u,
        540697955u,
        1937072672u,
        1936027241u,
        1851859059u,
        1768300644u,
        1668178286u,
        543973737u,
        1869508461u,
        2037149552u,
        1886593068u,
        1819632485u,
        1869182049u,
        1914711150u,
        1818977125u,
        544437093u,
        1802396002u,
        744975977u,
        1634493216u,
        1629516659u,
        1734440046u,
        1936289391u,
        1931488365u,
        1769235301u,
        1818324591u,
        745370473u,
        1918990112u,
        1869770784u,
        1702127974u,
        1852404325u,
        538979943u,
        2036688980u,
        1684105248u,
        1734697504u,
        1948282485u,
        1868767343u,
        1684632430u,
        1948283493u,
        1193305448u,
        1919252079u,
        1852140910u,
        1718558836u,
        1701344288u,
        1768838432u,
        543450484u,
        1952543827u,
        1629516645u,
        543236211u,
        1701995885u,
        1886413088u,
        1633971813u,
        1948280167u,
        1752440943u,
        544368997u,
        544110447u,
        1634100833u,
        779317865u,
        1700208672u,
        1869507360u,
        1869488247u,
        1752440951u,
        1193309281u,
        1919252079u,
        1852140910u,
        2036473972u,
        1735552800u,
        2053729889u,
        1830839397u,
        2036690543u,
        544434464u,
        1953723754u,
        544432416u,
        1735287140u,
        1970238053u,
        1935745139u,
        1987004192u,
        1835954789u,
        544501349u,
        1864399202u,
        1851877234u,
        1684372073u,
        1651469600u,
        1310728238u,
        1919252069u,
        1717920288u,
        543519343u,
        1629515369u,
        1864395884u,
        1746956917u,
        1869902697u,
        1746958706u,
        543520353u,
        1936025716u,
        1868963941u,
        1936024434u,
        1701143072u,
        1869815918u,
        1768846624u,
        543450484u,
        1767991137u,
        544502638u,
        543518319u,
        1684955491u,
        1952539753u,
        1935745125u,
        1701344288u,
        1953702009u,
        543452769u,
        1633972084u,
        538979961u,
        2036688980u,
        1701994784u,
        1634628896u,
        1869441390u,
        1763734389u,
        1752440942u,
        544368997u,
        1702125928u,
        1919903264u,
        744844576u,
        1684955424u,
        1998604576u,
        1868786789u,
        1948280173u,
        1919509864u,
        1952540704u,
        778331506u,
        0u /* sentinel */
};

int fail( const char* message, const char* file, int line )
{
    std::cout << "Failure:  " << message <<" at " << file <<':' << line <<'\n';
    return EXIT_FAILURE;
}

bool test_string( const char* SPEECH, const unsigned* ar, const std::string& sum )
{
    int speechsz = strlen( SPEECH );

    kjb::MD5 d2;

    if ( d2.is_finalized() )
        return FAIL( "d2 is unused but says it is finalized" ); // wrong answer

    d2.write( (void*) SPEECH, speechsz * sizeof( char ) );

    if ( d2.is_finalized() )
        return FAIL( "d2 is unread but says it is finalized" ); // wrong answer

    std::ostringstream os;
    os.setf( std::ios_base::hex, std::ios_base::basefield );

    const kjb::MD5::Digest::const_iterator PE = d2.end();
    for( kjb::MD5::Digest::const_iterator p = d2.begin(); p != PE; )
        os << std::setw( 2 ) << std::setfill( '0' ) << unsigned( *p++ );

    if ( kjb_c::is_interactive() )
        std::cout << "Reference sum: " << sum
            << "\nComputed sum:  " << os.str() << '\n'
            << ( sum == os.str() ? "success:  they match" : "FAIL: MISMATCH" )
            << '\n';

    if ( sum != os.str() )
        return FAIL( "Speech checksum is incorrect" );

    if ( ! d2.is_finalized() )  // now it IS finalized
        return FAIL( "d2 has been read but says it is not finalized" );

    // Test the iterator interface
    kjb::MD5 d3;
    d3.write( SPEECH, SPEECH + speechsz );
    if ( ! std::equal( d2.begin(), d2.end(), d3.begin() ) )
        return FAIL( "d3 does not match d2" );
    if ( ! std::equal( d3.begin(), d3.end(), d2.begin() ) )
        return FAIL( "reverse d3-d2 comparison disagrees" );

    // Test it again
    const std::string cnu7( SPEECH );
    kjb::MD5 d4;
    d4.write( cnu7.begin(), cnu7.end() );
    if ( ! std::equal( d2.begin(), d2.end(), d4.begin() ) )
        return FAIL( "d4 does not match d2" );

    /*
     * Dangerously architecture-dependent test (works on v01 though);
     * should work on all common little-endian archtectures of the 2010 era.
     */
    kjb::MD5 d5;
    size_t sz = 0;
    while( ar[ sz ] )
        ++sz;
    std::vector< unsigned > w( ar, ar + sz );
    if ( w.size() * sizeof(unsigned) != speechsz ) {
        std::cout << "skipping architecture-dependent test because we are"
                " obviously running on an incompatible architecture; failure "
                "is thus a certainty, but not because of a bug in the code.\n";
        return EXIT_SUCCESS; // never mind
    }

    /*
     * The following line is what we really want to try now:
     * The point is, we want to write arbitrary objects (e.g., unsigned ints)
     * from a container (i.e., via begin and end iterators).
     */
    d5.write( w.begin(), w.end() );

    // Cross your fingers -- this test works at least sometimes, I should add.
    if ( ! std::equal( d5.begin(), d5.end(), d2.begin() ) )
        return FAIL( "d5 does not match d2 (possibly spurious)" );

    return EXIT_SUCCESS;
}

}


int main() {
    if ( test_string( EVD, debs, evdsum ) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    if ( test_string( FDR, roos, fdrsum ) != EXIT_SUCCESS )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
