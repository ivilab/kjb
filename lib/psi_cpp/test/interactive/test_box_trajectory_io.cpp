#include <psi_cpp/psi_trajectory.h>
#include <l_cpp/l_exception.h>
#include <exception>

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

    try
    {
        Box_trajectory_map trajectories;
        trajectories.parse(argv[1], "person");

        trajectories.write("box_trajectories");
    }
    catch(const kjb::Exception& kex)
    {
        kex.print_details();
    }
    catch(const exception& sex)
    {
        cerr << sex.what() << endl;
    }

    return EXIT_SUCCESS;
}

