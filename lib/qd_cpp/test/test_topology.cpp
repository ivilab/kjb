/*
 * $Id: test_topology.cpp 20623 2016-04-06 03:20:03Z predoehl $
 */

#include <l/l_debug.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/redblack.h>
#include <set>

namespace
{

typedef kjb::qd::Redblack_subtree_sum<char> RBT;

const int data[] = { 
        25686,
        30421,
        22085,
        1822,
        29744,
        15160,
        26830,

        16066,
        23206,
        30536,
        23000,
        23100
};


struct Nodus
{
    int key;
    std::vector<bool> zigzag;

    bool operator==(const Nodus& n) const
    {
        return key == n.key && zigzag == n.zigzag;
    }
    bool operator<(const Nodus& n) const
    {
        if (key != n.key)
        {
            return key < n.key;
        }
        if (zigzag.size() != n.zigzag.size())
        {
            return zigzag.size() < n.zigzag.size();
        }
        for (size_t i = 0; i < zigzag.size(); ++i)
        {
            if (zigzag[i] != n.zigzag[i])
            {
                return n.zigzag[i];
            }
        }
        return false;
    }

    Nodus(int k, const char* s)
    :   key(k),
        zigzag(s ? strlen(s) : 0)
    {
        for (size_t i = 0; *s; ++i)
        {
            zigzag[i] = 'L' == *s++;
        }
    }
};


const Nodus answers[] = {
    Nodus(25686, ""),
        Nodus(15160, "L"),                  
            Nodus(1822,"LL"),
            Nodus(22085,"LR"),
                Nodus(16066,"LRL"),
                Nodus(23100,"LRR"),
                    Nodus(23000,"LRRL"),
                    Nodus(23206,"LRRR"),
        Nodus(29744, "R"),
            Nodus(26830,"RL"),
            Nodus(30421,"RR"),
                Nodus(30536,"RRR")
};


}


int main(int argc, char** argv)
{ 
    RBT rbt;
    std::vector< RBT::Loc_tp > locs;
    for (const int* d = data; d < data + sizeof data / sizeof(int); )
    {
        locs.push_back(rbt.insert(*d++, 0));
    }
    if (locs.size() != sizeof data / sizeof(int))
    {
        KJB(TEST_PSE(("Bad number of locators")));
        return EXIT_FAILURE;
    }
    std::set<Nodus> result,
                    reference(answers,
                                answers + sizeof answers / sizeof(Nodus));
    for (size_t i = 0; i < locs.size(); ++i)
    {
        float k;
        Nodus n(0, "");
        const bool suc = rbt.access_loc(locs[i], &k, 00);
        if (!suc)
        {
            KJB(TEST_PSE(("Cannot accesss node\n")));
            return EXIT_FAILURE;
        }
        n.key = k;
        const int kerr = rbt.get_topology_to_node(locs[i], &n.zigzag);
        KJB(EPE(kerr));
        if (kjb_c::ERROR == kerr)
        {
            KJB(TEST_PSE(("Cannot obtain node topology\n")));
            return EXIT_FAILURE;
        }
        result.insert(n);
    }

    if (result != reference) 
    {
        KJB(TEST_PSE(("Bad results.\n")));
        return EXIT_FAILURE;
    }

    RETURN_VICTORIOUSLY();
}
