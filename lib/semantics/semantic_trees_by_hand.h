#ifndef SEMANTIC_TREES_BY_HAND_H_
#define SEMANTIC_TREES_BY_HAND_H_

/*!
 * @file semantic_trees_by_hand.h
 *
 * @author Colin Dawson 
 * $Id: semantic_trees_by_hand.h 18606 2015-02-28 22:30:03Z cdawson $ 
 */

#include "semantics/Elaboration_tree.h"
#include <string>
#include <vector>
#include <boost/make_shared.hpp>

namespace semantics
{
    
struct Sentence_sem
{
    std::string example_id;  // just for referring to these examples taken from captions
    std::string caption_id;  // corresponds to caption number
    std::string sentence;
    std::string image;
    semantics::Elaboration_tree::Self_ptr etree;
    std::string parse;
};

Sentence_sem build_semantic_tree(size_t index);

inline std::vector<Sentence_sem> collect_semantic_trees()
{
    initialize_semantic_maps();
    
	/* // TEST specifying config files by config_file_path_map
	// comment out any of the four semantic_trait category config file
	// path specifications and that semantic_trait will not be filled
	// in.  E.g., for the case that we don't yet handle colors and 
	// sizes, comment out those lines.
    std::map<std::string,std::string> config_file_path_map;
    config_file_path_map["colors"] = "config/colors.config";
    config_file_path_map["sizes"] = "config/sizes.config";
    config_file_path_map["objects"] = "config/objects.config";
    config_file_path_map["binary_relations"] = "config/binary_relations.config";
    initialize_semantic_maps(config_file_path_map);
    */

    std::vector<Sentence_sem> result;
    result.push_back(build_semantic_tree(9));
    // result.push_back(build_semantic_tree_2());
    result.push_back(build_semantic_tree(3));
    // result.push_back(build_semantic_tree_3a());
    // result.push_back(build_semantic_tree_4());
    result.push_back(build_semantic_tree(5));
    result.push_back(build_semantic_tree(6));
    result.push_back(build_semantic_tree(7));
    result.push_back(build_semantic_tree(8));
    result.push_back(build_semantic_tree(9));
    result.push_back(build_semantic_tree(10));
    result.push_back(build_semantic_tree(11));
    result.push_back(build_semantic_tree(12));
    return result;
}


inline Sentence_sem build_semantic_tree(size_t index)
{
    typedef semantics::Elaboration_tree Elaboration_tree;
    
    semantics::Elaboration_tree::Self_ptr e;
    Sentence_sem result;
    switch(index)
    {
    case 1:
	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_color("BROWN");
	e->elaborate_with_relation("FRONT_OF", "COUCH");
	// e->elaborate_with_relation("SUPPORTS", "SAILBOAT");
	e->null_elaboration();
	// e->null_elaboration();
	// e->elaborate_size("SMALL");
	// e->null_elaboration();

	result.example_id = "1";
	result.caption_id = "14";
	result.sentence =
	    "In front of the couch is a wood table with a small sailboat "
	    "model on top . ";
	result.image = "indoor_0021.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (S (NP (DT The) (NN table) ) (VP (VBZ has) "
	    "(NP (NP (DT some) (NNS flowers) ) (PP (IN on) (NP (PRP it) "
	    ") ) ) ) (. .) ) )";

	return result;
	
    case 2:
	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_size("SMALL");
	e->elaborate_with_relation("NEXT_TO", "COUCH");
	e->elaborate_with_relation("SUPPORTS", "LAMP");
	e->null_elaboration();
	e->null_elaboration();
	e->null_elaboration();
	
	result.example_id = "2";
	result.caption_id = "14";
	result.sentence =
	    "Next to the couch is a small table with a lit lamp on top . ";
	result.image = "indoor_0021.jpg";
	result.etree = e;
	result.parse = "";
	
	return result;
	
    case 3:
	// implicitly references "right of" the room
	// simplifies by only referring to one of the chairs and
	// one of the benches

	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_with_relation("CONTAINED_BY_RIGHT", "ROOM");
	e->elaborate_with_relation("NEAR", "CHAIR");
	e->elaborate_with_relation("NEAR", "CHAIR");
	e->elaborate_size("LONG");
	e->elaborate_color("BROWN");
	e->null_elaboration();
	e->elaborate_color("BROWN");
	e->null_elaboration();
	e->elaborate_size("LONG");
	e->null_elaboration();

	result.example_id = "3";
	result.caption_id = "33";
	result.sentence =
	    "One the right is a long wooden table with a wooden chair "
	    "and a long bench around it . ";
	// "On the right is a long wooden table with two wooden chairs
	// "and two long benches around it . "
	result.image = "indoor_0436.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (S (PP (IN On) (NP (DT the) (NN right) ) ) (VP (VBZ is) (NP (NP (DT a) (JJ long) (JJ wooden) (NN table) ) (PP (IN with) (NP (NP (CD two) (JJ wooden) (NNS chairs) ) (CC and) (NP (CD two) ) ) ) ) ) (NP (NP (JJ long) (NNS benches) ) (PP (IN around) (NP (PRP it) ) ) ) (. .) ) )";

	return result;

    case 999:
	// simplifies 3 by removing "right of" the room
	// simplifies by only referring to one of the chairs and one of the benches
	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_with_relation("NEAR", "CHAIR");
	e->elaborate_with_relation("NEAR", "CHAIR");
	e->elaborate_size("LONG");
	e->elaborate_color("BROWN");
	e->null_elaboration();
	e->elaborate_color("BROWN");
	e->null_elaboration();
	e->elaborate_size("LONG");
	e->null_elaboration();

	result.example_id = "3a";
	result.caption_id = "33";
	result.sentence =
	    "There is a long wooden table with a wooden chair and a "
	    "long bench around it . ";
	    // "On the right is a long wooden table with two wooden chairs "
	    // "and two long benches around it . "
	result.image = "indoor_0436.jpg";
	result.etree = e;
	result.parse = "";

	return result;

    case 4:
	// "In front of the table on the left is a couch where the
	// front of the couch is facing right . "
	// Implicitly reference relative to the room: "...on the left..."
	// Also, removed the "where" clause, which references the "front"
	// of the couch "facing right"

	e = boost::make_shared<Elaboration_tree>("COUCH");
	e->elaborate_with_relation("FRONT_OF", "TABLE");
	e->null_elaboration();
	e->elaborate_with_relation("CONTAINED_BY_LEFT", "ROOM");

	result.example_id = "4";
	result.caption_id = "36";
	result.sentence =
	    "In front of the table on the left is a couch . ";
	result.image = "living-room-modern.jpg";
	result.etree = e;
	result.parse = "";

	return result;

    case 5:
	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_with_relation("LEFT_OF", "COUCH");
	// e->elaborate_with_relation("SUPPORTS", "VASE");
	// e->elaborate_with_relation("CENTER", "VASE");
	e->elaborate_color("BLACK");
	// e->null_elaboration();
	// e->null_elaboration();
	// e->elaborate_with_relation("CONTAINS","FLOWERS");
	// e->null_elaboration();
	// e->elaborate_color("WHITE");

	result.example_id = "5";
	result.caption_id = "36";
	result.sentence =
	    "To the left of the couch is a black topped table "
	    "with a vase of white flowers on the center .";
	result.image = "living-room-modern.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (SINV (PP (TO To) (NP (NP (DT the) (NN left) ) (PP (IN of) (NP (DT the) (NN couch) ) ) ) ) (VP (VBZ is) ) (NP (NP (DT a) (JJ black) (VBN topped) (NN table) ) (PP (IN with) (NP (NP (DT a) (NN vase) ) (PP (IN of) (NP (JJ white) (NNS flowers) ) ) ) ) (PP (IN on) (NP (DT the) (NN center) ) ) ) (. .) ) )";

	return result;
    
    case 6:
	e = boost::make_shared<Elaboration_tree>("TABLE"); 
	e->elaborate_with_relation("FRONT_OF", "COUCH");
	e->elaborate_size("TALL");
	// e->elaborate_with_relation("SUPPORTS", "CUP");

	result.example_id = "6";
	result.caption_id = "36";
	result.sentence =
	    "In front of the couch is a tall thin stand with a cup on it . ";
	result.image = "living-room-modern.jpg";
	result.etree = e;
	result.parse = 
	    "(ROOT (SINV (PP (IN In) (NP (NP (NN front) ) (PP (IN of) (NP (DT the) (NN couch) ) ) ) ) (VP (VBZ is) ) (NP (NP (DT a) (JJ tall) (JJ thin) (NN stand) ) (PP (IN with) (NP (NP (DT a) (NN cup) ) (PP (IN on) (NP (PRP it) ) ) ) ) ) (. .) ) )";

	return result;
	
    case 7:
	e = boost::make_shared<Elaboration_tree>("CHAIR");
	e->elaborate_with_relation("FRONT_OF", "BED");
	e->elaborate_with_relation("NEAR", "TABLE");
	e->elaborate_with_relation("NEAR", "MIRROR");

	result.example_id = "7";
	result.caption_id = "80";
	result.sentence =
	    "In front of the bed is a chair with a table and mirror . ";
	result.image = "yellow-bed-room-l.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (SINV (PP (IN In) (NP (NP (NN front) ) (PP (IN of) (NP (DT the) (NN bed) ) ) ) ) (VP (VBZ is) ) (NP (NP (DT a) (NN chair) ) (PP (IN with) (NP (DT a) (NN table) (CC and) (NN mirror) ) ) ) (. .) ) )";
    
	return result;
	
    case 8:
	// Another implicit reference spatial relation relative 
	// to the room, or camera, or image

	e = boost::make_shared<Elaboration_tree>("TABLE");
	e->elaborate_with_relation("CONTAINED_BY", "ROOM");
	e->elaborate_color("WHITE");

	result.example_id = "8";
	result.caption_id = "83";
	result.sentence =
	    "There is a table with a white tablecloth "
	    "in the middle of the room . ";
	result.image = "0000000013.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (S (NP (EX There) ) (VP (VBZ is) (NP (NP (DT a) (NN table) ) (PP (IN with) (NP (NP (DT a) (JJ white) (NN tablecloth) ) (PP (IN in) (NP (NP (DT the) (NN middle) ) (PP (IN of) (NP (DT the) (NN room) ) ) ) ) ) ) ) ) (. .) ) )";

	return result;
	
    case 9:
	e = boost::make_shared<Elaboration_tree>("TABLE");
	// e->elaborate_with_relation("SUPPORTS", "FLOWERS");

	result.example_id = "9";
	result.caption_id = "83";
	result.sentence = "The table has some flowers on it.";
	result.image = "0000000013.jpg";
	result.etree = e;
	result.parse =
	    "(ROOT (S (NP (DT The) (NN table) ) (VP (VBZ has) (NP (NP (DT some) (NNS flowers) ) (PP (IN on) (NP (PRP it) ) ) ) ) (. .) ) )";

	return result;
	
    case 10:
	// lack representation of corner of object (such as the room)
	// this is something we should add to the Room representation

	e = boost::make_shared<Elaboration_tree>("ROOM");
	e->elaborate_with_relation("CONTAINS", "TABLE");
	e->null_elaboration();
	e->elaborate_with_relation("SUPPORTS", "LAMP");
	e->elaborate_color("BROWN");

	result.example_id = "10";
	result.caption_id = "78";
	result.sentence =
	    "A corner of a room with a single wooden table with a lamp on it . ";
	result.image = "IMG_2435.jpg";
	result.etree = e;
	result.parse =
	    "(NP (NP (DT A) (NN corner) ) (PP (IN of) (NP (NP (DT a) (NN room) ) (PP (IN with) (NP (DT a) (JJ single) (JJ wooden) (NN table) ) ) ) ) (PP (IN with) (NP (NP (DT a) (NN lamp) ) (PP (IN on) (NP (PRP it) ) ) ) ) (. .) ) )";

	return result;
	
    case 11:
	e = boost::make_shared<Elaboration_tree>("BED");
	e->elaborate_with_relation("CONTAINED_BY", "ROOM");

	result.example_id = "11";
	result.caption_id = "88";
	result.sentence = "In the middle of the room is a bed . ";
	result.image = "2333_2.jpg";
	result.etree = e;
	result.parse =
	    "(SINV (PP (IN In) (NP (NP (DT the) (NN middle) ) (PP (IN of) (NP (DT the) (NN room) ) ) ) ) (VP (VBZ is) ) (NP (DT a) (NN bed) ) (. .) ) )";
	
	return result;
	
    case 12:
	// see if we can match while glossing over counts/plurals
	e = boost::make_shared<Elaboration_tree>("LAMP");
	e->elaborate_color("GREEN"); // modifies LAMP
	e->elaborate_with_relation("ABOVE", "COUCH");
	// e->elaborate_with_relation("NE", "PICTURE");

	result.example_id = "12";
	result.caption_id = "704";
	result.sentence = 
	    "Above the couch are two green lamps "
	    "and a keep calm carry on poster .";
	result.image = "2086268311_4315366940_m.jpg";
	result.etree = e;
	result.parse =
	    "(S (S (ADVP (IN Above) ) (NP (DT the) (NN couch) ) (VP (VBP are) (NP (CD two) (JJ green) (NNS lamps) ) ) ) (CC and) (S (NP (DT a) ) (VP (VB keep) (ADJP (JJ calm) (SBAR (S (VP (VBP carry) (PP (IN on) (NP (NN poster) ) ) ) ) ) ) ) ) (. .) ) )";

	return result;
    default:
	return result;
    }
}

};

#endif
