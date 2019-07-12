#include <psi_cpp/psi_trajectory.h>
#include <psi_cpp/psi_trajectory_util.h>
#include <st_cpp/st_perspective_camera.h>

using namespace kjb;
using namespace psi;
using namespace std;

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        cout << "Usage: " << argv[0] << " traj-dir cam-file\n";
        return EXIT_SUCCESS;
    }

    Trajectory_map trajectories;
    trajectories.parse(argv[1], "person");

    Perspective_camera cam;
    load(cam, argv[2]);

    Box_trajectory_map box_trajectories =
                get_body_box_trajectory(trajectories, cam);

    trajectories.write("projection_output/trajectories");
    box_trajectories.write("projection_output/box_trajectories");

    return EXIT_SUCCESS;
}

