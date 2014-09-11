#ifndef _XACC_DATA_STRUCT
#define _XACC_DATA_STRUCT

//XACC
typedef  int acc_device_t;
typedef struct _XACC_device_type {
  acc_device_t acc_device;
  int lb;
  int ub;
  int step;
  int size;

} _XACC_device_t;

typedef struct _XACC_array_info_type {
  int device_layout_manner;

  int par_lower;
  int par_upper;
  int par_stride;
  int par_size;
  int local_lower;
  int local_upper;
  int local_stride;
  //  int local_size;
  int alloc_size;

  //  int shadow_type;
  int shadow_size_lo;
  int shadow_size_hi;

  unsigned long long device_dim_acc;
}_XACC_array_info_t;

typedef struct _XACC_array_type {
  _XACC_array_info_t *info;
  unsigned long long alloc_offset;
  unsigned long long alloc_size;  
}_XACC_array_t;

typedef struct _XACC_arrays_type{
  _XACC_device_t *device_type;
  _XACC_array_t *device_array;
  int dim;
}_XACC_arrays_t;  


#endif //_XACC_DATA_STRUCT