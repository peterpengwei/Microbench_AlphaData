# Define the solution for SDAccel
create_solution -name micro_bench_host -dir . -force
add_device -vbnv xilinx:adm-pcie-7v3:1ddr:1.1

# Host Compiler Flags
set_property -name host_cflags -value "-g -Wall -D FPGA_DEVICE"  -objects [current_solution]

# Host Source Files
add_files "test-cl.c"

# Compile the host C program only
compile_host -arch x86_64

# Package the application binaries
package_system

