#!/usr/bin/python

import sys
import os
from decimal import Decimal

filename_dict = {4:"64", 5:"128", 6:"256", 7:"512", 8:"1k", 9:"2k",
        10:"4k", 11:"8k", 12:"16k", 13:"32k", 14:"64k", 15:"128k", 16:"256k", 17:"512k", 18:"1m",
        19:"2m", 20:"4m", 21:"8m", 22:"16m", 23:"32m", 24:"64m", 25:"128m", 26:"256m", 27:"512m", 28:"1g", 29:"2g", 30:"4g", 31:"8g"}

start = int(sys.argv[1])
end = int(sys.argv[2])

group_list = [[], [], [], [], [], [], [], [], []]
group_list[0].append("I/O datasize")
group_list[1].append("Preprocess")
group_list[2].append("Buffer Allocation")
group_list[3].append("Memory Copy")
group_list[4].append("Set Arguments")
group_list[5].append("Kernel API")
group_list[6].append("PCIe Transfer")
group_list[7].append("Dev2Host")
group_list[8].append("Postprocess")

for i in xrange(start, end+1):
  dir_name = "res_" + filename_dict[i]
  group_list[0].append(filename_dict[i])
  prof_stat = "prof_res_" + filename_dict[i] + ".stat"
  stat_file = open("eval_results/" + dir_name + "/" + prof_stat)
  preprocess = []
  buf_alloc = []
  mem_cpy = []
  set_args = []
  kernel_call = []
  pcie_trans = []
  dev2host = []
  postprocess = []
  for line in stat_file:
    tokens = line.strip().split()
    if "one-time preprocess" in line:
      preprocess.append(Decimal(tokens[2]))
    if "buffer allocation" in line:
      buf_alloc.append(Decimal(tokens[2]))
    if "memory copy" in line:
      mem_cpy.append(Decimal(tokens[2]))
    if "set arguments" in line:
      set_args.append(Decimal(tokens[2]))
    if "kernel calling" in line:
      kernel_call.append(Decimal(tokens[2]))
    if "kernel execution" in line:
      pcie_trans.append(Decimal(tokens[2]))
    if "device2host time" in line:
      dev2host.append(Decimal(tokens[2]))
    if "one-time postprocess" in line:
      postprocess.append(Decimal(tokens[2]))
  assert (len(preprocess) == 20)
  assert (len(buf_alloc) == 20)
  assert (len(mem_cpy) == 20)
  assert (len(set_args) == 20)
  assert (len(kernel_call) == 20)
  assert (len(pcie_trans) == 20)
  assert (len(dev2host) == 20)
  assert (len(postprocess) == 20)
  preprocess.sort()
  buf_alloc.sort()
  mem_cpy.sort()
  set_args.sort()
  kernel_call.sort()
  pcie_trans.sort()
  dev2host.sort()
  postprocess.sort()
  group_list[1].append(str(reduce(lambda x, y: x + y, preprocess)/Decimal("20.0")))
  group_list[2].append(str(reduce(lambda x, y: x + y, buf_alloc)/Decimal("20.0")))
  group_list[3].append(str(reduce(lambda x, y: x + y, mem_cpy)/Decimal("20.0")))
  group_list[4].append(str(reduce(lambda x, y: x + y, set_args)/Decimal("20.0")))
  group_list[5].append(str(reduce(lambda x, y: x + y, kernel_call)/Decimal("20.0")))
  group_list[6].append(str(reduce(lambda x, y: x + y, pcie_trans)/Decimal("20.0")))
  group_list[7].append(str(reduce(lambda x, y: x + y, dev2host)/Decimal("20.0")))
  group_list[8].append(str(reduce(lambda x, y: x + y, postprocess)/Decimal("20.0")))
  stat_file.close()
output_file = open("final_stat.csv", 'w')
output_file.write(','.join(group_list[0]) + '\n')
output_file.write(','.join(group_list[1]) + '\n')
output_file.write(','.join(group_list[2]) + '\n')
output_file.write(','.join(group_list[3]) + '\n')
output_file.write(','.join(group_list[4]) + '\n')
output_file.write(','.join(group_list[5]) + '\n')
output_file.write(','.join(group_list[6]) + '\n')
output_file.write(','.join(group_list[7]) + '\n')
output_file.write(','.join(group_list[8]) + '\n')
output_file.close()
