#include <psi_cpp/psi_trajectory.h>
#include <l_cpp/l_exception.h>

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
        Face_trajectory_map trajectories;
        trajectories.parse(argv[1], "person");

        trajectories.write("face_trajectories");
    }
    catch(kjb::Exception kex)
    {
        kex.print_details();
    }
    catch(std::exception e)
    {
        cerr << "exception caught: " << e.what() << endl;
    }
    

    return EXIT_SUCCESS;
}

