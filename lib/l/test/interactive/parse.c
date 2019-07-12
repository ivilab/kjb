
/* $Id: parse.c 21602 2017-07-31 20:36:24Z kobus $ */


#include "l/l_incl.h" 

/*ARGSUSED*/
int main(int argc, char **argv)
{
    char line[ 3000 ]; 
    char word[ 1000 ]; 
    char left_str[ 1000 ]; 
    char right_str[ 1000 ]; 
    int  parse_res; 
    char base_name[ 100 ];
    char suffix[ 100 ]; 
    static const char *suffixes[ 5 ] = { "h", "c", NULL } ; 

    /*
    kjb_set_debug_level(1); 
    */

    term_puts("Testing get_last_token with NULL second argument \n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        while ((parse_res = BUFF_GEN_GET_LAST_TOKEN(line, NULL, " */+-"))!=NO_MORE_TOKENS)
        {
            dbi(parse_res); 
            dbb(line); 
            p_stderr("-------------\n");
        }

        dbi(parse_res); 
        dbb(line); 

        p_stderr("===================\n\n");
    }

    term_puts("Testing get_last_token (should be same as TRIM_BEFORE \n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        while ((parse_res = BUFF_GEN_GET_LAST_TOKEN(line, word, " */+-"))!=NO_MORE_TOKENS)
        {
            dbi(parse_res); 
            dbb(line); 
            dbb(word); 
            p_stderr("-------------\n");
        }

        dbi(parse_res); 
        dbb(line); 
        dbb(word); 

        p_stderr("===================\n\n");
    }

    term_puts("Testing get_last_token_2 with TRIM_BEFORE\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        while ((parse_res = BUFF_GEN_GET_LAST_TOKEN_2(line, word, " */+-", TRIM_BEFORE))!=NO_MORE_TOKENS)
        {
            dbi(parse_res); 
            dbb(line); 
            dbb(word); 
            p_stderr("-------------\n");
        }

        dbi(parse_res); 
        dbb(line); 
        dbb(word); 

        p_stderr("===================\n\n");
    }

    term_puts("Testing get_last_token_2 with TRIM_AFTER\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        while ((parse_res = BUFF_GEN_GET_LAST_TOKEN_2(line, word, " */+-", TRIM_AFTER))!=NO_MORE_TOKENS)
        {
            dbi(parse_res); 
            dbb(line); 
            dbb(word); 
            p_stderr("-------------\n");
        }

        dbi(parse_res); 
        dbb(line); 
        dbb(word); 

        p_stderr("===================\n\n");
    }

    term_puts("Testing gen_split_at_last_token (TRIM_BEFORE)\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        parse_res = BUFF_GEN_SPLIT_AT_LAST_TOKEN_2(line, left_str, right_str, " */+-", TRIM_BEFORE);
        
        dbi(parse_res); 
        dbb(left_str); 
        dbb(right_str); 
        p_stderr("-------------\n");

        p_stderr("===================\n\n");
    }

    term_puts("Testing gen_split_at_last_token (TRIM_AFTER)\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        parse_res = BUFF_GEN_SPLIT_AT_LAST_TOKEN_2(line, left_str, right_str, " */+-", TRIM_AFTER);
        
        dbi(parse_res); 
        dbb(left_str); 
        dbb(right_str); 
        p_stderr("-------------\n");

        p_stderr("===================\n\n");
    }

    term_puts("\n");
    term_puts("Testing get_base_path  with NULL suffixes\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        EPE(get_base_path(line, base_name,  sizeof(base_name),
                          suffix, sizeof(suffix), NULL));

        dbb(base_name); 
        dbb(suffix); 

        p_stderr("===================\n\n");
    }

    term_puts("\n");
    term_puts("Testing get_base_path  with suffixes \".c\" and \".h\"\n\n");

    while ((BUFF_STDIN_GET_LINE("parse> ", line)) != EOF) 
    {
        p_stderr("^^^^^^^^^^^^^\n");

        EPE(get_base_path(line, base_name,  sizeof(base_name),
                          suffix, sizeof(suffix), suffixes));

        dbb(base_name); 
        dbb(suffix); 

        p_stderr("===================\n\n");
    }

    return EXIT_SUCCESS; 
}


