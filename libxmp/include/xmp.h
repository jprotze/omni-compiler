/*
 * $TSUKUBA_Release: $
 * $TSUKUBA_Copyright:
 *  $
 */

#ifndef _XMP_USERAPI
#define _XMP_USERAPI

#include "stddef.h"

typedef void *xmp_desc_t;

// ----- libxmp
extern void	xmp_MPI_comm(void **comm);
extern int	xmp_num_nodes(void);
extern int	xmp_node_num(void);
extern void	xmp_barrier(void);
extern int	xmp_all_num_nodes(void);
extern int	xmp_all_node_num(void);
extern double	xmp_wtime(void);
extern double	xmp_wtick(void);
extern int      xmp_array_ndim(xmp_desc_t d);
extern size_t   xmp_array_type_size(xmp_desc_t d);
extern int      xmp_array_gsize(xmp_desc_t d, int dim);
extern int      xmp_array_lsize(xmp_desc_t d, int dim);
extern int      xmp_array_gcllbound(xmp_desc_t d, int dim);
extern int      xmp_array_gclubound(xmp_desc_t d, int dim);
extern int      xmp_array_lcllbound(xmp_desc_t d, int dim);
extern int      xmp_array_lclubound(xmp_desc_t d, int dim);
extern int      xmp_array_gcglbound(xmp_desc_t d, int dim);
extern int      xmp_array_gcgubound(xmp_desc_t d, int dim);
extern void    *xmp_array_laddr(xmp_desc_t d);
extern int      xmp_array_ushadow(xmp_desc_t d, int dim);
extern int      xmp_array_lshadow(xmp_desc_t d, int dim);
extern int      xmp_array_owner(xmp_desc_t d, int ndims, int index[], int dim);
extern int      xmp_array_lead_dim(xmp_desc_t d);
extern int      xmp_align_axis(xmp_desc_t d, int dim);
extern int      xmp_align_offset(xmp_desc_t d, int dim);
extern int      xmp_align_format(xmp_desc_t d, int dim);
extern int      xmp_align_size(xmp_desc_t d, int dim);
extern xmp_desc_t xmp_align_template(xmp_desc_t d);
extern _Bool    xmp_template_fixed(xmp_desc_t d);
extern int      xmp_template_ndim(xmp_desc_t d);
extern int      xmp_template_gsize(xmp_desc_t d, int dim);
extern int      xmp_template_lsize(xmp_desc_t d, int dim);
extern int      xmp_dist_format(xmp_desc_t d, int dim);
extern int      xmp_dist_size(xmp_desc_t d, int dim);
extern int      xmp_dist_stride(xmp_desc_t d, int dim);
extern xmp_desc_t xmp_dist_nodes(xmp_desc_t d);
extern int      xmp_nodes_ndim(xmp_desc_t d);
extern int      xmp_nodes_index(xmp_desc_t d, int dim);
extern int      xmp_nodes_size(xmp_desc_t d, int dim);
extern void     xmp_sched_template_index(int* local_start_index, int* local_end_index,
					 const int global_start_index, const int global_end_index, const int step,
					 const xmp_desc_t template, const int template_dim);
extern void    *xmp_malloc(xmp_desc_t d);

// ----- libxmp_gpu
#ifdef _XMP_ENABLE_GPU
extern int	xmp_get_gpu_count(void);
#endif

#endif // _XMP_USERAPI
