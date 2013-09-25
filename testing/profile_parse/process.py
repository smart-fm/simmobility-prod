#!/usr/bin/python2

import sys
import os
import re

#Use DecInt or the built-in BigInt?
UseDecInt = True

#Blacklist any low-frequency Workers (e.g., Signals)
OnlySingleTickWorkers = True


if UseDecInt:
  import DecInt

#Declare our regexes
LL_Key = '\"([^"]+)\"'
LL_Value = '\"([^"]+)\"'
LL_Property = " *%s *: *%s,? *" % (LL_Key, LL_Value)
LL_Line = "{(?:%s)+}" % LL_Property
LogPropRegex = re.compile(LL_Property)
LogLineRegex = re.compile(LL_Line)


#For parsing times:
#(sec,X),(nano,Y)
LogTimeRegex = re.compile('\( *sec *, *([0-9]+) *\) *, *\( *nano *, *([0-9]+) *\)')

#Some helper classes.
class WorkerTick:
  def __init__(self):
    self.startTime = None
    self.endTime = None
    self.numAgents = None


#Containers of helper classes.
workerTicks = {} #(workerId,tick) => WorkerTick


def parse_time(inStr):
  global UseDecInt
  mRes = LogTimeRegex.match(inStr)
  if not mRes:
    raise Exception("Invalid time: %s" % inStr)

  #Use a big decimal representation?
  nanoscale = 0
  if UseDecInt:
    nanoscale = DecInt.DecInt(10) ** 9
  else:
    nanoscale = 10 ** 9

  #Convert
  sec = long(mRes.group(1))
  nano = long(mRes.group(2))
  res = nanoscale*sec + nano 
  return res


def parse_work_tick(workerTicks, props, parseBegin):
  #Required properties.
  if not('real-time' in props and 'worker' in props and 'tick' in props):
    print "Warning: Skipping Worker; missing required properties."
    return
  if parseBegin and not 'num-agents' in props:
    print "Warning: Skipping Worker; missing required properties."
    return

  #Add it
  key = (props['worker'], props['tick'])
  if not key in workerTicks:
    workerTicks[key] = WorkerTick()

  if props['worker'] == '0x2c903c0':
    print "Tick: %s" % (props['tick'])

  #Make sure we are not overwriting anything:
  if parseBegin and (workerTicks[key].startTime is not None or workerTicks[key].numAgents is not None):
    print ("Warning: skipping existing WorkTick startTime for: (%s,%s)" % (props['worker'], props['tick']))
    return
  if not parseBegin and workerTicks[key].endTime is not None:
    print ("Warning: skipping existing WorkTick endTime for: (%s,%s)" % (props['worker'], props['tick']))
    return

  #Save it
  if parseBegin:
    workerTicks[key].startTime = parse_time(props['real-time'])
    workerTicks[key].numAgents = props['num-agents']
  else:
    workerTicks[key].endTime = parse_time(props['real-time'])


def print_work_ticks(out, lines):
  global OnlySingleTickWorkers
  for key in lines:
    #Skip unless tick +1 or -1 exists
    if OnlySingleTickWorkers:
      tick = int(key[1])
      if not ((key[0], str(tick+1)) in lines or (key[0], str(tick-1)) in lines):
        continue

    #Print
    wrk = lines[key]
    if wrk.startTime and wrk.endTime and wrk.numAgents:
      out.write('%s\t%s\t%s\t%s\n' % (key[0], key[1], wrk.endTime-wrk.startTime, wrk.numAgents))
    else:
      print "Warning: skipping output agent (some properties missing)."


def dispatch_auramgr_update_action(act, props):
  #TODO
  pass

def dispatch_query_action(act, props):
  #TODO
  pass


def dispatch_worker_update_action(act, props):
  global workerTicks
  if act=='worker-update-begin':
    parse_work_tick(workerTicks, props, True)
  elif act=='worker-update-end':
    parse_work_tick(workerTicks, props, False)
  else:
    print ("Unknown worker action: %s" % act)


def dispatch_line(props):
  #Make sure we have some key properties
  if not 'action' in props:
    raise Exception("Can't parse a line without an 'action'.")
  if not 'real-time' in props:
    raise Exception("Can't parse a line without a 'real-time'.")

  #The rest we dispatch based on the action-type
  act = props['action']
  if act.startswith('worker-update-'):
    dispatch_worker_update_action(act, props)
  elif act.startswith('query-'):
    dispatch_query_action(act, props)
  elif act.startswith('auramgr-update-'):
    dispatch_auramgr_update_action(act, props)
  else:
    print ("Unknown action: %s" % act)


def run_main(inFilePath):
  global workerTicks

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
        if key in props:
          raise Exception('Duplicate property: %s => %s' % (key, val))
        props[key] = val
        pos = mr.end(0)
    else:
      print ('Skipped: %s' % line)
      continue

    #Dispatch it
    dispatch_line(props)

  #Print output
  out = open("workers.csv", 'w')
  out.write('%s\t%s\t%s\t%s\n' % ('WrkID', 'Tick', 'Len(Nano)', 'NumAgents'))
  print_work_ticks(out, workerTicks)




if __name__ == "__main__":
  if len(sys.argv) < 2:
    print ('Usage:\n' , sys.argv[0] , '<profile_trace.txt>')
    sys.exit(0)

  run_main(sys.argv[1])
  print("Done")

