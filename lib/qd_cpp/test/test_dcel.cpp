/**
 * @file
 * @brief unit test for doubly connected edge list
 * @author Andrew Predoehl
 */
/*
 * $Id: test_dcel.cpp 20167 2015-12-09 22:13:05Z predoehl $
 */

#include <l/l_sys_std.h>
#include <l/l_sys_io.h>
#include <l/l_error.h>
#include <l/l_global.h>
#include <l/l_init.h>
#include <l_cpp/l_test.h>
#include <qd_cpp/dcel.h>

#include <cctype>
#include <iostream>
#include <algorithm>

namespace
{

using kjb::qd::Doubly_connected_edge_list;
using kjb::qd::RatPoint_line_segment;
using kjb::qd::RatPoint;


// C++ is harrassing me about a locale parameter if I don't wrap this up. :-(
//struct { bool operator()(char c) { return std::isgraph(c); } } isgraf;
struct is_not_graf { bool operator()(char c) { return ! std::isgraph(c); } };


bool match_modulo_whitespace(const std::string& a, const std::string& b)
{
    std::string u, v;
#if 0
    // copy_if is C++11
    std::copy_if(a.begin(), a.end(), std::back_inserter(u), isgraf);
    std::copy_if(b.begin(), b.end(), std::back_inserter(v), isgraf);
#else
    std::remove_copy_if(a.begin(),a.end(), std::back_inserter(u), is_not_graf());
    std::remove_copy_if(b.begin(),b.end(), std::back_inserter(v), is_not_graf());
#endif
    return u == v;
}


bool xml_match(const std::string& a, const std::string& b)
{
    const char* magic = "<?xml version='1.0' ?>";
    const size_t n = strlen(magic);
    return    a.substr(0, n) == magic
           && b.substr(0, n) == magic
           && match_modulo_whitespace(a, b);
}



bool input_parse_match(
    const std::string& xml,
    const Doubly_connected_edge_list& dcel
)
{
    std::istringstream s(xml);
    return dcel == Doubly_connected_edge_list::ctor_xml_stream(s);
}


// Reference results for test1, test2, etc.
const char* o1 =
    "<?xml version='1.0' ?>"
    "<dcel>"
        "<vertices></vertices>"
        "<edges></edges>"
        "<faces><face index='0'><icomponents></icomponents></face></faces>"
    "</dcel>";
const char* o2 =
    "<?xml version='1.0' ?>"
    "<dcel>"
        "<vertices>"
            "<vertex index='0'>"
                "<outedge>0</outedge>"
                "<location><x><rational>"
                    "<numerator>1</numerator>"
                    "<denominator>1</denominator></rational>"
                "</x>"
                "<y><rational>"
                    "<numerator>1</numerator>"
                    "<denominator>1</denominator></rational>"
                "</y>"
                "</location>"
            "</vertex>"
            "<vertex index='1'>"
                "<outedge>1</outedge>"
                "<location><x><rational>"
                    "<numerator>2</numerator>"
                    "<denominator>1</denominator></rational>"
                "</x>"
                "<y><rational>"
                    "<numerator>1</numerator>"
                    "<denominator>1</denominator></rational>"
                "</y>"
                "</location>"
            "</vertex>"
        "</vertices>"
        "<edges>"
            "<edge index='0'>"
                "<origin>0</origin>"
                "<twin>1</twin>"
                "<incface>0</incface>"
                "<next>1</next>"
                "<prev>1</prev>"
            "</edge>"
            "<edge index='1'>"
                "<origin>1</origin>"
                "<twin>0</twin>"
                "<incface>0</incface>"
                "<next>0</next>"
                "<prev>0</prev>"
            "</edge>"
        "</edges>"
        "<faces>"
            "<face index='0'>"
                "<icomponents>"
                    "<edgeindex>0</edgeindex>"
                "</icomponents>"
            "</face>"
        "</faces>"
    "</dcel>";
const char* o3 =
    "<?xml version='1.0' ?>"
    "<dcel>"
    "<vertices>"
        "<vertex index='0'>"
        "<outedge>0</outedge>"
        "<location><x><rational>"
            "<numerator>3</numerator>"
            "<denominator>1</denominator></rational>"
        "</x>"
        "<y><rational>"
            "<numerator>0</numerator>"
            "<denominator>1</denominator></rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='1'>"
            "<outedge>1</outedge>"
            "<location><x><rational>"
                "<numerator>3</numerator>"
                "<denominator>1</denominator></rational>"
            "</x>"
            "<y><rational>"
                "<numerator>2</numerator>"
                "<denominator>1</denominator></rational>"
            "</y>"
            "</location>"
        "</vertex>"
        "<vertex index='2'>"
            "<outedge>3</outedge>"
            "<location><x><rational>"
                "<numerator>1</numerator>"
                "<denominator>1</denominator></rational>"
            "</x>"
            "<y><rational>"
                "<numerator>1</numerator>"
                "<denominator>1</denominator></rational>"
            "</y>"
            "</location>"
        "</vertex>"
        "<vertex index='3'>"
            "<outedge>2</outedge>"
            "<location><x><rational>"
                "<numerator>2</numerator>"
                "<denominator>1</denominator></rational>"
            "</x>"
            "<y><rational>"
                "<numerator>1</numerator>"
                "<denominator>1</denominator></rational>"
            "</y>"
            "</location>"
        "</vertex>"
    "</vertices>"
    "<edges>"
        "<edge index='0'>"
            "<origin>0</origin>"
            "<twin>1</twin>"
            "<incface>0</incface>"
            "<next>1</next>"
            "<prev>1</prev>"
        "</edge>"
        "<edge index='1'>"
            "<origin>1</origin>"
            "<twin>0</twin>"
            "<incface>0</incface>"
            "<next>0</next>"
            "<prev>0</prev>"
        "</edge>"
        "<edge index='2'>"
            "<origin>3</origin>"
            "<twin>3</twin>"
            "<incface>0</incface>"
            "<next>3</next>"
            "<prev>3</prev>"
        "</edge>"
        "<edge index='3'>"
            "<origin>2</origin>"
            "<twin>2</twin>"
            "<incface>0</incface>"
            "<next>2</next>"
            "<prev>2</prev>"
        "</edge>"
    "</edges>"
    "<faces>"
        "<face index='0'>"
            "<icomponents>"
                "<edgeindex>1</edgeindex>"
                "<edgeindex>3</edgeindex>"
            "</icomponents>"
        "</face>"
    "</faces>"
    "</dcel>";

const char* o4 =
    "<?xml version='1.0' ?>"
    "<dcel>"
    "<vertices>"
        "<vertex index='0'>"
            "<outedge>0</outedge>"
            "<location>"
            "<x>"
            "<rational>"
            "<numerator>1</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</x>"
            "<y>"
            "<rational>"
            "<numerator>0</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</y>"
            "</location>"
        "</vertex>"
        "<vertex index='1'>"
            "<outedge>3</outedge>"
            "<location>"
            "<x>"
            "<rational>"
            "<numerator>1</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</x>"
            "<y>"
            "<rational>"
            "<numerator>2</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</y>"
            "</location>"
        "</vertex>"
        "<vertex index='2'>"
            "<outedge>5</outedge>"
            "<location>"
            "<x>"
            "<rational>"
            "<numerator>1</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</x>"
            "<y>"
            "<rational>"
            "<numerator>1</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</y>"
            "</location>"
        "</vertex>"
        "<vertex index='3'>"
            "<outedge>4</outedge>"
            "<location>"
            "<x>"
            "<rational>"
            "<numerator>2</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</x>"
            "<y>"
            "<rational>"
            "<numerator>1</numerator>"
            "<denominator>1</denominator>"
            "</rational>"
            "</y>"
            "</location>"
        "</vertex>"
    "</vertices>"
    "<edges>"
        "<edge index='0'>"
            "<origin>0</origin>"
            "<twin>1</twin>"
            "<incface>0</incface>"
            "<next>2</next>"
            "<prev>1</prev>"
        "</edge>"
        "<edge index='1'>"
            "<origin>2</origin>"
            "<twin>0</twin>"
            "<incface>0</incface>"
            "<next>0</next>"
            "<prev>4</prev>"
        "</edge>"
        "<edge index='2'>"
            "<origin>2</origin>"
            "<twin>3</twin>"
            "<incface>0</incface>"
            "<next>3</next>"
            "<prev>0</prev>"
        "</edge>"
        "<edge index='3'>"
            "<origin>1</origin>"
            "<twin>2</twin>"
            "<incface>0</incface>"
            "<next>5</next>"
            "<prev>2</prev>"
        "</edge>"
        "<edge index='4'>"
            "<origin>3</origin>"
            "<twin>5</twin>"
            "<incface>0</incface>"
            "<next>1</next>"
            "<prev>5</prev>"
        "</edge>"
        "<edge index='5'>"
            "<origin>2</origin>"
            "<twin>4</twin>"
            "<incface>0</incface>"
            "<next>4</next>"
            "<prev>3</prev>"
        "</edge>"
    "</edges>"
    "<faces>"
        "<face index='0'>"
        "<icomponents>"
        "<edgeindex>5</edgeindex>"
        "</icomponents>"
        "</face>"
    "</faces>"
"</dcel>";
const char* o5 =
    "<?xml version='1.0' ?>"
    "<dcel>"
    "<vertices>"
    "<vertex index='0'>"
    "<outedge>6</outedge>"
    "<location>"
    "<x>"
    "<rational>"
    "<numerator>1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</x>"
    "<y>"
    "<rational>"
    "<numerator>-1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</y>"
    "</location>"
    "</vertex>"
    "<vertex index='1'>"
    "<outedge>3</outedge>"
    "<location>"
    "<x>"
    "<rational>"
    "<numerator>-1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</x>"
    "<y>"
    "<rational>"
    "<numerator>-1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</y>"
    "</location>"
    "</vertex>"
    "<vertex index='2'>"
    "<outedge>4</outedge>"
    "<location>"
    "<x>"
    "<rational>"
    "<numerator>-1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</x>"
    "<y>"
    "<rational>"
    "<numerator>1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</y>"
    "</location>"
    "</vertex>"
    "<vertex index='3'>"
    "<outedge>7</outedge>"
    "<location>"
    "<x>"
    "<rational>"
    "<numerator>1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</x>"
    "<y>"
    "<rational>"
    "<numerator>1</numerator>"
    "<denominator>1</denominator>"
    "</rational>"
    "</y>"
    "</location>"
    "</vertex>"
    "</vertices>"
    "<edges>"
    "<edge index='0'>"
    "<origin>0</origin>"
    "<twin>1</twin>"
    "<incface>0</incface>"
    "<next>3</next>"
    "<prev>7</prev>"
    "</edge>"
    "<edge index='1'>"
    "<origin>1</origin>"
    "<twin>0</twin>"
    "<incface>1</incface>"
    "<next>6</next>"
    "<prev>2</prev>"
    "</edge>"
    "<edge index='2'>"
    "<origin>2</origin>"
    "<twin>3</twin>"
    "<incface>1</incface>"
    "<next>1</next>"
    "<prev>5</prev>"
    "</edge>"
    "<edge index='3'>"
    "<origin>1</origin>"
    "<twin>2</twin>"
    "<incface>0</incface>"
    "<next>4</next>"
    "<prev>0</prev>"
    "</edge>"
    "<edge index='4'>"
    "<origin>2</origin>"
    "<twin>5</twin>"
    "<incface>0</incface>"
    "<next>7</next>"
    "<prev>3</prev>"
    "</edge>"
    "<edge index='5'>"
    "<origin>3</origin>"
    "<twin>4</twin>"
    "<incface>1</incface>"
    "<next>2</next>"
    "<prev>6</prev>"
    "</edge>"
    "<edge index='6'>"
    "<origin>0</origin>"
    "<twin>7</twin>"
    "<incface>1</incface>"
    "<next>5</next>"
    "<prev>1</prev>"
    "</edge>"
    "<edge index='7'>"
    "<origin>3</origin>"
    "<twin>6</twin>"
    "<incface>0</incface>"
    "<next>0</next>"
    "<prev>4</prev>"
    "</edge>"
    "</edges>"
    "<faces>"
    "<face index='0'>"
    "<icomponents>"
    "<edgeindex>7</edgeindex>"
    "</icomponents>"
    "</face>"
    "<face index='1'>"
    "<ocomponent><edgeindex>6</edgeindex>"
    "</ocomponent>"
    "<icomponents>"
    "</icomponents>"
    "</face>"
    "</faces>"
    "</dcel>";
const char* o6 =
    "<?xml version='1.0' ?>"
    "<dcel>"
    "<vertices>"
        "<vertex index='0'>"
        "<outedge>0</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>3</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='1'>"
        "<outedge>7</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-3</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='2'>"
        "<outedge>19</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>-1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='3'>"
        "<outedge>14</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>-1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='4'>"
        "<outedge>17</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='5'>"
        "<outedge>20</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='6'>"
        "<outedge>32</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>-2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='7'>"
        "<outedge>25</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>-2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='8'>"
        "<outedge>28</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='9'>"
        "<outedge>31</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='10'>"
        "<outedge>15</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='11'>"
        "<outedge>21</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>1</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='12'>"
        "<outedge>27</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>-2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
        "<vertex index='13'>"
        "<outedge>33</outedge>"
        "<location>"
        "<x>"
        "<rational>"
        "<numerator>2</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</x>"
        "<y>"
        "<rational>"
        "<numerator>0</numerator>"
        "<denominator>1</denominator>"
        "</rational>"
        "</y>"
        "</location>"
        "</vertex>"
    "</vertices>"
    "<edges>"
        "<edge index='0'>"
        "<origin>0</origin>"
        "<twin>1</twin>"
        "<incface>0</incface>"
        "<next>33</next>"
        "<prev>1</prev>"
        "</edge>"
        "<edge index='1'>"
        "<origin>13</origin>"
        "<twin>0</twin>"
        "<incface>0</incface>"
        "<next>0</next>"
        "<prev>31</prev>"
        "</edge>"
        "<edge index='2'>"
        "<origin>10</origin>"
        "<twin>3</twin>"
        "<incface>1</incface>"
        "<next>24</next>"
        "<prev>14</prev>"
        "</edge>"
        "<edge index='3'>"
        "<origin>12</origin>"
        "<twin>2</twin>"
        "<incface>2</incface>"
        "<next>12</next>"
        "<prev>26</prev>"
        "</edge>"
        "<edge index='4'>"
        "<origin>11</origin>"
        "<twin>5</twin>"
        "<incface>3</incface>"
        "<next>15</next>"
        "<prev>19</prev>"
        "</edge>"
        "<edge index='5'>"
        "<origin>10</origin>"
        "<twin>4</twin>"
        "<incface>4</incface>"
        "<next>21</next>"
        "<prev>13</prev>"
        "</edge>"
        "<edge index='6'>"
        "<origin>12</origin>"
        "<twin>7</twin>"
        "<incface>0</incface>"
        "<next>7</next>"
        "<prev>25</prev>"
        "</edge>"
        "<edge index='7'>"
        "<origin>1</origin>"
        "<twin>6</twin>"
        "<incface>0</incface>"
        "<next>27</next>"
        "<prev>6</prev>"
        "</edge>"
        "<edge index='8'>"
        "<origin>13</origin>"
        "<twin>9</twin>"
        "<incface>1</incface>"
        "<next>18</next>"
        "<prev>32</prev>"
        "</edge>"
        "<edge index='9'>"
        "<origin>11</origin>"
        "<twin>8</twin>"
        "<incface>2</incface>"
        "<next>30</next>"
        "<prev>20</prev>"
        "</edge>"
        "<edge index='10'>"
        "<origin>3</origin>"
        "<twin>11</twin>"
        "<incface>3</incface>"
        "<next>19</next>"
        "<prev>15</prev>"
        "</edge>"
        "<edge index='11'>"
        "<origin>2</origin>"
        "<twin>10</twin>"
        "<incface>1</incface>"
        "<next>14</next>"
        "<prev>18</prev>"
        "</edge>"
        "<edge index='12'>"
        "<origin>10</origin>"
        "<twin>13</twin>"
        "<incface>2</incface>"
        "<next>17</next>"
        "<prev>3</prev>"
        "</edge>"
        "<edge index='13'>"
        "<origin>4</origin>"
        "<twin>12</twin>"
        "<incface>4</incface>"
        "<next>5</next>"
        "<prev>16</prev>"
        "</edge>"
        "<edge index='14'>"
        "<origin>3</origin>"
        "<twin>15</twin>"
        "<incface>1</incface>"
        "<next>2</next>"
        "<prev>11</prev>"
        "</edge>"
        "<edge index='15'>"
        "<origin>10</origin>"
        "<twin>14</twin>"
        "<incface>3</incface>"
        "<next>10</next>"
        "<prev>4</prev>"
        "</edge>"
        "<edge index='16'>"
        "<origin>5</origin>"
        "<twin>17</twin>"
        "<incface>4</incface>"
        "<next>13</next>"
        "<prev>21</prev>"
        "</edge>"
        "<edge index='17'>"
        "<origin>4</origin>"
        "<twin>16</twin>"
        "<incface>2</incface>"
        "<next>20</next>"
        "<prev>12</prev>"
        "</edge>"
        "<edge index='18'>"
        "<origin>11</origin>"
        "<twin>19</twin>"
        "<incface>1</incface>"
        "<next>11</next>"
        "<prev>8</prev>"
        "</edge>"
        "<edge index='19'>"
        "<origin>2</origin>"
        "<twin>18</twin>"
        "<incface>3</incface>"
        "<next>4</next>"
        "<prev>10</prev>"
        "</edge>"
        "<edge index='20'>"
        "<origin>5</origin>"
        "<twin>21</twin>"
        "<incface>2</incface>"
        "<next>9</next>"
        "<prev>17</prev>"
        "</edge>"
        "<edge index='21'>"
        "<origin>11</origin>"
        "<twin>20</twin>"
        "<incface>4</incface>"
        "<next>16</next>"
        "<prev>5</prev>"
        "</edge>"
        "<edge index='22'>"
        "<origin>6</origin>"
        "<twin>23</twin>"
        "<incface>0</incface>"
        "<next>25</next>"
        "<prev>33</prev>"
        "</edge>"
        "<edge index='23'>"
        "<origin>7</origin>"
        "<twin>22</twin>"
        "<incface>1</incface>"
        "<next>32</next>"
        "<prev>24</prev>"
        "</edge>"
        "<edge index='24'>"
        "<origin>12</origin>"
        "<twin>25</twin>"
        "<incface>1</incface>"
        "<next>23</next>"
        "<prev>2</prev>"
        "</edge>"
        "<edge index='25'>"
        "<origin>7</origin>"
        "<twin>24</twin>"
        "<incface>0</incface>"
        "<next>6</next>"
        "<prev>22</prev>"
        "</edge>"
        "<edge index='26'>"
        "<origin>8</origin>"
        "<twin>27</twin>"
        "<incface>2</incface>"
        "<next>3</next>"
        "<prev>29</prev>"
        "</edge>"
        "<edge index='27'>"
        "<origin>12</origin>"
        "<twin>26</twin>"
        "<incface>0</incface>"
        "<next>28</next>"
        "<prev>7</prev>"
        "</edge>"
        "<edge index='28'>"
        "<origin>8</origin>"
        "<twin>29</twin>"
        "<incface>0</incface>"
        "<next>31</next>"
        "<prev>27</prev>"
        "</edge>"
        "<edge index='29'>"
        "<origin>9</origin>"
        "<twin>28</twin>"
        "<incface>2</incface>"
        "<next>26</next>"
        "<prev>30</prev>"
        "</edge>"
        "<edge index='30'>"
        "<origin>13</origin>"
        "<twin>31</twin>"
        "<incface>2</incface>"
        "<next>29</next>"
        "<prev>9</prev>"
        "</edge>"
        "<edge index='31'>"
        "<origin>9</origin>"
        "<twin>30</twin>"
        "<incface>0</incface>"
        "<next>1</next>"
        "<prev>28</prev>"
        "</edge>"
        "<edge index='32'>"
        "<origin>6</origin>"
        "<twin>33</twin>"
        "<incface>1</incface>"
        "<next>8</next>"
        "<prev>23</prev>"
        "</edge>"
        "<edge index='33'>"
        "<origin>13</origin>"
        "<twin>32</twin>"
        "<incface>0</incface>"
        "<next>22</next>"
        "<prev>0</prev>"
        "</edge>"
    "</edges>"
    "<faces>"
        "<face index='0'>"
            "<icomponents>"
            "<edgeindex>33</edgeindex>"
            "</icomponents>"
        "</face>"
        "<face index='1'>"
            "<ocomponent><edgeindex>32</edgeindex>"
            "</ocomponent>"
            "<icomponents>"
            "</icomponents>"
        "</face>"
        "<face index='2'>"
            "<ocomponent><edgeindex>30</edgeindex>"
            "</ocomponent>"
            "<icomponents>"
            "</icomponents>"
        "</face>"
        "<face index='3'>"
            "<ocomponent><edgeindex>19</edgeindex>"
            "</ocomponent>"
            "<icomponents>"
            "</icomponents>"
        "</face>"
        "<face index='4'>"
            "<ocomponent><edgeindex>21</edgeindex>"
            "</ocomponent>"
            "<icomponents>"
            "</icomponents>"
        "</face>"
    "</faces>"
    "</dcel>";



int test1()
{
    const Doubly_connected_edge_list d; // empty
    KJB(ERE(is_valid(d)));
    TEST_TRUE(xml_match(o1, xml_output(d)));
    TEST_TRUE(input_parse_match(o1, d));
    return kjb_c::NO_ERROR;
}


int test2()
{
    const RatPoint_line_segment s(RatPoint(1, 1), RatPoint(2, 1));
    const Doubly_connected_edge_list d(s);
    KJB(ERE(is_valid(d)));
    TEST_TRUE(xml_match(o2, xml_output(d)));
    TEST_TRUE(input_parse_match(o2, d));
    return kjb_c::NO_ERROR;
}


int test3()
{
    const RatPoint_line_segment s1(RatPoint(1, 1), RatPoint(2, 1)),
                                s2(RatPoint(3, 0), RatPoint(3, 2));
    const Doubly_connected_edge_list d1(s1);
    const Doubly_connected_edge_list d2(s2);
    const Doubly_connected_edge_list d3(d1.merge(d2));
    KJB(ERE(is_valid(d1)));
    KJB(ERE(is_valid(d2)));
    KJB(ERE(is_valid(d3)));

    std::istringstream ds(o3);
    const Doubly_connected_edge_list
        d_ref(Doubly_connected_edge_list::ctor_xml_stream(ds));
    TEST_TRUE(is_isomorphic(d3, d_ref, 00));
    return kjb_c::NO_ERROR;
}


int test4()
{
    const RatPoint_line_segment s1(RatPoint(1, 1), RatPoint(2, 1)),
                                s2(RatPoint(1, 0), RatPoint(1, 2));
    const Doubly_connected_edge_list d1(s1), d2(s2);
    d1.merge(d2);
    const Doubly_connected_edge_list d3(d1.merge(d2));
    KJB(ERE(is_valid(d1)));
    KJB(ERE(is_valid(d2)));
    KJB(ERE(is_valid(d3)));

    std::istringstream ds(o4);
    const Doubly_connected_edge_list
        d_ref(Doubly_connected_edge_list::ctor_xml_stream(ds));
    TEST_TRUE(is_isomorphic(d3, d_ref, 00));

    return kjb_c::NO_ERROR;
}


Doubly_connected_edge_list square()
{
    const RatPoint_line_segment s1(RatPoint(1, -1), RatPoint(1, 1));
    const RatPoint::Rat rot[9] = {0, -1, 0, 1, 0, 0, 0, 0, 1};
    const Doubly_connected_edge_list d1(s1);
    ETX(is_valid(d1));

    Doubly_connected_edge_list d2(d1), d3(d1), d4;
    d2.transform(rot);
    d3.translate(RatPoint(-2,0));
    d4 = d3;
    d4.transform(rot);
    ETX(is_valid(d2));
    ETX(is_valid(d3));
    ETX(is_valid(d4));

    const Doubly_connected_edge_list d12(d1.merge(d2));
    ETX(is_valid(d12));
    const Doubly_connected_edge_list d34(d3.merge(d4));
    ETX(is_valid(d34));

    return d12.merge(d34);
}


int test5()
{
    const Doubly_connected_edge_list d1234(square());
    KJB(ERE(is_valid(d1234)));

    std::istringstream ds(o5);
    const Doubly_connected_edge_list
        d_ref(Doubly_connected_edge_list::ctor_xml_stream(ds));
    TEST_TRUE(is_isomorphic(d1234, d_ref, 00));

    return kjb_c::NO_ERROR;
}


int test6()
{
    const RatPoint_line_segment s1(RatPoint(3, 0), RatPoint(-3, 0));
    const Doubly_connected_edge_list d1(square()), d4(s1);
    Doubly_connected_edge_list d2(d1);
    const RatPoint::Rat grow[9] = {2, 0, 0, 0, 2, 0, 0, 0, 1};
    d2.transform(grow);
    const Doubly_connected_edge_list d3(d2.merge(d1));
    const Doubly_connected_edge_list d5(d3.merge(d4));
    KJB(ERE(is_valid(d5)));

    std::istringstream ds(o6);
    const Doubly_connected_edge_list
        d_ref(Doubly_connected_edge_list::ctor_xml_stream(ds));
    TEST_TRUE(is_isomorphic(d5, d_ref, 00));

    return kjb_c::NO_ERROR;
}


// Verify that "is_isomorphic" is capable of returning false, too.
int test7()
{
    const char *refs[] = {o1, o2, o3, o4, o5, o6};
    const size_t ref_count = sizeof(refs) / sizeof(char*);
    KJB(ASSERT(6 == ref_count));

    for (size_t i = 0; i < ref_count-1; ++i)
    {
        std::istringstream si(refs[i]);
        const Doubly_connected_edge_list
            di(Doubly_connected_edge_list::ctor_xml_stream(si));
        for (size_t j = i; j < ref_count; ++j)
        {
            std::istringstream sj(refs[j]);
            const Doubly_connected_edge_list
                dj(Doubly_connected_edge_list::ctor_xml_stream(sj));
            if (j > i)
            {
                // All 6 tests are based on different DCELs -- not isomorphic.
                TEST_FALSE(is_isomorphic(di, dj, 00));
                // The code would have to be pretty bad to fail this test:
                TEST_FALSE(xml_output(di) == xml_output(dj));
            }
            else
            {
                // di, dj are different objects but constructed from same xml.
                // That means they are bitwise identical,
                // so they had better test as isomorphic!
                TEST_TRUE(is_isomorphic(di, dj, 00));
                // The code would have to be extremely rotten to fail this:
                TEST_TRUE(xml_output(di) == xml_output(dj));
            }
        }
    }

    return kjb_c::NO_ERROR;
}


#define NOW_PERFORM(f) KJB(EPETE(f()))
// do { std::cout << "now perform " << #f << '\n'; KJB(EPETE(f())); } while(0)

}


int main(int argc, char** argv)
{
    KJB(EPETE(kjb_init()));

    try
    {
        NOW_PERFORM(test1);
        NOW_PERFORM(test2);
        NOW_PERFORM(test3);
        NOW_PERFORM(test4);
        NOW_PERFORM(test5);
        NOW_PERFORM(test6);
        NOW_PERFORM(test7);
    }
    catch (const kjb::Exception& e)
    {
        e.print_details();
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "std::exception caught: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    kjb_c::kjb_cleanup();
    RETURN_VICTORIOUSLY();
}

