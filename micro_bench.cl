/*******************************************************************************
 * Microbenchmark Kernel: micro_bench
*******************************************************************************/

__kernel __attribute__ ((reqd_work_group_size(1, 1, 1)))
void micro_bench(__global int* a)
{
  return;
}
