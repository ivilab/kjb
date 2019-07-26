#include <mcmcda_cpp/mcmcda_fs.h>

using namespace kjb;
using namespace kjb::mcmcda;

// sub directory names
const std::string Experiment_directory::preprocessed_dn = "preprocessed";
const std::string Experiment_directory::blobbed_dn = "blobbed";
const std::string Experiment_directory::pixels_dn = "image-pixels";

// file name formats
const std::string Experiment_directory::points_fnf = "points_t%02d";
const std::string Experiment_directory::preprocessed_fnf = "t%02d";
const std::string Experiment_directory::blobbed_fnf = "t%02d";
const std::string Experiment_directory::pixels_fnf = "t%02d";

// sub directory names
const std::string Association_directory::all_samples_dn = "all-samples";
const std::string Association_directory::tracked_images_dn = "tracked-images";
const std::string Association_directory::curved_images_dn = "curved-images";
const std::string Association_directory::curves_dn = "curves";

// file name formats
const std::string Association_directory::association_fn = "association";
const std::string Association_directory::all_samples_fnf = "sample_%04d";
const std::string Association_directory::tracked_images_fnf = "t%02d.jpg";
const std::string Association_directory::curved_images_fnf = "t%02d.jpg";
const std::string Association_directory::curves_fnf = "track_%04d";
