/* =========================================================================== *
   |
   |  Copyright (c) 1994-2011 by Kobus Barnard (author)
   |
   |  Personal and educational use of this code is granted, provided that this
   |  header is kept intact, and that the authorship is not misrepresented, that
   |  its use is acknowledged in publications, and relevant papers are cited.
   |
   |  For other use contact the author (kobus AT cs DOT arizona DOT edu).
   |
   |  Please note that the code in this file has not necessarily been adequately
   |  tested. Naturally, there is no guarantee of performance, support, or fitness
   |  for any particular task. Nonetheless, I am interested in hearing about
   |  problems that you encounter.
   |
   |  Author:  Kyle Simek, Jinyan Guan
 * =========================================================================== */

/* $Id: evaluate.cpp 16508 2014-03-06 17:35:37Z jguan1 $ */


#include <iostream>
#include <ostream>
#include <string>
#include <numeric>
#include <l_cpp/l_word_list.h>
#include <l_cpp/l_util.h>
#include <m_cpp/m_vector_d.h>
#include <libgen.h>

#include <boost/lexical_cast.hpp>

#include <tracking_cpp/tracking_trajectory.h>
#include <tracking_cpp/tracking_metrics.h>

#ifdef KJB_HAVE_BST_POPTIONS
#include <boost/program_options.hpp>

using namespace kjb;
using namespace kjb::tracking;
using namespace std;
using namespace boost;
namespace po = boost::program_options;

struct Complete_state
{
    Vector3 position;
    double body_dir;
    Vector2 face_dir;
    bool face_com;

    // This doesn't feel like it should be here; Complete_state is about
    // the 3D state of an actor, independent of the view, whereas the
    // visibility information depends on the view. However, practicality
    // dictates it be here...
    //Visibility visibility;

    /** @brief  Construct an empty (and invalid) complete state. */
    Complete_state() :
        body_dir(std::numeric_limits<double>::max()),
        face_dir(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max()),
        face_com(false)
    {}

    /** @brief  Construct a (invalid) complete state with the given position. */
    Complete_state(const Vector3& pos) :
        position(pos),
        body_dir(std::numeric_limits<double>::max()),
        face_dir(std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max()),
        face_com(false)
    {}

    /** @brief  Construct a complete state with the given parameters. */
    Complete_state
    (
        const Vector3& pos,
        double body_direction,
        const Vector2& face_directions
    ) :
        position(pos),
        body_dir(body_direction),
        face_dir(face_directions),
        face_com(false)
    {}
};

namespace kjb
{
    namespace tracking
    {
        template <>
        bool Generic_trajectory_element<Complete_state>::parse(const std::string& line)
        {
            using namespace std;

            istringstream istr(line);
            vector<double> elems;

            copy(istream_iterator<double>(istr), istream_iterator<double>(),
                 back_inserter(elems));

            KJB(ASSERT(elems.size() > 3));
            IFT(elems.size() <= 7 && elems.size() != 6, Runtime_error,
                "Cannot read trajectory element: line has wrong format.");

            if(elems.back() == 0.0)
            {
                return false;
            }

            // defaults
            value.body_dir = std::numeric_limits<double>::max();
            value.face_dir[0] = value.face_dir[1] = std::numeric_limits<double>::max();

            copy(elems.begin(), elems.begin() + 3, value.position.begin());

            if(elems.size() > 4)
            {
                value.body_dir = elems[3];
            }

            if(elems.size() > 5)
            {
                copy(elems.begin() + 4, elems.begin() + 6, value.face_dir.begin());
            }

            return true;
        }

        template <>
        bool Generic_trajectory<Complete_state>::parse_header(const std::string& line)
        {
            using namespace std;

            istringstream istr(line);
            vector<double> elems;

            copy(istream_iterator<double>(istr), istream_iterator<double>(),
                 back_inserter(elems));

            IFT(istr.eof() || !istr.fail(), IO_error,
                "Trajectory line has invalid format.");

            if(elems.size() > 3)
            {
                return false;
            }

            height = elems[0];

            if(elems.size() > 1)
            {
                width = elems[1];
            }

            if(elems.size() > 2)
            {
                girth = elems[2];
            }

            return true;
        }

    }
}

typedef Generic_trajectory_element<Complete_state> Trajectory_element;
typedef Generic_trajectory<Complete_state> Trajectory;
typedef Generic_trajectory_map<Complete_state> Trajectory_map;

double get_distance(const Complete_state c1, Complete_state c2)
{
    return vector_distance(c1.position, c2.position);
}

class Options
{
public:
    std::string hyp_movie_path;
    std::string gt_movie_path;
    
    std::string entity_name;
    bool per_frame_mode;
    bool shorten;
    size_t shorten_size;
    string shorten_location;
    std::string config_fname;

    double threshold;

    po::options_description make_options()
    {
        po::options_description options("Options");
        options.add_options()
            ("help", "Display this help message")
            ("config", "Configuration file.")
            ("hypothesis-path", po::value<string>(&hyp_movie_path),
                "Path to hypothsis tracks for a single movie")
            ("ground-truth-path", po::value<string>(&gt_movie_path),
                "Path for ground-truth tracks.")
            ("threshold,t", po::value<double>(&threshold)->default_value(0.5),
                "Correspondence threshold (in meters).")
            ("entity-name,e", po::value<string>(&entity_name),
                "Evaluate only tracks for the specified entity")
            ("per-frame,F", po::bool_switch(&per_frame_mode),
                "Output per-frame metrics (single-path mode only)")
            ("shorten-size", po::value<size_t>(&shorten_size),
                "see tracker help")
            ("shorten-location", po::value<std::string>(&shorten_location)
                                                    ->default_value("middle"),
                "see tracker help");

        return options;
    }

    void process_config(const std::string& config_fname, const po::options_description& options, po::variables_map& vm)
    {
        std::ifstream ifs(config_fname.c_str());

        if(ifs.fail())
        {
            std::cerr << "Error opening config file: " << config_fname << std::endl;
            exit(1);
        }

        po::store(po::parse_config_file(ifs, options), vm);
    }

    void parse(int argc, char** argv)
    {
        po::variables_map vm;

        try {

            po::options_description options = make_options();
            po::positional_options_description pstnl;

            if(argc == 1)
            {
                // REGRESSION TEST MODE
                process_config("inputs/evaluate_people_tracking.conf", options, vm);
                po::notify(vm);
                shorten = vm.count("shorten-size");
                return;
            }

            // process options
            po::store(po::command_line_parser(argc, argv).options(options)
                                                .positional(pstnl).run(), vm);

            if(vm.count("help"))
            {
                char* app_name = basename(argv[0]);
                std::cout << "Usage: " <<  app_name 
                          << " [options]\n"
                          << options << "\n";

                exit(EXIT_SUCCESS);
            }

            if(vm.count("config") > 0)
            {
                std::vector<std::string> config_fnames = vm["config"].as<std::vector<std::string> >();

                for(size_t i = 0; i < config_fnames.size(); ++i)
                {
                    process_config(config_fnames[i], options, vm);
                }
            }

            po::notify(vm);

            shorten = vm.count("shorten-size");
        }
        catch(const po::error& err)
        {
            KJB_THROW_2(kjb::Exception, err.what());
        }    

    }
};

Options options;

void init_trajectory
(
    Trajectory_map& traj, 
    const std::string& path, 
    const std::string& entity
)
{
    try
    {
        traj.parse(path, entity);

        if(traj.duration() == 0)
        {
            return;
        }

        size_t num_frames = traj.duration();
        size_t begin_frame, end_frame;
        if(options.shorten)
        {
            size_t ssz = options.shorten_size / 2;
            if(options.shorten_location == "beginning")
            {
                begin_frame = 1;
                end_frame = min(traj.duration(), ssz);
            }
            else if(options.shorten_location == "middle")
            {
                begin_frame = max((size_t)1, traj.duration()/2 - ssz + 1);
                end_frame = min(num_frames, traj.duration()/2 + ssz);
            }
            else
            {
                begin_frame = max((size_t)1, traj.duration() - ssz + 1);
                end_frame = num_frames;
            }
        }
        else
        {
            begin_frame = 1;
            end_frame = num_frames;
        }

        BOOST_FOREACH(Trajectory_map::value_type& pr, traj)
        {
            Trajectory& tr = pr.second;
            for(size_t i = 0; i < begin_frame; i++)
            {
                tr[i] = boost::none;
            }

            for(size_t i = end_frame; i < traj.size(); i++)
            {
                tr[i] = boost::none;
            }
        }
    }
    catch(Exception& ex)
    {
        KJB_THROW_3(IO_error, "%s\nFailed to parse tracks.", 
                    (ex.get_msg().c_str()));
    }

}

kjb::Vector get_vector_(const Complete_state& s) {
    return kjb::Vector(s.position.begin(), s.position.end());
}

int main(int argc, char** argv)
{
    try
    {
        options.parse(argc, argv);

        string gt_dir = options.gt_movie_path;
        if(gt_dir[gt_dir.size()-1] != '/')
            gt_dir += '/';

        string results_dir = options.hyp_movie_path;
        if(results_dir[results_dir.size()-1] != '/')
            results_dir += '/';

        string entity_name = options.entity_name;
        double threshold = options.threshold;

        Trajectory_map hyp_trajs;
        Trajectory_map gt_trajs;

        init_trajectory(gt_trajs, gt_dir, entity_name);
        if(gt_trajs.duration() == 0)
        {
            cout << "Ground truth does not contain any trajectories\n";
            return EXIT_SUCCESS;
        }

        init_trajectory(hyp_trajs, results_dir, entity_name);
        if(hyp_trajs.duration() == 0)
        {
            hyp_trajs.duration() == gt_trajs.duration();
        }
        assert(hyp_trajs.duration() == gt_trajs.duration());

        Canonical_trajectory_map gt_trajs_canonical = gt_trajs.to_canonical(get_vector_);
        Canonical_trajectory_map hyp_trajs_canonical = hyp_trajs.to_canonical(get_vector_);
        vector<Correspondence> traj_matching = 
                            get_correspondence(
                                    gt_trajs_canonical,
                                    hyp_trajs_canonical,
                                    threshold);

        vector<size_t> mme_ct;
        vector<size_t> fp_ct;
        vector<size_t> miss_ct;
        vector<size_t> match_ct;
        std::vector<size_t> obj_ct;
        std::vector<double> dists;
        get_counts_and_distances(
            traj_matching, 
            gt_trajs_canonical,
            hyp_trajs_canonical,
            mme_ct,
            fp_ct,
            miss_ct,
            match_ct,
            obj_ct,
            dists
        );

        double mota;
        double motp;
        get_mota_and_motp(mme_ct, fp_ct, miss_ct, match_ct, obj_ct, dists, 
                mota, motp);

        double mt;
        double ml;
        size_t frags;
        size_t id_switch;
        get_mt_ml_fragment_and_id_switch(
                traj_matching,
                gt_trajs_canonical,
                hyp_trajs_canonical,
                    mt, ml, frags, id_switch);

        size_t mme = std::accumulate(mme_ct.begin(), mme_ct.end(), 0);
        size_t fp = std::accumulate(fp_ct.begin(), fp_ct.end(), 0);
        size_t miss = std::accumulate(miss_ct.begin(), miss_ct.end(), 0);
        size_t match = std::accumulate(match_ct.begin(), match_ct.end(), 0);
        size_t obj = std::accumulate(obj_ct.begin(), obj_ct.end(), 0);
        double D = std::accumulate(dists.begin(), dists.end(), 0.0);

        cout << "motp = " << motp << endl;
        cout << "mota = " << mota << endl;
        cout << "mot_distance = " << D << endl;
        cout << "mot_matches = " << match << endl;
        cout << "mot_errors = " << fp + miss + mme  << endl;
        cout << "mot_objects = " << obj << endl;;
        cout << "mt = " << mt << endl;
        cout << "ml = " << ml << endl;
        cout << "pt = " << 1 - mt - ml << endl;
        cout << "frag = " << frags << endl;
        cout << "ids = " << id_switch << endl;
    }
    catch(Exception& ex)
    {
        //cerr << ex.get_msg() << endl;
        ex.print_details();
        return EXIT_FAILURE;
    }

    return 0;
}

#else // KJB_HAVE_BOOST_POPTION
#include <iostream>
int main()
{
    std::cerr << "Boost Program Options library not installed." << std::endl;
}
#endif
