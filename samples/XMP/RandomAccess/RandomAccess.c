/*
 * The original code has been contributed by the DARPA HPCS program.
 * And the code has been modified by Hitoshi Murai at RIKEN AICS 
 *
 *
 * GUPS (Giga UPdates per Second) is a measurement that profiles the memory
 * architecture of a system and is a measure of performance similar to MFLOPS.
 * The HPCS HPCchallenge RandomAccess benchmark is intended to exercise the
 * GUPS capability of a system, much like the LINPACK benchmark is intended to
 * exercise the MFLOPS capability of a computer.  In each case, we would
 * expect these benchmarks to achieve close to the "peak" capability of the
 * memory system. The extent of the similarities between RandomAccess and
 * LINPACK are limited to both benchmarks attempting to calculate a peak system
 * capability.
 *
 * GUPS is calculated by identifying the number of memory locations that can be
 * randomly updated in one second, divided by 1 billion (1e9). The term "randomly"
 * means that there is little relationship between one address to be updated and
 * the next, except that they occur in the space of one half the total system
 * memory.  An update is a read-modify-write operation on a table of 64-bit words.
 * An address is generated, the value at that address read from memory, modified
 * by an integer operation (add, and, or, xor) with a literal value, and that
 * new value is written back to memory.
 *
 * We are interested in knowing the GUPS performance of both entire systems and
 * system subcomponents --- e.g., the GUPS rating of a distributed memory
 * multiprocessor the GUPS rating of an SMP node, and the GUPS rating of a
 * single processor.  While there is typically a scaling of FLOPS with processor
* count, a similar phenomenon may not always occur for GUPS.
 *
 * Select the memory size to be the power of two such that 2^n <= 1/2 of the
 * total memory.  Each CPU operates on its own address stream, and the single
 * table may be distributed among nodes. The distribution of memory to nodes
 * is left to the implementer.  A uniform data distribution may help balance
 * the workload, while non-uniform data distributions may simplify the
 * calculations that identify processor location by eliminating the requirement
 * for integer divides. A small (less than 1%) percentage of missed updates
 * are permitted.
 *
 * When implementing a benchmark that measures GUPS on a distributed memory
 * multiprocessor system, it may be required to define constraints as to how
 * far in the random address stream each node is permitted to "look ahead".
 * Likewise, it may be required to define a constraint as to the number of
 * update messages that can be stored before processing to permit multi-level
 * parallelism for those systems that support such a paradigm.  The limits on
 * "look ahead" and "stored updates" are being implemented to assure that the
 * benchmark meets the intent to profile memory architecture and not induce
 * significant artificial data locality. For the purpose of measuring GUPS,
 * we will stipulate that each process is permitted to look ahead no more than
 * 1024 random address stream samples with the same number of update messages
 * stored before processing.
 *
 * The supplied MPI-1 code generates the input stream {A} on all processors
 * and the global table has been distributed as uniformly as possible to
 * balance the workload and minimize any Amdahl fraction.  This code does not
 * exploit "look-ahead".  Addresses are sent to the appropriate processor
 * where the table entry resides as soon as each address is calculated.
 * Updates are performed as addresses are received.  Each message is limited
 * to a single 64 bit long integer containing element ai from {A}.
* Local offsets for T[ ] are extracted by the destination processor.
 *
 * If the number of processors is equal to a power of two, then the global
 * table can be distributed equally over the processors.  In addition, the
 * processor number can be determined from that portion of the input stream
 * that identifies the address into the global table by masking off log2(p)
 * bits in the address.
 *
 * If the number of processors is not equal to a power of two, then the global
 * table cannot be equally distributed between processors.  In the MPI-1
 * implementation provided, there has been an attempt to minimize the differences
 * in workloads and the largest difference in elements of T[ ] is one.  The
 * number of values in the input stream generated by each processor will be
 * related to the number of global table entries on each processor.
 *
 * The MPI-1 version of RandomAccess treats the potential instance where the
 * number of processors is a power of two as a special case, because of the
 * significant simplifications possible because processor location and local
 * offset can be determined by applying masks to the input stream values.
 * The non power of two case uses an integer division to determine the processor
 * location.  The integer division will be more costly in terms of machine
 * cycles to perform than the bit masking operations
 *
 * For additional information on the GUPS metric, the HPCchallenge RandomAccess
 * Benchmark,and the rules to run RandomAccess or modify it to optimize
 * performance -- see http://icl.cs.utk.edu/hpcc/
 *
*/
/* Jan 2005
 *
 * This code has been modified to allow local bucket sorting of updates.
 * The total maximum number of updates in the local buckets of a process
 * is currently defined in "RandomAccess.h" as MAX_TOTAL_PENDING_UPDATES.
 * When the total maximum number of updates is reached, the process selects
 * the bucket (or destination process) with the largest number of
 * updates and sends out all the updates in that bucket. See buckets.c
 * for details about the buckets' implementation.
 *
 * This code also supports posting multiple MPI receive descriptors (based
 * on a contribution by David Addison).
 *
 * In addition, this implementation provides an option for limiting
 * the execution time of the benchmark to a specified time bound
 * (see time_bound.c). The time bound is currently defined in
 * time_bound.h, but it should be a benchmark parameter. By default
 * the benchmark will execute the recommended number of updates,
 * that is, four times the global table size.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xmp.h"
#include <mpi.h>

typedef unsigned long long	u64Int;
typedef signed long long	s64Int;

#define POLY	          0x0000000000000007ULL
#define PERIOD	          1317624576693539401LL
#define ZERO64B           0LL
#define HPCC_TRUE         1
#define HPCC_FALSE        0
#define HPCC_DONE         0

// Parameters
#define PROCS		  (8)
#define MAXLOGPROCS       18
#define XMP_TABLE_SIZE	  (1024L * PROCS)
#define LOCAL_SIZE	  (XMP_TABLE_SIZE/PROCS)
#define NUPDATE           (4 * XMP_TABLE_SIZE)
#define CHUNK             1024
#define CHUNKBIG          (32*CHUNK)
#define RCHUNK            (16384)
#define PITER             8

u64Int Table[LOCAL_SIZE];

// Define nodes and coarrays
#pragma xmp nodes p(*)
u64Int recv[PITER][MAXLOGPROCS][RCHUNK+1], send[2][CHUNKBIG+1];
#pragma xmp coarray recv, send : [*]
int status;

u64Int HPCC_starts(s64Int n){
  int i, j;
  u64Int m2[64], temp, ran;

  while (n < 0) n += PERIOD;
  while (n > PERIOD) n -= PERIOD;
  if (n == 0) return 0x1;

  temp = 0x1;
  for (i = 0; i < 64; i++){
    m2[i] = temp;
    temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
    temp = (temp << 1) ^ ((s64Int) temp < 0 ? POLY : 0);
  }

  for (i = 62; i >= 0; i--)
    if ((n >> i) & 1) break;

  ran = 0x2;
  while (i > 0){
    temp = 0;
    for (j = 0; j < 64; j++)
      if ((ran >> j) & 1) temp ^= m2[j];
    ran = temp;
    i -= 1;
    if ((n >> i) & 1)
      ran = (ran << 1) ^ ((s64Int) ran < 0 ? POLY : 0);
  }

  return ran;
}

static void sort_data(u64Int *source, u64Int *nomatch, u64Int *match, int number,
		      int *nnomatch, int *nmatch, int mask_shift){
  int i, dindex, myselect[8], counts[2];
  int div_num = number / 8;
  int loop_total = div_num * 8;
  u64Int procmask = ((u64Int) 1) << mask_shift;
  u64Int *buffers[2];

  buffers[0] = nomatch;
  counts[0] = *nnomatch;
  buffers[1] = match;
  counts[1] = *nmatch;

  for (i = 0; i < div_num; i++) {
    dindex = i*8;
    myselect[0] = (source[dindex] & procmask) >> mask_shift;
    myselect[1] = (source[dindex+1] & procmask) >> mask_shift;
    myselect[2] = (source[dindex+2] & procmask) >> mask_shift;
    myselect[3] = (source[dindex+3] & procmask) >> mask_shift;
    myselect[4] = (source[dindex+4] & procmask) >> mask_shift;
    myselect[5] = (source[dindex+5] & procmask) >> mask_shift;
    myselect[6] = (source[dindex+6] & procmask) >> mask_shift;
    myselect[7] = (source[dindex+7] & procmask) >> mask_shift;
    buffers[myselect[0]][counts[myselect[0]]++] = source[dindex];
    buffers[myselect[1]][counts[myselect[1]]++] = source[dindex+1];
    buffers[myselect[2]][counts[myselect[2]]++] = source[dindex+2];
    buffers[myselect[3]][counts[myselect[3]]++] = source[dindex+3];
    buffers[myselect[4]][counts[myselect[4]]++] = source[dindex+4];
    buffers[myselect[5]][counts[myselect[5]]++] = source[dindex+5];
    buffers[myselect[6]][counts[myselect[6]]++] = source[dindex+6];
    buffers[myselect[7]][counts[myselect[7]]++] = source[dindex+7];
  }
  for (i = loop_total; i < number; i++) {
    u64Int mydata = source[i];
    if (mydata & procmask) buffers[1][counts[1]++] = mydata;
    else buffers[0][counts[0]++] = mydata;
  }

  *nnomatch = counts[0];
  *nmatch = counts[1];
}

/* Manual unrolling is a significant win if -Msafeptr is used -KDU */
static void update_table(u64Int *data, u64Int *table, int number, int nlocalm1){
  int i,dindex,index;
  int div_num = number / 8;
  int loop_total = div_num * 8;
  u64Int index0,index1,index2,index3,index4,index5,index6,index7;
#ifdef _LARGE_TABLE
  u64Int ltable0,ltable1,ltable2,ltable3,ltable4,ltable5,ltable6,ltable7;
#endif

  for (i = 0; i < div_num; i++) {
    dindex = i*8;

    index0 = data[dindex] & nlocalm1;
    index1 = data[dindex+1] & nlocalm1;
    index2 = data[dindex+2] & nlocalm1;
    index3 = data[dindex+3] & nlocalm1;
    index4 = data[dindex+4] & nlocalm1;
    index5 = data[dindex+5] & nlocalm1;
    index6 = data[dindex+6] & nlocalm1;
    index7 = data[dindex+7] & nlocalm1;

#ifdef _LARGE_TABLE
    // Note: The order of these assign statement is invalid.
    // Especially when table size is small, error count is big.
    // Therefore we recommend to use the statement below "#else."
    ltable0 = table[index0];
    ltable1 = table[index1];
    ltable2 = table[index2];
    ltable3 = table[index3];
    ltable4 = table[index4];
    ltable5 = table[index5];
    ltable6 = table[index6];
    ltable7 = table[index7];

    table[index0] = ltable0 ^ data[dindex];
    table[index1] = ltable1 ^ data[dindex+1];
    table[index2] = ltable2 ^ data[dindex+2];
    table[index3] = ltable3 ^ data[dindex+3];
    table[index4] = ltable4 ^ data[dindex+4];
    table[index5] = ltable5 ^ data[dindex+5];
    table[index6] = ltable6 ^ data[dindex+6];
    table[index7] = ltable7 ^ data[dindex+7];
#else
    table[index0] = table[index0] ^ data[dindex];
    table[index1] = table[index1] ^ data[dindex+1];
    table[index2] = table[index2] ^ data[dindex+2];
    table[index3] = table[index3] ^ data[dindex+3];
    table[index4] = table[index4] ^ data[dindex+4];
    table[index5] = table[index5] ^ data[dindex+5];
    table[index6] = table[index6] ^ data[dindex+6];
    table[index7] = table[index7] ^ data[dindex+7];
#endif
  }

  for (i = loop_total; i < number; i++) {
    u64Int datum = data[i];
    index = datum & nlocalm1;
    table[index] ^= datum;
  }
}

void Power2NodesXMPRandomAccessUpdate(u64Int logTableSize, s64Int LocalTableSize, u64Int GlobalStartMyProc,
				      int logNumProcs, int MyProc, s64Int ProcNumUpdates){
  int i, j, logTableLocal, ipartner, iterate, niterate, iter_mod;
  int nkeep, nsend, nrecv, nlocalm1, nkept, isend;
  u64Int ran, *data, jpartner;

  /* setup: should not really be part of this timed routine */
  data = (u64Int *) malloc(CHUNKBIG*sizeof(u64Int));
  ran = HPCC_starts(4*GlobalStartMyProc);
  niterate = ProcNumUpdates / CHUNK;
  logTableLocal = logTableSize - logNumProcs;
  nlocalm1 = LocalTableSize - 1;

  /* actual update loop: this is only section that should be timed */
  for (iterate = 0; iterate < niterate; iterate++) {
    iter_mod = iterate % PITER;
    for (i = 0; i < CHUNK; i++) {
      ran = (ran << 1) ^ ((s64Int) ran < ZERO64B ? POLY : ZERO64B);
      data[i] = ran;
    }
    nkept = CHUNK;

    for (j = 0; j < logNumProcs; j++) {
      nkeep = nsend = 0;
      isend = j % 2;
      ipartner = (1 << j) ^ MyProc;

      if (ipartner > MyProc) {
      	sort_data(data, data, &send[isend][1], nkept, &nkeep, &nsend, logTableLocal+j);
        if (j > 0) {
	  jpartner = (1 << (j-1)) ^ MyProc;
#pragma xmp wait(p(jpartner+1))
	  xmp_sync_memory(&status);
	  nrecv = recv[iter_mod][j-1][0];
      	  sort_data(&recv[iter_mod][j-1][1], data, &send[isend][1], nrecv, &nkeep, &nsend, logTableLocal+j);
        }
      }
      else {
        sort_data(data, &send[isend][1], data, nkept, &nsend, &nkeep, logTableLocal+j);
        if (j > 0) {
	  jpartner = (1 << (j-1)) ^ MyProc;
#pragma xmp wait(p(jpartner+1))
	  xmp_sync_memory(&status);
	  nrecv = recv[iter_mod][j-1][0];
          sort_data(&recv[iter_mod][j-1][1], &send[isend][1], data, nrecv, &nsend, &nkeep, logTableLocal+j);
        }
      }

      send[isend][0] = nsend;
      recv[iter_mod][j][0:nsend+1]:[ipartner+1] = send[isend][0:nsend+1];
      xmp_sync_memory(&status);
#pragma xmp post(p(ipartner+1), 0)

      if (j == (logNumProcs - 1)) update_table(data, Table, nkeep, nlocalm1);

      nkept = nkeep;
    }

    if (logNumProcs == 0) update_table(data, Table, nkept, nlocalm1);
    else {
      jpartner = (1 << (j-1)) ^ MyProc;
#pragma xmp wait(p(jpartner+1))
      xmp_sync_memory(&status);
      nrecv = recv[iter_mod][j-1][0];
      update_table(&recv[iter_mod][j-1][1], Table, nrecv, nlocalm1);
    }
  }

  /* clean up: should not really be part of this timed routine */
  free(data);
}

#define BUCKET_SIZE 1024
#define SLOT_CNT 1
#define FIRST_SLOT 2

void HPCC_Power2NodesXMPRandomAccessCheck(u64Int logTableSize, u64Int LocalTableSize, u64Int GlobalStartMyProc, int logNumProcs,
					  int NumProcs, int MyProc, u64Int ProcNumUpdates, MPI_Datatype UINT64_DT, s64Int *NumErrors){
  u64Int Ran, RanTmp;
  u64Int *LocalBuckets, *GlobalBuckets;     /* buckets used in verification phase */
  s64Int NextSlot, WhichPe, PeBucketBase, SendCnt, errors, *PeCheckDone;
  int i, j, LocalAllDone = HPCC_FALSE;

  LocalBuckets  = (u64Int*)malloc(sizeof(u64Int)*(NumProcs*(BUCKET_SIZE+FIRST_SLOT)));
  GlobalBuckets = (u64Int*)malloc(sizeof(u64Int)*(NumProcs*(BUCKET_SIZE+FIRST_SLOT)));
  SendCnt = ProcNumUpdates; /*  SendCnt = 4 * LocalTableSize; */
  Ran = HPCC_starts (4 * GlobalStartMyProc);
  PeCheckDone = (s64Int*)malloc(sizeof(s64Int)*NumProcs);

  for (i=0; i<NumProcs; i++)
    PeCheckDone[i] = HPCC_FALSE;

  while(LocalAllDone == HPCC_FALSE){
    if (SendCnt > 0) {
      /* Initalize local buckets */
      for (i=0; i<NumProcs; i++){
        PeBucketBase = i * (BUCKET_SIZE+FIRST_SLOT);
        LocalBuckets[PeBucketBase+SLOT_CNT] = FIRST_SLOT;
        LocalBuckets[PeBucketBase+HPCC_DONE] = HPCC_FALSE;
      }

      /* Fill local buckets until one is full or out of data */
      NextSlot = FIRST_SLOT;
      while(NextSlot != (BUCKET_SIZE+FIRST_SLOT) && SendCnt>0 ) {
        Ran = (Ran << 1) ^ ((s64Int) Ran < ZERO64B ? POLY : ZERO64B);
        WhichPe = (Ran >> (logTableSize - logNumProcs)) & (NumProcs - 1);
        PeBucketBase = WhichPe * (BUCKET_SIZE+FIRST_SLOT);
        NextSlot = LocalBuckets[PeBucketBase+SLOT_CNT];
        LocalBuckets[PeBucketBase+NextSlot] = Ran;
        LocalBuckets[PeBucketBase+SLOT_CNT] = ++NextSlot;
        SendCnt--;
      }

      if (SendCnt == 0)
        for (i=0; i<NumProcs; i++)
          LocalBuckets[i*(BUCKET_SIZE+FIRST_SLOT)+HPCC_DONE] = HPCC_TRUE;

    } /* End of sending loop */

    LocalAllDone = HPCC_TRUE;

    /* Now move all the buckets to the appropriate pe */
    MPI_Alltoall(LocalBuckets, (BUCKET_SIZE+FIRST_SLOT), UINT64_DT, GlobalBuckets, 
		 (BUCKET_SIZE+FIRST_SLOT), UINT64_DT, MPI_COMM_WORLD);

    for (i = 0; i < NumProcs; i ++) {
      if(PeCheckDone[i] == HPCC_FALSE) {
        PeBucketBase = i * (BUCKET_SIZE+FIRST_SLOT);
        PeCheckDone[i] = GlobalBuckets[PeBucketBase+HPCC_DONE];
        for (j = FIRST_SLOT; j < GlobalBuckets[PeBucketBase+SLOT_CNT]; j ++) {
          RanTmp = GlobalBuckets[PeBucketBase+j];
          Table[RanTmp & (LocalTableSize-1)] ^= RanTmp;
        }
        LocalAllDone &= PeCheckDone[i];
      }
    }
  }

  errors = 0;
  s64Int ii;
  for (ii=0; ii<LocalTableSize; ii++)
    if (Table[ii] != ii + GlobalStartMyProc)
      errors++;

  *NumErrors = errors;
  free( PeCheckDone );
  free( GlobalBuckets );
  free( LocalBuckets );

  return;
}

int main(int argc, char **argv){
  // Check number of processes
  if(xmp_num_nodes() != PROCS){
    if(xmp_node_num() == 1)
      fprintf(stderr, "Specified number(%d) is not PROCS(%d).\n", xmp_num_nodes(), PROCS);
    exit(1);
  }
  int log2procs = (int)log2((double)PROCS);
  if(pow(2, log2procs) != (double)PROCS){
    if(xmp_node_num() == 1)
      fprintf(stderr, "PROCS(%d) must be a power of 2.\n", PROCS);
    exit(1);
  }
  if(PROCS > pow(2, MAXLOGPROCS)){
    if(xmp_node_num() == 1)
      fprintf(stderr, "2^MAXLOGPROCS(%d) must be grater than PROCS(%d).\n", MAXLOGPROCS, PROCS);
    exit(1);
  }

  u64Int i, b;
  s64Int NumErrors;
  double time, GUPs;
  int rank = xmp_node_num() - 1;

  b = (u64Int)rank * LOCAL_SIZE;
  for (i = 0; i < LOCAL_SIZE; i++) Table[i] = b + i;

  u64Int logTableSize = log2(XMP_TABLE_SIZE);
  u64Int GlobalStartMyProc = LOCAL_SIZE * rank;
  int logNumProcs = log2(PROCS);
  s64Int ProcNumUpdates = 4 * LOCAL_SIZE;

#pragma xmp barrier
  time = -xmp_wtime();

  Power2NodesXMPRandomAccessUpdate(logTableSize, LOCAL_SIZE, GlobalStartMyProc,
                                   logNumProcs, rank, ProcNumUpdates);

  time += xmp_wtime();
  GUPs = (time > 0.0 ? 1.0 / time : -1.0);
  GUPs *= 1e-9*NUPDATE;

#pragma xmp reduction(+:GUPs) on p
#pragma xmp reduction(MAX:time) on p

  if(rank == 0)
    printf("%f Billion(10^9) updates per second [GUP/s] \t %f sec.\n", GUPs/PROCS, time);

  time = -xmp_wtime();
  HPCC_Power2NodesXMPRandomAccessCheck(logTableSize, LOCAL_SIZE, GlobalStartMyProc, logNumProcs,
				       PROCS, rank, ProcNumUpdates, MPI_LONG_LONG_INT, &NumErrors);
  time += xmp_wtime();

#pragma xmp reduction(+:NumErrors) on p
#pragma xmp reduction(MAX:time) on p

  if(rank == 0)
    printf("Found %lld errors in %lld locations (%s). \t %f sec.\n",
	   NumErrors, XMP_TABLE_SIZE, (NumErrors <= 0.01*XMP_TABLE_SIZE) ? "passed" : "failed", time);

  return 0;
}