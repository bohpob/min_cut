#!/bin/bash

source /etc/profile.d/zz-cray-pe.sh

export MV2_HOMOGENEOUS_CLUSTER=1
export MV2_SUPPRESS_JOB_STARTUP_PERFORMANCE_WARNING=1

module load cray-mvapich2_pmix_nogpu

export MV2_ENABLE_AFFINITY=0

srun ./mpi 15 graf_mhr/graf_30_20.txt

exit 0
