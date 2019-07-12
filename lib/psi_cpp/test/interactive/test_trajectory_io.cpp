#include <psi_cpp/psi_trajectory.h>

using namespace kjb;
using namespace psi;
using namespace std;

int main(int argc, char** argv)
{
    if(argc != 2)
    {
        cout << "Usage: " << argv[0] << " movie-gt-dir\n";
        return EXIT_SUCCESS;
    }

    Trajectory_map trajectories;
    trajectories.parse(argv[1], "person");

    trajectories.write("trajectories");

    return EXIT_SUCCESS;
}

