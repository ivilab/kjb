
/* $Id: segment.c 4727 2009-11-16 20:53:54Z kobus $ */

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
%               SSSSS  EEEEE   GGGG  M   M  EEEEE  N   N  TTTTT               %
%               SS     E      G      MM MM  E      NN  N    T                 %
%                SSS   EEE    G GGG  M M M  EEE    N N N    T                 %
%                  SS  E      G   G  M   M  E      N  NN    T                 %
%               SSSSS  EEEEE   GGGG  M   M  EEEEE  N   N    T                 %
%                                                                             %
%                                                                             %
%     Segment an Image with Thresholding and the Fuzzy c-Means Technique.     %
%                                                                             %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                April 1993                                   %
%                                                                             %
%                                                                             %
%  Copyright 1996 E. I. Dupont de Nemours and Company                         %
%                                                                             %
%  Permission to use, copy, modify, distribute, and sell this software and    %
%  its documentation for any purpose is hereby granted without fee,           %
%  provided that the above Copyright notice appear in all copies and that     %
%  both that Copyright notice and this permission notice appear in            %
%  supporting documentation, and that the name of E. I. Dupont de Nemours     %
%  and Company not be used in advertising or publicity pertaining to          %
%  distribution of the software without specific, written prior               %
%  permission.  E. I. Dupont de Nemours and Company makes no representations  %
%  about the suitability of this software for any purpose.  It is provided    %
%  "as is" without express or implied warranty.                               %
%                                                                             %
%  E. I. Dupont de Nemours and Company disclaims all warranties with regard   %
%  to this software, including all implied warranties of merchantability      %
%  and fitness, in no event shall E. I. Dupont de Nemours and Company be      %
%  liable for any special, indirect or consequential damages or any           %
%  damages whatsoever resulting from loss of use, data or profits, whether    %
%  in an action of contract, negligence or other tortuous action, arising     %
%  out of or in connection with the use or performance of this software.      %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Segment segments an image by analyzing the histograms of the color
%  components and identifying units that are homogeneous with the fuzzy
%  c-means technique.  The scale-space filter analyzes the histograms of
%  the three color components of the image and identifies a set of classes.
%  The extents of each class is used to coarsely segment the image with
%  thresholding.  The color associated with each class is determined by
%  the mean color of all pixels within the extents of a particular class.
%  Finally, any unclassified pixels are assigned to the closest class with
%  the fuzzy c-means technique.
%
%  The fuzzy c-Means algorithm can be summarized as follows:
%
%    o Build a histogram, one for each color component of the image.
%
%    o For each histogram, successively apply the scale-space
%      filter and build an interval tree of zero crossings in
%      the second derivative at each scale.  Analyze this
%      scale-space ``fingerprint'' to determine which peaks and
%      valleys in the histogram are most predominant.
%
%    o The fingerprint defines intervals on the axis of the
%      histogram.  Each interval contains either a minima or a
%      maxima in the original signal.  If each color component
%      lies within the maxima interval, that pixel is considered
%      ``classified'' and is assigned an unique class number.
%
%    o Any pixel that fails to be classified in the above
%      thresholding pass is classified using the fuzzy
%      c-Means technique.  It is assigned to one of the classes
%      discovered in the histogram analysis phase.
%
%  The fuzzy c-Means technique attempts to cluster a pixel by finding
%  the local minima of the generalized within group sum of squared error
%  objective function.  A pixel is assigned to the closest class of which
%  the fuzzy membership has a maximum value.
%
%  Segment is strongly based on software written by Andy Gallo, University
%  of Delaware.
%
%  The following reference was used in creating this program:
%
%    Young Won Lim, Sang Uk Lee, "On The Color Image Segmentation Algorithm
%    Based on the Thresholding and the Fuzzy c-Means Techniques", Pattern
%    Recognition, Volume 23, Number 9, pages 935-952, 1990.
%
%
*/

#include "magick/magick.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */


/*
  Define declarations.
*/
#define Blue  2
#define Dimension  3
#define Green  1
#define Red  0
#define SafeMargin  3
#define SegmentImageText  "  Segmenting image...  "

/*
  Typedef declarations.
*/
typedef struct _ExtentPacket
{
  int
    index,
    left,
    right;

  long
    center;
} ExtentPacket;

typedef struct _IntervalTree
{
  float
    tau;

  int
    left,
    right;

  float
    mean_stability,
    stability;

  struct _IntervalTree
    *sibling,
    *child;
} IntervalTree;

typedef struct _ZeroCrossing
{
  float
    tau,
    histogram[MaxRGB+1];

  short
    crossings[MaxRGB+1];
} ZeroCrossing;

/*
  Function prototypes.
*/
static double
  OptimalTau  _Declare((long *,double,double,double,double,short *,
    unsigned int));

static int
  DefineRegion _Declare((short *,ExtentPacket *));

static IntervalTree
  *InitializeIntervalTree _Declare((ZeroCrossing *,unsigned int));

static void
  ActiveNodes _Declare((IntervalTree *)),
  Classify _Declare((Image *,short **,double,double,unsigned int)),
  ConsolidateCrossings _Declare((ZeroCrossing *,unsigned int)),
  DerivativeHistogram _Declare((float *,float *)),
  FreeNodes _Declare((IntervalTree *)),
  InitializeHistogram _Declare((Image *,long **)),
  InitializeList _Declare((IntervalTree *)),
  MeanStability _Declare((IntervalTree *)),
  ScaleSpace _Declare((long *,double,float *)),
  Stability _Declare((IntervalTree *)),
  ZeroCrossHistogram _Declare((float *,double,short *));

/*
  Global declarations.
*/
static int
  number_nodes;

static IntervalTree
  *list[600];

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l a s s i f y                                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function Classify defines on ore more classes.  Each pixel is thresholded
%  to determine which class it belongs to.  If not class is identified it
%  is assigned to the closest class based on the fuzzy c-Means technique.
%
%  The format of the Classify routine is:
%
%      Classify(image,extrema,cluster_threshold,weighting_exponent,verbose)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o extrema:  Specifies a pointer to an array of shortegers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
%    o cluster_threshold:  This float represents the minimum number of pixels
%      contained in a hexahedra before it can be considered valid (expressed
%      as a percentage).
%
%    o weighting_exponent: Specifies the membership weighting exponent.
%
%    o verbose:  A value greater than zero prints detailed information about
%      the identified classes.
%
%
*/
static void Classify(Image *image, short int **extrema, double cluster_threshold, double weighting_exponent, unsigned int verbose)
{
  typedef struct _Cluster
  {
    short
      id;

    ExtentPacket
      red,
      green,
      blue;

    long
      count;

    struct _Cluster
      *next;
  } Cluster;

  Cluster
    *cluster,
    *head,
    *last_cluster,
    *next_cluster;

  double
    distance_squared,
    local_minima,
    numerator,
    ratio,
    sum;

  ExtentPacket
    blue,
    green,
    red;

  int
    distance,
    count;

  register int
    i,
    j,
    k;

  register RunlengthPacket
    *p,
    *q;

  register unsigned int
    *squares;

  unsigned int
    number_clusters;

  /*
    Form clusters.
  */
  cluster=(Cluster *) NULL;
  head=(Cluster *) NULL;
  red.index=0;
  while (DefineRegion(extrema[Red],&red))
  {
    green.index=0;
    while (DefineRegion(extrema[Green],&green))
    {
      blue.index=0;
      while (DefineRegion(extrema[Blue],&blue))
      {
        /*
          Allocate a new class.
        */
        if (head != (Cluster *) NULL)
          {
            cluster->next=(Cluster *) malloc(sizeof(Cluster));
            cluster=cluster->next;
          }
        else
          {
            cluster=(Cluster *) malloc(sizeof(Cluster));
            head=cluster;
          }
        if (cluster == (Cluster *) NULL)
          Error("Unable to allocate memory",(char *) NULL);
        /*
          Initialize a new class.
        */
        cluster->count=0;
        cluster->red=red;
        cluster->green=green;
        cluster->blue=blue;
        cluster->next=(Cluster *) NULL;
      }
    }
  }
  if (head == (Cluster *) NULL)
    {
      /*
        No classes were identified-- create one.
      */
      cluster=(Cluster *) malloc(sizeof(Cluster));
      if (cluster == (Cluster *) NULL)
        Error("Unable to allocate memory",(char *) NULL);
      /*
        Initialize a new class.
      */
      cluster->count=0;
      cluster->red=red;
      cluster->green=green;
      cluster->blue=blue;
      cluster->next=(Cluster *) NULL;
      head=cluster;
    }
  /*
    Count the pixels for each cluster.
  */
  count=0;
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      if (((int) p->red >= (cluster->red.left-SafeMargin)) &&
          ((int) p->red <= (cluster->red.right+SafeMargin)) &&
          ((int) p->green >= (cluster->green.left-SafeMargin)) &&
          ((int) p->green <= (cluster->green.right+SafeMargin)) &&
          ((int) p->blue >= (cluster->blue.left-SafeMargin)) &&
          ((int) p->blue <= (cluster->blue.right+SafeMargin)))
        {
          /*
            Count this pixel.
          */
          count+=(p->length+1);
          cluster->count+=(p->length+1);
          cluster->red.center+=(p->red*(p->length+1));
          cluster->green.center+=(p->green*(p->length+1));
          cluster->blue.center+=(p->blue*(p->length+1));
          break;
        }
    p++;
  }
  /*
    Remove clusters that do not meet minimum cluster threshold.
  */
  cluster_threshold*=(float) count*0.01;
  count=0;
  last_cluster=head;
  next_cluster=head;
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    if (cluster->count >= cluster_threshold)
      {
        /*
          Initialize cluster.
        */
        cluster->id=count;
        cluster->red.center=(cluster->red.center+(cluster->count >> 1))/
          cluster->count;
        cluster->green.center=(cluster->green.center+(cluster->count >> 1))/
          cluster->count;
        cluster->blue.center=(cluster->blue.center+(cluster->count >> 1))/
          cluster->count;
        count++;
        last_cluster=cluster;
      }
    else
      {
        /*
          Delete cluster.
        */
        if (cluster == head)
          head=next_cluster;
        else
          last_cluster->next=next_cluster;
        free((char *) cluster);
      }
  }
  number_clusters=count;
  if (verbose)
    {
      /*
        Print cluster statistics.
      */
      (void) fprintf(stderr,"Fuzzy c-Means Statistics\n");
      (void) fprintf(stderr,"===================\n\n");
      (void) fprintf(stderr,"\tTotal Number of Clusters = %u\n\n",
        number_clusters);
      /*
        Print the total number of points per cluster.
      */
      (void) fprintf(stderr,"\n\nNumber of Vectors Per Cluster\n");
      (void) fprintf(stderr,"=============================\n\n");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
        (void) fprintf(stderr,"Cluster #%d = %ld\n",cluster->id,cluster->count);
      /*
        Print the cluster extents.
      */
      (void) fprintf(stderr,
        "\n\n\nCluster Extents:        (Vector Size: %d)\n",Dimension);
      (void) fprintf(stderr, "================");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      {
        (void) fprintf(stderr,"\n\nCluster #%d\n\n",cluster->id);
        (void) fprintf(stderr,"%d-%d  %d-%d  %d-%d\n",
          cluster->red.left,cluster->red.right,
          cluster->green.left,cluster->green.right,
          cluster->blue.left,cluster->blue.right);
      }
      /*
        Print the cluster center values.
      */
      (void) fprintf(stderr,
        "\n\n\nCluster Center Values:        (Vector Size: %d)\n",Dimension);
      (void) fprintf(stderr, "=====================");
      for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      {
        (void) fprintf(stderr,"\n\nCluster #%d\n\n",cluster->id);
        (void) fprintf(stderr,"%ld  %ld  %ld\n",cluster->red.center,
          cluster->green.center,cluster->blue.center);
      }
      (void) fprintf(stderr,"\n");
    }
  /*
    Allocate image colormap.
  */
  image->matte=False;
  image->class=PseudoClass;
  if (image->colormap != (ColorPacket *) NULL)
    free((char *) image->colormap);
  image->colormap=(ColorPacket *)
    malloc((unsigned int) number_clusters*sizeof(ColorPacket));
  if (image->colormap == (ColorPacket *) NULL)
    Error("Unable to allocate memory",(char *) NULL);
  if (image->signature != (char *) NULL)
    free((char *) image->signature);
  image->signature=(char *) NULL;
  image->colors=number_clusters;
  i=0;
  for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
  {
    image->colormap[i].red=(Quantum) cluster->red.center;
    image->colormap[i].green=(Quantum) cluster->green.center;
    image->colormap[i].blue=(Quantum) cluster->blue.center;
    i++;
  }
  /*
    Speed up distance calculations.
  */
  squares=(unsigned int *) malloc((MaxRGB+MaxRGB+1)*sizeof(unsigned int));
  if (squares == (unsigned int *) NULL)
    Error("Unable to allocate memory",(char *) NULL);
  squares+=MaxRGB;
  for (i=(-MaxRGB); i <= MaxRGB; i++)
    squares[i]=i*i;
  /*
    Do course grain classification.
  */
  q=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    for (cluster=head; cluster != (Cluster *) NULL; cluster=cluster->next)
      if (((int) q->red >= (cluster->red.left-SafeMargin)) &&
          ((int) q->red <= (cluster->red.right+SafeMargin)) &&
          ((int) q->green >= (cluster->green.left-SafeMargin)) &&
          ((int) q->green <= (cluster->green.right+SafeMargin)) &&
          ((int) q->blue >= (cluster->blue.left-SafeMargin)) &&
          ((int) q->blue <= (cluster->blue.right+SafeMargin)))
        {
          /*
            Classify this pixel.
          */
          q->index=cluster->id;
          break;
        }
    if (cluster == (Cluster *) NULL)
      {
        /*
          Compute fuzzy membership.
        */
        local_minima=0.0;
        for (j=0; j < image->colors; j++)
        {
          sum=0.0;
          distance=image->colormap[j].red-(int) q->red;
          distance_squared=squares[distance];
          distance=image->colormap[j].green-(int) q->green;
          distance_squared+=squares[distance];
          distance=image->colormap[j].blue-(int) q->blue;
          distance_squared+=squares[distance];
          numerator=sqrt(distance_squared);
          for (k=0; k < image->colors; k++)
          {
            distance=image->colormap[k].red-(int) q->red;
            distance_squared=squares[distance];
            distance=image->colormap[k].green-(int) q->green;
            distance_squared+=squares[distance];
            distance=image->colormap[k].blue-(int) q->blue;
            distance_squared+=squares[distance];
            ratio=numerator/sqrt(distance_squared);
            sum+=pow(ratio,(double) (2.0/(weighting_exponent-1.0)));
          }
          if ((1.0/sum) > local_minima)
            {
              /*
                Classify this pixel.
              */
              local_minima=1.0/sum;
              q->index=j;
            }
        }
      }
    q++;
    if (QuantumTick(i,image))
      ProgressMonitor(SegmentImageText,i,image->packets);
  }
  /*
    Free memory.
  */
  for (cluster=head; cluster != (Cluster *) NULL; cluster=next_cluster)
  {
    next_cluster=cluster->next;
    free((char *) cluster);
  }
  squares-=MaxRGB;
  free((char *) squares);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o n s o l i d a t e C r o s s i n g s                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function ConsolidateCrossings guarantees that an even number of zero
%  crossings always lie between two crossings.
%
%  The format of the ConsolidateCrossings routine is:
%
%      ConsolidateCrossings(zero_crossing,number_crossings)
%
%  A description of each parameter follows.
%
%    o zero_crossing: Specifies an array of structures of type ZeroCrossing.
%
%    o number_crossings: This unsigned int specifies the number of elements
%      in the zero_crossing array.
%
%
*/
static void ConsolidateCrossings(ZeroCrossing *zero_crossing, unsigned int number_crossings)
{
  int
    center,
    correct,
    count,
    left,
    right;

  register int
    i,
    j,
    k,
    l;

  /*
    Consolidate zero crossings.
  */
  for (i=number_crossings-1; i >= 0; i--)
    for (j=0; j <= MaxRGB; j++)
      if (zero_crossing[i].crossings[j] != 0)
        {
          /*
            Find the entry that is closest to j and still preserves the
            property that there are an even number of crossings between
            intervals.
          */
          for (k=j-1; k > 0; k--)
            if (zero_crossing[i+1].crossings[k] != 0)
              break;
          left=Max(k,0);
          center=j;
          for (k=j+1; k < MaxRGB; k++)
            if (zero_crossing[i+1].crossings[k] != 0)
              break;
          right=Min(k,MaxRGB);
          /*
            K is the zero crossing just left of j.
          */
          for (k=j-1; k > 0; k--)
            if (zero_crossing[i].crossings[k] != 0)
              break;
          if (k < 0)
            k=0;
          /*
            Check center for an even number of crossings between k and j.
          */
          correct=(-1);
          if (zero_crossing[i+1].crossings[j] != 0)
            {
              count=0;
              for (l=k+1; l < center; l++)
                if (zero_crossing[i+1].crossings[l] != 0)
                  count++;
              if ((count % 2) == 0)
                if (center != k)
                  correct=center;
            }
          /*
            Check left for an even number of crossings between k and j.
          */
          if (correct == -1)
            {
              count=0;
              for (l=k+1; l < left; l++)
                if (zero_crossing[i+1].crossings[l] != 0)
                  count++;
              if ((count % 2) == 0)
                if (left != k)
                  correct=left;
            }
          /*
            Check right for an even number of crossings between k and j.
          */
          if (correct == -1)
            {
              count=0;
              for (l=k+1; l < right; l++)
                if (zero_crossing[i+1].crossings[l] != 0)
                  count++;
              if ((count % 2) == 0)
                if (right != k)
                  correct=right;
            }
          l=zero_crossing[i].crossings[j];
          zero_crossing[i].crossings[j]=0;
          if (correct != -1)
            zero_crossing[i].crossings[correct]=l;
        }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e f i n e R e g i o n                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function DefineRegion defines the left and right boundaries of a peak
%  region.
%
%  The format of the DefineRegion routine is:
%
%      status=DefineRegion(extrema,extents)
%
%  A description of each parameter follows.
%
%    o extrema:  Specifies a pointer to an array of shortegers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
%    o extents:  This pointer to an ExtentPacket represent the extends
%      of a particular peak or valley of a color component.
%
%
*/
static int DefineRegion(short int *extrema, ExtentPacket *extents)
{
  /*
    Initialize to default values.
  */
  extents->left=0;
  extents->center=0;
  extents->right=MaxRGB;
  /*
    Find the left side (maxima).
  */
  for ( ; extents->index <= MaxRGB; extents->index++)
    if (extrema[extents->index] > 0)
      break;
  if (extents->index > MaxRGB)
    return(False);  /* no left side - no region exists */
  extents->left=extents->index;
  /*
    Find the right side (minima).
  */
  for ( ; extents->index <= MaxRGB; extents->index++)
    if (extrema[extents->index] < 0)
      break;
  extents->right=extents->index-1;
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e r i v a t i v e H i s t o g r a m                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function DerivativeHistogram determines the derivative of the histogram
%  using central differencing.
%
%  The format of the DerivativeHistogram routine is:
%
%      DerivativeHistogram(histogram,derivative)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of floats representing the number of
%      pixels for each intensity of a paritcular color component.
%
%    o derivative: This array of floats is initialized by DerivativeHistogram
%      to the derivative of the histogram using centeral differencing.
%
%
*/
static void DerivativeHistogram(float *histogram, float *derivative)
{
  register int
    i,
    n;

  /*
    Compute endpoints using second order polynomial interpolation.
  */
  n=MaxRGB;
  derivative[0]=(-1.5*histogram[0]+2.0*histogram[1]-0.5*histogram[2]);
  derivative[n]=(0.5*histogram[n-2]-2.0*histogram[n-1]+1.5*histogram[n]);
  /*
    Compute derivative using central differencing.
  */
  for (i=1; i < n; i++)
    derivative[i]=(histogram[i+1]-histogram[i-1])/2;
  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  I n i t i a l i z e H i s t o g r a m                                      %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function InitializeHistogram computes the histogram for an image.
%
%  The format of the InitializeHistogram routine is:
%
%      InitializeHistogram(image,histogram)
%
%  A description of each parameter follows.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o histogram: Specifies an array of longegers representing the number
%      of pixels for each intensity of a paritcular color component.
%
%
*/
static void InitializeHistogram(Image *image, long int **histogram)
{
  register int
    i;

  register RunlengthPacket
    *p;

  /*
    Initialize histogram.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    histogram[Red][i]=0;
    histogram[Green][i]=0;
    histogram[Blue][i]=0;
  }
  p=image->pixels;
  for (i=0; i < image->packets; i++)
  {
    histogram[Red][p->red]+=(p->length+1);
    histogram[Green][p->green]+=(p->length+1);
    histogram[Blue][p->blue]+=(p->length+1);
    p++;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I n i t i a l i z e I n t e r v a l T r e e                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function InitializeIntervalTree initializes an interval tree from the
%  lists of zero crossings.
%
%  The format of the InitializeIntervalTree routine is:
%
%      InitializeIntervalTree(zero_crossing,number_crossings)
%
%  A description of each parameter follows.
%
%    o zero_crossing: Specifies an array of structures of type ZeroCrossing.
%
%    o number_crossings: This unsigned int specifies the number of elements
%      in the zero_crossing array.
%
%
*/
static void InitializeList(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->child == (IntervalTree *) NULL)
    list[number_nodes++]=node;
  InitializeList(node->sibling);
  InitializeList(node->child);
}

static void MeanStability(IntervalTree *node)
{
  register IntervalTree
    *child;

  if (node == (IntervalTree *) NULL)
    return;
  node->mean_stability=0.0;
  child=node->child;
  if (child != (IntervalTree *) NULL)
    {
      register double
        sum;

      register int
        count;

      sum=0.0;
      count=0;
      for ( ; child != (IntervalTree *) NULL; child=child->sibling)
      {
        sum+=child->stability;
        count++;
      }
      node->mean_stability=sum/(double) count;
    }
  MeanStability(node->sibling);
  MeanStability(node->child);
}

static void Stability(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->child == (IntervalTree *) NULL)
    node->stability=0.0;
  else
    node->stability=node->tau-(node->child)->tau;
  Stability(node->sibling);
  Stability(node->child);
}

static IntervalTree *InitializeIntervalTree(ZeroCrossing *zero_crossing, unsigned int number_crossings)
{
  int
    left;

  IntervalTree
    *head,
    *node,
    *root;

  register int
    i,
    j,
    k;

  /*
    The root is the entire histogram.
  */
  root=(IntervalTree *) malloc(sizeof(IntervalTree));
  root->child=(IntervalTree *) NULL;
  root->sibling=(IntervalTree *) NULL;
  root->tau=0.0;
  root->left=0;
  root->right=MaxRGB;
  for (i=(-1); i < (int) number_crossings; i++)
  {
    /*
      Initialize list with all nodes with no children.
    */
    for (j=0; j < 600; j++)
      list[j]=(IntervalTree *) NULL;
    number_nodes=0;
    InitializeList(root);
    /*
      Split list.
    */
    for (j=0; j < number_nodes; j++)
    {
      head=list[j];
      left=head->left;
      node=head;
      for (k=head->left+1; k < head->right; k++)
      {
        if (zero_crossing[i+1].crossings[k] != 0)
          {
            if (node == head)
              {
                node->child=(IntervalTree *) malloc(sizeof(IntervalTree));
                node=node->child;
              }
            else
              {
                node->sibling=(IntervalTree *) malloc(sizeof(IntervalTree));
                node=node->sibling;
              }
            node->tau=zero_crossing[i+1].tau;
            node->child=(IntervalTree *) NULL;
            node->sibling=(IntervalTree *) NULL;
            node->left=left;
            node->right=k;
            left=k;
          }
        }
        if (left != head->left)
          {
            node->sibling=(IntervalTree *) malloc(sizeof(IntervalTree));
            node=node->sibling;
            node->tau=zero_crossing[i+1].tau;
            node->child=(IntervalTree *) NULL;
            node->sibling=(IntervalTree *) NULL;
            node->left=left;
            node->right=head->right;
          }
      }
    }
  /*
    Determine the stability: difference between a nodes tau and its child.
  */
  Stability(root->child);
  MeanStability(root->child);
  return(root);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   O p t i m a l T a u                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function OptimalTau finds the optimal tau for each band of the histogram.
%
%  The format of the OptimalTau routine is:
%
%      OptimalTau(histogram,max_tau,min_tau,delta_tau,smoothing_threshold,
%        extrema)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of longegers representing the number
%      of pixels for each intensity of a paritcular color component.
%
%    o extrema:  Specifies a pointer to an array of shortegers.  They
%      represent the peaks and valleys of the histogram for each color
%      component.
%
%
*/
static void ActiveNodes(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  if (node->stability >= node->mean_stability)
    {
      list[number_nodes++]=node;
      ActiveNodes(node->sibling);
    }
  else
    {
      ActiveNodes(node->sibling);
      ActiveNodes(node->child);
    }
}

static void FreeNodes(IntervalTree *node)
{
  if (node == (IntervalTree *) NULL)
    return;
  FreeNodes(node->sibling);
  FreeNodes(node->child);
  free((char *) node);
}

static double OptimalTau(long int *histogram, double max_tau, double min_tau, double delta_tau, double smoothing_threshold, short int *extrema, unsigned int plane)
{
  float
    average_tau,
    derivative[MaxRGB+1],
    second_derivative[MaxRGB+1],
    tau,
    value;

  int
    index,
    peak,
    x;

  IntervalTree
    *node,
    *root;

  register int
    i,
    j,
    k;

  unsigned int
    count,
    number_crossings;

  ZeroCrossing
    *zero_crossing;

  /*
    Allocate zero crossing list.
  */
  count=((max_tau-min_tau)/delta_tau)+2;
  zero_crossing=(ZeroCrossing *) malloc(count*sizeof(ZeroCrossing));
  if (zero_crossing == (ZeroCrossing *) NULL)
    Error("Unable to allocate memory",(char *) NULL);
  for (i=0; i < count; i++)
    zero_crossing[i].tau=(-1);
  /*
    Initialize zero crossing list.
  */
  i=0;
  for (tau=max_tau; tau >= min_tau; tau-=delta_tau)
  {
    zero_crossing[i].tau=tau;
    ScaleSpace(histogram,tau,zero_crossing[i].histogram);
    DerivativeHistogram(zero_crossing[i].histogram,derivative);
    DerivativeHistogram(derivative,second_derivative);
    ZeroCrossHistogram(second_derivative,smoothing_threshold,
      zero_crossing[i].crossings);
    i++;
  }
  /*
    Add an entry for the original histogram.
  */
  zero_crossing[i].tau=0.0;
  for (j=0; j <= MaxRGB; j++)
    zero_crossing[i].histogram[j]=histogram[j];
  DerivativeHistogram(zero_crossing[i].histogram,derivative);
  DerivativeHistogram(derivative,second_derivative);
  ZeroCrossHistogram(second_derivative,smoothing_threshold,
    zero_crossing[i].crossings);
  number_crossings=i;
  /*
    Ensure the scale-space fingerprints form lines in scale-space, not loops.
  */
  ConsolidateCrossings(zero_crossing,number_crossings);
  /*
    Force endpoints to be included in the interval.
  */
  for (i=0; i <= number_crossings; i++)
  {
    for (j=0; j < MaxRGB; j++)
      if (zero_crossing[i].crossings[j] != 0)
        break;
    zero_crossing[i].crossings[0]=(-zero_crossing[i].crossings[j]);
    for (j=MaxRGB; j > 0; j--)
      if (zero_crossing[i].crossings[j] != 0)
        break;
    zero_crossing[i].crossings[MaxRGB]=(-zero_crossing[i].crossings[j]);
  }
  /*
    Initialize interval tree.
  */
  root=InitializeIntervalTree(zero_crossing,number_crossings);
  /*
    Find active nodes:  stabilty is greater (or equal) to the mean stability of
    its children.
  */
  for (i = 0; i < 600; i++)
    list[i]=(IntervalTree *) NULL;
  number_nodes=0;
  ActiveNodes(root->child);
  /*
    Initialize extrema.
  */
  for (i=0; i <= MaxRGB; i++)
    extrema[i]=0;
  for (i=0; i < number_nodes; i++)
  {
    /*
      Find this tau in zero crossings list.
    */
    k=0;
    node=list[i];
    for (j=0; j <= number_crossings; j++)
      if (zero_crossing[j].tau == node->tau)
        k=j;
    /*
      Find the value of the peak.
    */
    peak=zero_crossing[k].crossings[node->right] == -1;
    index=node->left;
    value=zero_crossing[k].histogram[index];
    for (x=node->left; x <= node->right; x++)
    {
      if (peak)
        {
          if (zero_crossing[k].histogram[x] > value)
            {
              value=zero_crossing[k].histogram[x];
              index=x;
            }
        }
      else
        if (zero_crossing[k].histogram[x] < value)
          {
            value=zero_crossing[k].histogram[x];
            index=x;
          }
    }
    for (x=node->left; x <= node->right; x++)
    {
      if (index == 0)
        index=MaxRGB+1;
      if (peak)
        extrema[x]=index;
      else
        extrema[x]=(-index);
    }
  }
  /*
    Determine the average tau.
  */
  average_tau=0.0;
  for (i=0; i < number_nodes; i++)
    average_tau+=list[i]->tau;
  average_tau/=(float) number_nodes;
  /*
    Free memory.
  */
  FreeNodes(root);
  free((char *) zero_crossing);
  return(average_tau);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e S p a c e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function ScaleSpace performs a scale-space filter on the 1D histogram.
%
%  The format of the ScaleSpace routine is:
%
%      ScaleSpace(histogram,tau,scaled_histogram)
%
%  A description of each parameter follows.
%
%    o histogram: Specifies an array of floats representing the number of
%      pixels for each intensity of a paritcular color component.
%
%
*/
static void ScaleSpace(long int *histogram, double tau, float *scaled_histogram)
{
  float
    alpha,
    beta;

  double
    sum;

  register int
    u,
    x;

  alpha=1.0/(tau*sqrt((double) (2.0*M_PI)));
  beta=(-1.0/(2.0*tau*tau));
  for (x=0; x <= MaxRGB; x++)
  {
    sum=0.0;
    for (u=0; u <= MaxRGB; u++)
      sum+=histogram[u]*exp((double) (beta*(x-u)*(x-u)));
    scaled_histogram[x]=alpha*sum;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z e r o C r o s s H i s t o g r a m                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%  Function ZeroCrossHistogram find the zero crossings in a histogram and
%  marks directions as:  1 is negative to positive; 0 is zero crossing; and
%  -1 is positive to negative.
%
%  The format of the ZeroCrossHistogram routine is:
%
%      ZeroCrossHistogram(second_derivative,smoothing_threshold,crossings)
%
%  A description of each parameter follows.
%
%    o second_derivative: Specifies an array of floats representing the
%      second derivative of the histogram of a particular color component.
%
%    o crossings:  This array of shortegers is initialized with
%      -1, 0, or 1 representing the slope of the first derivative of the
%      of a particular color component.
%
%
*/
static void ZeroCrossHistogram(float *second_derivative, double smoothing_threshold, short int *crossings)
{
  int
    parity;

  register int
    i;

  /*
    Merge low numbers to zero to help prevent noise.
  */
  for (i=0; i <= MaxRGB; i++)
    if ((second_derivative[i] < smoothing_threshold) &&
        (second_derivative[i] > -smoothing_threshold))
      second_derivative[i]=0.0;
  /*
    Mark zero crossings.
  */
  parity=0;
  for (i=0; i <= MaxRGB; i++)
  {
    crossings[i]=0;
    if (second_derivative[i] < 0.0)
      {
        if (parity > 0)
          crossings[i]=(-1);
        parity=1;
      }
    else
      if (second_derivative[i] > 0.0)
        {
          if (parity < 0)
            crossings[i]=1;
          parity=(-1);
        }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  S e g m e n t I m a g e                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function SegmentImage analyzes the colors within a reference image and
%
%  The format of the SegmentImage routine is:
%
%      colors=SegmentImage(image,colorspace,verbose)
%
%  A description of each parameter follows.
%
%    o colors: The SegmentImage function returns this integer
%      value.  It is the actual number of colors allocated in the
%      colormap.
%
%    o image: Specifies a pointer to an Image structure;  returned from
%      ReadImage.
%
%    o colorspace: An unsigned integer value that indicates the colorspace.
%      Empirical evidence suggests that distances in YUV or YIQ correspond to
%      perceptual color differences more closely than do distances in RGB
%      space.  The image is then returned to RGB colorspace after color
%      reduction.
%
%    o verbose:  A value greater than zero prints detailed information about
%      the identified classes.
%
%
*/
void SegmentImage(Image *image, unsigned int colorspace, unsigned int verbose, double smoothing_threshold, double cluster_threshold)
{
#define DeltaTau  0.5
#define Tau  5.2
#define WeightingExponent  2.0

  long
    *histogram[Dimension];

  register int
    i;

  short
    *extrema[Dimension];

  /*
    Allocate histogram and extrema.
  */
  for (i=0; i < Dimension; i++)
  {
    histogram[i]=(long *) malloc((MaxRGB+1)*sizeof(long));
    extrema[i]=(short *) malloc((MaxRGB+1)*sizeof(short));
    if ((histogram[i] == (long *) NULL) || (extrema[i] == (short *) NULL))
      Error("Unable to allocate memory",(char *) NULL);
  }
  if (colorspace != RGBColorspace)
    RGBTransformImage(image,colorspace);
  /*
    Initialize histogram.
  */
  InitializeHistogram(image,histogram);
  (void) OptimalTau(histogram[Red],Tau,0.2,DeltaTau,smoothing_threshold,
    extrema[Red],Red);
  (void) OptimalTau(histogram[Green],Tau,0.2,DeltaTau,smoothing_threshold,
    extrema[Green],Green);
  (void) OptimalTau(histogram[Blue],Tau,0.2,DeltaTau,smoothing_threshold,
    extrema[Blue],Blue);
  /*
    Classify using the fuzzy c-Means technique.
  */
  Classify(image,extrema,cluster_threshold,WeightingExponent,verbose);
  if (colorspace != RGBColorspace)
    TransformRGBImage(image,colorspace);
  /*
    Free memory.
  */
  for (i=0; i < Dimension; i++)
  {
    free((char *) extrema[i]);
    free((char *) histogram[i]);
  }
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

#ifdef __cplusplus
}
#endif


/* -------------------------------------------------------------------------- */

#endif   /* #ifdef KJB_HAVE_X11  */

#endif   /* #ifndef __C2MAN__  */

