
#include "l/l_incl.h"


#define DEBUG

int main(void)
{
    char  line[ 1000 ];
    char  the_type[ 100 ] = { '\0' };
    char* line_pos;
    int   in_type = FALSE;


     while (BUFF_FGET_LINE(stdin, line) != EOF)
     {
         line_pos = line;

         if (in_type)
         {
             if (*line_pos == ')')
             {
                 put_line("}");
                 kjb_puts(the_type);
                 kjb_puts(";\n");

                 in_type = FALSE;
             }
             else if (*line_pos == '(')
             {
                 put_line("{");
             }
             else
             {
                 if (line[ 0 ] == '.')
                 {
                     put_line(line);
                 }
                 else
                 {
                     trim_end(line);
                     char_for_char_translate(line, ',', ';');

                     kjb_puts(line);

                     if (isalpha(last_char(line)))
                     {
                         kjb_puts(";");
                     }

                     kjb_puts("\n");
                 }
             }
         }
         else if (HEAD_CMP_EQ(line_pos, "typedef_struct "))
         {
             in_type = TRUE;
             kjb_puts("typedef struct ");
             line_pos += 15;
             put_line(line_pos);
             BUFF_CPY(the_type, line_pos);
         }
         else
         {
             put_line(line);
         }
     }

     return EXIT_SUCCESS;
 }


