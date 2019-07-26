#include <psi_cpp/psi_skeleton.h>
#include <psi_cpp/psi_trajectory.h>
#include <string>

using namespace kjb;
using namespace kjb::psi;
using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout<<"Usuage: ./test_psi_skeleton movie_dir  \n";
    }
    
    string movie_dir = argv[1];
    Skeleton_trajectory_map traj;
    traj.parse(movie_dir, "person");
    traj.write("skel_trajectories");
    return EXIT_SUCCESS;
}
