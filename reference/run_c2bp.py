#!/usr/bin/python

import sys
import io
import re
import subprocess
from types import *
import datetime 
import time
import os
import copy
import locale
from collections import namedtuple

EXEC_NAME = './c2bpy'
ENCODE_BUFFERS_OP = 'ENCODE_BUFFERS'
SLAM_BOOL_PROG_OP = 'SLAM_BOOL_PROG'
CUPEX_BOOL_PROG_OP = 'X_BOOL_PROG'
RMM =  'PSO' # / 'TSO'
FLUSH_FLAG = 'YF' # / 'NF'
MODE = 'sc'

mod = ''

class Row:
  pass
# nr_fences, fence_cfg, nr_calls_smt, c2bp_time
# nr_states, fender_time, fender_memory, nr_errors

def remove_comments_last_line(file_name):
  f = open(file_name, 'r')
  lines = f.readlines()
  f.close()
  f = open(file_name, 'w')
  idx = 0;
  pc_line = ''
  assert_line = ''
  for line in lines:
    if 'program counters:' in line:
      pc_line = line

    if 'assert always' in line:
      assert_line = line
      # assumes that assert is the last line of the program
      break
    f.write(line)
    idx += 1

  # transform the pc line
  pc_line = pc_line.replace('program counters:', '')
  pc_line = pc_line.replace('/*', '')
  pc_line = pc_line.replace('*/', '')
  pc_line = pc_line.replace('\n', '')
   
#  print lines[idx]
# remove comments from the assert always line
  assert_line = assert_line.replace('/*', '')
  assert_line = assert_line.replace('*/', '')
  assert_line = assert_line.replace(';', '')
  assert_line = assert_line.replace('\n', '')
  assert_line = assert_line.replace('assert always', '')

  assert_line = assert_line + pc_line

  # add overflow predicate
  if MODE == 'sc':
    assert_line = 'assert always (' + assert_line + ');\n' 
  else:
    assert_line = 'assert always (' + assert_line + ') && (B0 == 1);\n' 

  f.write(assert_line)
  f.close()


# returns: folder/fence_N/[1, 0, ...1]
def get_name_file_with_fences(fence_array, orig_file_name):
  folder_name = os.path.dirname(orig_file_name)
  nr_fences = sum(fence_array)
  if nr_fences == len(fence_array):
    return folder_name + '/all_fences'
  else: 
    return folder_name + '/' + RMM + '_' + str(nr_fences) + 'f_' + str(fence_array)

def create_bool_prog_with_fences(fence_array, orig_bool_prog):
  f = open(orig_bool_prog, 'r')
  lines = f.readlines()
  f.close()
  fence_idx = -1
  in_fence = 0
  new_prog = orig_bool_prog + '.' + str(fence_array)
  f = open(new_prog, 'w') 
  for line in lines:
    if 'fence_start' in line:
      in_fence = 1
      fence_idx = fence_idx + 1
      assert fence_idx < len(fence_array), 'fence_idx < array length'

    if in_fence == 0:
      f.write(line)
    elif fence_array[fence_idx] == 1:
      f.write(line) 

    if 'fence_end' in line:
      in_fence = 0

  f.close()
  return new_prog

def create_file_with_fences(fence_array, file_name):
  f = open(file_name, 'r')
  lines = f.readlines()
  f.close()

  file_to_create = get_name_file_with_fences(fence_array, file_name) 
 
  f = open(file_to_create, 'w')
  fence_nr = 0
  start_counting = 0

  for line in lines:
    f.write(line)
    if 'endinit' in line:
      start_counting = 1
    if 'store' in line and \
	'/*' not in line and \
	start_counting == 1:
      if fence_array[fence_nr] == 1: 
        f.write('  fence;\n')
      fence_nr += 1 

  f.close()

  
def generate_bitvectors(vector, position, lines, file_name, folder_name):
  if position == len(vector):
    create_file_with_fences(vector, lines, file_name, folder_name) 
  else:
    vector[position] = 1
    generate_bitvectors(vector, position + 1, lines, file_name, folder_name)
    vector[position] = 0
    generate_bitvectors(vector, position + 1, lines, file_name, folder_name)
 

def nr_fences(file_name):
  f = open(file_name, 'r')
  lines = f.readlines()
  f.close()
  nr_stores = 0
  start_counting = 0
  for line in lines:
    if 'endinit' in line:
      start_counting = 1
    if 'store' in line \
	and '/*' not in line \
	and start_counting == 1:
      nr_stores += 1

  print 'nr_stores= ', nr_stores
  return nr_stores



  generate_bitvectors(a, 0, lines, file_name, folder_name)
  return nr_stores
  

def call_fender(file_name, row):
  java_path = '/home/dani/workspace/jre1.7.0_05/bin/java'
  jvm_gc = '-XX:+UseConcMarkSweepGC' #'-XX:+UseParallelOldGC'  #'-XX:+UseParallelGC'  
  jvm_mem = '-Xmx80G'
  jvm_jar = '-jar'
 # fender_path = '../../fender-sb-bdd/lib/fender_experimental.jar'
  fender_path = '/home/dani/workspace/fender-sb-bdd/lib/fender.jar'
  output_name = file_name + '_fender_output'
# call fender
  print 'Running fender..'
  f = open(output_name, 'w')
  p = subprocess.Popen([java_path, jvm_gc, \
	jvm_mem, jvm_jar, fender_path, file_name, 'sc', '-invokeGCBeforeEndStatistics'], \
	stdout=f)#, stderr=subprocess.PIPE)
  p.communicate()

  retval = p.wait()
  f.close()
  f = open(output_name)
  print 'retval = ' + str(retval)

  if retval != 0:
    print 'Error encountered:'
    for line in f:
      print line,
    return 1
  else:
    for line in f:
      m = re.match(r'States: (\d+)', line)
      if type(m) is not NoneType:
        n = int(m.group(1))
        s = locale.format("%d", n, grouping=True)
        print '#States: ', s
        row.nr_states = s
          
      m = re.match(r'Running Time: (\d+)', line)
      if type(m) is not NoneType:
        time = float(m.group(1)) / 1000
        s = locale.format("%.1f", time, grouping=True)
        print 'Time (s): ', s
        row.fender_time = s

      m = re.match(r'Memory usage: (\d+)', line)
      if type(m) is not NoneType:
        n = int(m.group(1))
        s = locale.format("%d", n, grouping=True)
        print 'Memory (MB): ', s
        row.fender_memory = s

      m = re.match(r'Error states: (\d+)', line)
      if type(m) is not NoneType:
        if m.group(1) == '0':
          print 'No errors'
          row.nr_errors = '0'
        else:
          print 'Error states found'
          row.nr_errors = '1'
          
    if row.nr_errors == '1':
      return 1
    return 0


def gen_bool_prog(op, program_file, pred_file, max_cube_size, row):
# generate the boolean program
  c2bp_output = program_file + '_c2bp_output'
  f = open(c2bp_output, 'w')
  print 'Generating boolean program..'
  p = subprocess.Popen([EXEC_NAME, op, \
	max_cube_size, program_file, pred_file], \
	stdout=f)#, stderr=subprocess.STDOUT)

  p.communicate()
  retval = p.wait()
  f.close()

  if retval != 0:
    print 'Error encountered:'
    f = open(c2bp_output, 'r')
    for line in f:
       print line,
    f.close()
    return 1

  bool_prog_file = program_file + '.bl'

# remove comments from boolean program last line, if they exist
  remove_comments_last_line(bool_prog_file)

# get time it took to build the boolean program
  f = open(bool_prog_file + '.stats')
  for line in f:
    m = re.match(r'(\d+) calls to SMT', line) 
    if type(m) is not NoneType:
      n = int(m.group(1))
      s = locale.format("%d", n, grouping=True)
      print '#SMT calls:', s
      row.nr_calls_smt = s

    m = re.match(r'(\d+) time', line) 
    if type(m) is not NoneType:
      time = float(m.group(1)) / 1000
      s = locale.format("%.1f", time, grouping=True)
      print 'c2bp time (s):', s
      row.c2bp_time = s
  return 0

def count_preds_cubes(file_name):
  
# get number of lines in pred file
  f = open(file_name, 'r')
  flag = 0
  nr_preds = 0
  nr_cubes = 0
  for line in f:
    if len(line) == 0:
      flag = 1
      continue
    if flag == 0:
      nr_preds += 1 
    else:
      assert(flag == 1)
      nr_cubes += 1
  f.close()

  print '#preds = ', nr_preds
  print '#cubes = ', nr_cubes


def solve_sc(program_file, pred_file, max_cube_size, row):
  gen_bool_prog(SLAM_BOOL_PROG_OP, \
	program_file, pred_file, max_cube_size, row)
  call_fender(program_file + '.bl', row)

def extract_cubes(pred_file, stats_file):
  f = open(pred_file, 'r')
  g = open(pred_file + '+cubes', 'w')
  for line in f:
    g.write(line)
  f.close()

  if mod is 'X':
    f = open(stats_file, 'r')
    reading_cubes_flag = 0
    for line in f:
      if reading_cubes_flag == 0:
        m = re.match(r'(\d+) distinct cubes:', line)
        if type(m) is not NoneType:
          reading_cubes_flag = 1
          if int(m.group(1)) is not 0:
            g.write('\n')
      else:
        g.write(line)
    f.close()

  g.close()


def get_prog_with_buffers(orig_file, nr_vars_relaxed, buffer_size):
  return orig_file + '.' + RMM+ '.b' + buffer_size + \
	'.' + nr_vars_relaxed + 'v.' + FLUSH_FLAG + '.prog'


def get_pred_with_buffers(orig_file, nr_vars_relaxed, buffer_size):
  return orig_file + '.' + RMM + '.b' + buffer_size + \
	'.' + nr_vars_relaxed + 'v.' + FLUSH_FLAG + '.pred'

def get_bool_prog_name(orig_file):
  return orig_file + '.bl'
     
 
def encode_buffers(program_file, pred_file, buffer_size, nr_relaxed_vars): 
  print 'Encoding buffers..'
  p = subprocess.Popen([EXEC_NAME, ENCODE_BUFFERS_OP, RMM, \
	program_file, pred_file, nr_relaxed_vars, \
	buffer_size, FLUSH_FLAG], stdout=subprocess.PIPE, \
	stderr=subprocess.STDOUT)
  return p.wait()


def make_fences_folders(prog_file, nr_fences):
  folder_name = os.path.dirname(prog_file)
  for i in range(0, nr_fences + 1):
    p = subprocess.Popen(['mkdir', folder_name + '/' + RMM + '_fences_' + str(i)], \
	stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    retval = p.wait()
  
  
      
def check_config(fence_cfg, orig_bool_prog):

  print(str(fence_cfg))
  sys.stdout.flush()
  row = Row()

  row.nr_fences = str(sum(fence_cfg))
  row.fence_cfg = str(fence_cfg)

  bool_prog_file = create_bool_prog_with_fences(fence_cfg, orig_bool_prog)
  # call model checker
  fender_retval = call_fender(bool_prog_file, row)
  return row

def print_attr(row, res_file, color, align, attr):
    res_file.write('<td align=\"' + align + '\">' + '<font color=\"' + color + '\">'+ \
	getattr(row, attr, '-') + '</font>' + '</td>\n')
  
def print_row(row, res_file, color):
  res_file.write('<tr>\n')

  fields = ['nr_fences', 'fence_cfg', 'nr_calls_smt', 'c2bp_time', \
	    'nr_states', 'fender_time', 'fender_memory']
  for field in fields:
    print_attr(row, res_file, color, 'center', field)
  
  res_file.write('</tr>\n') 
  res_file.flush()

def failed_config(row):
  if hasattr(row, 'nr_states') and \
	hasattr(row, 'nr_errors') and \
	row.nr_errors == '0':
    return False
  return True

def test_less_fences(fence_cfg, orig_bool_prog, res_file, d):

  # start with a given fence configuration
  row = check_config(fence_cfg, orig_bool_prog)

  if failed_config(row):
    d[str(fence_cfg)] = 0
    return False # fail

  # verification successfull
  d[str(fence_cfg)] = 1

  minimal = True
  for i in range(0, nr_fences):
    if fence_cfg[i] == 1:
      # remove one fence
      new_fence_cfg = copy.copy(fence_cfg)
      new_fence_cfg[i] = 0
      if str(new_fence_cfg) in d:
        if d[str(new_fence_cfg)] == 1:
          minimal = False
      else:
        if test_less_fences(new_fence_cfg, orig_bool_prog, res_file, d) is True:
          minimal = False

  if minimal is True:
    print_row(row, res_file, "blue")
  else:
    print_row(row, res_file, "black")
  return True

def print_html_antet(res):

  res.write('<!DOCTYPE html>\n')
  res.write('<html>\n')
  res.write('<body>\n')
  res.write('<table border=\"1\"\n')
  res.write('<tr>\n')
  res.write('<td> # fences </td>\n')
  res.write('<td> Fence cfg </td>\n')
  res.write('<td> #SMT calls </td>\n')
  res.write('<td> c2bp time (s) </td>\n')
  res.write('<td> #states </td>\n')
  # res.write('<td> Error states </td>\n')
  res.write('<td> Fender time (s) </td>\n')
  res.write('<td> Memory usage (MB) </td>\n')
  res.write('</tr>\n')

locale.setlocale(locale.LC_ALL, '')

if len(sys.argv) < 4:
  print 'usage: ./run_c2bp.py \
<folder_name> <sc/pso/tso> \
<cube_size> [#_vars_relaxed] [buffer_size] [X/SLAM]\n'
  exit(1)

folder = sys.argv[1]
if folder[-1] == '/':
  folder[-1] = ''
MODE = sys.argv[2]

max_cube_size = sys.argv[3]

prog_file = folder + '/orig'
pred_file = folder + '/pred'

if MODE == 'sc':
  nr_vars_relaxed = '_'
  buffer_size = '_'
else:
  nr_vars_relaxed = sys.argv[4]
  buffer_size = sys.argv[5]
  mod = sys.argv[6]
  

cubes_file = folder + '/orig.bl.stats'
results_file = folder + '/' + folder + '.' + MODE + '.b' + buffer_size + '.' + \
	nr_vars_relaxed + 'v.' + FLUSH_FLAG + '.results@(' + \
	time.asctime(time.localtime((time.time()))) + ').html'
res = open(results_file, 'w')

print_html_antet(res)

if MODE == 'sc':
  row = Row()
  row.nr_fences = str(0)
  row.fence_cfg = 'SC'


  solve_sc(prog_file, pred_file, max_cube_size, row)
  print_row(row, res, "black")
  extract_cubes(pred_file, cubes_file)
else:


  RMM = MODE.upper() 

# how many fences we may place in the file
  nr_fences = nr_fences(prog_file)

# create fences folders
  #make_fences_folders(prog_file, nr_fences)

# configuration with all fences on
  fence_cfg = [1] * nr_fences

  d = dict()

# generate boolean program

# add fences in the SC file
  create_file_with_fences(fence_cfg, prog_file)
  prog_with_fences = get_name_file_with_fences(fence_cfg, prog_file)

  print(str(fence_cfg))
  sys.stdout.flush()
  row = Row()
  row.nr_fences = str(sum(fence_cfg))
  row.fence_cfg = str(fence_cfg)

  # encode wmm buffers in the prog
  encode_buffers(prog_with_fences, \
	pred_file + '+cubes', buffer_size, nr_vars_relaxed)

  prog_with_fences_and_buffers = get_prog_with_buffers(prog_with_fences, \
	nr_vars_relaxed, buffer_size)

  pred_with_buffers =  get_pred_with_buffers(prog_with_fences, \
  	nr_vars_relaxed, buffer_size)
  #pred_with_buffers = pred_file + '.fixed'

  # generate boolean program  
  if mod is 'X':
    code = gen_bool_prog(CUPEX_BOOL_PROG_OP, \
  	prog_with_fences_and_buffers, pred_with_buffers, max_cube_size, row)
  else:
    assert(mod == 'SLAM')
    code = gen_bool_prog(SLAM_BOOL_PROG_OP, \
  	prog_with_fences_and_buffers, pred_with_buffers, max_cube_size, row)

  print_row(row, res, "black")

  bool_prog_file = get_bool_prog_name(prog_with_fences_and_buffers)
  # call model checker
  # fender_retval = call_fender(bool_prog_file, row)

  test_less_fences(fence_cfg, bool_prog_file, res, d)

res.write('</body>\n')
res.write('</html>\n')

res.close()
