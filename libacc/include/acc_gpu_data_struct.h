#ifndef _ACC_GPU_DATA_STRUCT
#define _ACC_GPU_DATA_STRUCT

#include <stdlib.h>
#include <stdbool.h>

typedef struct _ACC_gpu_data_type {
  //  _Bool is_aligned_array;
  void *host_addr; //pointer of var or first element of array on host
  void *device_addr; //pointer of device memory. 
  //  _XMP_array_t *host_array_desc;
  //  _XMP_gpu_array_t *device_array_desc;
  size_t offset;
  size_t size;
  _Bool is_original;
  //  _Bool is_heap;
  _Bool is_pagelocked;  // true if it is already pagelocked
  _Bool is_registered;  // true if it is pagelocked by cudaHostRegister in OpenACC Runtime
} _ACC_gpu_data_t;

#endif //_ACC_GPU_DATA_STRUCT