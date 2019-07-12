
/* $Id: utility.c 4727 2009-11-16 20:53:54Z kobus $ */

#ifndef __C2MAN__     

/*
 * Make sure that these files are not a liability when there is no X11. If this
 * is the case, then comment out the whole thing.
*/
#ifdef KJB_HAVE_X11 

/* -------------------------------------------------------------------------- */

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%             U   U  TTTTT  IIIII  L      IIIII  TTTTT  Y   Y                 %
%             U   U    T      I    L        I      T     Y Y                  %
%             U   U    T      I    L        I      T      Y                   %
%             U   U    T      I    L        I      T      Y                   %
%              UUU     T    IIIII  LLLLL  IIIII    T      Y                   %
%                                                                             %
%                                                                             %
%                       ImageMagick Utility Routines                          %
%                                                                             %
%                                                                             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                              January 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright 1996 E. I. du Pont de Nemours and Company                        %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. du Pont de Nemours    %
%  and Company not be used in advertising or publicity pertaining to          %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. du Pont de Nemours and Company makes no representations %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. du Pont de Nemours and Company disclaims all warranties with regard  %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. du Pont de Nemours and Company be     %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortuous action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/magick.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
  Function prototypes.
*/
static int
  ColorCompare _Declare((const void *,const void *)),
  FileCompare _Declare((const void *,const void *));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A p p e n d I m a g e F o r m a t                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function AppendImageFormat appends the image format type to the filename.
%  If an extension to the file already exists, it is first removed.
%
%  The format of the AppendImageFormat routine is:
%
%       AppendImageFormat(format,filename)
%
%  A description of each parameter follows.
%
%   o  format:  Specifies a pointer to an array of characters.  This is the
%      format of the image.
%
%   o  filename:  Specifies a pointer to an array of characters.  The unique
%      file name is returned in this array.
%
%
*/
void AppendImageFormat(char *format, char *filename)
{
  char
    staging[MaxTextLength];

  register char
    *p;

  if ((format == (char *) NULL) || (*format == '\0'))
    return;
  if ((filename == (char *) NULL) || (*filename == '\0'))
    return;
  p=filename+strlen(filename)-1;
  while ((p > filename) && (*(p-1) != '.'))
    p--;
  if ((p > filename) && (*(p-1) == '.'))
    (void) strcpy(p,format);
  else
    {
      (void) sprintf(staging,"%s:%s",format,filename);
      (void) strcpy(filename,staging);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l i e n t N a m e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ClientName removes the path name component and any extensions.
%
%  The format of the ClientName function is:
%
%      ClientName(filename)
%
%  A description of each parameter follows:
%
%    o filename: Specifies a pointer to an character array that contains the
%      filename.
%
%
*/
char *ClientName(char *filename)
{
  register char
    *p;

  static char
    client_name[MaxTextLength];

  if (filename == (char *) NULL)
    return((char *) NULL);
  /*
    Get basename of client.
  */
  (void) strcpy(client_name,filename);
  p=client_name+(strlen(client_name)-1);
  while (p > client_name)
  {
    if (*p == *BasenameSeparator)
      {
        (void) strcpy(client_name,p+1);
        break;
      }
    p--;
  }
  /*
    Delete any extension.
  */
  p=client_name+(strlen(client_name)-1);
  while (p > client_name)
  {
    if (*p == '.')
      {
        *p='\0';
        break;
      }
    p--;
  }
  return(client_name);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d F i l e n a m e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ExpandFilename expands '~' in a filename.
%
%  The format of the ExpandFilename function is:
%
%      ExpandFilename(filename)
%
%  A description of each parameter follows:
%
%    o filename: Specifies a pointer to an character array that contains the
%      filename.
%
%
*/
void ExpandFilename(char *filename)
{
#ifndef vms
  char
    expanded_filename[MaxTextLength];

  register char
    *p;

  if (filename == (char *) NULL)
    return;
  if (*filename != '~')
    return;
  if (*(filename+1) == '/')
    {
      /*
        Substitute ~ with $HOME.
      */
      p=(char *) getenv("HOME");
      if (p == (char *) NULL)
        p=".";
      (void) strcpy(expanded_filename,p);
      (void) strcat(expanded_filename,filename+1);
    }
  else
    {
      char
        username[MaxTextLength];

      struct passwd
        *entry;

      /*
        Substitute ~ with home directory from password file.
      */
      (void) strcpy(username,filename+1);
      p=strchr(username,'/');
      if (p != (char *) NULL)
        *p='\0';
      entry=getpwnam(username);
      if (entry == (struct passwd *) NULL)
        return;
      (void) strcpy(expanded_filename,entry->pw_dir);
      if (p != (char *) NULL)
        {
          (void) strcat(expanded_filename,"/");
          (void) strcat(expanded_filename,p+1);
        }
    }
  (void) strcpy(filename,expanded_filename);
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E x p a n d F i l e n a m e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ExpandFilenames checks each argument of the command line vector and
%  expands it if they have a wildcard character.  For example, *.jpg might
%  expand to:  bird.jpg rose.jpg tiki.jpg.
%
%  The format of the ExpandFilenames function is:
%
%      ExpandFilenames(argc,argv)
%
%  A description of each parameter follows:
%
%    o argc: Specifies a pointer to an integer describing the number of
%      elements in the argument vector.
%
%    o argv: Specifies a pointer to a text array containing the command line
%      arguments.
%
%
*/
void ExpandFilenames(int *argc, char ***argv)
{
  char
    **filelist,
    home_directory[MaxTextLength],
    *option,
    **vector,
    working_directory[MaxTextLength];

  int
    count,
    expanded,
    number_files,
    status;

  ImageInfo
    image_info;

  register char
    *p,
    *q;

  register int
    i,
    j;

  struct stat
    file_info;

  /*
    Allocate argument vector.
  */
  vector=(char **) malloc((*argc+1)*sizeof(char *));
  if (vector == (char **) NULL)
    {
      Warning("Unable to expand filenames",(char *) NULL);
      return;
    }
  /*
    Expand any wildcard filenames.
  */
  (void) getcwd(home_directory,MaxTextLength-1);
  GetImageInfo(&image_info);
  expanded=False;
  count=0;
  for (i=0; i < *argc; i++)
  {
    option=(*argv)[i];
    vector[count++]=option;
    if (((int) strlen(option) > 1) && ((*option == '-') || (*option == '+')))
      continue;
    (void) strcpy(image_info.filename,option);
    SetImageMagick(&image_info);
    if ((*option == '"') || (*option == '\''))
      continue;
    if (!IsGlob(option))
      {
        /*
          Silently skip directories.
        */
        status=stat(option,&file_info);
        if (status == 0)
          if (S_ISDIR(file_info.st_mode))
            count--;
        continue;
      }
    if (strcmp(image_info.magick,"VID") == 0)
      continue;
    if (image_info.subimage != 0)
      continue;
    /*
      Get the list of image file names.
    */
    (void) getcwd(working_directory,MaxTextLength-1);
    for (p=option+strlen(option)-1; p > option; p--)
      if (*p == *BasenameSeparator)
        {
          /*
            Filename includes a directory name.
          */
          q=working_directory;
          for (j=0; j < (p-option+1); j++)
            *q++=option[j];
          *q='\0';
          p++;
          break;
        }
    filelist=ListFiles(working_directory,p,&number_files);
    if (filelist == (char **) NULL)
      continue;
    for (j=0; j < number_files; j++)
    {
      status=stat(filelist[j],&file_info);
      if (status == 0)
        if (!S_ISDIR(file_info.st_mode))
          break;
    }
    if (j == number_files)
      {
        for (j=0; j < number_files; j++)
          free((char *) filelist[j]);
        free((char *) filelist);
        continue;
      }
    /*
      Transfer file list to argument vector.
    */
    vector=(char **)
      realloc(vector,(*argc+count+number_files)*sizeof(char *));
    if (vector == (char **) NULL)
      {
        Warning("Unable to expand filenames",(char *) NULL);
        return;
      }
    count--;
    for (j=0; j < number_files; j++)
    {
      status=stat(filelist[j],&file_info);
      if (status == 0)
        if (S_ISDIR(file_info.st_mode))
          {
            free((char *) filelist[j]);
            continue;
          }
      expanded=True;
      vector[count]=(char *)
        malloc(((p-option)+strlen(filelist[j])+MaxTextLength+1)*sizeof(char));
      if (vector[count] == (char *) NULL)
        {
          Warning("Unable to expand filenames",(char *) NULL);
          for ( ; j < number_files; j++)
            free((char *) filelist[j]);
          free((char *) filelist);
          return;
        }
      (void) sprintf(vector[count],"%.*s%s",p-option,option,filelist[j]);
      free((char *) filelist[j]);
      count++;
    }
    free((char *) filelist);
  }
  free((char *) image_info.filename);
  (void) chdir(home_directory);
  if (!expanded)
    {
      free((char *) vector);
      return;
    }
  *argc=count;
  *argv=vector;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G l o b E x p r e s s i o n                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function GlobExpression returns True if the expression matches the pattern.
%
%  The format of the GlobExpression function is:
%
%      GlobExpression(expression,pattern)
%
%  A description of each parameter follows:
%
%    o expression: Specifies a pointer to a text string containing a file name.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%
*/
int GlobExpression(char *expression, char *pattern)
{
  int
    done;

  if (pattern == (char *) NULL)
    return(True);
  if (strlen(pattern) == 0)
    return(True);
  if (strcmp(pattern,"*") == 0)
    return(True);
  done=False;
  while ((*pattern != '\0') && !done)
  {
    if (*expression == '\0')
      if ((*pattern != '{') && (*pattern != '*'))
        break;
    switch (*pattern)
    {
      case '\\':
      {
        pattern++;
        if (*pattern != '\0')
          pattern++;
        break;
      }
      case '*':
      {
        int
          status;

        pattern++;
        status=False;
        while ((*expression != '\0') && !status)
          status=GlobExpression((char *) expression++,pattern);
        if (status)
          {
            while (*expression != '\0')
              expression++;
            while (*pattern != '\0')
              pattern++;
          }
        break;
      }
      case '[':
      {
        char
          c;

        pattern++;
        for ( ; ; )
        {
          if ((*pattern == '\0') || (*pattern == ']'))
            {
              done=True;
              break;
            }
          if (*pattern == '\\')
            {
              pattern++;
              if (*pattern == '\0')
                {
                  done=True;
                  break;
                }
             }
          if (*(pattern+1) == '-')
            {
              c=(*pattern);
              pattern+=2;
              if (*pattern == ']')
                {
                  done=True;
                  break;
                }
              if (*pattern == '\\')
                {
                  pattern++;
                  if (*pattern == '\0')
                    {
                      done=True;
                      break;
                    }
                }
              if ((*expression < c) || (*expression > *pattern))
                {
                  pattern++;
                  continue;
                }
            }
          else
            if (*pattern != *expression)
              {
                pattern++;
                continue;
              }
          pattern++;
          while ((*pattern != ']') && (*pattern != '\0'))
          {
            if ((*pattern == '\\') && (*(pattern+1) != '\0'))
              pattern++;
            pattern++;
          }
          if (*pattern != '\0')
            {
              pattern++;
              expression++;
            }
          break;
        }
        break;
      }
      case '?':
      {
        pattern++;
        expression++;
        break;
      }
      case '{':
      {
        int
          match;

        register char
          *p;

        pattern++;
        while ((*pattern != '}') && (*pattern != '\0'))
        {
          p=expression;
          match=True;
          while ((*p != '\0') && (*pattern != '\0') &&
                 (*pattern != ',') && (*pattern != '}') && match)
          {
            if (*pattern == '\\')
              pattern++;
            match=(*pattern == *p);
            p++;
            pattern++;
          }
          if (*pattern == '\0')
            {
              match=False;
              done=True;
              break;
            }
          else
            if (match)
              {
                expression=p;
                while ((*pattern != '}') && (*pattern != '\0'))
                {
                  pattern++;
                  if (*pattern == '\\')
                    {
                      pattern++;
                      if (*pattern == '}')
                        pattern++;
                    }
                }
              }
            else
              {
                while ((*pattern != '}') && (*pattern != ',') &&
                       (*pattern != '\0'))
                {
                  pattern++;
                  if (*pattern == '\\')
                    {
                      pattern++;
                      if ((*pattern == '}') || (*pattern == ','))
                        pattern++;
                    }
                }
              }
            if (*pattern != '\0')
              pattern++;
          }
        break;
      }
      default:
      {
        if (*expression != *pattern)
          done=True;
        else
          {
            expression++;
            pattern++;
          }
      }
    }
  }
  while (*pattern == '*')
    pattern++;
  return((*expression == '\0') && (*pattern == '\0'));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t C o l o r s                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ListColors reads the X client color database and returns a list
%  of colors contained in the database sorted in ascending alphabetic order.
%
%  The format of the ListColors function is:
%
%      filelist=ListColors(pattern,number_colors)
%
%  A description of each parameter follows:
%
%    o filelist: Function ListColors returns a list of colors contained
%      in the database.  If the database cannot be read, a NULL list is
%      returned.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_colors:  This integer returns the number of colors in the list.
%
%
*/
static int ColorCompare(const void *x, const void *y)
{
  register char
    **p,
    **q;

  p=(char **) x;
  q=(char **) y;
  return(strcmp(*p,*q));
}

char **ListColors(char *pattern, int *number_colors)
{
  static char
    *ColornameDatabase[]=
    {
      "AliceBlue", "AntiqueWhite", "AntiqueWhite1", "AntiqueWhite2",
      "AntiqueWhite3", "AntiqueWhite4", "BlanchedAlmond", "BlueViolet",
      "CadetBlue", "CadetBlue1", "CadetBlue2", "CadetBlue3", "CadetBlue4",
      "CornflowerBlue", "DarkGoldenrod", "DarkGoldenrod1", "DarkGoldenrod2",
      "DarkGoldenrod3", "DarkGoldenrod4", "DarkGreen", "DarkKhaki",
      "DarkOliveGreen", "DarkOliveGreen1", "DarkOliveGreen2",
      "DarkOliveGreen3", "DarkOliveGreen4", "DarkOrange", "DarkOrange1",
      "DarkOrange2", "DarkOrange3", "DarkOrange4", "DarkOrchid", "DarkOrchid1",
      "DarkOrchid2", "DarkOrchid3", "DarkOrchid4", "DarkSalmon",
      "DarkSeaGreen", "DarkSeaGreen1", "DarkSeaGreen2", "DarkSeaGreen3",
      "DarkSeaGreen4", "DarkSlateBlue", "DarkSlateGray", "DarkSlateGray1",
      "DarkSlateGray2", "DarkSlateGray3", "DarkSlateGray4", "DarkSlateGrey",
      "DarkTurquoise", "DarkViolet", "DeepPink", "DeepPink1", "DeepPink2",
      "DeepPink3", "DeepPink4", "DeepSkyBlue", "DeepSkyBlue1", "DeepSkyBlue2",
      "DeepSkyBlue3", "DeepSkyBlue4", "DimGray", "DimGrey", "DodgerBlue",
      "DodgerBlue1", "DodgerBlue2", "DodgerBlue3", "DodgerBlue4",
      "FloralWhite", "ForestGreen", "GhostWhite", "GreenYellow", "HotPink",
      "HotPink1", "HotPink2", "HotPink3", "HotPink4", "IndianRed",
      "IndianRed1", "IndianRed2", "IndianRed3", "IndianRed4", "LavenderBlush",
      "LavenderBlush1", "LavenderBlush2", "LavenderBlush3", "LavenderBlush4",
      "LawnGreen", "LemonChiffon", "LemonChiffon1", "LemonChiffon2",
      "LemonChiffon3", "LemonChiffon4", "LightBlue", "LightBlue1",
      "LightBlue2", "LightBlue3", "LightBlue4", "LightCoral", "LightCyan",
      "LightCyan1", "LightCyan2", "LightCyan3", "LightCyan4", "LightGoldenrod",
      "LightGoldenrod1", "LightGoldenrod2", "LightGoldenrod3",
      "LightGoldenrod4", "LightGoldenrodYellow", "LightGray", "LightGrey",
      "LightPink", "LightPink1", "LightPink2", "LightPink3", "LightPink4",
      "LightSalmon", "LightSalmon1", "LightSalmon2", "LightSalmon3",
      "LightSalmon4", "LightSeaGreen", "LightSkyBlue", "LightSkyBlue1",
      "LightSkyBlue2", "LightSkyBlue3", "LightSkyBlue4", "LightSlateBlue",
      "LightSlateGray", "LightSlateGrey", "LightSteelBlue", "LightSteelBlue1",
      "LightSteelBlue2", "LightSteelBlue3", "LightSteelBlue4", "LightYellow",
      "LightYellow1", "LightYellow2", "LightYellow3", "LightYellow4",
      "LimeGreen", "MediumAquamarine", "MediumBlue", "MediumOrchid",
      "MediumOrchid1", "MediumOrchid2", "MediumOrchid3", "MediumOrchid4",
      "MediumPurple", "MediumPurple1", "MediumPurple2", "MediumPurple3",
      "MediumPurple4", "MediumSeaGreen", "MediumSlateBlue",
      "MediumSpringGreen", "MediumTurquoise", "MediumVioletRed",
      "MidnightBlue", "MintCream", "MistyRose", "MistyRose1", "MistyRose2",
      "MistyRose3", "MistyRose4", "NavajoWhite", "NavajoWhite1",
      "NavajoWhite2", "NavajoWhite3", "NavajoWhite4", "NavyBlue", "OldLace",
      "OliveDrab", "OliveDrab1", "OliveDrab2", "OliveDrab3", "OliveDrab4",
      "OrangeRed", "OrangeRed1", "OrangeRed2", "OrangeRed3", "OrangeRed4",
      "PaleGoldenrod", "PaleGreen", "PaleGreen1", "PaleGreen2", "PaleGreen3",
      "PaleGreen4", "PaleTurquoise", "PaleTurquoise1", "PaleTurquoise2",
      "PaleTurquoise3", "PaleTurquoise4", "PaleVioletRed", "PaleVioletRed1",
      "PaleVioletRed2", "PaleVioletRed3", "PaleVioletRed4", "PapayaWhip",
      "PeachPuff", "PeachPuff1", "PeachPuff2", "PeachPuff3", "PeachPuff4",
      "PowderBlue", "RosyBrown", "RosyBrown1", "RosyBrown2", "RosyBrown3",
      "RosyBrown4", "RoyalBlue", "RoyalBlue1", "RoyalBlue2", "RoyalBlue3",
      "RoyalBlue4", "SaddleBrown", "SandyBrown", "SeaGreen", "SeaGreen1",
      "SeaGreen2", "SeaGreen3", "SeaGreen4", "SkyBlue", "SkyBlue1", "SkyBlue2",
      "SkyBlue3", "SkyBlue4", "SlateBlue", "SlateBlue1", "SlateBlue2",
      "SlateBlue3", "SlateBlue4", "SlateGray", "SlateGray1", "SlateGray2",
      "SlateGray3", "SlateGray4", "SlateGrey", "SpringGreen", "SpringGreen1",
      "SpringGreen2", "SpringGreen3", "SpringGreen4", "SteelBlue",
      "SteelBlue1", "SteelBlue2", "SteelBlue3", "SteelBlue4", "VioletRed",
      "VioletRed1", "VioletRed2", "VioletRed3", "VioletRed4", "WhiteSmoke",
      "YellowGreen", "alice blue", "antique white", "aquamarine",
      "aquamarine1", "aquamarine2", "aquamarine3", "aquamarine4", "azure",
      "azure1", "azure2", "azure3", "azure4", "beige", "bisque", "bisque1",
      "bisque2", "bisque3", "bisque4", "black", "blanched almond",
      "blue violet", "blue", "blue1", "blue2", "blue3", "blue4", "brown",
      "brown1", "brown2", "brown3", "brown4", "burlywood", "burlywood1",
      "burlywood2", "burlywood3", "burlywood4", "cadet blue", "chartreuse",
      "chartreuse1", "chartreuse2", "chartreuse3", "chartreuse4", "chocolate",
      "chocolate1", "chocolate2", "chocolate3", "chocolate4", "coral",
      "coral1", "coral2", "coral3", "coral4", "cornflower blue", "cornsilk",
      "cornsilk1", "cornsilk2", "cornsilk3", "cornsilk4", "cyan", "cyan1",
      "cyan2", "cyan3", "cyan4", "dark goldenrod", "dark green", "dark khaki",
      "dark olive green", "dark orange", "dark orchid", "dark salmon",
      "dark sea green", "dark slate blue", "dark slate gray",
      "dark slate grey", "dark turquoise", "dark violet", "deep pink",
      "deep sky blue", "dim gray", "dim grey", "dodger blue",
      "firebrick", "firebrick1", "firebrick2", "firebrick3",
      "firebrick4", "floral white", "forest green", "gainsboro", "ghost white",
      "gold", "gold1", "gold2", "gold3", "gold4", "goldenrod", "goldenrod1",
      "goldenrod2", "goldenrod3", "goldenrod4", "gray", "gray0", "gray1",
      "gray10", "gray100", "gray11", "gray12", "gray13", "gray14", "gray15",
      "gray16", "gray17", "gray18", "gray19", "gray2", "gray20", "gray21",
      "gray22", "gray23", "gray24", "gray25", "gray26", "gray27", "gray28",
      "gray29", "gray3", "gray30", "gray31", "gray32", "gray33", "gray34",
      "gray35", "gray36", "gray37", "gray38", "gray39", "gray4", "gray40",
      "gray41", "gray42", "gray43", "gray44", "gray45", "gray46", "gray47",
      "gray48", "gray49", "gray5", "gray50", "gray51", "gray52", "gray53",
      "gray54", "gray55", "gray56", "gray57", "gray58", "gray59", "gray6",
      "gray60", "gray61", "gray62", "gray63", "gray64", "gray65", "gray66",
      "gray67", "gray68", "gray69", "gray7", "gray70", "gray71", "gray72",
      "gray73", "gray74", "gray75", "gray76", "gray77", "gray78", "gray79",
      "gray8", "gray80", "gray81", "gray82", "gray83", "gray84", "gray85",
      "gray86", "gray87", "gray88", "gray89", "gray9", "gray90", "gray91",
      "gray92", "gray93", "gray94", "gray95", "gray96", "gray97", "gray98",
      "gray99", "green yellow", "green", "green1", "green2", "green3",
      "green4", "grey", "grey0", "grey1", "grey10", "grey100", "grey11",
      "grey12", "grey13", "grey14", "grey15", "grey16", "grey17", "grey18",
      "grey19", "grey2", "grey20", "grey21", "grey22", "grey23", "grey24",
      "grey25", "grey26", "grey27", "grey28", "grey29", "grey3", "grey30",
      "grey31", "grey32", "grey33", "grey34", "grey35", "grey36", "grey37",
      "grey38", "grey39", "grey4", "grey40", "grey41", "grey42", "grey43",
      "grey44", "grey45", "grey46", "grey47", "grey48", "grey49", "grey5",
      "grey50", "grey51", "grey52", "grey53", "grey54", "grey55", "grey56",
      "grey57", "grey58", "grey59", "grey6", "grey60", "grey61", "grey62",
      "grey63", "grey64", "grey65", "grey66", "grey67", "grey68", "grey69",
      "grey7", "grey70", "grey71", "grey72", "grey73", "grey74", "grey75",
      "grey76", "grey77", "grey78", "grey79", "grey8", "grey80", "grey81",
      "grey82", "grey83", "grey84", "grey85", "grey86", "grey87", "grey88",
      "grey89", "grey9", "grey90", "grey91", "grey92", "grey93", "grey94",
      "grey95", "grey96", "grey97", "grey98", "grey99", "honeydew",
      "honeydew1", "honeydew2", "honeydew3", "honeydew4", "hot pink",
      "indian red", "ivory", "ivory1", "ivory2", "ivory3", "ivory4", "khaki",
      "khaki1", "khaki2", "khaki3", "khaki4", "lavender blush", "lavender",
      "lawn green", "lemon chiffon", "light blue", "light coral",
      "light cyan", "light goldenrod yellow", "light goldenrod",
      "light gray", "light grey", "light pink", "light salmon",
      "light sea green", "light sky blue", "light slate blue",
      "light slate gray", "light slate grey", "light steel blue",
      "light yellow", "lime green", "linen", "magenta", "magenta1", "magenta2",
      "magenta3", "magenta4", "maroon", "maroon1", "maroon2", "maroon3",
      "maroon4", "medium aquamarine", "medium blue", "medium orchid",
      "medium purple", "medium sea green", "medium slate blue",
      "medium spring green", "medium turquoise", "medium violet red",
      "midnight blue", "mint cream", "misty rose", "moccasin", "navajo white",
      "navy blue", "navy", "old lace", "olive drab", "orange red", "orange",
      "orange1", "orange2", "orange3", "orange4", "orchid", "orchid1",
      "orchid2", "orchid3", "orchid4", "pale goldenrod", "pale green",
      "pale turquoise", "pale violet red", "papaya whip", "peach puff", "peru",
      "pink", "pink1", "pink2", "pink3", "pink4", "plum", "plum1", "plum2",
      "plum3", "plum4", "powder blue", "purple", "purple1", "purple2",
      "purple3", "purple4", "red", "red1", "red2", "red3", "red4",
      "rosy brown", "royal blue", "saddle brown", "salmon", "salmon1",
      "salmon2", "salmon3", "salmon4", "sandy brown", "sea green", "seashell",
      "seashell1", "seashell2", "seashell3", "seashell4", "sienna", "sienna1",
      "sienna2", "sienna3", "sienna4", "sky blue", "slate blue", "slate gray",
      "slate grey", "snow", "snow1", "snow2", "snow3", "snow4", "spring green",
      "steel blue", "tan", "tan1", "tan2", "tan3", "tan4", "thistle",
      "thistle1", "thistle2", "thistle3", "thistle4", "tomato", "tomato1",
      "tomato2", "tomato3", "tomato4", "turquoise", "turquoise1", "turquoise2",
      "turquoise3", "turquoise4", "violet red", "violet", "wheat", "wheat1",
      "wheat2", "wheat3", "wheat4", "white smoke", "white", "yellow green",
      "yellow", "yellow1", "yellow2", "yellow3", "yellow4", (char *) NULL,
    };

  char
    color[MaxTextLength],
    **colorlist,
    text[MaxTextLength];

  FILE
    *database;

  int
    blue,
    count,
    green,
    red;

  register char
    **p;

  unsigned int
    max_colors;

  /*
    Allocate color list.
  */
  max_colors=0;
  for (p=ColornameDatabase; *p != (char *) NULL; p++)
    max_colors++;
  colorlist=(char **) malloc(max_colors*sizeof(char *));
  if (colorlist == (char **) NULL)
    {
      Warning("Unable to read color name database","Memory allocation failed");
      return((char **) NULL);
    }
  /*
    Open database.
  */
  *number_colors=0;
  database=fopen(RGBColorDatabase,"r");
  if (database == (FILE *) NULL)
    {
      /*
        Can't find server color database-- use our color list.
      */
      for (p=ColornameDatabase; *p != (char *) NULL; p++)
        if (GlobExpression(*p,pattern))
          {
            colorlist[*number_colors]=(char *) malloc(strlen(*p)+1);
            if (colorlist[*number_colors] == (char *) NULL)
              break;
            (void) strcpy(colorlist[*number_colors],*p);
            (*number_colors)++;
          }
      return(colorlist);
    }
  while (fgets(text,MaxTextLength-1,database) != (char *) NULL)
  {
    count=sscanf(text,"%d %d %d %[^\n]\n",&red,&green,&blue,color);
    if (count != 4)
      continue;
    if (GlobExpression(color,pattern))
      {
        if (*number_colors >= max_colors)
          {
            max_colors<<=1;
            colorlist=(char **)
              realloc((char **) colorlist,max_colors*sizeof(char *));
            if (colorlist == (char **) NULL)
              {
                Warning("Unable to read color name database",
                  "Memory allocation failed");
                (void) fclose(database);
                return((char **) NULL);
              }
          }
        colorlist[*number_colors]=(char *) malloc(strlen(color)+1);
        if (colorlist[*number_colors] == (char *) NULL)
          break;
        (void) strcpy(colorlist[*number_colors],color);
        (*number_colors)++;
      }
  }
  (void) fclose(database);
  /*
    Sort colorlist in ascending order.
  */
  qsort((void *) colorlist,*number_colors,sizeof(char **),
    (int (*) _Declare((const void *, const void *))) ColorCompare);
  return(colorlist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   L i s t F i l e s                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ListFiles reads the directory specified and returns a list
%  of filenames contained in the directory sorted in ascending alphabetic
%  order.
%
%  The format of the ListFiles function is:
%
%      filelist=ListFiles(directory,pattern,number_entries)
%
%  A description of each parameter follows:
%
%    o filelist: Function ListFiles returns a list of filenames contained
%      in the directory.  If the directory specified cannot be read or it is
%      a file a NULL list is returned.
%
%    o directory: Specifies a pointer to a text string containing a directory
%      name.
%
%    o pattern: Specifies a pointer to a text string containing a pattern.
%
%    o number_entries:  This integer returns the number of filenames in the
%      list.
%
%
*/
static int FileCompare(const void *x, const void *y)
{
  register char
    **p,
    **q;

  p=(char **) x;
  q=(char **) y;
  return(strcmp(*p,*q));
}

char **ListFiles(char *directory, char *pattern, int *number_entries)
{
  char
    **filelist;

  DIR
    *current_directory;

  int
    status;

  struct dirent
    *entry;

  struct stat
    file_info;

  unsigned int
    max_entries;

  /*
    Open directory.
  */
  *number_entries=0;
  status=chdir(directory);
  if (status != 0)
    return((char **) NULL);
  (void) getcwd(directory,MaxTextLength-1);
  current_directory=opendir(directory);
  if (current_directory == (DIR *) NULL)
    return((char **) NULL);
  /*
    Allocate filelist.
  */
  max_entries=2048;
  filelist=(char **) malloc(max_entries*sizeof(char *));
  if (filelist == (char **) NULL)
    {
      (void) closedir(current_directory);
      return((char **) NULL);
    }
  /*
    Save the current and change to the new directory.
  */
  (void) chdir(directory);
  entry=readdir(current_directory);
  while (entry != (struct dirent *) NULL)
  {
    if (*entry->d_name == '.')
      {
        entry=readdir(current_directory);
        continue;
      }
    status=stat(entry->d_name,&file_info);
    if (status == 0)
      if (S_ISDIR(file_info.st_mode) || GlobExpression(entry->d_name,pattern))
        {
          if (*number_entries >= max_entries)
            {
              max_entries<<=1;
              filelist=(char **)
                realloc((char **) filelist,max_entries*sizeof(char *));
              if (filelist == (char **) NULL)
                {
                  (void) closedir(current_directory);
                  return((char **) NULL);
                }
            }
          filelist[*number_entries]=(char *) malloc(strlen(entry->d_name)+2);
          if (filelist[*number_entries] == (char *) NULL)
            break;
          (void) strcpy(filelist[*number_entries],entry->d_name);
          if (S_ISDIR(file_info.st_mode))
            (void) strcat(filelist[*number_entries],DirectorySeparator);
          (*number_entries)++;
        }
    entry=readdir(current_directory);
  }
  (void) closedir(current_directory);
  /*
    Sort filelist in ascending order.
  */
  qsort((void *) filelist,*number_entries,sizeof(char **),
    (int (*) _Declare((const void *, const void *))) FileCompare);
  return(filelist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L S B F i r s t R e a d L o n g                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function LSBFirstReadLong reads a long value as a 32 bit quantity in
%  least-significant Byte first order.
%
%  The format of the LSBFirstReadLong routine is:
%
%       value=LSBFirstReadLong(file)
%
%  A description of each parameter follows.
%
%    o value:  Function LSBFirstReadLong returns an unsigned long read from
%      the file.
%
%   o  file:  Specifies the file to read the data from.
%
%
*/
unsigned long LSBFirstReadLong(FILE *file)
{
  unsigned char
    buffer[4];

  unsigned int
    status;

  unsigned long
    value;

  status=ReadData((char *) buffer,1,4,file);
  if (status == False)
    return((unsigned long) ~0);
  value=(unsigned int) (buffer[3] << 24);
  value|=(unsigned int) (buffer[2] << 16);
  value|=(unsigned int) (buffer[1] << 8);
  value|=(unsigned int) (buffer[0]);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L S B F i r s t R e a d S h o r t                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function LSBFirstReadShort reads a short value as a 16 bit quantity in
%  least-significant Byte first order.
%
%  The format of the LSBFirstReadShort routine is:
%
%       value=LSBFirstReadShort(file)
%
%  A description of each parameter follows.
%
%    o value:  Function LSBFirstReadShort returns an unsigned short read from
%      the file.
%
%   o  file:  Specifies the file to read the data from.
%
%
*/
unsigned short LSBFirstReadShort(FILE *file)
{
  unsigned char
    buffer[2];

  unsigned int
    status;

  unsigned short
    value;

  status=ReadData((char *) buffer,1,2,file);
  if (status == False)
    return((unsigned short) ~0);
  value=(unsigned short) (buffer[1] << 8);
  value|=(unsigned short) (buffer[0]);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L S B F i r s t W r i t e L o n g                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function LSBFirstWriteLong writes a long value as a 32 bit quantity in
%  least-significant Byte first order.
%
%  The format of the LSBFirstWriteLong routine is:
%
%       LSBFirstWriteLong(value,file)
%
%  A description of each parameter follows.
%
%   o  value:  Specifies the value to write.
%
%   o  file:  Specifies the file to write the data to.
%
%
*/
void LSBFirstWriteLong(long unsigned int value, FILE *file)
{
  unsigned char
    buffer[4];

  buffer[0]=(unsigned char) (value);
  buffer[1]=(unsigned char) ((value) >> 8);
  buffer[2]=(unsigned char) ((value) >> 16);
  buffer[3]=(unsigned char) ((value) >> 24);
  (void) fwrite((char *) buffer,1,4,file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  L S B F i r s t W r i t e S h o r t                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function LSBFirstWriteShort writes a long value as a 16 bit quantity in
%  least-significant Byte first order.
%
%  The format of the LSBFirstWriteShort routine is:
%
%       LSBFirstWriteShort(value,file)
%
%  A description of each parameter follows.
%
%   o  value:  Specifies the value to write.
%
%   o  file:  Specifies the file to write the data to.
%
%
*/
void LSBFirstWriteShort(unsigned int value, FILE *file)
{
  unsigned char
    buffer[2];

  buffer[0]=(unsigned char) (value);
  buffer[1]=(unsigned char) ((value) >> 8);
  (void) fwrite((char *) buffer,1,2,file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t O r d e r L o n g                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstOrderLong converts a least-significant Byte first buffer
%  of integers to most-significant Byte first.
%
%  The format of the MSBFirstOrderLong routine is:
%
%       MSBFirstOrderLong(p,length);
%
%  A description of each parameter follows.
%
%   o  p:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
%
*/
void MSBFirstOrderLong(register char *p, register unsigned int length)
{
  register char
    c,
    *q,
    *sp;

  q=p+length;
  while (p < q)
  {
    sp=p+3;
    c=(*sp);
    *sp=(*p);
    *p++=c;
    sp=p+1;
    c=(*sp);
    *sp=(*p);
    *p++=c;
    p+=2;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t O r d e r S h o r t                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstOrderShort converts a least-significant Byte first buffer
%  of integers to most-significant Byte first.
%
%  The format of the MSBFirstOrderShort routine is:
%
%       MSBFirstOrderLongShort(p,length);
%
%  A description of each parameter follows.
%
%   o  p:  Specifies a pointer to a buffer of integers.
%
%   o  length:  Specifies the length of the buffer.
%
%
*/
void MSBFirstOrderShort(register char *p, register unsigned int length)
{
  register char
    c,
    *q;

  q=p+length;
  while (p < q)
  {
    c=(*p);
    *p=(*(p+1));
    p++;
    *p++=c;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t R e a d S h o r t                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstReadShort reads a short value as a 16 bit quantity in
%  most-significant Byte first order.
%
%  The format of the MSBFirstReadShort routine is:
%
%       value=MSBFirstReadShort(file)
%
%  A description of each parameter follows.
%
%    o value:  Function MSBFirstReadShort returns an unsigned short read from
%      the file.
%
%   o  file:  Specifies the file to read the data from.
%
%
*/
unsigned short MSBFirstReadShort(FILE *file)
{
  unsigned char
    buffer[2];

  unsigned int
    status;

  unsigned short
    value;

  status=ReadData((char *) buffer,1,2,file);
  if (status == False)
    return((unsigned short) ~0);
  value=(unsigned int) (buffer[0] << 8);
  value|=(unsigned int) (buffer[1]);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t R e a d L o n g                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstReadLong reads a long value as a 32 bit quantity in
%  most-significant Byte first order.
%
%  The format of the MSBFirstReadLong routine is:
%
%       value=MSBFirstReadLong(file)
%
%  A description of each parameter follows.
%
%    o value:  Function MSBFirstReadLong returns an unsigned long read from
%      the file.
%
%   o  file:  Specifies the file to read the data from.
%
%
*/
unsigned long MSBFirstReadLong(FILE *file)
{
  unsigned char
    buffer[4];

  unsigned int
    status;

  unsigned long
    value;

  status=ReadData((char *) buffer,1,4,file);
  if (status == False)
    return((unsigned long) ~0);
  value=(unsigned int) (buffer[0] << 24);
  value|=(unsigned int) (buffer[1] << 16);
  value|=(unsigned int) (buffer[2] << 8);
  value|=(unsigned int) (buffer[3]);
  return(value);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t W r i t e L o n g                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstWriteLong writes a long value as a 32 bit quantity in
%  most-significant Byte first order.
%
%  The format of the MSBFirstWriteLong routine is:
%
%       MSBFirstWriteLong(value,file)
%
%  A description of each parameter follows.
%
%   o  value:  Specifies the value to write.
%
%   o  file:  Specifies the file to write the data to.
%
%
*/
void MSBFirstWriteLong(long unsigned int value, FILE *file)
{
  unsigned char
    buffer[4];

  buffer[0]=(unsigned char) ((value) >> 24);
  buffer[1]=(unsigned char) ((value) >> 16);
  buffer[2]=(unsigned char) ((value) >> 8);
  buffer[3]=(unsigned char) (value);
  (void) fwrite((char *) buffer,1,4,file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M S B F i r s t W r i t e S h o r t                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function MSBFirstWriteShort writes a long value as a 16 bit quantity in
%  most-significant Byte first order.
%
%  The format of the MSBFirstWriteShort routine is:
%
%       MSBFirstWriteShort(value,file)
%
%  A description of each parameter follows.
%
%   o  value:  Specifies the value to write.
%
%   o  file:  Specifies the file to write the data to.
%
%
*/
void MSBFirstWriteShort(unsigned int value, FILE *file)
{
  unsigned char
    buffer[2];

  buffer[0]=(unsigned char) ((value) >> 8);
  buffer[1]=(unsigned char) (value);
  (void) fwrite((char *) buffer,1,2,file);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d D a t a                                                            %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadData reads data from the image file and returns it.  If it
%  cannot read the requested number of items, False is returned indicating
%  an error.
%
%  The format of the ReadData routine is:
%
%      status=ReadData(data,size,number_items,file)
%
%  A description of each parameter follows:
%
%    o status:  Function ReadData returns True if all the data requested
%      is obtained without error, otherwise False.
%
%    o data:  Specifies an area to place the information reuested from
%      the file.
%
%    o size:  Specifies an integer representing the length of an
%      individual item to be read from the file.
%
%    o number_items:  Specifies an integer representing the number of items
%      to read from the file.
%
%    o file:  Specifies a file to read the data.
%
%
*/
unsigned int ReadData(char *data, unsigned int size, unsigned int number_items, FILE *file)
{
  int
    bytes,
    count;

  count=0;
  for (bytes=size*number_items; bytes > 0; bytes-=count)
  {
    count=fread(data,1,bytes,file);
    if (count <= 0)
      return(False);
    data+=count;
  }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e a d D a t a B l o c k                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function ReadDataBlock reads data from the image file and returns it.  The
%  amount of data is determined by first reading a count Byte.  If
%  ReadDataBlock cannot read the requested number of items, `-1' is returned
%  indicating an error.
%
%  The format of the ReadData routine is:
%
%      status=ReadData(data,file)
%
%  A description of each parameter follows:
%
%    o status:  Function ReadData returns the number of characters read
%      unless there is an error, otherwise `-1'.
%
%    o data:  Specifies an area to place the information reuested from
%      the file.
%
%    o file:  Specifies a file to read the data.
%
%
*/
int ReadDataBlock(char *data, FILE *file)
{
  unsigned char
    count;

  unsigned int
    status;

  status=ReadData((char *) &count,1,1,file);
  if (status == False)
    return(-1);
  if (count == 0)
    return(0);
  status=ReadData(data,1,(unsigned int) count,file);
  if (status == False)
    return(-1);
  return(count);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S t r i n g T o L i s t                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function StringToList converts a text string into a list by segmenting the
%  text string at each carriage return discovered.  The list is converted to
%  HEX characters if any non-printable characters are discovered within the
%  text string.
%
%  The format of the StringToList routine is:
%
%      list=StringToList(text)
%
%  A description of each parameter follows:
%
%    o list:  Function StringToList returns the string list unless an error
%      occurs, otherwise NULL.
%
%    o text:  Specifies the string to segment into a list.
%
%
*/
char **StringToList(char *text)
{
  char
    **textlist;

  register char
    *p,
    *q;

  register int
    i;

  unsigned int
    lines;

  if (text == (char *) NULL)
    return((char **) NULL);
  for (p=text; *p != '\0'; p++)
    if (!(isprint(*p) || isspace(*p)))
      break;
  if (*p == '\0')
    {
      /*
        Convert string to an ASCII list.
      */
      lines=1;
      for (p=text; *p != '\0'; p++)
        if (*p == '\n')
          lines++;
      textlist=(char **) malloc((lines+1)*sizeof(char *));
      if (textlist == (char **) NULL)
        {
          Warning("Unable to convert text","Memory allocation failed");
          return((char **) NULL);
        }
      p=text;
      for (i=0; i < lines; i++)
      {
        for (q=p; *q != '\0'; q++)
          if ((*q == '\r') || (*q == '\n'))
            break;
        textlist[i]=(char *) malloc((q-p+1)*sizeof(char));
        if (textlist[i] == (char *) NULL)
          {
            Warning("Unable to convert text","Memory allocation failed");
            return((char **) NULL);
          }
        (void) strncpy(textlist[i],p,q-p);
        textlist[i][q-p]='\0';
        if (*q == '\r')
          q++;
        p=q+1;
      }
    }
  else
    {
      char
        hex_string[MaxTextLength];

      register int
        j;

      /*
        Convert string to a HEX list.
      */
      lines=((int) strlen(text)/0x14)+1;
      textlist=(char **) malloc((lines+1)*sizeof(char *));
      if (textlist == (char **) NULL)
        {
          Warning("Unable to convert text","Memory allocation failed");
          return((char **) NULL);
        }
      p=text;
      for (i=0; i < lines; i++)
      {
        textlist[i]=(char *) malloc(900*sizeof(char));
        if (textlist[i] == (char *) NULL)
          {
            Warning("Unable to convert text","Memory allocation failed");
            return((char **) NULL);
          }
        (void) sprintf(textlist[i],"0x%08x: ",(unsigned int) (i*0x14));
        q=textlist[i]+strlen(textlist[i]);
        for (j=1; j <= Min((int) strlen(p),0x14); j++)
        {
          (void) sprintf(hex_string,"%02x",(unsigned int) (*(p+j)));
          (void) strcpy(q,hex_string);
          q+=2;
          if ((j % 0x04) == 0)
            *q++=' ';
        }
        for (; j <= 0x14; j++)
        {
          *q++=' ';
          *q++=' ';
          if ((j % 0x04) == 0)
            *q++=' ';
        }
        *q++=' ';
        for (j=1; j <= Min((int) strlen(p),0x14); j++)
        {
          if (isprint(*p))
            *q++=(*p);
          else
            *q++='-';
          p++;
        }
        *q='\0';
      }
    }
  textlist[i]=(char *) NULL;
  return(textlist);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P o s t s c r i p t G e o m e t r y                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PostscriptGeometry replaces any page mneumonic with the equivalent
%  size in picas.
%
%  The format of the PostscriptGeometry routine is:
%
%       geometry=PostscriptGeometry(page)
%
%  A description of each parameter follows.
%
%   o  page:  Specifies a pointer to an array of characters.  The string is
%      either a Postscript page name (e.g. A4) or a postscript page geometry
%      (e.g. 612x792+36+36).
%
%
*/
char *PostscriptGeometry(char *page)
{
  static char
    *PageSizes[][2]=
    {
      { "LETTER", "612x792" },
      { "TABLOID", "792x1224" },
      { "LEDGER", "1224x792" },
      { "LEGAL", " 612x1008" },
      { "STATEMENT", "396x612" },
      { "EXECUTIVE", "540x720" },
      { "A3", "842x1191" },
      { "A4", "595x842" },
      { "A5", "421x595" },
      { "B4", "729x1032" },
      { "B5", "516x729" },
      { "FOLIO", "612x936" },
      { "QUARTO", "610x780" },
      { "10x14", "720x1008" },
      { (char *) NULL, (char *) NULL }
    };

  char
    c,
    *geometry;

  register char
    *p;

  register int
    i;

  /*
    Allocate page geometry memory.
  */
  geometry=(char *) malloc((strlen(page)+MaxTextLength)*sizeof(char));
  if (geometry == (char *) NULL)
    {
       Warning("Unable to translate page geometry","Memory allocation failed");
       return(page);
    }
  /*
    Comparison is case insensitive.
  */
  (void) strcpy(geometry,page);
  if (!isdigit(*geometry))
    for (p=geometry; *p != '\0'; p++)
    {
      c=(*p);
      if (isascii(c) && islower(c))
        *p=toupper(c);
    }
  /*
    Comparison is case insensitive.
  */
  for (i=0; *PageSizes[i] != (char *) NULL; i++)
    if (strncmp(PageSizes[i][0],geometry,strlen(PageSizes[i][0])) == 0)
      {
        /*
          Replace mneumonic with the equivalent size in dots-per-inch.
        */
        (void) strcpy(geometry,PageSizes[i][1]);
        (void) strcat(geometry,page+strlen(PageSizes[i][0]));
        break;
      }
  return(geometry);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  T e m p o r a r y F i l e n a m e                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function TemporaryFilename replaces the contents of the string pointed to
%  by filename by a unique file name.
%
%  The format of the TemporaryFilename routine is:
%
%       TemporaryFilename(filename)
%
%  A description of each parameter follows.
%
%   o  filename:  Specifies a pointer to an array of characters.  The unique
%      file name is returned in this array.
%
%
*/
void TemporaryFilename(char *filename)
{
  char
    *directory;

  directory=(char *) getenv("TMPDIR");
  if (directory == (char *) NULL)
    directory=TemporaryDirectory;
  (void) sprintf(filename,TemporaryTemplate,directory);
  (void) mktemp(filename);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

