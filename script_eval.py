#!/usr/bin/python

import sys
import os

filename_dict = {4:"64", 5:"128", 6:"256", 7:"512", 8:"1k", 9:"2k",
        10:"4k", 11:"8k", 12:"16k", 13:"32k", 14:"64k", 15:"128k", 16:"256k", 17:"512k", 18:"1m",
        19:"2m", 20:"4m", 21:"8m", 22:"16m", 23:"32m", 24:"64m", 25:"128m", 26:"256m", 27:"512m", 28:"1g", 29:"2g", 30:"4g", 31:"8g"}

start = int(sys.argv[1])
end = int(sys.argv[2])

os.system("rm -rf eval_results; mkdir eval_results")

for i in xrange(start, end+1):
  dir_name = "res_" + filename_dict[i]
  cmd = "mkdir eval_results/" + dir_name
  os.system(cmd)
  prof_html = "sdaccel_profile_summary_" + filename_dict[i] + ".csv"
  prof_stat = "prof_res_" + filename_dict[i] + ".stat"
  cmd0 = "./micro_bench_host/pkg/pcie/micro_bench_host.exe micro_bench.xclbin " + str(i) + " | tee " + prof_stat
  cmd1 = "mv " + prof_stat + " eval_results/" + dir_name
  cmd2 = "mv " + "sdaccel_profile_summary.csv eval_results/" + dir_name + "/" + prof_html
  os.system(cmd0)
  os.system(cmd1)
  os.system(cmd2)
  for k in xrange(1, 20):
    cmd3 = "./micro_bench_host/pkg/pcie/micro_bench_host.exe micro_bench.xclbin " + str(i) + " >> eval_results/" + dir_name + "/" + prof_stat
    cmd4 = "cat sdaccel_profile_summary.csv >> eval_results/" + dir_name + "/" + prof_html
    os.system(cmd3)
    os.system(cmd4)
