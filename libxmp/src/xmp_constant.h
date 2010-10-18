#ifndef _XCALABLEMP_CONSTANT
#define _XCALABLEMP_CONSTANT

#define _XCALABLEMP_N_INT_TRUE				1
#define _XCALABLEMP_N_INT_FALSE				0
#define _XCALABLEMP_N_MAX_DIM				7

// constants for mpi tag
#define _XCALABLEMP_N_MPI_TAG_GMOVE			10
#define _XCALABLEMP_N_MPI_TAG_REFLECT_LO		11
#define _XCALABLEMP_N_MPI_TAG_REFLECT_HI		12

// constants used in runtime functions
#define _XCALABLEMP_N_INVALID_RANK			-1
#define _XCALABLEMP_N_NO_ALIGNED_TEMPLATE		-1
#define _XCALABLEMP_N_NO_ONTO_NODES			-1

// defined in exc.xcalablemp.XMPtemplate
#define _XCALABLEMP_N_DIST_DUPLICATION			100
#define _XCALABLEMP_N_DIST_BLOCK			101
#define _XCALABLEMP_N_DIST_CYCLIC			102

// defined in exc.object.BasicType
#define _XCALABLEMP_N_TYPE_NONE				200
#define _XCALABLEMP_N_TYPE_GENERAL			201
#define _XCALABLEMP_N_TYPE_BOOL				202
#define _XCALABLEMP_N_TYPE_CHAR				203
#define _XCALABLEMP_N_TYPE_UNSIGNED_CHAR		204
#define _XCALABLEMP_N_TYPE_SHORT			205
#define _XCALABLEMP_N_TYPE_UNSIGNED_SHORT		206
#define _XCALABLEMP_N_TYPE_INT				207
#define _XCALABLEMP_N_TYPE_UNSIGNED_INT			208
#define _XCALABLEMP_N_TYPE_LONG				209
#define _XCALABLEMP_N_TYPE_UNSIGNED_LONG		210
#define _XCALABLEMP_N_TYPE_LONGLONG			211
#define _XCALABLEMP_N_TYPE_UNSIGNED_LONGLONG		212
#define _XCALABLEMP_N_TYPE_FLOAT			213
#define _XCALABLEMP_N_TYPE_DOUBLE			214
#define _XCALABLEMP_N_TYPE_LONG_DOUBLE			215
#define _XCALABLEMP_N_TYPE_FLOAT_IMAGINARY		216
#define _XCALABLEMP_N_TYPE_DOUBLE_IMAGINARY		217
#define _XCALABLEMP_N_TYPE_LONG_DOUBLE_IMAGINARY	218
#define _XCALABLEMP_N_TYPE_FLOAT_COMPLEX		219
#define _XCALABLEMP_N_TYPE_DOUBLE_COMPLEX		220
#define _XCALABLEMP_N_TYPE_LONG_DOUBLE_COMPLEX		221

// defined in exc.xcalablemp.XMPcollective
#define _XCALABLEMP_N_REDUCE_SUM			300
#define _XCALABLEMP_N_REDUCE_PROD			301
#define _XCALABLEMP_N_REDUCE_BAND			302
#define _XCALABLEMP_N_REDUCE_LAND			303
#define _XCALABLEMP_N_REDUCE_BOR			304
#define _XCALABLEMP_N_REDUCE_LOR			305
#define _XCALABLEMP_N_REDUCE_BXOR			306
#define _XCALABLEMP_N_REDUCE_LXOR			307
#define _XCALABLEMP_N_REDUCE_MAX			308
#define _XCALABLEMP_N_REDUCE_MIN			309
#define _XCALABLEMP_N_REDUCE_FIRSTMAX			310
#define _XCALABLEMP_N_REDUCE_FIRSTMIN			311
#define _XCALABLEMP_N_REDUCE_LASTMAX			312
#define _XCALABLEMP_N_REDUCE_LASTMIN			313

// defined in exc.xcalablemp.XMPshadow
#define _XCALABLEMP_N_SHADOW_NONE			400
#define _XCALABLEMP_N_SHADOW_NORMAL			401
#define _XCALABLEMP_N_SHADOW_FULL			402

#endif // _XCALABLEMP_CONSTANT
