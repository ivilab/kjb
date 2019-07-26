
/* $Id: colors.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%                  CCCC   OOO   L       OOO   RRRR    SSSSS                   %
%                 C      O   O  L      O   O  R   R   SS                      %
%                 C      O   O  L      O   O  RRRR     SSS                    %
%                 C      O   O  L      O   O  R R        SS                   %
%                  CCCC   OOO   LLLLL   OOO   R  R    SSSSS                   %
%                                                                             %
%                                                                             %
%                       Count the Colors in an Image                          %
%                                                                             %
%                                                                             %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                              July 1992                                      %
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
  Define declarations.
*/
#define MaxTreeDepth  8
#define NodesInAList  2048

/*
  Structures.
*/
typedef struct _ColorsList
{
  Quantum
    red,
    green,
    blue;

  unsigned long
    count;
} ColorsList;

typedef struct _Node
{
  unsigned char
    level;

  unsigned long
    number_unique;

  ColorsList
    *list;

  struct _Node
    *child[8];
} Node;

typedef struct _Nodes
{
  Node
    nodes[NodesInAList];

  struct _Nodes
    *next;
} Nodes;

typedef struct _Cube
{
  Node
    *root;

  unsigned int
    colors;

  unsigned int
    free_nodes;

  Node
    *node;

  Nodes
    *node_list;
} Cube;

/*
  Function prototypes.
*/
static Node
  *InitializeNode _Declare((unsigned int));

static void
  DestroyList _Declare((register Node *)),
  Histogram _Declare((register Node *,FILE *));

/*
  Global variables.
*/
static Cube
  cube;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D e s t r o y L i s t                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure DestroyList traverses the color cube tree and free the list of
%  unique colors.
%
%  The format of the DestroyList routine is:
%
%      DestroyList(node)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void DestroyList(register Node *node)
{
  register unsigned int
    id;

  /*
    Traverse any children.
  */
  for (id=0; id < 8; id++)
    if (node->child[id] != (Node *) NULL)
      DestroyList(node->child[id]);
  if (node->level == MaxTreeDepth)
    if (node->list != (ColorsList *) NULL)
      free((char *) node->list);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  H i s t o g r a m                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure Histogram traverses the color cube tree and produces a list of
%  unique pixel field values and the number of times each occurs in the image.
%
%  The format of the Histogram routine is:
%
%      Histogram(node,file)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void Histogram(register Node *node, FILE *file)
{
  register int
    i;

  register unsigned int
    id;

  /*
    Traverse any children.
  */
  for (id=0; id < 8; id++)
    if (node->child[id] != (Node *) NULL)
      Histogram(node->child[id],file);
  if (node->level == MaxTreeDepth)
    for (i=0; i < node->number_unique; i++)
      (void) fprintf(file,"%10lu %6d %6d %6d\n",node->list[i].count,
        node->list[i].red,node->list[i].green,node->list[i].blue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n i t i a l i z e N o d e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function InitializeNode allocates memory for a new node in the color cube
%  tree and presets all fields to zero.
%
%  The format of the InitializeNode routine is:
%
%      node=InitializeNode(level)
%
%  A description of each parameter follows.
%
%    o level: Specifies the level in the classification the node resides.
%
%
*/
static Node *InitializeNode(unsigned int level)
{
  register int
    i;

  register Node
    *node;

  if (cube.free_nodes == 0)
    {
      register Nodes
        *nodes;

      /*
        Allocate a new nodes of nodes.
      */
      nodes=(Nodes *) malloc(sizeof(Nodes));
      if (nodes == (Nodes *) NULL)
        return((Node *) NULL);
      nodes->next=cube.node_list;
      cube.node_list=nodes;
      cube.node=nodes->nodes;
      cube.free_nodes=NodesInAList;
    }
  cube.free_nodes--;
  node=cube.node++;
  for (i=0; i < 8; i++)
    node->child[i]=(Node *) NULL;
  node->level=level;
  node->number_unique=0;
  node->list=(ColorsList *) NULL;
  return(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I s P s e u d o C l a s s                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function IsPseudoClass returns True if the image is PseudoClass or has 256
%  unique colors or less.  If the image is DirectClass and has less 256 colors
%  or less, the image is demoted to PseudoClass.
%
%  The format of the IsPseudoClass routine is:
%
%      status=IsPseudoClass(image)
%
%  A description of each parameter follows.
%
%    o status:  Function IsPseudoClass returns True is the image is
%      PseudoClass or has 256 color or less.
%
%    o image: The address of a Byte (8 bits) array of run-length
%      encoded pixel data of your source image.  The sum of the
%      run-length counts in the source image must be equal to or exceed
%      the number of pixels.
%
%
*/
unsigned int IsPseudoClass(Image *image)
{
  Nodes
    *nodes;

  register RunlengthPacket
    *p;

  register int
    i,
    j;

  register Node
    *node;

  register unsigned int
    id,
    index,
    level;

  unsigned int
    count;

  if (image->class == PseudoClass)
    return(True);
  if (image->matte)
    return(False);
  /*
    Initialize color description tree.
  */
  cube.node_list=(Nodes *) NULL;
  cube.colors=0;
  cube.free_nodes=0;
  cube.root=InitializeNode(0);
  if (cube.root == (Node *) NULL)
    {
      Warning("Unable to count colors","Memory allocation failed");
      return(False);
    }
  p=image->pixels;
  for (i=0; ((i < image->packets) && (cube.colors <= 256)); i++)
  {
    /*
      Start at the root and proceed level by level.
    */
    count=p->length+1;
    node=cube.root;
    index=MaxTreeDepth-1;
    for (level=1; level <= MaxTreeDepth; level++)
    {
      id=(((Quantum) DownScale(p->red) >> index) & 0x01) << 2 |
         (((Quantum) DownScale(p->green) >> index) & 0x01) << 1 |
         (((Quantum) DownScale(p->blue) >> index) & 0x01);
      if (node->child[id] == (Node *) NULL)
        {
          node->child[id]=InitializeNode(level);
          if (node->child[id] == (Node *) NULL)
            {
              Warning("Unable to count colors","Memory allocation failed");
              return(False);
            }
        }
      node=node->child[id];
      index--;
      if (level != MaxTreeDepth)
        continue;
      for (j=0; j < node->number_unique; j++)
         if ((p->red == node->list[j].red) &&
             (p->green == node->list[j].green) &&
             (p->blue == node->list[j].blue))
           break;
      if (j < node->number_unique)
        {
          node->list[j].count+=count;
          continue;
        }
      if (node->number_unique == 0)
        node->list=(ColorsList *) malloc(sizeof(ColorsList));
      else
        node->list=(ColorsList *) realloc(node->list,(j+1)*sizeof(ColorsList));
      if (node->list == (ColorsList *) NULL)
        {
          Warning("Unable to count colors","Memory allocation failed");
          return(False);
        }
      node->list[j].red=p->red;
      node->list[j].green=p->green;
      node->list[j].blue=p->blue;
      node->list[j].count=1;
      node->number_unique++;
      cube.colors++;
    }
    p++;
  }
  /*
    Release color cube tree storage.
  */
  DestroyList(cube.root);
  do
  {
    nodes=cube.node_list->next;
    free((char *) cube.node_list);
    cube.node_list=nodes;
  }
  while (cube.node_list != (Nodes *) NULL);
  if (cube.colors <= 256)
    {
      /*
        Demote DirectClass to PseudoClass.
      */
      QuantizeImage(image,256,8,False,RGBColorspace);
      SyncImage(image);
    }
  return(image->class == PseudoClass);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  N u m b e r C o l o r s                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function NumberColors returns the number of unique colors in an image.
%
%  The format of the NumberColors routine is:
%
%      NumberColors(image,file)
%
%  A description of each parameter follows.
%
%    o image: The address of a Byte (8 bits) array of run-length
%      encoded pixel data of your source image.  The sum of the
%      run-length counts in the source image must be equal to or exceed
%      the number of pixels.
%
%    o file:  An pointer to a FILE.  If it is non-null a list of unique pixel
%      field values and the number of times each occurs in the image is
%      written to the file.
%
%
%
*/
void NumberColors(Image *image, FILE *file)
{
  Nodes
    *nodes;

  register RunlengthPacket
    *p;

  register int
    i,
    j;

  register Node
    *node;

  register unsigned int
    id,
    index,
    level;

  unsigned int
    count;

  /*
    Initialize color description tree.
  */
  image->total_colors=0;
  cube.node_list=(Nodes *) NULL;
  cube.colors=0;
  cube.free_nodes=0;
  cube.root=InitializeNode(0);
  if (cube.root == (Node *) NULL)
    {
      Warning("Unable to count colors","Memory allocation failed");
      return;
    }
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    /*
      Start at the root and proceed level by level.
    */
    count=p->length+1;
    node=cube.root;
    index=MaxTreeDepth-1;
    for (level=1; level <= MaxTreeDepth; level++)
    {
      id=(((Quantum) DownScale(p->red) >> index) & 0x01) << 2 |
         (((Quantum) DownScale(p->green) >> index) & 0x01) << 1 |
         (((Quantum) DownScale(p->blue) >> index) & 0x01);
      if (node->child[id] == (Node *) NULL)
        {
          node->child[id]=InitializeNode(level);
          if (node->child[id] == (Node *) NULL)
            {
              Warning("Unable to count colors","Memory allocation failed");
              return;
            }
        }
      node=node->child[id];
      index--;
      if (level != MaxTreeDepth)
        continue;
      for (j=0; j < node->number_unique; j++)
         if ((p->red == node->list[j].red) &&
             (p->green == node->list[j].green) &&
             (p->blue == node->list[j].blue))
           break;
      if (j < node->number_unique)
        {
          node->list[j].count+=count;
          continue;
        }
      if (node->number_unique == 0)
        node->list=(ColorsList *) malloc(sizeof(ColorsList));
      else
        node->list=(ColorsList *) realloc(node->list,(j+1)*sizeof(ColorsList));
      if (node->list == (ColorsList *) NULL)
        {
          Warning("Unable to count colors","Memory allocation failed");
          return;
        }
      node->list[j].red=p->red;
      node->list[j].green=p->green;
      node->list[j].blue=p->blue;
      node->list[j].count=count;
      node->number_unique++;
      cube.colors++;
    }
    p++;
  }
  if (file != (FILE *) NULL)
    {
      (void) fprintf(file,"\n  Histogram of colors:\n");
      (void) fprintf(file,"\n     Count    Red  Green   Blue\n");
      Histogram(cube.root,file);
    }
  /*
    Release color cube tree storage.
  */
  DestroyList(cube.root);
  do
  {
    nodes=cube.node_list->next;
    free((char *) cube.node_list);
    cube.node_list=nodes;
  }
  while (cube.node_list != (Nodes *) NULL);
  image->total_colors=cube.colors;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

