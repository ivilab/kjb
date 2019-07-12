
/* $Id: jv_lap.c 20654 2016-05-05 23:13:43Z kobus $ */

/*
// I have hacked this to fit better with my stuff, and  to make it work better
// with doubles. Unfortunately there still are problems, especially when
// optimized with gcc on athlon. When this code does work, it is at least twice
// as fast as straight hungarian method for matrices with roughly equal numbers
// of rows and columns. The hungarian code in hungarian.c explicitly saves time
// when the matrices are not square, whereas for this code we square up the
// matrices with zeros, thus solving a bigger problem than needed. Presumably
// this code could be modified so that this is not necessary.
//
// Since I don't trust the code with the doubles (dbl_jv_lap()), I have
// implemented the double case a second way (jv_lap()) by scaling and
// converting to integers, and solving that problem with the integer version of
// the code (int_jv_lap()).  This method is less accurate, but the tolerance is
// bounded in advance and easily calculated (see the variable "tol"). However,
// on real problems, this also does not seem to work because even the integer
// code takes an inordinate number of iterations on the ocasional data set. In
// this code the number of interations is limited to quite a large number
// (MAX_LOOP_FACTOR*prvnumfree).  I don't understand the algorithm well enough
// to know if convergence should be expected eventually.
//
// Due to these problems, I currently use the code in hungarian.c.
*/

/************************************************************************
*
*  lap.cpp
   version 1.0 - 4 September 1996
   author: Roy Jonker @ MagicLogic Optimization Inc.
   e-mail: roy_jonker@magiclogic.com

   Code for Linear Assignment Problem, according to

   "A Shortest Augmenting Path Algorithm for Dense and Sparse Linear
    Assignment Problems," Computing 38, 325-340, 1987

   by

   R. Jonker and A. Volgenant, University of Amsterdam.
*
*************************************************************************/

#include "m/m_incl.h"
#include "graph/jv_lap.h"

#define MAX_LOOP_FACTOR 10000000

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

static int int_checklap
(
    int   dim,
    int** assigncost,
    int*  rowsol,
    int*  colsol,
    int*  u,
    int*  v
);

static int dbl_checklap
(
    int      dim,
    double** assigncost,
    int*     rowsol,
    int*     colsol,
    double*  u,
    double*  v
);

/* -------------------------------------------------------------------------- */

int jv_lap(const Matrix* cost_mp, Int_vector** row_assignment_vpp, double* cost_ptr)
{
    int         m           = cost_mp->num_rows;
    int         n           = cost_mp->num_cols;
    Int_matrix* int_cost_mp = NULL;
    Matrix*     scaled_mp   = NULL;
    double      max_elem;
    double      scale_up;
    double      factor;
    int         int_cost;
    int result = NO_ERROR;
    /*
    double tol;
    */

    max_elem = max_abs_matrix_element(cost_mp);

    /* 2e9 is about the largest integer. */
    scale_up = 2e9 / MAX_OF(n, m);
    factor = scale_up / max_elem;
    /* tol = MAX_OF(m, n) / factor; */

    if (result != ERROR)
    {
        result = multiply_matrix_by_scalar(&scaled_mp, cost_mp, factor);
    }

    if (result != ERROR)
    {
        result = copy_matrix_to_int_matrix(&int_cost_mp, scaled_mp);
    }


    if (result != ERROR)
    {
        result = int_jv_lap(int_cost_mp, row_assignment_vpp, &int_cost);
    }


    if (result != ERROR)
    {
        *cost_ptr = (double)int_cost / factor;
    }

    free_matrix(scaled_mp);
    free_int_matrix(int_cost_mp);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

int int_jv_lap(const Int_matrix* cost_mp, Int_vector** row_assignment_vpp, int* cost_ptr)
{
  int unassignedfound;
  int      imin;
  int      numfree    = 0;
  int      prvnumfree;
  int      freerow;
  int*     pred;
  int*     free;
  int      i, j, j1, j2, f, i0, k;
  int      endofpath;
  int      last;
  int      low, up;
  int*     collist;
  int*     matches;
  int   min, h;
  int   umin;
  int   usubmin;
  int   v2;
  int*  d;
  int      loopcnt;
  int   lapcost    = 0;
  int** assigncost;
  Int_matrix*  assigncost_mp = NULL;
  int*  v;
  /*
  int   s;
  */
  int      m        = cost_mp->num_rows;
  int      n        = cost_mp->num_cols;
  int      dim      = MAX_OF(m, n);
  int      result   = NO_ERROR;
  int*     colsol;
  int*     rowsol;
  int* u;
  int* row_assignments;


  ERE(get_target_int_vector(row_assignment_vpp, m));

  row_assignments = (*row_assignment_vpp)->elements;


  if (m == n)
  {
      ERE(copy_int_matrix(&assigncost_mp, cost_mp));
  }
  else
  {
      ERE(get_zero_int_matrix(&assigncost_mp, dim, dim));
      ERE(ow_copy_int_matrix_block(assigncost_mp, 0, 0, cost_mp,
                                   0, 0, m, n));
  }

  assigncost = assigncost_mp->elements;


  NRE(rowsol = INT_MALLOC(dim));
  NRE(colsol = INT_MALLOC(dim));
  NRE(free = INT_MALLOC(dim));       /* list of unassigned rows. */
  NRE(collist = INT_MALLOC(dim));    /* list of columns to be scanned in various ways. */
  NRE(matches = INT_MALLOC(dim));    /* counts how many times a row could be assigned. */
  NRE(d = INT_MALLOC(dim));         /* 'int-distance' in augmenting path calculation. */
  NRE(v = INT_MALLOC(dim));
  NRE(pred = INT_MALLOC(dim));       /* row-predecessor of column in augmenting/alternating path. */

  /* init how many times a row will be assigned in the column reduction. */
  for (i = 0; i < dim; i++)
  {
    matches[i] = 0;
  }

  /* COLUMN REDUCTION  */
  for (j = dim-1; j >= 0; j--)    /* reverse order gives better results. */
  {
    /* find minimum int over rows. */
    min = assigncost[0][j];
    imin = 0;

    for (i = 1; i < dim; i++)
    {
        if (assigncost[i][j] < min)
        {
            min = assigncost[i][j];
            imin = i;
        }
    }
    v[j] = min;

    if (++matches[imin] == 1)
    {
        /* init assignment if minimum row assigned for first time. */
        rowsol[imin] = j;
        colsol[j] = imin;
    }
    else
    {
        colsol[j] = -1;        /* row already assigned, column not assigned. */
    }
  }

  /* REDUCTION TRANSFER */
  for (i = 0; i < dim; i++)
  {
      if (matches[i] == 0)     /* fill list of unassigned 'free' rows. */
      {
          free[numfree++] = i;
      }
      else
      {
          if (matches[i] == 1)   /* transfer reduction from rows that are assigned once. */
          {
              j1 = rowsol[i];
              min = INT_MAX;

              for (j = 0; j < dim; j++)
              {
                  if (j != j1)
                  {
                      if (assigncost[i][j] - v[j] < min)
                      {
                          min = assigncost[i][j] - v[j];
                      }
                  }
              }
              v[j1] = v[j1] - min;
          }
      }
  }

  /* AUGMENTING ROW REDUCTION  */
  loopcnt = 0;           /* do-loop to be done twice. */
  do
  {
      loopcnt++;

      /* scan all free rows. */
      /* in some cases, a free row may be replaced with another one to be scanned next. */
      k = 0;
      prvnumfree = numfree;
      numfree = 0;             /* start list of rows still free after augmenting row reduction. */
      while (k < prvnumfree)
      {
          i = free[k];
          k++;

          /* find minimum and second minimum reduced int over columns. */
          umin = assigncost[i][0] - v[0];
          j1 = 0;
          usubmin = INT_MAX;

          for (j = 1; j < dim; j++)
          {
              h = assigncost[i][j] - v[j];

              if (h < usubmin)
              {
                  if (h >= umin)
                  {
                      usubmin = h;
                      j2 = j;
                  }
                  else
                  {
                      usubmin = umin;
                      umin = h;
                      j2 = j1;
                      j1 = j;
                  }
              }
          }

          i0 = colsol[j1];
          if (umin < usubmin)
          {
              /* change the reduction of the minimum column to increase the minimum */
              /* reduced int in the row to the subminimum. */
              v[j1] = v[j1] - (usubmin - umin);
          }
          else                   /* minimum and subminimum equal. */
          {
              if (i0 >= 0)         /* minimum column j1 is assigned. */
              {
                  /* swap columns j1 and j2, as j2 may be unassigned. */
                  j1 = j2;
                  i0 = colsol[j2];
              }
          }

          /* (re-)assign i to j1, possibly de-assigning an i0. */
          rowsol[i] = j1;
          colsol[j1] = i;

          if (i0 >= 0)           /* minimum column j1 assigned earlier. */
          {
              if (umin < usubmin)
              {
                  /* put in current k, and go back to that k. */
                  /* continue augmenting path i - j1 with i0. */
                  free[--k] = i0;
              }
              else
              {
                  /* no further augmenting reduction possible. */
                  /* store i0 in list of free rows for next phase. */
                  free[numfree++] = i0;
              }
          }
      }
  }
  while (loopcnt < 2);       /* repeat once. */

  /* AUGMENT SOLUTION for each free row. */
  for (f = 0; f < numfree; f++)
  {
      freerow = free[f];       /* start row of augmenting path. */

      /* Dijkstra shortest path algorithm. */
      /* runs until unassigned column added to shortest path tree. */
      for (j = 0; j < dim; j++)
      {
          d[j] = assigncost[freerow][j] - v[j];
          pred[j] = freerow;
          collist[j] = j;        /* init column list. */
      }

      low = 0; /* columns in 0..low-1 are ready, now none. */
      up = 0;  /* columns in low..up-1 are to be scanned for current minimum, now none. */
      /* columns in up..dim-1 are to be considered later to find new minimum,  */
      /* at this stage the list simply contains all columns  */
      unassignedfound = FALSE;
      do
      {
          if (up == low)         /* no more columns to be scanned for current minimum. */
          {
              last = low - 1;

              /* scan columns for up..dim-1 to find all indices for which new minimum occurs. */
              /* store these indices between low..up-1 (increasing up).  */
              min = d[collist[up++]];

              for (k = up; k < dim; k++)
              {
                  j = collist[k];
                  h = d[j];

                  if (h <= min)
                  {
                      if (h < min)     /* new minimum. */
                      {
                          up = low;      /* restart list at index low. */
                          min = h;
                      }
                      /* new index with same minimum, put on undex up, and extend list. */
                      collist[k] = collist[up];
                      collist[up++] = j;
                  }
              }

              /* check if any of the minimum columns happens to be unassigned. */
              /* if so, we have an augmenting path right away. */
              for (k = low; k < up; k++)
              {
                  if (colsol[collist[k]] < 0)
                  {
                      endofpath = collist[k];
                      unassignedfound = TRUE;
                      break;
                  }
              }
          }

          if (!unassignedfound)
          {
              /* update 'distances' between freerow and all unscanned columns, via next scanned column. */
              j1 = collist[low];
              low++;
              i = colsol[j1];
              h = assigncost[i][j1] - v[j1] - min;

              for (k = up; k < dim; k++)
              {
                  j = collist[k];
                  v2 = assigncost[i][j] - v[j] - h;

                  if (v2 < d[j])
                  {
                      pred[j] = i;

                      if (v2 == min)
                      {
                          if (colsol[j] < 0)
                          {
                              /* if unassigned, shortest augmenting path is complete. */
                              endofpath = j;
                              unassignedfound = TRUE;
                              break;
                          }
                          /* else add to list to be scanned right away. */
                          else
                          {
                              collist[k] = collist[up];
                              collist[up++] = j;
                          }
                      }
                      d[j] = v2;
                  }
              }
          }
      }
      while (!unassignedfound);

      /* update column prices. */
      for (k = 0; k <= last; k++)
      {
          j1 = collist[k];
          v[j1] = v[j1] + d[j1] - min;
      }

      /* reset row and column assignments along the alternating path. */
      do
      {
          i = pred[endofpath];
          colsol[endofpath] = i;
          j1 = endofpath;
          endofpath = rowsol[i];
          rowsol[i] = j1;
      }
      while (i != freerow);
  }

  if (result != ERROR)
  {
      /*
      // TEST code. This code is too buggy, especiallay when
      // optimized, to run without a test at the end. I.e, it is a bad idea to
      // conditionally compile this.
      */
      NRE(u = INT_MALLOC(dim));

      for (i = 0; i < dim; i++)
      {
          j = rowsol[i];
          u[i] = assigncost[i][j] - v[j];
      }

      result = int_checklap(dim, assigncost, rowsol, colsol, u, v);

      kjb_free(u);

      /* End TEST */

      for (i = 0; i < m; i++)
      {
          j = rowsol[ i ];

          if (j < n)
          {
              row_assignments[ i ] = j;
          }
          else
          {
              row_assignments[ i ] = -1;
          }
      }
  }

  kjb_free(rowsol);
  kjb_free(colsol);
  kjb_free(pred);
  kjb_free(free);
  kjb_free(collist);
  kjb_free(matches);
  kjb_free(d);
  kjb_free(v);
  free_int_matrix(assigncost_mp);

  if (result == ERROR) return ERROR;


  /* calculate optimal cost. */
  for (i = 0; i < m; i++)
  {
    j = row_assignments[i];

    if (j >= 0)
    {
        lapcost += cost_mp->elements[i][j];
    }
  }

  *cost_ptr = lapcost;

  return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int int_checklap
(
    int   dim,
    int** assigncost,
    int*  rowsol,
    int*  colsol,
    int*  u,
    int*  v
)
{
    int  i;
    int  j;
    int redcost = 0;
    int *matched = NULL;
    int result = NO_ERROR;


    for (i = 0; i < dim; i++)
    {
        for (j = 0; j < dim; j++)
        {
            redcost = assigncost[i][j] - u[i] - v[j];

            if (redcost < 0)
            {
                if (result != ERROR)
                {
                    kjb_clear_error();
                }

                add_error("negative reduced cost i %d j %d redcost %d",
                          i, j, redcost);
                cat_error(" dim %5d", dim);

                result = ERROR;
            }
        }
    }

    for (i = 0; i < dim; i++)
    {
        redcost = assigncost[i][rowsol[i]] - u[i] - v[rowsol[i]];

        if (redcost != 0)
        {
            if (result != ERROR)
            {
                kjb_clear_error();
            }

            add_error("non-null reduced cost i %d soli %d redcost %d",
                      i, rowsol[i], redcost);
            cat_error(" dim %5d", dim);
            result = ERROR;
        }
    }

    NRE(matched = N_TYPE_MALLOC(int, dim));

    for (j = 0; j < dim; j++)
    {
        matched[j] = FALSE;
    }

    for (i = 0; i < dim; i++)
    {
        if (matched[rowsol[i]])
        {
            if (result != ERROR)
            {
                kjb_clear_error();
            }

            add_error("column matched more than once - i %d soli %d", i, rowsol[i]);
            cat_error(" dim %5d ", dim);

            result = ERROR;
        }
        else
        {
            matched[ rowsol[i] ] = TRUE;
        }
    }

    for (i = 0; i < dim; i++)
    {
        if (colsol[rowsol[i]] != i)
        {
            if (result != ERROR)
            {
                kjb_clear_error();
            }

            add_error("error in row solution i %d soli %d solsoli %d", i, rowsol[i], colsol[rowsol[i]]);
            cat_error(" dim %5d ", dim);

            result = ERROR;
        }
    }

    for (j = 0; j < dim; j++)
    {
        if (rowsol[colsol[j]] != j)
        {
            if (result != ERROR)
            {
                kjb_clear_error();
            }

            add_error("error in col solution j %d solj %d solsolj %d", j, colsol[j], rowsol[colsol[j]]);
            cat_error(" dim %5d", dim);

            result = ERROR;
        }
    }

    kjb_free(matched);

    return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

/*
 * FIXME
 *
 * This is not finished and should be considered BUGGY.
*/

int dbl_jv_lap(const Matrix* cost_mp, Int_vector** row_assignment_vpp, double* cost_ptr)
{
  int unassignedfound;
  int      imin;
  int      numfree    = 0;
  int      prvnumfree;
  int      freerow;
  int*     pred;
  int*     free;
  int      i, j, j1, j2, f, i0, k;
  int      endofpath;
  int      last;
  int      low, up;
  int*     collist;
  int*     matches;
  double   min, h;
  double   umin;
  double   usubmin;
  double   v2;
  double*  d;
  int      loopcnt;
  double   lapcost    = 0.0;
  double** assigncost;
  Matrix*  assigncost_mp = NULL;
  double*  v;
  /*
  double   s;
  */
  int      m        = cost_mp->num_rows;
  int      n        = cost_mp->num_cols;
  int      dim      = MAX_OF(m, n);
  int      result   = NO_ERROR;
  int*     colsol;
  int*     rowsol;
  double* u;
  int* row_assignments;


  ERE(get_target_int_vector(row_assignment_vpp, m));

  row_assignments = (*row_assignment_vpp)->elements;

  if (m == n)
  {
      ERE(copy_matrix(&assigncost_mp, cost_mp));
  }
  else
  {
      ERE(get_zero_matrix(&assigncost_mp, dim, dim));
      ERE(ow_copy_matrix_block(assigncost_mp, 0, 0, cost_mp,
                               0, 0, m, n));
  }

  /*
  // Emphasizes bug whereby this code can behave differently with and without
  // optimization.
  //
  // ERE(ow_scale_matrix_by_max(assigncost_mp));
  */

  assigncost = assigncost_mp->elements;

  /*
  // Emphasizes bug whereby this code can behave differently with and without
  // optimization.
  //
  for(j= 0;j<dim;j++)
  {
      s = assigncost[ 0 ][ j ];

      for(i= 1;i<dim;i++)
      {
          if(assigncost[ i ][ j ] < s)
          {
              s = assigncost[ i ][ j ];
          }
      }

      for(i= 0; i<dim; i++)
      {
          assigncost[ i ][ j ] -= s;
      }
  }
  */

  NRE(rowsol = INT_MALLOC(dim));
  NRE(colsol = INT_MALLOC(dim));
  NRE(free = INT_MALLOC(dim));       /* list of unassigned rows. */
  NRE(collist = INT_MALLOC(dim));    /* list of columns to be scanned in various ways. */
  NRE(matches = INT_MALLOC(dim));    /* counts how many times a row could be assigned. */
  NRE(d = DBL_MALLOC(dim));         /* 'double-distance' in augmenting path calculation. */
  NRE(v = DBL_MALLOC(dim));
  NRE(pred = INT_MALLOC(dim));       /* row-predecessor of column in augmenting/alternating path. */

  /* init how many times a row will be assigned in the column reduction. */
  for (i = 0; i < dim; i++)
  {
    matches[i] = 0;
  }

  /* COLUMN REDUCTION  */
  for (j = dim-1; j >= 0; j--)    /* reverse order gives better results. */
  {
    /* find minimum double over rows. */
    min = assigncost[0][j];
    imin = 0;

    for (i = 1; i < dim; i++)
    {
        if (assigncost[i][j] < min)
        {
            min = assigncost[i][j];
            imin = i;
        }
    }
    v[j] = min;

    if (++matches[imin] == 1)
    {
        /* init assignment if minimum row assigned for first time. */
        rowsol[imin] = j;
        colsol[j] = imin;
    }
    else
    {
        colsol[j] = -1;        /* row already assigned, column not assigned. */
    }
  }

  /* REDUCTION TRANSFER */
  for (i = 0; i < dim; i++)
  {
      if (matches[i] == 0)     /* fill list of unassigned 'free' rows. */
      {
          free[numfree++] = i;
      }
      else
      {
          if (matches[i] == 1)   /* transfer reduction from rows that are assigned once. */
          {
              j1 = rowsol[i];
              min = DBL_MAX;

              for (j = 0; j < dim; j++)
              {
                  if (j != j1)
                  {
                      if (assigncost[i][j] - v[j] < min)
                      {
                          min = assigncost[i][j] - v[j];
                      }
                  }
              }
              v[j1] = v[j1] - min;
          }
      }
  }

  /* AUGMENTING ROW REDUCTION  */
  loopcnt = 0;           /* do-loop to be done twice. */
  do
  {
      int infinite_loop_guard = 0;   /* Kobus */

      loopcnt++;

      /* scan all free rows. */
      /* in some cases, a free row may be replaced with another one to be scanned next. */
      k = 0;
      prvnumfree = numfree;
      numfree = 0;             /* start list of rows still free after augmenting row reduction. */
      while (k < prvnumfree)
      {

          /* Kobus
          //
          // There is a possible oscillation here, presumably due to precision
          // problems?
          */
          if (infinite_loop_guard > MAX_LOOP_FACTOR * prvnumfree)
          {
              set_error("Possible precision problem with dbl_jv_lap.\n");
              add_error("Cutting off lap after %d its.\n",
                        MAX_LOOP_FACTOR * prvnumfree);
              result = ERROR;
              break;
          }

          i = free[k];
          k++;

          /* find minimum and second minimum reduced double over columns. */
          umin = assigncost[i][0] - v[0];
          j1 = 0;
          usubmin = DBL_MAX;

          for (j = 1; j < dim; j++)
          {
              h = assigncost[i][j] - v[j];

              if (h < usubmin)
              {
                  if (h >= umin)
                  {
                      usubmin = h;
                      j2 = j;
                  }
                  else
                  {
                      usubmin = umin;
                      umin = h;
                      j2 = j1;
                      j1 = j;
                  }
              }
          }

          i0 = colsol[j1];
          if (umin < usubmin)
          {
              /* change the reduction of the minimum column to increase the minimum */
              /* reduced double in the row to the subminimum. */
              v[j1] = v[j1] - (usubmin - umin);
          }
          else                   /* minimum and subminimum equal. */
          {
              if (i0 >= 0)         /* minimum column j1 is assigned. */
              {
                  /* swap columns j1 and j2, as j2 may be unassigned. */
                  j1 = j2;
                  i0 = colsol[j2];
              }
          }

          /* (re-)assign i to j1, possibly de-assigning an i0. */
          rowsol[i] = j1;
          colsol[j1] = i;

          if (i0 >= 0)           /* minimum column j1 assigned earlier. */
          {
              if (umin < usubmin)
              {
                  /* put in current k, and go back to that k. */
                  /* continue augmenting path i - j1 with i0. */
                  free[--k] = i0;
              }
              else
              {
                  /* no further augmenting reduction possible. */
                  /* store i0 in list of free rows for next phase. */
                  free[numfree++] = i0;
              }
          }

          infinite_loop_guard++;   /* Kobus */
      }
  }
  while (loopcnt < 2);       /* repeat once. */

  /* AUGMENT SOLUTION for each free row. */
  for (f = 0; f < numfree; f++)
  {
      freerow = free[f];       /* start row of augmenting path. */

      /* Dijkstra shortest path algorithm. */
      /* runs until unassigned column added to shortest path tree. */
      for (j = 0; j < dim; j++)
      {
          d[j] = assigncost[freerow][j] - v[j];
          pred[j] = freerow;
          collist[j] = j;        /* init column list. */
      }

      low = 0; /* columns in 0..low-1 are ready, now none. */
      up = 0;  /* columns in low..up-1 are to be scanned for current minimum, now none. */
      /* columns in up..dim-1 are to be considered later to find new minimum,  */
      /* at this stage the list simply contains all columns  */
      unassignedfound = FALSE;
      do
      {
          if (up == low)         /* no more columns to be scanned for current minimum. */
          {
              last = low - 1;

              /* scan columns for up..dim-1 to find all indices for which new minimum occurs. */
              /* store these indices between low..up-1 (increasing up).  */
              min = d[collist[up++]];

              for (k = up; k < dim; k++)
              {
                  j = collist[k];
                  h = d[j];

                  if (h <= min)
                  {
                      if (h < min)     /* new minimum. */
                      {
                          up = low;      /* restart list at index low. */
                          min = h;
                      }
                      /* new index with same minimum, put on undex up, and extend list. */
                      collist[k] = collist[up];
                      collist[up++] = j;
                  }
              }

              /* check if any of the minimum columns happens to be unassigned. */
              /* if so, we have an augmenting path right away. */
              for (k = low; k < up; k++)
              {
                  if (colsol[collist[k]] < 0)
                  {
                      endofpath = collist[k];
                      unassignedfound = TRUE;
                      break;
                  }
              }
          }

          if (!unassignedfound)
          {
              /* update 'distances' between freerow and all unscanned columns, via next scanned column. */
              j1 = collist[low];
              low++;
              i = colsol[j1];
              h = assigncost[i][j1] - v[j1] - min;

              for (k = up; k < dim; k++)
              {
                  j = collist[k];
                  v2 = assigncost[i][j] - v[j] - h;

                  if (v2 < d[j])
                  {
                      pred[j] = i;

                      /* if (v2 == min) */  /* new column found at same minimum value */
                      if (    (    (ABS_OF(v2) < 100000.0 * DBL_EPSILON)
                                && (ABS_OF(min) < 100000.0 * DBL_EPSILON)
                              )
                           || (ABS_OF(v2 - min) / (ABS_OF(v2) + ABS_OF(min)) < 100000.0 * DBL_EPSILON)
                         )
                      {
                          if (colsol[j] < 0)
                          {
                              /* if unassigned, shortest augmenting path is complete. */
                              endofpath = j;
                              unassignedfound = TRUE;
                              break;
                          }
                          /* else add to list to be scanned right away. */
                          else
                          {
                              collist[k] = collist[up];
                              collist[up++] = j;
                          }
                      }
                      d[j] = v2;
                  }
              }
          }
      }
      while (!unassignedfound);

      /* update column prices. */
      for (k = 0; k <= last; k++)
      {
          j1 = collist[k];
          v[j1] = v[j1] + d[j1] - min;
      }

      /* reset row and column assignments along the alternating path. */
      do
      {
          i = pred[endofpath];
          colsol[endofpath] = i;
          j1 = endofpath;
          endofpath = rowsol[i];
          rowsol[i] = j1;
      }
      while (i != freerow);
  }

  if (result != ERROR)
  {
      /*
      // TEST code. Unfortunately, this code is too buggy, especiallay when
      // optimized, to run without a test at the end. I.e, it is a bad idea to
      // conditionally compile this.
      */
      NRE(u = DBL_MALLOC(dim));

      for (i = 0; i < dim; i++)
      {
          j = rowsol[i];
          u[i] = assigncost[i][j] - v[j];
      }

      result = dbl_checklap(dim, assigncost, rowsol, colsol, u, v);

      kjb_free(u);

      /* End TEST */

      for (i = 0; i < m; i++)
      {
          j = rowsol[ i ];

          if (j < n)
          {
              row_assignments[ i ] = j;
          }
          else
          {
              row_assignments[ i ] = -1;
          }
      }
  }

  /* free reserved memory. */
  kjb_free(rowsol);
  kjb_free(colsol);
  kjb_free(pred);
  kjb_free(free);
  kjb_free(collist);
  kjb_free(matches);
  kjb_free(d);
  kjb_free(v);
  free_matrix(assigncost_mp);

  if (result == ERROR) return ERROR;

  /* calculate optimal cost. */
  for (i = 0; i < m; i++)
  {
    j = row_assignments[i];

    if (j >= 0)
    {
        lapcost += cost_mp->elements[i][j];
    }
  }

  *cost_ptr = lapcost;

  return result;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */

static int dbl_checklap
(
    int      dim,
    double** assigncost,
    int*     rowsol,
    int*     colsol,
    double*  u,
    double*  v
)
{
  int  i;
  int  j;
  double redcost = 0;
  int *matched = NULL;


  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
    {
      redcost = assigncost[i][j] - u[i] - v[j];

      if (redcost < -1e-7)
      {
        set_error("negative reduced cost i %d j %d redcost %e", i, j, redcost);
        cat_error(" dim %5d", dim);
        return ERROR;
      }
    }

  for (i = 0; i < dim; i++)
  {
    redcost = assigncost[i][rowsol[i]] - u[i] - v[rowsol[i]];

    if (ABS_OF(redcost) > 1e-7)
    {
      set_error("non-null reduced cost i %d soli %d redcost %e",
                i, rowsol[i], redcost);
      cat_error(" dim %5d", dim);
      return ERROR;
    }
  }

  NRE(matched = N_TYPE_MALLOC(int, dim));

  for (j = 0; j < dim; j++)
  {
    matched[j] = FALSE;
  }

  for (i = 0; i < dim; i++)
  {
    if (matched[rowsol[i]])
    {
      set_error("column matched more than once - i %d soli %d", i, rowsol[i]);
      cat_error(" dim %5d ", dim);
      kjb_free(matched);
      return ERROR;
    }
    else
    {
      matched[rowsol[i]] = TRUE;
    }
  }


  for (i = 0; i < dim; i++)
  {
    if (colsol[rowsol[i]] != i)
    {
      set_error("error in row solution i %d soli %d solsoli %d", i, rowsol[i], colsol[rowsol[i]]);
      cat_error(" dim %5d ", dim);
      kjb_free(matched);
      return ERROR;
    }
  }

  for (j = 0; j < dim; j++)
  {
    if (rowsol[colsol[j]] != j)
    {
      set_error("error in col solution j %d solj %d solsolj %d", j, colsol[j], rowsol[colsol[j]]);
      cat_error(" dim %5d", dim);
      kjb_free(matched);
      return ERROR;
    }
  }

  kjb_free(matched);

  return NO_ERROR;
}

/*  /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\   */


#ifdef __cplusplus
}
#endif

