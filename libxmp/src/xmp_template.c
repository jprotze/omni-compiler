#include <stdarg.h>
#include "xmp_constant.h"
#include "xmp_internal.h"
#include "xmp_math_function.h"

static void _XCALABLEMP_calc_template_size(_XCALABLEMP_template_t *t);
static void _XCALABLEMP_validate_template_ref(long long *lower, long long *upper, int *stride,
                                              long long lb, long long ub);
static _Bool _XCALABLEMP_check_template_ref_inclusion(long long ref_lower, long long ref_upper, int ref_stride,
                                                      _XCALABLEMP_template_chunk_t *chunk);

static void _XCALABLEMP_calc_template_size(_XCALABLEMP_template_t *t) {
  assert(t != NULL);

  int dim;
  if (t->is_fixed) {
    dim = t->dim;
  }
  else {
    dim = t->dim - 1;
  }

  for (int i = 0; i < dim; i++) {
    int ser_lower = t->info[i].ser_lower;
    int ser_upper = t->info[i].ser_upper;

    if (ser_lower > ser_upper) {
      _XCALABLEMP_fatal("the lower bound of template should be less than or equal to the upper bound");
    }

    t->info[i].ser_size = _XCALABLEMP_M_COUNTi(ser_lower, ser_upper);
  }
}

static void _XCALABLEMP_validate_template_ref(long long *lower, long long *upper, int *stride,
                                              long long lb, long long ub) {
  assert(lower != NULL);
  assert(upper != NULL);
  assert(stride != NULL);

  // setup temporary variables
  long long l, u;
  int s = *stride;
  if (s > 0) {
    l = *lower;
    u = *upper;
  }
  else if (s < 0) {
    l = *upper;
    u = *lower;
  }
  else {
    _XCALABLEMP_fatal("the stride of <template-ref> is 0");
    l = 0; u = 0; // XXX dummy
  }

  // check boundary
  if (lb > l) {
    _XCALABLEMP_fatal("<template-ref> is out of bounds, <ref-lower> is less than the template lower bound");
  }

  if (l > u) {
    _XCALABLEMP_fatal("<template-ref> is out of bounds, <ref-upper> is less than <ref-lower>");
  }

  if (u > ub) {
    _XCALABLEMP_fatal("<template-ref> is out of bounds, <ref-upper> is greater than the template upper bound");
  }

  // validate values
  if (s > 0) {
    u = u - ((u - l) % s);
    *upper = u;
  }
  else {
    s = -s;
    l = l + ((u - l) % s);
    *lower = l;
    *upper = u;
    *stride = s;
  }
}

static _Bool _XCALABLEMP_check_template_ref_inclusion(long long ref_lower, long long ref_upper, int ref_stride,
                                                      _XCALABLEMP_template_chunk_t *chunk) {
  assert(chunk != NULL);

  switch (chunk->dist_manner) {
    case _XCALABLEMP_N_DIST_DUPLICATION:
      return true;
    case _XCALABLEMP_N_DIST_BLOCK:
      {
        long long template_lower = chunk->par_lower;
        long long template_upper = chunk->par_upper;

        if (ref_stride != 1) {
          int ref_stride_mod = _XCALABLEMP_modi_ll_i(ref_lower, ref_stride);
          /* normalize template lower */
          int template_lower_mod = _XCALABLEMP_modi_ll_i(template_lower, ref_stride);
          if (template_lower_mod != ref_stride_mod) {
            if (template_lower_mod < ref_stride_mod) {
              template_lower += (ref_stride_mod - template_lower_mod);
            }
            else {
              template_lower += (ref_stride - template_lower_mod + ref_stride_mod);
            }
          }

          if (template_lower > template_upper) {
            return false;
          }

          /* normalize template upper */
          int template_upper_mod = _XCALABLEMP_modi_ll_i(template_upper, ref_stride);
          if (template_upper_mod != ref_stride_mod) {
            if (ref_stride_mod < template_upper_mod) {
              template_upper -= (template_upper_mod - ref_stride_mod);
            }
            else {
              template_upper -= (ref_stride - ref_stride_mod + template_upper_mod);
            }
          }
        }

        if (ref_upper < template_lower) {
          return false;
        }

        if (template_upper < ref_lower) {
          return false;
        }

        return true;
      }
    case _XCALABLEMP_N_DIST_CYCLIC:
      {
        if (ref_stride == 1) {
          int nodes_size = (chunk->onto_nodes_info)->size;
          int par_lower_mod = _XCALABLEMP_modi_ll_i(chunk->par_lower, nodes_size);
          int ref_lower_mod = _XCALABLEMP_modi_ll_i(ref_lower, nodes_size);
          if (par_lower_mod != ref_lower_mod) {
            if (ref_lower_mod < par_lower_mod) ref_lower += (par_lower_mod - ref_lower_mod);
            else ref_lower += (nodes_size - ref_lower_mod + par_lower_mod);
          }

          if (ref_upper < ref_lower) {
            return false;
          }
          else {
            return true;
          }
        }
        else {
          _XCALABLEMP_fatal("not implemented condition: (stride is not 1 && cyclic distribution)");
          return false; // XXX dummy
        }
      }
    default:
      _XCALABLEMP_fatal("unknown distribution manner");
      return false; // XXX dummy
  }
}

void _XCALABLEMP_init_template_FIXED(_XCALABLEMP_template_t **template, int dim, ...) {
  // alloc descriptor
  _XCALABLEMP_template_t *t = _XCALABLEMP_alloc(sizeof(_XCALABLEMP_template_t) +
                                                sizeof(_XCALABLEMP_template_info_t) * (dim - 1));

  // calc members
  t->is_owner = false;
  t->is_fixed = true;
  t->dim = dim;

  t->onto_nodes = NULL;
  t->chunk = NULL;

  va_list args;
  va_start(args, dim);
  for (int i = 0; i < dim; i++) {
    t->info[i].ser_lower = va_arg(args, long long);
    t->info[i].ser_upper = va_arg(args, long long);
  }
  va_end(args);

  _XCALABLEMP_calc_template_size(t);

  *template = t;
}

void _XCALABLEMP_init_template_UNFIXED(_XCALABLEMP_template_t **template, int dim, ...) {
  // alloc descriptor
  _XCALABLEMP_template_t *t = _XCALABLEMP_alloc(sizeof(_XCALABLEMP_template_t) +
                                                sizeof(_XCALABLEMP_template_info_t) * (dim - 1));

  // calc members
  t->is_owner = false;
  t->is_fixed = false;
  t->dim = dim;

  t->onto_nodes = NULL;
  t->chunk = NULL;

  va_list args;
  va_start(args, dim);
  for(int i = 0; i < dim - 1; i++) {
    t->info[i].ser_lower = va_arg(args, long long);
    t->info[i].ser_upper = va_arg(args, long long);
  }
  va_end(args);

  _XCALABLEMP_calc_template_size(t);

  *template = t;
}

void _XCALABLEMP_init_template_chunk(_XCALABLEMP_template_t *template, _XCALABLEMP_nodes_t *nodes) {
  assert(template != NULL);
  assert(nodes != NULL);

  template->is_owner = nodes->is_member;

  template->onto_nodes = nodes;
  template->chunk = _XCALABLEMP_alloc(sizeof(_XCALABLEMP_template_chunk_t) * (template->dim));
}

void _XCALABLEMP_finalize_template(_XCALABLEMP_template_t *template) {
  assert(template != NULL);

  _XCALABLEMP_free(template->chunk);
  _XCALABLEMP_free(template);
}

void _XCALABLEMP_dist_template_DUPLICATION(_XCALABLEMP_template_t *template, int template_index) {
  assert(template != NULL);

  _XCALABLEMP_template_chunk_t *chunk = &(template->chunk[template_index]);
  _XCALABLEMP_template_info_t *ti = &(template->info[template_index]);

  chunk->onto_nodes_index = _XCALABLEMP_N_NO_ONTO_NODES;
  chunk->onto_nodes_info = NULL;
  chunk->par_lower = ti->ser_lower;
  chunk->par_upper = ti->ser_upper;

  chunk->par_stride = 1;
  chunk->par_chunk_width = ti->ser_size;
  chunk->dist_manner = _XCALABLEMP_N_DIST_DUPLICATION;
  chunk->is_regular_chunk = true;
}

void _XCALABLEMP_dist_template_BLOCK(_XCALABLEMP_template_t *template, int template_index, int nodes_index) {
  assert(template != NULL);

  _XCALABLEMP_nodes_t *nodes = template->onto_nodes;

  _XCALABLEMP_template_chunk_t *chunk = &(template->chunk[template_index]);
  _XCALABLEMP_template_info_t *ti = &(template->info[template_index]);
  _XCALABLEMP_nodes_info_t *ni = &(nodes->info[nodes_index]);

  long long nodes_size = (long long)ni->size;

  // check template size
  if (ti->ser_size < nodes_size) {
    _XCALABLEMP_fatal("template is too small to distribute");
  }

  // calc parallel members
  unsigned long long chunk_width = _XCALABLEMP_M_FLOORi(ti->ser_size, nodes_size);

  chunk->onto_nodes_index = nodes_index;
  chunk->onto_nodes_info = ni;
  if (nodes->is_member) {
    long long nodes_rank = (long long)ni->rank;

    chunk->par_lower = nodes_rank * chunk_width + ti->ser_lower;
    if (nodes_rank == (nodes_size - 1)) {
      chunk->par_upper = ti->ser_upper;
    }
    else {
      chunk->par_upper = chunk->par_lower + chunk_width - 1;
    }
  }

  chunk->par_stride = 1;
  chunk->par_chunk_width = chunk_width;
  chunk->dist_manner = _XCALABLEMP_N_DIST_BLOCK;
  if ((ti->ser_size % nodes_size) == 0) {
    chunk->is_regular_chunk = true;
  }
  else {
    chunk->is_regular_chunk = false;
  }
}

void _XCALABLEMP_dist_template_CYCLIC(_XCALABLEMP_template_t *template, int template_index, int nodes_index) {
  assert(template != NULL);

  _XCALABLEMP_nodes_t *nodes = template->onto_nodes;

  _XCALABLEMP_template_chunk_t *chunk = &(template->chunk[template_index]);
  _XCALABLEMP_template_info_t *ti = &(template->info[template_index]);
  _XCALABLEMP_nodes_info_t *ni = &(nodes->info[nodes_index]);

  long long nodes_size = (long long)ni->size;

  // check template size
  if (ti->ser_size < nodes_size) {
    _XCALABLEMP_fatal("template is too small to distribute");
  }

  chunk->onto_nodes_index = nodes_index;
  chunk->onto_nodes_info = ni;
  if (nodes->is_member) {
    long long nodes_rank = (long long)ni->rank;
    unsigned long long div = ti->ser_size / nodes_size;
    unsigned long long mod = ti->ser_size % nodes_size;
    unsigned long long par_size = 0;
    if(mod == 0) {
      par_size = div;
    }
    else {
      if(nodes_rank >= mod) {
        par_size = div;
      }
      else {
        par_size = div + 1;
      }
    }

    chunk->par_lower = ti->ser_lower + nodes_rank;
    chunk->par_upper = chunk->par_lower + nodes_size * (par_size - 1);
  }

  chunk->par_stride = nodes_size;
  chunk->par_chunk_width = _XCALABLEMP_M_CEILi(ti->ser_size, nodes_size);
  chunk->dist_manner = _XCALABLEMP_N_DIST_CYCLIC;
  if ((ti->ser_size % nodes_size) == 0) {
    chunk->is_regular_chunk = true;
  }
  else {
    chunk->is_regular_chunk = false;
  }
}

_Bool _XCALABLEMP_exec_task_TEMPLATE_PART(int get_upper, _XCALABLEMP_template_t *ref_template, ...) {
  assert(ref_template != NULL);

  if (!(ref_template->is_owner)) {
    return false;
  }

  _XCALABLEMP_nodes_t *onto_nodes = ref_template->onto_nodes;

  int color = 1;
  _Bool is_member = true;
  int acc_nodes_size = 1;
  int ref_dim = ref_template->dim;
  long long ref_lower, ref_upper;
  int ref_stride;

  va_list args;
  va_start(args, ref_template);
  for (int i = 0; i < ref_dim; i++) {
    _XCALABLEMP_template_info_t *info = &(ref_template->info[i]);
    _XCALABLEMP_template_chunk_t *chunk = &(ref_template->chunk[i]);

    int size, rank;
    if (chunk->dist_manner == _XCALABLEMP_N_DIST_DUPLICATION) {
      size = 1;
      rank = 0;
    }
    else {
      _XCALABLEMP_nodes_info_t *onto_nodes_info = chunk->onto_nodes_info;
      size = onto_nodes_info->size;
      rank = onto_nodes_info->rank;
    }

    if (va_arg(args, int) == 1) {
      color += (acc_nodes_size * rank);
    }
    else {
      ref_lower = va_arg(args, long long);
      if ((i == (ref_dim - 1)) && (get_upper == 1)) {
        ref_upper = info->ser_upper;
      }
      else {
        ref_upper = va_arg(args, long long);
      }
      ref_stride = va_arg(args, int);

      _XCALABLEMP_validate_template_ref(&ref_lower, &ref_upper, &ref_stride, info->ser_lower, info->ser_upper);

      is_member = is_member && _XCALABLEMP_check_template_ref_inclusion(ref_lower, ref_upper, ref_stride, chunk);
    }

    acc_nodes_size *= size;
  }

  MPI_Comm *comm = _XCALABLEMP_alloc(sizeof(MPI_Comm));
  if (!is_member) {
    color = 0;
  }

  MPI_Comm_split(*(onto_nodes->comm), color, onto_nodes->comm_rank, comm);

  if (is_member) {
    _XCALABLEMP_push_comm(comm);
    return true;
  }
  else {
    _XCALABLEMP_finalize_comm(comm);
    return false;
  }
}
