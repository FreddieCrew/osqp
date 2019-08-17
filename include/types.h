#ifndef OSQP_TYPES_H
#define OSQP_TYPES_H

# ifdef __cplusplus
extern "C" {
# endif // ifdef __cplusplus

# include "glob_opts.h"
# include "osqp.h"

/******************
* Internal types *
******************/

/*
 * OSQPVector types.  Not defined here since it
 *   is implementation specific
 */

/* integer valued vectors */
typedef struct OSQPVectori_ OSQPVectori;

/* float valued vectors*/
typedef struct OSQPVectorf_ OSQPVectorf;

/**
 * Linear system solver structure (sublevel objects initialize it differently)
 */

typedef struct linsys_solver LinSysSolver;

/**
 * OSQP Timer for statistics
 */
typedef struct OSQP_TIMER OSQPTimer;

/**
 * Problem scaling matrices stored as vectors
 */
typedef struct {
  c_float  c;         ///< cost function scaling
  OSQPVectorf *D;     ///< primal variable scaling
  OSQPVectorf *E;     ///< dual variable scaling
  c_float  cinv;      ///< cost function rescaling
  OSQPVectorf *Dinv;  ///< primal variable rescaling
  OSQPVectorf *Einv;  ///< dual variable rescaling
} OSQPScaling;




# ifndef EMBEDDED

/**
 * Polish structure
 */
typedef struct {
  csc *Ared;          ///< active rows of A
  ///<    Ared = vstack[Alow, Aupp]
  c_int    n_low;     ///< number of lower-active rows
  c_int    n_upp;     ///< number of upper-active rows
  OSQPVectori   *A_to_Alow; ///< Maps indices in A to indices in Alow
  OSQPVectori   *A_to_Aupp; ///< Maps indices in A to indices in Aupp
  OSQPVectori   *Alow_to_A; ///< Maps indices in Alow to indices in A
  OSQPVectori   *Aupp_to_A; ///< Maps indices in Aupp to indices in A
  OSQPVectorf *x;         ///< optimal x-solution obtained by polish
  OSQPVectorf *z;         ///< optimal z-solution obtained by polish
  OSQPVectorf *y;         ///< optimal y-solution obtained by polish
  c_float  obj_val;   ///< objective value at polished solution
  c_float  pri_res;   ///< primal residual at polished solution
  c_float  dua_res;   ///< dual residual at polished solution
} OSQPPolish;
# endif // ifndef EMBEDDED


/**********************************
* Main structures and Data Types *
**********************************/

/**
 * QP problem data (possibly internally scaled)
 */
typedef struct {
  c_int    n; ///< number of variables n
  c_int    m; ///< number of constraints m
  csc     *P; ///< the upper triangular part of the quadratic cost matrix P in csc format (size n x n).
  csc     *A; ///< linear constraints matrix A in csc format (size m x n)
  OSQPVectorf *q; ///< dense array for linear part of cost function (size n)
  OSQPVectorf *l; ///< dense array for lower bound (size m)
  OSQPVectorf *u; ///< dense array for upper bound (size m)
} OSQPData;


/**
 * OSQP Workspace
 */
typedef struct OSQPWorkspace_ {
  /// Problem data to work on (possibly scaled)
  OSQPData *data;

  /// Linear System solver structure
  LinSysSolver *linsys_solver;

# ifndef EMBEDDED
  /// Polish structure
  OSQPPolish *pol;
# endif // ifndef EMBEDDED

  /**
   * @name Vector used to store a vectorized rho parameter
   * @{
   */
  OSQPVectorf *rho_vec;     ///< vector of rho values
  OSQPVectorf *rho_inv_vec; ///< vector of inv rho values

  /** @} */

# if EMBEDDED != 1
  OSQPVectori *constr_type; ///< Type of constraints: loose (-1), equality (1), inequality (0)
# endif // if EMBEDDED != 1

  /**
   * @name Iterates
   * @{
   */
  OSQPVectorf *x;           ///< Iterate x
  OSQPVectorf *y;           ///< Iterate y
  OSQPVectorf *z;           ///< Iterate z
  OSQPVectorf *xz_tilde;    ///< Iterate xz_tilde
  OSQPVectorf *xtilde_view; ///< xtilde view into xz_tilde
  OSQPVectorf *ztilde_view; ///< ztilde view into xz_tilde

  OSQPVectorf *x_prev;   ///< Previous x

  /**< NB: Used also as workspace vector for dual residual */
  OSQPVectorf *z_prev;   ///< Previous z

  /**< NB: Used also as workspace vector for primal residual */

  /**
   * @name Primal and dual residuals workspace variables
   *
   * Needed for residuals computation, tolerances computation,
   * approximate tolerances computation and adapting rho
   * @{
   */
  OSQPVectorf *Ax;  ///< scaled A * x
  OSQPVectorf *Px;  ///< scaled P * x
  OSQPVectorf *Aty; ///< scaled A * x

  /** @} */

  /**
   * @name Primal infeasibility variables
   * @{
   */
  OSQPVectorf *delta_y;   ///< difference between consecutive dual iterates
  OSQPVectorf *Atdelta_y; ///< A' * delta_y

  /** @} */

  /**
   * @name Dual infeasibility variables
   * @{
   */
  OSQPVectorf *delta_x;  ///< difference between consecutive primal iterates
  OSQPVectorf *Pdelta_x; ///< P * delta_x
  OSQPVectorf *Adelta_x; ///< A * delta_x

  /** @} */

  /**
   * @name Temporary vectors used in scaling
   * @{
   */

  OSQPVectorf *D_temp;   ///< temporary primal variable scaling vectors
  OSQPVectorf *D_temp_A; ///< temporary primal variable scaling vectors storing norms of A columns
  OSQPVectorf *E_temp;   ///< temporary constraints scaling vectors storing norms of A' columns

  /** @} */
  OSQPScaling  *scaling;  ///< scaling vectors

# ifdef PROFILING
  OSQPTimer *timer;       ///< timer object

  /// flag indicating whether the solve function has been run before
  c_int first_run;

  /// flag indicating whether the update_time should be cleared
  c_int clear_update_time;

  /// flag indicating that osqp_update_rho is called from osqp_solve function
  c_int rho_update_from_solve;
# endif // ifdef PROFILING

# ifdef PRINTING
  c_int summary_printed; ///< Has last summary been printed? (true/false)
# endif // ifdef PRINTING

} OSQPWorkspace;


/**
 * Define linsys_solver prototype structure
 *
 * NB: The details are defined when the linear solver is initialized depending
 *      on the choice
 */
struct linsys_solver {
  enum linsys_solver_type type;                 ///< linear system solver type functions
  c_int (*solve)(LinSysSolver *self,
                 c_float      *b);              ///< solve linear system

# ifndef EMBEDDED
  void (*free)(LinSysSolver *self);             ///< free linear system solver (only in desktop version)
# endif // ifndef EMBEDDED

# if EMBEDDED != 1
  c_int (*update_matrices)(LinSysSolver *s,
                           const csc *P,            ///< update matrices P
                           const csc *A);           //   and A in the solver

  c_int (*update_rho_vec)(LinSysSolver  *s,
                          const c_float *rho_vec);  ///< Update rho_vec
# endif // if EMBEDDED != 1

# ifndef EMBEDDED
  c_int nthreads; ///< number of threads active
# endif // ifndef EMBEDDED
};


# ifdef __cplusplus
}
# endif // ifdef __cplusplus

#endif // ifndef OSQP_TYPES_H
