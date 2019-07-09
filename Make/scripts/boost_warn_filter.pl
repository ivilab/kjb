#!/usr/bin/env perl
# $Id: boost_warn_filter.pl 20108 2015-11-20 16:38:45Z predoehl $
#use strict; use warnings; # turned off in "production" mode, for speed.
my $any_path = "[\\w/-]*";                          # foo/bar//baz (no dots!)
my $fun_name = "'[^']+'";                           # 'foo bar baz'
my $extension = "\\.(?:cpp|cxx|cc|C|h|hpp|hxx)";    # C++ source file suffixes
my $location = "\\d+(?::\\d+)?";                    # line#, optional column#
my $source_line = "$any_path$extension:$location";  # foo/bar.hpp:12
my $from_a_file = "\\s*from $source_line";
my $in_file = "(?:^In file included(?:$from_a_file,\n)*$from_a_file:\n)";
my $be = "^$any_path/boost/$any_path\\.hpp:";       # /foo/boost/bar/baz.hpp:
my $src_from = "$source_line:\\s*(?:instantiat|requir)ed from";
my $skipic = "\\[ skipping \\d+ instantiation context[^\\]]*\\]";
my $src_skip = "(?:$source_line:\\s*$skipic\n(?:$src_from $fun_name\n)+)";
my $src_chain = "(?:(?:$src_from $fun_name\n)*$src_skip?$src_from here\n)";
my $show_me_where = "(?:.*\n\\s*\\^\n)";
my $bw_proper = "(?:$be$location: warning:.*\n$show_me_where?)";
my $func = "(?:(?:static )?member )?function";
my $in_something = "(?:In (?:$func|constructor|destructor) $fun_name)";
my $in_ins_of = "(?:In instantiation of $fun_name)";
my $at_global = "(?:At global scope(?::\n$be $in_ins_of)?)"; # had to hack it
my $context = "(?:$at_global|$in_something|$in_ins_of)";
my $boost_warning = "$in_file(?:(?:$be $context:\n)?$src_chain?$bw_proper+)+";

# Warning filter for macro BOOST_CONCEPT_ASSERT (used outside boost source)
my $bw_concept = "$in_file(?:$any_path$extension: $context:\n$src_chain?"
               . "(?:^$source_line: warning: typedef "
               . "'boost_concept_check\\d+' locally defined but not used.*\n"
               . "\\s*BOOST_CONCEPT_ASSERT$show_me_where)+)+";
{
    local $/;   # slurp mode
    while (<STDIN>) {
        s!$boost_warning!!gmo;
        s!$bw_concept!!gmo;
        die "$_\n" if /Compile status is: [^0]/;
        print;
    }
}
