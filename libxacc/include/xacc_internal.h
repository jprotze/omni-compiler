//#include<openacc.h>
/* typedef  int acc_device_t; */
/* //_XACC_device_t *current_device; */

/* typedef struct _XACC_device_type { */
/*   acc_device_t acc_device; */
/*   int lb; */
/*   int ub; */
/*   int step; */
/*   int size; */
/* } _XACC_device_t; */

/* void _XACC_init_device(void* desc, acc_device_t device, int lower, int upper, int step); */
/* int _XACC_get_num_current_devices(); */
/* acc_device_t _XACC_get_current_device(); */
/* void _XACC_get_current_device_info(int* lower, int* upper, int* step); */
/* void _XACC_get_device_info(void *desc, int* lower, int* upper, int* step); */

/* void _XMP_init_device(void* desc, acc_device_t device, int lower, int upper, int step); */
/* void _XMP_get_device_info(void *desc, int* lower, int* upper, int* step); */

#include "xacc_data_struct.h"

void _XACC_init_device(_XACC_device_t** desc, acc_device_t device, int lower, int upper, int step);
int _XACC_get_num_current_devices();
acc_device_t _XACC_get_current_device();
void _XACC_get_current_device_info(int* lower, int* upper, int* step);
void _XACC_get_device_info(void *desc, int* lower, int* upper, int* step);

//void _XMP_init_device(void* desc, acc_device_t device, int lower, int upper, int step);
//void _XMP_get_device_info(void *desc, int* lower, int* upper, int* step);

void _XACC_init_layouted_array(_XACC_arrays_t **array, _XMP_array_t *alignedArray, _XACC_device_t* device);
void _XACC_split_layouted_array_BLOCK(_XACC_arrays_t* array, int dim);
void _XACC_split_layouted_array_DUPLICATION(_XACC_arrays_t* array, int dim);
void _XACC_calc_size(_XACC_arrays_t* array);

void _XACC_get_size(_XACC_arrays_t* array, unsigned long long* offset,
               unsigned long long* size, int deviceNum);
void _XACC_sched_loop_layout_BLOCK(int init,
                                   int cond,
                                   int step,
                                   int* sched_init,
                                   int* sched_cond,
                                   int* sched_step,
                                   _XACC_arrays_t* array_desc,
                                   int dim,
                                   int deviceNum);
void _XACC_set_shadow_NORMAL(_XACC_arrays_t* array_desc, int dim , int lo, int hi);