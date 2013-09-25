#!/usr/bin/python2

import sys
import os
import re

#Declare our regexes
LL_Key = '\"([^"]+)\"'
LL_Value = '\"([^"]+)\"'
LL_Property = " *%s *: *%s,? *" % (LL_Key, LL_Value)
LL_Line = "{(?:%s)+}" % LL_Property
LogPropRegex = re.compile(LL_Property)
LogLineRegex = re.compile(LL_Line)

print(LL_Line)

#For parsing times:
#(sec,X),(nano,Y)
LogTimeRegex = re.compile('\( *sec *, *([0-9]+) *\) *, *\( *nano *, *([0-9]+) *\)')

#Some helper classes.
class WorkerTick:
  def __init__(self):
    self.time = None
    self.id = None
    self.tick = None
    self.numAgents = None


#Instances of the helper classes
wrkStart = None



def parse_work_tick(props, startTick, parseBegin):
  if parseBegin and startTick:
    print "Warning: skipping existing WorkTick"
    return None
  if not parseBegin and not startTick:
    print "Warning: missing beginning WorkTick"
    return None

  if not('real-time' in props and 'worker' in props and 'tick' in props):
    print "Warning: Skipping Worker; missing required properties."
    return None
  if parseBegin and not 'num-agents' in props:
    print "Warning: Skipping Worker; missing required properties."
    return None

  res = WorkerTick()
  res.time = parse_time(props['real-time'])
  res.id = props['worker']
  res.tick = props['tick']
  if parseBegin:
    res.numAgents = props['num-agents']
  return res


def print_work_tick(out, wStart, wEnd):
  if not (wStart and wEnd):
    print "Warning: skipping null start/end WorkTick"
    return
  if wStart.id != wEnd.id:
    print "Warning: skipping start/end WorkTick; IDs don't match."
    return
  if wStart.tick != wEnd.tick:
    print "Warning: skipping start/end WorkTick; time ticks don't match."
    return

  out.write('\n' % (wStart.id, wStart.tick, wEnd.time-wStart.time, wStart.numAgents))


def dispatch_auramgr_update_action(out, act, props):
  #TODO
  pass

def dispatch_query_action(out, act, props):
  #TODO
  pass


def dispatch_worker_update_action(out, act, props):
  if act=='worker-update-begin':
    wrkStart = parse_work_tick(props, wrkStart, True)
  elif act=='worker-update-end':
    wrkEnd = parse_work_tick(props, wrkStart, False)
    print_work_tick(out, wrkStart, wrkEnd)
  else:
    print ("Unknown worker action: %s" % act)


def dispatch_line(out, props):
  #Make sure we have some key properties
  if not 'action' in props:
    raise Exception("Can't parse a line without an 'action'.")
  if not 'real-time' in props:
    raise Exception("Can't parse a line without a 'real-time'.")

  #The rest we dispatch based on the action-type
  act = props['action']
  if act.startswith('worker-update-'):
    dispatch_worker_update_action(out, act, props)
  elif act.startswith('query-'):
    dispatch_query_action(act, props)
  elif act.startswith('auramgr-update-'):
    dispatch_auramgr_update_action(act, props)
  else:
    print ("Unknown action: %s" % act)


def run_main(inFilePath):
  #Prepare an output file.
  out = open("workers.csv", 'w')

  for line in open(inFilePath):
    #Skip empty lines, comments
    line = line.strip()
    if (len(line)==0) or line.startswith('#'):
      continue

    #Parse it.
    if LogLineRegex.match(line):
      props = {}
      pos = 0
      while True:
        mr = LogPropRegex.search(line, pos)
        if not mr:
          break

        key = mr.group(1)
        val = mr.group(2)
        print pos
        if key in props:
          raise Exception('Duplicate property: %s => %s' % (key, val))
        props[key] = val
        pos = mr.end(0)
    else:
      print ('Skipped: %s' % line)
      continue

    #Dispatch it
    dispatch_line(out, props)




if __name__ == "__main__":
  if len(sys.argv) < 2:
    print ('Usage:\n' , sys.argv[0] , '<profile_trace.txt>')
    sys.exit(0)

  run_main(sys.argv[1])
  print("Done")

