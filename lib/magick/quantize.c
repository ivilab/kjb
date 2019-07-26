
/* $Id: quantize.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%           QQQ   U   U   AAA   N   N  TTTTT  IIIII   ZZZZZ  EEEEE            %
%          Q   Q  U   U  A   A  NN  N    T      I        ZZ  E                %
%          Q   Q  U   U  AAAAA  N N N    T      I      ZZZ   EEEEE            %
%          Q  QQ  U   U  A   A  N  NN    T      I     ZZ     E                %
%           QQQQ   UUU   A   A  N   N    T    IIIII   ZZZZZ  EEEEE            %
%                                                                             %
%                                                                             %
%              Reduce the Number of Unique Colors in an Image                 %
%                                                                             %
%                                                                             %
%                                                                             %
%                           Software Design                                   %
%                             John Cristy                                     %
%                              July 1992                                      %
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
%  Realism in computer graphics typically requires using 24 bits/pixel to
%  generate an image.  Yet many graphic display devices do not contain
%  the amount of memory necessary to match the spatial and color
%  resolution of the human eye.  The QUANTIZE program takes a 24 bit
%  image and reduces the number of colors so it can be displayed on
%  raster device with less bits per pixel.  In most instances, the
%  quantized image closely resembles the original reference image.
%
%  A reduction of colors in an image is also desirable for image
%  transmission and real-time animation.
%
%  Function Quantize takes a standard RGB or monochrome images and quantizes
%  them down to some fixed number of colors.
%
%  For purposes of color allocation, an image is a set of n pixels, where
%  each pixel is a point in RGB space.  RGB space is a 3-dimensional
%  vector space, and each pixel, pi,  is defined by an ordered triple of
%  red, green, and blue coordinates, (ri, gi, bi).
%
%  Each primary color component (red, green, or blue) represents an
%  intensity which varies linearly from 0 to a maximum value, cmax, which
%  corresponds to full saturation of that color.  Color allocation is
%  defined over a domain consisting of the cube in RGB space with
%  opposite vertices at (0,0,0) and (cmax,cmax,cmax).  QUANTIZE requires
%  cmax = 255.
%
%  The algorithm maps this domain onto a tree in which each node
%  represents a cube within that domain.  In the following discussion
%  these cubes are defined by the coordinate of two opposite vertices:
%  The vertex nearest the origin in RGB space and the vertex farthest
%  from the origin.
%
%  The tree's root node represents the the entire domain, (0,0,0) through
%  (cmax,cmax,cmax).  Each lower level in the tree is generated by
%  subdividing one node's cube into eight smaller cubes of equal size.
%  This corresponds to bisecting the parent cube with planes passing
%  through the midpoints of each edge.
%
%  The basic algorithm operates in three phases: Classification,
%  Reduction, and Assignment.  Classification builds a color
%  description tree for the image.  Reduction collapses the tree until
%  the number it represents, at most, the number of colors desired in the
%  output image.  Assignment defines the output image's color map and
%  sets each pixel's color by reclassification in the reduced tree.
%  Our goal is to minimize the numerical discrepancies between the original
%  colors and quantized colors (quantization error).
%
%  Classification begins by initializing a color description tree of
%  sufficient depth to represent each possible input color in a leaf.
%  However, it is impractical to generate a fully-formed color
%  description tree in the classification phase for realistic values of
%  cmax.  If colors components in the input image are quantized to k-bit
%  precision, so that cmax= 2k-1, the tree would need k levels below the
%  root node to allow representing each possible input color in a leaf.
%  This becomes prohibitive because the tree's total number of nodes is
%  1 + sum(i=1,k,8k).
%
%  A complete tree would require 19,173,961 nodes for k = 8, cmax = 255.
%  Therefore, to avoid building a fully populated tree, QUANTIZE: (1)
%  Initializes data structures for nodes only as they are needed;  (2)
%  Chooses a maximum depth for the tree as a function of the desired
%  number of colors in the output image (currently log2(colormap size)).
%
%  For each pixel in the input image, classification scans downward from
%  the root of the color description tree.  At each level of the tree it
%  identifies the single node which represents a cube in RGB space
%  containing the pixel's color.  It updates the following data for each
%  such node:
%
%    n1: Number of pixels whose color is contained in the RGB cube
%    which this node represents;
%
%    n2: Number of pixels whose color is not represented in a node at
%    lower depth in the tree;  initially,  n2 = 0 for all nodes except
%    leaves of the tree.
%
%    Sr, Sg, Sb: Sums of the red, green, and blue component values for
%    all pixels not classified at a lower depth. The combination of
%    these sums and n2  will ultimately characterize the mean color of a
%    set of pixels represented by this node.
%
%    E: The distance squared in RGB space between each pixel contained
%    within a node and the nodes' center.  This represents the quantization
%    error for a node.
%
%  Reduction repeatedly prunes the tree until the number of nodes with
%  n2 > 0 is less than or equal to the maximum number of colors allowed
%  in the output image.  On any given iteration over the tree, it selects
%  those nodes whose E count is minimal for pruning and merges their
%  color statistics upward. It uses a pruning threshold, Ep, to govern
%  node selection as follows:
%
%    Ep = 0
%    while number of nodes with (n2 > 0) > required maximum number of colors
%      prune all nodes such that E <= Ep
%      Set Ep to minimum E in remaining nodes
%
%  This has the effect of minimizing any quantization error when merging
%  two nodes together.
%
%  When a node to be pruned has offspring, the pruning procedure invokes
%  itself recursively in order to prune the tree from the leaves upward.
%  n2,  Sr, Sg,  and  Sb in a node being pruned are always added to the
%  corresponding data in that node's parent.  This retains the pruned
%  node's color characteristics for later averaging.
%
%  For each node, n2 pixels exist for which that node represents the
%  smallest volume in RGB space containing those pixel's colors.  When n2
%  > 0 the node will uniquely define a color in the output image. At the
%  beginning of reduction,  n2 = 0  for all nodes except a the leaves of
%  the tree which represent colors present in the input image.
%
%  The other pixel count, n1, indicates the total number of colors
%  within the cubic volume which the node represents.  This includes n1 -
%  n2  pixels whose colors should be defined by nodes at a lower level in
%  the tree.
%
%  Assignment generates the output image from the pruned tree.  The
%  output image consists of two parts: (1)  A color map, which is an
%  array of color descriptions (RGB triples) for each color present in
%  the output image;  (2)  A pixel array, which represents each pixel as
%  an index into the color map array.
%
%  First, the assignment phase makes one pass over the pruned color
%  description tree to establish the image's color map.  For each node
%  with n2  > 0, it divides Sr, Sg, and Sb by n2 .  This produces the
%  mean color of all pixels that classify no lower than this node.  Each
%  of these colors becomes an entry in the color map.
%
%  Finally,  the assignment phase reclassifies each pixel in the pruned
%  tree to identify the deepest node containing the pixel's color.  The
%  pixel's value in the pixel array becomes the index of this node's mean
%  color in the color map.
%
%  For efficiency, QUANTIZE requires that the reference image be in a
%  run-length encoded format.
%
%  With the permission of USC Information Sciences Institute, 4676 Admiralty
%  Way, Marina del Rey, California  90292, this code was adapted from module
%  ALCOLS written by Paul Raveling.
%
%  The names of ISI and USC are not used in advertising or publicity
%  pertaining to distribution of the software without prior specific
%  written permission from ISI.
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
#define MaxNodes  266817
#define MaxSpan ((1 << MaxTreeDepth)-1)
#define MaxTreeDepth  8
#define NodesInAList  2048

/*
  Structures.
*/
typedef struct _Node
{
  unsigned char
    id,
    level,
    census,
    mid_red,
    mid_green,
    mid_blue;

  unsigned int
    color_number,
    number_unique;

  double
    quantization_error,
    total_red,
    total_green,
    total_blue;

  struct _Node
    *parent,
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
    depth;

  unsigned long
    colors;

  ColorPacket
    color,
    *colormap;

  double
    distance,
    pruning_threshold,
    next_pruning_threshold;

  unsigned int
    *squares,
    nodes,
    free_nodes,
    color_number;

  Node
    *next_node;

  Nodes
    *node_queue;
} Cube;

/*
  Global variables.
*/
static Cube
  cube;

/*
  Function prototypes.
*/
static Node
  *InitializeNode _Declare((unsigned int,unsigned int,Node *,unsigned int,
    unsigned int, unsigned int));

static unsigned int
  DitherImage _Declare((Image *));

static void
  Assignment _Declare((Image *,unsigned int,unsigned int,unsigned int)),
  Classification _Declare((Image *)),
  ClosestColor _Declare((Node *)),
  DefineColormap _Declare((Node *)),
  InitializeCube _Declare((int)),
  PruneChild _Declare((register Node *)),
  PruneLevel _Declare((Node *)),
  Reduce _Declare((register Node *)),
  Reduction _Declare((unsigned int));

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  A s s i g n m e n t                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure Assignment generates the output image from the pruned tree.  The
%  output image consists of two parts: (1)  A color map, which is an
%  array of color descriptions (RGB triples) for each color present in
%  the output image;  (2)  A pixel array, which represents each pixel as
%  an index into the color map array.
%
%  First, the assignment phase makes one pass over the pruned color
%  description tree to establish the image's color map.  For each node
%  with n2  > 0, it divides Sr, Sg, and Sb by n2 .  This produces the
%  mean color of all pixels that classify no lower than this node.  Each
%  of these colors becomes an entry in the color map.
%
%  Finally,  the assignment phase reclassifies each pixel in the pruned
%  tree to identify the deepest node containing the pixel's color.  The
%  pixel's value in the pixel array becomes the index of this node's mean
%  color in the color map.
%
%  The format of the Assignment routine is:
%
%      Assignment(image,number_colors,dither,colorspace)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  Here we use this value
%      to determine the depth of the color description tree.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%    o colorspace: An unsigned integer value that indicates the colorspace.
%
%
*/
static void Assignment(Image *image, unsigned int number_colors, unsigned int dither, unsigned int colorspace)
{
#define AssignImageText  "  Assigning image colors...  "

  register int
    i;

  register Node
    *node;

  register RunlengthPacket
    *p;

  unsigned int
    id,
    index;

  /*
    Allocate image colormap.
  */
  if (image->colormap != (ColorPacket *) NULL)
    free((char *) image->colormap);
  image->colormap=(ColorPacket *)
    malloc(cube.colors*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    {
      Warning("Unable to quantize image","Memory allocation failed");
      exit(1);
    }
  if (image->signature != (char *) NULL)
    free((char *) image->signature);
  image->signature=(char *) NULL;
  cube.colormap=image->colormap;
  cube.colors=0;
  DefineColormap(cube.root);
  if ((number_colors == 2)  && (colorspace == GRAYColorspace))
    {
      unsigned int
        polarity;

      /*
        Monochrome image.
      */
      polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
      image->colormap[polarity].red=0;
      image->colormap[polarity].green=0;
      image->colormap[polarity].blue=0;
      image->colormap[!polarity].red=MaxRGB;
      image->colormap[!polarity].green=MaxRGB;
      image->colormap[!polarity].blue=MaxRGB;
    }
  image->matte=False;
  image->class=PseudoClass;
  image->colors=cube.colors;
  /*
    Create a reduced color image.
  */
  if (dither)
    dither=!DitherImage(image);
  p=image->pixels;
  if (!dither)
    for (i=0; i < image->packets; i++)
    {
      /*
        Identify the deepest node containing the pixel's color.
      */
      node=cube.root;
      for (index=MaxTreeDepth-1; (int) index > 0; index--)
      {
        id=(((Quantum) DownScale(p->red) >> index) & 0x01) << 2 |
           (((Quantum) DownScale(p->green) >> index) & 0x01) << 1 |
           (((Quantum) DownScale(p->blue) >> index) & 0x01);
        if ((node->census & (1 << id)) == 0)
          break;
        node=node->child[id];
      }
      /*
        Find closest color among siblings and their children.
      */
      cube.color.red=p->red;
      cube.color.green=p->green;
      cube.color.blue=p->blue;
      cube.distance=3.0*(MaxRGB+1)*(MaxRGB+1);
      ClosestColor(node->parent);
      p->index=cube.color_number;
      p++;
      if (QuantumTick(i,image))
        ProgressMonitor(AssignImageText,i,image->packets);
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C l a s s i f i c a t i o n                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure Classification begins by initializing a color description tree
%  of sufficient depth to represent each possible input color in a leaf.
%  However, it is impractical to generate a fully-formed color
%  description tree in the classification phase for realistic values of
%  cmax.  If colors components in the input image are quantized to k-bit
%  precision, so that cmax= 2k-1, the tree would need k levels below the
%  root node to allow representing each possible input color in a leaf.
%  This becomes prohibitive because the tree's total number of nodes is
%  1 + sum(i=1,k,8k).
%
%  A complete tree would require 19,173,961 nodes for k = 8, cmax = 255.
%  Therefore, to avoid building a fully populated tree, QUANTIZE: (1)
%  Initializes data structures for nodes only as they are needed;  (2)
%  Chooses a maximum depth for the tree as a function of the desired
%  number of colors in the output image (currently log2(colormap size)).
%
%  For each pixel in the input image, classification scans downward from
%  the root of the color description tree.  At each level of the tree it
%  identifies the single node which represents a cube in RGB space
%  containing It updates the following data for each such node:
%
%    n1 : Number of pixels whose color is contained in the RGB cube
%    which this node represents;
%
%    n2 : Number of pixels whose color is not represented in a node at
%    lower depth in the tree;  initially,  n2 = 0 for all nodes except
%    leaves of the tree.
%
%    Sr, Sg, Sb : Sums of the red, green, and blue component values for
%    all pixels not classified at a lower depth. The combination of
%    these sums and n2  will ultimately characterize the mean color of a
%    set of pixels represented by this node.
%
%    E: The distance squared in RGB space between each pixel contained
%    within a node and the nodes' center.  This represents the quantization
%    error for a node.
%
%  The format of the Classification routine is:
%
%      Classification(image)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%
*/
static void Classification(Image *image)
{
#define ClassifyImageText  "  Classifying image colors...  "

  double
    distance_squared;

  int
    distance;

  register int
    i;

  register Node
    *node;

  register RunlengthPacket
    *p;

  register unsigned int
    index;

  unsigned int
    bisect,
    count,
    id,
    level;

  cube.root->quantization_error+=
    3.0*(MaxRGB+1)*(MaxRGB+1)*image->columns*image->rows;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    if (cube.nodes > MaxNodes)
      {
        /*
          Prune one level if the color tree is too large.
        */
        PruneLevel(cube.root);
        cube.depth--;
      }
    /*
      Start at the root and descend the color cube tree.
    */
    count=p->length+1;
    node=cube.root;
    index=MaxTreeDepth-1;
    for (level=1; level <= cube.depth; level++)
    {
      id=(((Quantum) DownScale(p->red) >> index) & 0x01) << 2 |
         (((Quantum) DownScale(p->green) >> index) & 0x01) << 1 |
         (((Quantum) DownScale(p->blue) >> index) & 0x01);
      if (node->child[id] == (Node *) NULL)
        {
          /*
            Set colors of new node to contain pixel.
          */
          node->census|=1 << id;
          bisect=(unsigned int) (1 << (MaxTreeDepth-level)) >> 1;
          node->child[id]=InitializeNode(id,level,node,
            node->mid_red+(id & 4 ? bisect : -bisect),
            node->mid_green+(id & 2 ? bisect : -bisect),
            node->mid_blue+(id & 1 ? bisect : -bisect));
          if (node->child[id] == (Node *) NULL)
            {
              Warning("Unable to quantize image","Memory allocation failed");
              exit(1);
            }
          if (level == cube.depth)
            cube.colors++;
        }
      node=node->child[id];
      /*
        Approximate the quantization error represented by this node.
      */
      distance=(int) DownScale(p->red)-(int) node->mid_red;
      distance_squared=cube.squares[distance];
      distance=(int) DownScale(p->green)-(int) node->mid_green;
      distance_squared+=cube.squares[distance];
      distance=(int) DownScale(p->blue)-(int) node->mid_blue;
      distance_squared+=cube.squares[distance];
      node->quantization_error+=distance_squared*count;
      index--;
    }
    /*
      Increment unique color count and sum RGB values for this leaf for later
      derivation of the mean cube color.
    */
    node->number_unique+=count;
    node->total_red+=p->red*count;
    node->total_green+=p->green*count;
    node->total_blue+=p->blue*count;
    p++;
    if (QuantumTick(i,image))
      ProgressMonitor(ClassifyImageText,i,image->packets);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C l o s e s t C o l o r                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure ClosestColor traverses the color cube tree at a particular node
%  and determines which colormap entry best represents the input color.
%
%  The format of the ClosestColor routine is:
%
%      ClosestColor(node)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void ClosestColor(register Node *node)
{
  register unsigned int
    id;

  /*
    Traverse any children.
  */
  if (node->census != 0)
    for (id=0; id < 8; id++)
      if (node->census & (1 << id))
        ClosestColor(node->child[id]);
  if (node->number_unique != 0)
    {
      register float
        distance_squared;

      register int
        distance;

      register ColorPacket
        *color;

      /*
        Determine if this color is "closest".
      */
      color=cube.colormap+node->color_number;
      distance=(int) color->red-(int) cube.color.red;
      distance_squared=cube.squares[distance];
      distance=(int) color->green-(int) cube.color.green;
      distance_squared+=cube.squares[distance];
      distance=(int) color->blue-(int) cube.color.blue;
      distance_squared+=cube.squares[distance];
      if (distance_squared < cube.distance)
        {
          cube.distance=distance_squared;
          cube.color_number=(unsigned short) node->color_number;
        }
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D e f i n e C o l o r m a p                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure DefineColormap traverses the color cube tree and notes each
%  colormap entry.  A colormap entry is any node in the color cube tree where
%  the number of unique colors is not zero.
%
%  The format of the DefineColormap routine is:
%
%      DefineColormap(node)
%
%  A description of each parameter follows.
%
%    o node: The address of a structure of type Node which points to a
%      node in the color cube tree that is to be pruned.
%
%
*/
static void DefineColormap(register Node *node)
{
  register unsigned int
    id;

  register unsigned long
    number_unique;

  /*
    Traverse any children.
  */
  if (node->census != 0)
    for (id=0; id < 8; id++)
      if (node->census & (1 << id))
        DefineColormap(node->child[id]);
  if (node->number_unique != 0)
    {
      /*
        Colormap entry is defined by the mean color in this cube.
      */
      number_unique=node->number_unique;
      cube.colormap[cube.colors].red=(Quantum)
        ((node->total_red+(number_unique >> 1))/number_unique);
      cube.colormap[cube.colors].green=(Quantum)
        ((node->total_green+(number_unique >> 1))/number_unique);
      cube.colormap[cube.colors].blue=(Quantum)
        ((node->total_blue+(number_unique >> 1))/number_unique);
      node->color_number=cube.colors++;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  D i t h e r I m a g e                                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure DitherImage uses the Floyd-Steinberg algorithm to dither the
%  image.  Refer to "An Adaptive Algorithm for Spatial GreySscale", Robert W.
%  Floyd and Louis Steinberg, Proceedings of the S.I.D., Volume 17(2), 1976.
%
%  First find the closest representation to the reference pixel color in the
%  colormap, the node pixel is assigned this color.  Next, the colormap color
%  is subtracted from the reference pixels color, this represents the
%  quantization error.  Various amounts of this error are added to the pixels
%  ahead and below the node pixel to correct for this error.  The error
%  proportions are:
%
%            P     7/16
%      3/16  5/16  1/16
%
%  The error is distributed left-to-right for even scanlines and right-to-left
%  for odd scanlines.
%
%  The format of the DitherImage routine is:
%
%      DitherImage(image)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%
*/
static unsigned int DitherImage(Image *image)
{
#define CacheShift  (QuantumDepth-6)
#define DitherImageText  "  Dithering image...  "


  typedef struct _ErrorPacket
  {
    int
      red,
      green,
      blue;
  } ErrorPacket;

  ErrorPacket
    *error;

  int
    blue_error,
    green_error,
    red_error,
    step;

  Node
    *node;

  Quantum
    blue,
    green,
    red;

  register ErrorPacket
    *cs,
    *ns;

  register int
    *cache,
    *error_limit,
    i;

  register Quantum
    *range_limit;

  register RunlengthPacket
    *q;

  unsigned int
    id,
    index,
    quantum,
    x,
    y;

  /*
    Image must be uncompressed.
  */
  if (!UncompressImage(image))
    return(True);
  /*
    Allocate memory.
  */
  cache=(int *) malloc((1 << 18)*sizeof(int));
  error=(ErrorPacket *) malloc(((image->columns+2) << 1)*sizeof(ErrorPacket));
  error_limit=(int *) malloc((MaxRGB*2+1)*sizeof(int));
  range_limit=(Quantum *) malloc(3*(MaxRGB+1)*sizeof(Quantum));
  if ((cache == (int *) NULL) || (error == (ErrorPacket *) NULL) ||
      (error_limit == (int *) NULL) || (range_limit == (Quantum *) NULL))
    {
      Warning("Unable to dither image","Memory allocation failed");
      return(True);
    }
  /*
    Initialize color cache.
  */
  for (i=0; i < (1 << 18); i++)
    cache[i]=(-1);
  /*
    Initialize error tables.
  */
  for (i=0; i < ((image->columns+2) << 1); i++)
  {
    error[i].red=0;
    error[i].green=0;
    error[i].blue=0;
  }
  /*
    Initialize error limit (constrain error).
  */
  quantum=Max(image->colors >> 4,1);
  if ((quantum > 1) && (QuantumDepth != 8))
    quantum>>=1;
  error_limit+=MaxRGB;
  step=0;
  for (i=0; i < ((MaxRGB+1)/quantum); i++)
  {
    error_limit[i]=step;
    error_limit[-i]=(-step);
    step++;
  }
  if (quantum > 3)
    for ( ; i < (3*(MaxRGB+1)/quantum); i++)
    {
      error_limit[i]=step;
      error_limit[-i]=(-step);
      step+=(i & 0x01) ? 0 : 1;
    }
  for ( ; i <= MaxRGB; i++)
  {
    error_limit[i]=step;
    error_limit[-i]=(-step);
  }
  /*
    Initialize range tables.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    range_limit[i]=0;
    range_limit[i+(MaxRGB+1)]=(Quantum) i;
    range_limit[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit+=(MaxRGB+1);
  /*
    Dither image.
  */
  for (y=0; y < image->rows; y++)
  {
    q=image->pixels+image->columns*y;
    cs=error+1;
    ns=cs+image->columns+2;
    step=y & 0x01 ? -1 : 1;
    if (step < 0)
      {
        /*
          Distribute error right-to-left for odd scanlines.
        */
        q+=(image->columns-1);
        ns=error+image->columns;
        cs=ns+image->columns+2;
      }
    for (x=0; x < image->columns; x++)
    {
      red_error=error_limit[(cs->red+8)/16];
      green_error=error_limit[(cs->green+8)/16];
      blue_error=error_limit[(cs->blue+8)/16];
      red=range_limit[(int) q->red+red_error];
      green=range_limit[(int) q->green+green_error];
      blue=range_limit[(int) q->blue+blue_error];
      i=(blue >> CacheShift) << 12 | (green >> CacheShift) << 6 |
        (red >> CacheShift);
      if (cache[i] < 0)
        {
          /*
            Identify the deepest node containing the pixel's color.
          */
          node=cube.root;
          for (index=MaxTreeDepth-1; (int) index > 0; index--)
          {
            id=(((Quantum) DownScale(red) >> index) & 0x01) << 2 |
               (((Quantum) DownScale(green) >> index) & 0x01) << 1 |
               (((Quantum) DownScale(blue) >> index) & 0x01);
            if ((node->census & (1 << id)) == 0)
              break;
            node=node->child[id];
          }
          /*
            Find closest color among siblings and their children.
          */
          cube.color.red=red;
          cube.color.green=green;
          cube.color.blue=blue;
          cube.distance=3.0*(MaxRGB+1)*(MaxRGB+1);
          ClosestColor(node->parent);
          cache[i]=cube.color_number;
        }
      index=(unsigned int) cache[i];
      red_error=(int) red-(int) cube.colormap[index].red;
      green_error=(int) green-(int) cube.colormap[index].green;
      blue_error=(int) blue-(int) cube.colormap[index].blue;
      q->index=(unsigned short) index;
      q+=step;
      /*
        Propagate the error in these proportions:
                Q     7/16
          3/16  5/16  1/16
      */
      cs->red=0;
      cs->green=0;
      cs->blue=0;
      cs+=step;
      cs->red+=7*red_error;
      cs->green+=7*green_error;
      cs->blue+=7*blue_error;
      ns-=step;
      ns->red+=3*red_error;
      ns->green+=3*green_error;
      ns->blue+=3*blue_error;
      ns+=step;
      ns->red+=5*red_error;
      ns->green+=5*green_error;
      ns->blue+=5*blue_error;
      ns+=step;
      ns->red+=red_error;
      ns->green+=green_error;
      ns->blue+=blue_error;
    }
    ProgressMonitor(DitherImageText,y,image->rows);
  }
  /*
    Free up memory.
  */
  range_limit-=(MaxRGB+1);
  free((char *) range_limit);
  error_limit-=MaxRGB;
  free((char *) error_limit);
  free((char *) error);
  free((char *) cache);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n i t i a l i z e C u b e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function InitializeCube initialize the Cube data structure.
%
%  The format of the InitializeCube routine is:
%
%      InitializeCube(depth)
%
%  A description of each parameter follows.
%
%    o depth: Normally, this integer value is zero or one.  A zero or
%      one tells Quantize to choose a optimal tree depth of Log4(number_colors).
%      A tree of this depth generally allows the best representation of the
%      reference image with the least amount of memory and the fastest
%      computational speed.  In some cases, such as an image with low color
%      dispersion (a few number of colors), a value other than
%      Log4(number_colors) is required.  To expand the color tree completely,
%      use a value of 8.
%
%
*/
static void InitializeCube(int depth)
{
  register int
    i;

  /*
    Initialize tree to describe color cube.
  */
  cube.node_queue=(Nodes *) NULL;
  cube.nodes=0;
  cube.free_nodes=0;
  if (depth > MaxTreeDepth)
    depth=MaxTreeDepth;
  if (depth < 2)
    depth=2;
  cube.depth=depth;
  /*
    Initialize root node.
  */
  cube.root=InitializeNode(0,0,(Node *) NULL,(MaxSpan+1) >> 1,(MaxSpan+1) >> 1,
    (MaxSpan+1) >> 1);
  cube.squares=(unsigned int *) malloc((MaxRGB+MaxRGB+1)*sizeof(unsigned int));
  if ((cube.root == (Node *) NULL) || (cube.squares == (unsigned int *) NULL))
    {
      Warning("Unable to quantize image","Memory allocation failed");
      exit(1);
    }
  cube.root->parent=cube.root;
  cube.root->quantization_error=0.0;
  cube.colors=0;
  cube.squares+=MaxRGB;
  for (i=(-MaxRGB); i <= MaxRGB; i++)
    cube.squares[i]=i*i;
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
%      node=InitializeNode(node,id,level)
%
%  A description of each parameter follows.
%
%    o node: The InitializeNode routine returns this integer address.
%
%    o id: Specifies the child number of the node.
%
%    o level: Specifies the level in the classification the node resides.
%
%    o mid_red: Specifies the mid point of the red axis for this node.
%
%    o mid_green: Specifies the mid point of the green axis for this node.
%
%    o mid_blue: Specifies the mid point of the blue axis for this node.
%
*/
static Node *InitializeNode(unsigned int id, unsigned int level, Node *parent, unsigned int mid_red, unsigned int mid_green, unsigned int mid_blue)
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
      nodes->next=cube.node_queue;
      cube.node_queue=nodes;
      cube.next_node=nodes->nodes;
      cube.free_nodes=NodesInAList;
    }
  cube.nodes++;
  cube.free_nodes--;
  node=cube.next_node++;
  node->parent=parent;
  for (i=0; i < 8; i++)
    node->child[i]=(Node *) NULL;
  node->id=id;
  node->level=level;
  node->census=0;
  node->mid_red=mid_red;
  node->mid_green=mid_green;
  node->mid_blue=mid_blue;
  node->quantization_error=0.0;
  node->number_unique=0;
  node->total_red=0.0;
  node->total_green=0.0;
  node->total_blue=0.0;
  return(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P r u n e C h i l d                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function PruneChild deletes the given node and merges its statistics into
%  its parent.
%
%  The format of the PruneSubtree routine is:
%
%      PruneChild(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void PruneChild(register Node *node)
{
  register Node
    *parent;

  /*
    Merge color statistics into parent.
  */
  parent=node->parent;
  parent->census&=~(1 << node->id);
  parent->number_unique+=node->number_unique;
  parent->total_red+=node->total_red;
  parent->total_green+=node->total_green;
  parent->total_blue+=node->total_blue;
  cube.nodes--;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  P r u n e L e v e l                                                        %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure PruneLevel deletes all nodes at the bottom level of the color
%  tree merging their color statistics into their parent node.
%
%  The format of the PruneLevel routine is:
%
%      PruneLevel(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void PruneLevel(register Node *node)
{
  register int
    id;

  /*
    Traverse any children.
  */
  if (node->census != 0)
    for (id=0; id < 8; id++)
      if (node->census & (1 << id))
        PruneLevel(node->child[id]);
  if (node->level == cube.depth)
    PruneChild(node);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e d u c e                                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Reduce traverses the color cube tree and prunes any node whose
%  quantization error falls below a particular threshold.
%
%  The format of the Reduce routine is:
%
%      Reduce(node)
%
%  A description of each parameter follows.
%
%    o node: pointer to node in color cube tree that is to be pruned.
%
%
*/
static void Reduce(register Node *node)
{
  register unsigned int
    id;

  /*
    Traverse any children.
  */
  if (node->census != 0)
    for (id=0; id < 8; id++)
      if (node->census & (1 << id))
        Reduce(node->child[id]);
  if (node->quantization_error <= cube.pruning_threshold)
    PruneChild(node);
  else
    {
      /*
        Find minimum pruning threshold.
      */
      if (node->number_unique > 0)
        cube.colors++;
      if (node->quantization_error < cube.next_pruning_threshold)
        cube.next_pruning_threshold=node->quantization_error;
    }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  R e d u c t i o n                                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function Reduction repeatedly prunes the tree until the number of nodes
%  with n2 > 0 is less than or equal to the maximum number of colors allowed
%  in the output image.  On any given iteration over the tree, it selects
%  those nodes whose E value is minimal for pruning and merges their
%  color statistics upward. It uses a pruning threshold, Ep, to govern
%  node selection as follows:
%
%    Ep = 0
%    while number of nodes with (n2 > 0) > required maximum number of colors
%      prune all nodes such that E <= Ep
%      Set Ep to minimum E in remaining nodes
%
%  This has the effect of minimizing any quantization error when merging
%  two nodes together.
%
%  When a node to be pruned has offspring, the pruning procedure invokes
%  itself recursively in order to prune the tree from the leaves upward.
%  n2,  Sr, Sg,  and  Sb in a node being pruned are always added to the
%  corresponding data in that node's parent.  This retains the pruned
%  node's color characteristics for later averaging.
%
%  For each node, n2 pixels exist for which that node represents the
%  smallest volume in RGB space containing those pixel's colors.  When n2
%  > 0 the node will uniquely define a color in the output image. At the
%  beginning of reduction,  n2 = 0  for all nodes except a the leaves of
%  the tree which represent colors present in the input image.
%
%  The other pixel count, n1, indicates the total number of colors
%  within the cubic volume which the node represents.  This includes n1 -
%  n2  pixels whose colors should be defined by nodes at a lower level in
%  the tree.
%
%  The format of the Reduction routine is:
%
%      Reduction(number_colors)
%
%  A description of each parameter follows.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  The actual number of
%      colors allocated to the colormap may be less than this value, but
%      never more.
%
%
*/
static void Reduction(unsigned int number_colors)
{
#define ReduceImageText  "  Reducing image colors...  "

  cube.next_pruning_threshold=1;
  while (cube.colors > number_colors)
  {
    cube.pruning_threshold=cube.next_pruning_threshold;
    cube.next_pruning_threshold=cube.root->quantization_error-1.0;
    cube.colors=0;
    Reduce(cube.root);
    ProgressMonitor(ReduceImageText,number_colors,cube.colors);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a p I m a g e                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MapImage replaces the colors of an image with the closest color from
%  a reference image.
%
%  The format of the MapImage routine is:
%
%      MapImage(image,map_image,dither)
%
%  A description of each parameter follows:
%
%    o image: Specifies a pointer to a set of Image structures.
%
%    o map_image: Specifies a pointer to a Image structure.  Reduce
%      image to a set of colors represented by this image.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%
*/
void MapImage(Image *image, Image *map_image, unsigned int dither)
{
  Nodes
    *nodes;

  /*
    Classify image colors from the reference image.
  */
  InitializeCube(8);
  Classification(map_image);
  Assignment(image,(unsigned int) cube.colors,dither,RGBColorspace);
  CompressColormap(image);
  /*
    Release color cube tree storage.
  */
  do
  {
    nodes=cube.node_queue->next;
    free((char *) cube.node_queue);
    cube.node_queue=nodes;
  }
  while (cube.node_queue != (Nodes *) NULL);
  cube.squares-=MaxRGB;
  free((char *) cube.squares);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u a n t i z a t i o n E r r o r                                          %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function QuantizationError measures the difference between the original
%  and quantized images.  This difference is the total quantization error.
%  The error is computed by summing over all pixels in an image the distance
%  squared in RGB space between each reference pixel value and its quantized
%  value.  These values are computed:
%
%    o mean_error_per_pixel:  This value is the mean error for any single
%      pixel in the image.
%
%    o normalized_mean_square_error:  This value is the normalized mean
%      quantization error for any single pixel in the image.  This distance
%      measure is normalized to a range between 0 and 1.  It is independent
%      of the range of red, green, and blue values in the image.
%
%    o normalized_maximum_square_error:  Thsi value is the normalized
%      maximum quantization error for any single pixel in the image.  This
%      distance measure is normalized to a range between 0 and 1.  It is
%      independent of the range of red, green, and blue values in your image.
%
%
%  The format of the QuantizationError routine is:
%
%      QuantizationError(image)
%
%  A description of each parameter follows.
%
%    o image: The address of a Byte (8 bits) array of run-length
%      encoded pixel data of your reference image.  The sum of the
%      run-length counts in the reference image must be equal to or exceed
%      the number of pixels.
%
%
*/
void QuantizationError(Image *image)
{
  double
    total_error;

  float
    distance_squared,
    maximum_error_per_pixel;

  int
    distance;

  register int
    i;

  register RunlengthPacket
    *p;

  /*
    Initialize measurement.
  */
  image->mean_error_per_pixel=0;
  image->normalized_mean_error=0.0;
  image->normalized_maximum_error=0.0;
  NumberColors(image,(FILE *) NULL);
  cube.squares=(unsigned int *) malloc((MaxRGB+MaxRGB+1)*sizeof(unsigned int));
  if (cube.squares == (unsigned int *) NULL)
    {
      Warning("Unable to measure error","Memory allocation failed");
      return;
    }
  cube.squares+=MaxRGB;
  for (i=(-MaxRGB); i <= MaxRGB; i++)
    cube.squares[i]=i*i;
  /*
    For each pixel, collect error statistics.
  */
  maximum_error_per_pixel=0;
  total_error=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    distance=(int) p->red-(int) image->colormap[p->index].red;
    distance_squared=cube.squares[distance];
    distance=(int) p->green-(int) image->colormap[p->index].green;
    distance_squared+=cube.squares[distance];
    distance=(int) p->blue-(int) image->colormap[p->index].blue;
    distance_squared+=cube.squares[distance];
    total_error+=(distance_squared*((double) p->length+1.0));
    if (distance_squared > maximum_error_per_pixel)
      maximum_error_per_pixel=distance_squared;
    p++;
  }
  /*
    Compute final error statistics.
  */
  image->mean_error_per_pixel=(unsigned int)
    total_error/(image->columns*image->rows);
  image->normalized_mean_error=
    (image->mean_error_per_pixel)/(3.0*(MaxRGB+1)*(MaxRGB+1));
  image->normalized_maximum_error=
    maximum_error_per_pixel/(3.0*(MaxRGB+1)*(MaxRGB+1));
  cube.squares-=MaxRGB;
  free((char *) cube.squares);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  Q u a n t i z e I m a g e                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function QuantizeImage analyzes the colors within a reference image and
%  chooses a fixed number of colors to represent the image.  The goal of the
%  algorithm is to minimize the difference between the input and output image
%  while minimizing the processing time.
%
%  The format of the QuantizeImage routine is:
%
%      colors=QuantizeImage(image,number_colors,tree_depth,dither,color_space)
%
%  A description of each parameter follows.
%
%    o colors: The QuantizeImage function returns this integer
%      value.  It is the actual number of colors allocated in the
%      colormap.  Note, the actual number of colors allocated may be less
%      than the number of colors requested, but never more.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  The actual number of
%      colors allocated to the colormap may be less than this value, but
%      never more.
%
%    o tree_depth: Normally, this integer value is zero or one.  A zero or
%      one tells Quantize to choose a optimal tree depth of
%      Log4(number_colors).  A tree of this depth generally allows the best
%      representation of the reference image with the least amount of memory
%      and the fastest computational speed.  In some cases, such as an image
%      with low color dispersion (a few number of colors), a value other than
%      Log4(number_colors) is required.  To expand the color tree completely,
%      use a value of 8.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%    o colorspace: An unsigned integer value that indicates the colorspace.
%      Empirical evidence suggests that distances in YUV or YIQ correspond to
%      perceptual color differences more closely than do distances in RGB
%      space.  The image is then returned to RGB colorspace after color
%      reduction.
%
%
*/
void QuantizeImage(Image *image, unsigned int number_colors, unsigned int tree_depth, unsigned int dither, unsigned int colorspace)
{
  int
    depth;

  Nodes
    *nodes;

  if (number_colors == 0)
    number_colors=1;
  if (number_colors > MaxColormapSize)
    number_colors=MaxColormapSize;
  if (image->packets == (image->columns*image->rows))
    CompressImage(image);
  depth=tree_depth;
  if (depth == 0)
    {
      unsigned int
        colors;

      /*
        Depth of color classification tree is: Log4(colormap size)+2.
      */
      colors=number_colors;
      for (depth=1; colors != 0; depth++)
        colors>>=2;
      if (dither)
        depth--;
      if (image->class == PseudoClass)
        depth+=2;
    }
  /*
    Reduce the number of colors in the continuous tone image.
  */
  InitializeCube(depth);
  if (colorspace != RGBColorspace)
    RGBTransformImage(image,colorspace);
  Classification(image);
  if ((cube.colors >> 1) < number_colors)
    dither=False;
  Reduction(number_colors);
  Assignment(image,number_colors,dither,colorspace);
  if (colorspace != RGBColorspace)
    TransformRGBImage(image,colorspace);
  /*
    Release color cube tree storage.
  */
  do
  {
    nodes=cube.node_queue->next;
    free((char *) cube.node_queue);
    cube.node_queue=nodes;
  }
  while (cube.node_queue != (Nodes *) NULL);
  cube.squares-=MaxRGB;
  free((char *) cube.squares);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Q u a n t i z e I m a g e s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  QuantizeImages analyzes the colors within a set of reference images and
%  chooses a fixed number of colors to represent the set.  The goal of the
%  algorithm is to minimize the difference between the input and output images
%  while minimizing the processing time.
%
%  The format of the QuantizeImages routine is:
%
%      QuantizeImages(images,number_images,number_colors,tree_depth,dither)
%
%  A description of each parameter follows:
%
%    o images: Specifies a pointer to a set of Image structures.
%
%    o number_images:  Specifies an unsigned integer representing the number
%      images in the image set.
%
%    o number_colors: This integer value indicates the maximum number of
%      colors in the quantized image or colormap.  The actual number of colors
%      allocated to the colormap may be less than this value, but never more.
%
%    o tree_depth: Normally, this integer value is zero or one.  A zero or
%      one tells QUANTIZE to choose a optimal tree depth.  An optimal depth
%      generally allows the best representation of the reference image with the
%      fastest computational speed and the least amount of memory.  However,
%      the default depth is inappropriate for some images.  To assure the best
%      representation, try values between 2 and 8 for this parameter.
%
%    o dither: Set this integer value to something other than zero to
%      dither the quantized image.  The basic strategy of dithering is to
%      trade intensity resolution for spatial resolution by averaging the
%      intensities of several neighboring pixels.  Images which suffer
%      from severe contouring when quantized can be improved with the
%      technique of dithering.  Severe contouring generally occurs when
%      quantizing to very few colors, or to a poorly-chosen colormap.
%      Note, dithering is a computationally expensive process and will
%      increase processing time significantly.
%
%    o colorspace: An unsigned integer value that indicates the colorspace.
%      Empirical evidence suggests that distances in YUV or YIQ correspond to
%      perceptual color differences more closely than do distances in RGB
%      space.  The image is then returned to RGB colorspace after color
%      reduction.
%
%
*/
void QuantizeImages(Image **images, unsigned int number_images, unsigned int number_colors, unsigned int tree_depth, unsigned int dither, unsigned int colorspace)
{
  int
    depth;

  Nodes
    *nodes;

  register unsigned int
    i;

  if (number_colors == 0)
    number_colors=1;
  if (number_colors > MaxColormapSize)
    number_colors=MaxColormapSize;
  depth=tree_depth;
  if (depth == 0)
    {
      int
        pseudo_class;

      unsigned int
        colors;

      /*
        Depth of color classification tree is: Log4(colormap size)+2.
      */
      colors=number_colors;
      for (depth=1; colors != 0; depth++)
        colors>>=2;
      if (dither)
        depth--;
      pseudo_class=True;
      for (i=0; i < number_images; i++)
        pseudo_class|=(images[i]->class == PseudoClass);
      if (pseudo_class)
        depth+=2;
    }
  /*
    Reduce the number of colors in the continuous tone image sequence.
  */
  InitializeCube(depth);
  for (i=0; i < number_images; i+=Max((number_images+16) >> 5,1))
  {
    if (images[i]->packets == (images[i]->columns*images[i]->rows))
      CompressImage(images[i]);
    if (colorspace != RGBColorspace)
      RGBTransformImage(images[i],colorspace);
    Classification(images[i]);
  }
  if ((cube.colors >> 1) < number_colors)
    dither=False;
  Reduction(number_colors);
  for (i=0; i < number_images; i++)
  {
    Assignment(images[i],number_colors,dither,colorspace);
    if (colorspace != RGBColorspace)
      TransformRGBImage(images[i],colorspace);
  }
  /*
    Release color cube tree storage.
  */
  do
  {
    nodes=cube.node_queue->next;
    free((char *) cube.node_queue);
    cube.node_queue=nodes;
  }
  while (cube.node_queue != (Nodes *) NULL);
  cube.squares-=MaxRGB;
  free((char *) cube.squares);
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

