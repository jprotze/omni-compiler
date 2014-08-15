#PBS -S /bin/bash
#PBS -N mnakao_job
#PBS -A XMPTCA
#PBS -q tcaq-q1
#PBS -l select=1:ncpus=1:host=tca-0001-eth0+1:ncpus=1:host=tca-0002-eth0+1:ncpus=1:host=tca-0003-eth0+1:ncpus=1:host=tca-0004-eth0+1:ncpus=1:host=tca-0005-eth0+1:ncpus=1:host=tca-0006-eth0+1:ncpus=1:host=tca-0007-eth0+1:ncpus=1:host=tca-0008-eth0
NP=8
#PBS -l walltime=00:01:00
#PBS -o o_reflect_acc_2_massive
#PBS -e e_reflect_acc_2_massive
. /opt/Modules/default/init/bash
#---------------
# select=NODES:ncpus=CORES:mpiprocs=PROCS:ompthreads=THREADS:mem=MEMORY
# NODES   : num of nodes
# CORES   : num of cores per node
# PROCS   : num of procs per node
# THREADS : num of threads per process
#----------------
module purge
export MODULEPATH=/work/TCAPROF/hanawa/Modules:$MODULEPATH
module load cuda/6.0 pgi/13.10
cd $PBS_O_WORKDIR
PATH=/work/XMPTCA/mnakao/work/mvapich2-2.0/bin/:$PATH
mpirun_rsh -np $NP -hostfile $PBS_NODEFILE MV2_ENABLE_AFFINITY=0 MV2_SHOW_CPU_BINDING=1 numactl --cpunodebind=0 --localalloc ./reflect_acc_2_massive.x
