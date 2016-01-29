# Define the solution for SDAccel
create_solution -name micro_bench_bitstream -dir . -force
add_device -vbnv xilinx:adm-pcie-7v3:1ddr:1.1

# Host Compiler Flags
set_property -name host_cflags -value "-g -Wall -D FPGA_DEVICE"  -objects [current_solution]

# Host Source Files
add_files "test-cl.c"

# Kernel Definition
create_kernel micro_bench -type clc
add_files -kernel [get_kernels micro_bench] "micro_bench.cl"

# Define Binary Containers
create_opencl_binary micro_bench
set_property region "OCL_REGION_0" [get_opencl_binary micro_bench]
create_compute_unit -opencl_binary [get_opencl_binary micro_bench] -kernel [get_kernels micro_bench] -name k1

# Compile the design for CPU based emulation
compile_emulation -flow cpu -opencl_binary [get_opencl_binary micro_bench]

# Run the compiled application in CPU based emulation mode
run_emulation -flow cpu -args "micro_bench.xclbin 30"

# Compile the application to run on the accelerator card
build_system

# Package the application binaries
package_system

