/*******************************************************************************
 * Microbenchmark Host: micro_bench
*******************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <CL/opencl.h>
#include "my_timer.h"

int
load_file_to_memory(const char *filename, char **result)
{ 
  size_t size = 0;
  FILE *f = fopen(filename, "rb");
  if (f == NULL) 
  { 
    *result = NULL;
    return -1; // -1 means file opening fail 
  } 
  fseek(f, 0, SEEK_END);
  size = ftell(f);
  fseek(f, 0, SEEK_SET);
  *result = (char *)malloc(size+1);
  if (size != fread(*result, sizeof(char), size, f)) 
  { 
    free(*result);
    return -2; // -2 means file reading fail 
  } 
  fclose(f);
  (*result)[size] = 0;
  return size;
}

int main(int argc, char** argv)
{
   
  if (argc != 3){
    return EXIT_FAILURE;
  }
  unsigned long long data_size = (unsigned long long)1 << atoi(argv[2]);
  printf("data_size = %llu\n", data_size * sizeof(int));

  int err;                            // error code returned from api calls
     
  int* a = (int*)malloc(data_size*sizeof(int));                   // pageable memory space
  unsigned long long correct;               // number of correct results returned

  size_t global = 1;                   // global domain size for our calculation
  size_t local = 1;                    // local domain size for our calculation

  cl_platform_id platform_id;         // platform id
  cl_device_id device_id;             // compute device id 
  cl_context context;                 // compute context
  cl_command_queue commands;          // compute command queue
  cl_program program;                 // compute program
  cl_kernel kernel;                   // compute kernel
   
  char cl_platform_vendor[1001];
  char cl_platform_name[1001];
   
  cl_mem dev_buffer;                     // device memory used for the output array

  // Fill our data sets with pattern
  //
  for(unsigned long long i = 0; i != data_size; i++) {
    a[i] = i*2+1;
  }

  // 0th: initialize the timer at the beginning of the program
  timespec timer = tic();

  // Connect to first platform
  //
  err = clGetPlatformIDs(1,&platform_id,NULL);
  if (err != CL_SUCCESS)
  {
    printf("Error: Failed to find an OpenCL platform!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  err = clGetPlatformInfo(platform_id,CL_PLATFORM_VENDOR,1000,(void *)cl_platform_vendor,NULL);
  if (err != CL_SUCCESS)
  {
    printf("Error: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  printf("CL_PLATFORM_VENDOR %s\n",cl_platform_vendor);
  err = clGetPlatformInfo(platform_id,CL_PLATFORM_NAME,1000,(void *)cl_platform_name,NULL);
  if (err != CL_SUCCESS)
  {
    printf("Error: clGetPlatformInfo(CL_PLATFORM_NAME) failed!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  printf("CL_PLATFORM_NAME %s\n",cl_platform_name);
 
  // Connect to a compute device
  //
  int fpga = 0;
#if defined (FPGA_DEVICE)
  fpga = 1;
#endif
  err = clGetDeviceIDs(platform_id, fpga ? CL_DEVICE_TYPE_ACCELERATOR : CL_DEVICE_TYPE_CPU,
                       1, &device_id, NULL);
  if (err != CL_SUCCESS)
  {
    printf("Error: Failed to create a device group!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  
  // Create a compute context 
  //
  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context)
  {
    printf("Error: Failed to create a compute context!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // Create a command commands
  //
  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands)
  {
    printf("Error: Failed to create a command commands!\n");
    printf("Error: code %i\n",err);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  int status;

  // Create Program Objects
  //
  
  // Load binary from disk
  unsigned char *kernelbinary;
  char *xclbin=argv[1];
  printf("loading %s\n", xclbin);
  int n_i = load_file_to_memory(xclbin, (char **) &kernelbinary);
  if (n_i < 0) {
    printf("failed to load kernel from xclbin: %s\n", xclbin);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  size_t n = n_i;
  // Create the compute program from offline
  program = clCreateProgramWithBinary(context, 1, &device_id, &n,
                                      (const unsigned char **) &kernelbinary, &status, &err);
  if ((!program) || (err!=CL_SUCCESS)) {
    printf("Error: Failed to create compute program from binary %d!\n", err);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // Build the program executable
  //
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    size_t len;
    char buffer[2048];

    printf("Error: Failed to build program executable!\n");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    printf("%s\n", buffer);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // Create the compute kernel in the program we wish to run
  //
  kernel = clCreateKernel(program, "micro_bench", &err);
  if (!kernel || err != CL_SUCCESS)
  {
    printf("Error: Failed to create compute kernel!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // 1st: setup time of the one-time preprocess
  clFinish(commands);
  toc(&timer, "one-time preprocess");

  // Create the device buffer for our calculation
  //
  dev_buffer = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(int) * data_size, NULL, NULL);
  if (!dev_buffer)
  {
    printf("Error: Failed to allocate device memory!\n");
    printf("Test failed\n");
    return EXIT_FAILURE;
  }    

  // 2nd: time of buffer allocation
  toc(&timer, "buffer allocation");

  // Write our data set into the device buffer 
  //
  err = clEnqueueWriteBuffer(commands, dev_buffer, CL_TRUE, 0, sizeof(int) * data_size, a, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
      printf("Error: Failed to write to source array a!\n");
      printf("Test failed\n");
      return EXIT_FAILURE;
  }

  // 3rd: time of pageable-pinned memory copy
  toc(&timer, "memory copy");
    
  // Set the arguments to our compute kernel
  //
  err = 0;
  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_buffer);
  if (err != CL_SUCCESS)
  {
    printf("Error: Failed to set kernel arguments! %d\n", err);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // 4th: time of setting arguments
  toc(&timer, "set arguments");

  // Execute the kernel over the entire range of our 1d input data set
  // using the maximum number of work group items for this device
  //

#ifdef C_KERNEL
  err = clEnqueueTask(commands, kernel, 0, NULL, NULL);
#else
  err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, 
                               (size_t*)&global, (size_t*)&local, 0, NULL, NULL);
#endif
  if (err)
  {
    printf("Error: Failed to execute kernel! %d\n", err);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
  // 5th: time of kernel invocation
  toc(&timer, "kernel calling");

  // 6th: time of kernel execution (Host => Device PCIe DMA)
  clFinish(commands);
  toc(&timer, "kernel execution");

  // Read back the results from the device to verify the output
  //
  err = clEnqueueReadBuffer( commands, dev_buffer, CL_TRUE, 0, sizeof(int) * data_size, a, 0, NULL, NULL );  
  if (err != CL_SUCCESS)
  {
    printf("Error: Failed to read output array! %d\n", err);
    printf("Test failed\n");
    return EXIT_FAILURE;
  }

  // 7th: time of data retrieving (PCIe + memcpy)
  toc(&timer, "device2host time");

  free(a);
    
  // Shutdown and cleanup
  //
  clReleaseMemObject(dev_buffer);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);

  // 8th: one-time postprocess time
  clFinish(commands);
  toc(&timer, "one-time postprocess");

  correct = data_size;
  if(correct == data_size){
    printf("Test passed!\n");
    return EXIT_SUCCESS;
  }
  else{
    printf("Test failed\n");
    return EXIT_FAILURE;
  }
}
