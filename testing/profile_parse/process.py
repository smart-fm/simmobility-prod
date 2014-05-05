#!/usr/bin/python3

import sys
import os
import re

#Use DecInt or the built-in BigInt?
UseDecInt = False

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
class AuraMgrTick:
  def __init__(self):
    self.startTime = None
    self.endTime = None
class QueryTick:
  def __init__(self):
    self.startTime = None
    self.endTime = None
    self.wrkId = None
class CommsimTick:
  def __init__(self):
    self.startTime = None
    self.endTime = None
    self.wrkId = None
    self.numAgents = None
class CommsimGlob:
  def __init__(self):
    self.update = CommsimTick()
    self.localComp = CommsimTick()
    self.mixedComp = CommsimTick()
    self.androidComp = CommsimTick()


#Containers of helper classes.
workerTicks = {} #(workerId,tick) => WorkerTick
auraMgrTicks = {} #(aMgrId,tick) => AuraMgrTick
queryTicks = {} #(agentId,tick) => QueryTick
commsimTicks = {} #(agentId,tick) => CommsimGlob


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
  sec = int(mRes.group(1))
  nano = int(mRes.group(2))
  res = nanoscale*sec + nano 
  return res


def parse_work_tick(workerTicks, props, parseBegin):
  #Required properties.
  if not('real-time' in props and 'worker' in props and 'tick' in props):
    print ("Warning: Skipping Worker; missing required properties.")
    return
  if parseBegin and not 'num-agents' in props:
    print ("Warning: Skipping Worker; missing required properties.")
    return

  #Add it
  key = (props['worker'], props['tick'])
  if not key in workerTicks:
    workerTicks[key] = WorkerTick()

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


def parse_auramgr_tick(auraMgrTicks, props, parseBegin):
  #Required properties.
  if not('real-time' in props and 'auramgr' in props and 'tick' in props):
    print ("Warning: Skipping AuraMgr; missing required properties.")
    return

  #Add it
  key = (props['auramgr'], props['tick'])
  if not key in auraMgrTicks:
    auraMgrTicks[key] = AuraMgrTick()

  #Make sure we are not overwriting anything:
  if parseBegin and auraMgrTicks[key].startTime is not None:
    print ("Warning: skipping existing AuraMgr startTime for: (%s,%s)" % (props['auramgr'], props['tick']))
    return
  if not parseBegin and auraMgrTicks[key].endTime is not None:
    print ("Warning: skipping existing AuraMgr endTime for: (%s,%s)" % (props['auramgr'], props['tick']))
    return

  #Save it
  if parseBegin:
    auraMgrTicks[key].startTime = parse_time(props['real-time'])
  else:
    auraMgrTicks[key].endTime = parse_time(props['real-time'])


def parse_commsim_tick(commsimTicks, props, subType, parseBegin): #subtype: 'U','L','M','A' for update,local,mixed,android. 
  #Required properties.
  if not('real-time' in props and 'broker' in props and 'tick' in props and 'worker' in props):
    print ("Warning: Skipping CommSim; missing required properties[1].")
    return
  if (subType=='L' or subType=='M') and not 'num-agents' in props:
    print ("Warning: Skipping CommSim; missing required properties[2].")
    return

  #Add it
  key = (props['broker'], props['tick'])
  if not key in commsimTicks:
    commsimTicks[key] = CommsimGlob()

  #Pull out the appropriate CommsimTick
  commTick = None
  if subType=='U':
    commTick = commsimTicks[key].update
  elif subType=='L':
    commTick = commsimTicks[key].localComp
  elif subType=='M':
    commTick = commsimTicks[key].mixedComp
  elif subType=='A':
    commTick = commsimTicks[key].androidComp
  if not commTick:
    print("Warning: Skipping CommSim; unknown sub-type")
    return

  #Make sure we are not overwriting anything:
  if parseBegin and commTick.startTime is not None:
    print ("Warning: skipping existing CommSim startTime for: (%s,%s)" % (props['broker'], props['tick']))
    return
  if not parseBegin and commTick.endTime is not None:
    print ("Warning: skipping existing CommSim endTime for: (%s,%s)" % (props['broker'], props['tick']))
    return

  #Save it
  if parseBegin:
    commTick.startTime = parse_time(props['real-time'])
  else:
    commTick.endTime = parse_time(props['real-time'])

  #Save additional properties
  commTick.wrkId = props['worker']
  if 'num-agents' in props:
    if not commTick.numAgents:
      commTick.numAgents = props['num-agents']
    if commTick.numAgents != props['num-agents']:
      print("Warning: commsim has two different values for num-agents.")


def parse_query_tick(queryTicks, props, parseBegin):
  #Required properties.
  if not('real-time' in props and 'agent' in props and 'worker' in props and 'tick' in props):
    print ("Warning: Skipping Query; missing required properties.")
    return

  #Add it
  key = (props['agent'], props['tick'])
  if not key in queryTicks:
    queryTicks[key] = QueryTick()

  #Make sure we are not overwriting anything:
  if parseBegin and (queryTicks[key].startTime is not None or queryTicks[key].wrkId is not None):
    print ("Warning: skipping existing QueryTick startTime for: (%s,%s)" % (props['agent'], props['tick']))
    return
  if not parseBegin and queryTicks[key].endTime is not None:
    print ("Warning: skipping existing QueryTick endTime for: (%s,%s)" % (props['agent'], props['tick']))
    return
  if not parseBegin and queryTicks[key].wrkId != props['worker']:
    print ("Warning: skipping bad-worker QueryTick endTime for: (%s,%s)" % (props['agent'], props['tick']))
    return

  #Save it
  if parseBegin:
    queryTicks[key].startTime = parse_time(props['real-time'])
    queryTicks[key].wrkId = props['worker']
  else:
    queryTicks[key].endTime = parse_time(props['real-time'])



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
      print ("Warning: skipping output Worker (some properties missing).")



def print_auramgr_ticks(out, lines):
  for key in lines:
    #Print
    am = lines[key]
    if am.startTime and am.endTime:
      out.write('%s\t%s\t%s\n' % (key[0], key[1], am.endTime-am.startTime))
    else:
      print ("Warning: skipping output AuraManager (some properties missing).")

def print_query_ticks(out, lines):
  for key in lines:
    #Print
    qr = lines[key]
    if qr.startTime and qr.endTime:
      out.write('%s\t%s\t%s\t%s\n' % (key[0], key[1], qr.endTime-qr.startTime, qr.wrkId))
    else:
      print ("Warning: skipping output Query (some properties missing).")

def print_commsim_ticks(out, lines, wrkLines):
  #Guess the brokerId
  brokerId = None
  for key in lines:
    brokerId = key[0]
    break

  #We need to know the largest worker time tick and the total number of connected agents, barring the 
  #  Broker's threaed. Build a lookup now.
  wrkLookup = {} #tick => (time, connected)
  for key in wrkLines:
    if key[0] == brokerId:
      continue
    if key[1] not in wrkLookup:
      wrkLookup[key[1]] = [0,0]
    wrkLookup[key[1]][0] = max(wrkLookup[key[1]][0], wrkLines[key].endTime-wrkLines[key].startTime)
    wrkLookup[key[1]][1] += int(wrkLines[key].numAgents)

  #Now print
  for key in lines:
    #Check: only 1 broker
    if brokerId != key[0]:
      print("Warning: multiple Brokers not supported, skipping.")
      continue

    #Quick validate
    cm = lines[key]
    if not (cm.update.startTime and cm.update.endTime and cm.update.wrkId):
      print("Warning: skipping output Commsim; missing \"update\" tick.")
      continue
    if not (cm.localComp.startTime and cm.localComp.endTime and cm.localComp.wrkId and cm.localComp.numAgents):
      print("Warning: skipping output Commsim; missing \"local\" tick.")
      continue
    if not (cm.mixedComp.startTime and cm.mixedComp.endTime and cm.mixedComp.wrkId and cm.mixedComp.numAgents):
      print("Warning: skipping output Commsim; missing \"mixed\" tick.")
      continue
    if not (cm.androidComp.startTime and cm.androidComp.endTime and cm.androidComp.wrkId):
      print("Warning: skipping output Commsim; missing \"android\" tick.")
      continue

    #Retrieve the largest worker tick and the total connected agents.
    if not key[1] in wrkLookup:
      print("Warning: skipping output Commsim; missing worker lookup entry")
      continue
    wrkTime = wrkLookup[key[1]][0]
    wrkAgents = wrkLookup[key[1]][1]

    #Print
    updateTime = cm.update.endTime-cm.update.startTime
    localTime = cm.localComp.endTime-cm.localComp.startTime
    mixedTime = cm.mixedComp.endTime-cm.mixedComp.startTime
    androidTime = cm.androidComp.endTime-cm.androidComp.startTime
    out.write('%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % (key[1], str(int(key[1])//10), wrkTime, wrkAgents, cm.mixedComp.numAgents, updateTime, localTime, mixedTime, androidTime, localTime+mixedTime, localTime+2*mixedTime, localTime+mixedTime+androidTime))



def dispatch_auramgr_update_action(act, props):
  global auraMgrTicks
  if act=='auramgr-update-begin':
    parse_auramgr_tick(auraMgrTicks, props, True)
  elif act=='auramgr-update-end':
    parse_auramgr_tick(auraMgrTicks, props, False)
  else:
    print ("Unknown aura manager action: %s" % act)

def dispatch_commsim_update_action(act, props):
  global commsimTicks
  if act=='commsim-update-begin':
    parse_commsim_tick(commsimTicks, props, 'U', True)
  elif act=='commsim-update-end':
    parse_commsim_tick(commsimTicks, props, 'U', False)
  elif act=='commsim-local-compute-begin':
    parse_commsim_tick(commsimTicks, props, 'L', True)
  elif act=='commsim-local-compute-end':
    parse_commsim_tick(commsimTicks, props, 'L', False)
  elif act=='commsim-mixed-compute-begin':
    parse_commsim_tick(commsimTicks, props, 'M', True)
  elif act=='commsim-mixed-compute-end':
    parse_commsim_tick(commsimTicks, props, 'M', False)
  elif act=='commsim-android-compute-begin':
    parse_commsim_tick(commsimTicks, props, 'A', True)
  elif act=='commsim-android-compute-end':
    parse_commsim_tick(commsimTicks, props, 'A', False)
  else:
    print ("Unknown commsim action: %s" % act)

def dispatch_query_action(act, props):
  global queryTicks
  if act=='query-start':
    parse_query_tick(queryTicks, props, True)
  elif act=='query-end':
    parse_query_tick(queryTicks, props, False)
  else:
    print ("Unknown query action: %s" % act)


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
  elif act.startswith('commsim-'):
    dispatch_commsim_update_action(act, props)
  else:
    print ("Unknown action: %s" % act)


def run_main(inFilePath):
  global workerTicks
  global auraMgrTicks
  global queryTicks

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
  out1 = open("workers.csv", 'w')
  out1.write('%s\t%s\t%s\t%s\n' % ('WrkID', 'Tick', 'Len(Nano)', 'NumAgents'))
  print_work_ticks(out1, workerTicks)

  out2 = open("auramgr.csv", 'w')
  out2.write('%s\t%s\t%s\n' % ('AuraMgrID', 'Tick', 'Len(Nano)'))
  print_auramgr_ticks(out2, auraMgrTicks)

  out3 = open("queries.csv", 'w')
  out3.write('%s\t%s\t%s\t%s\n' % ('AgentID', 'Tick', 'Len(Nano)', 'OnWorker'))
  print_query_ticks(out3, queryTicks)

  out4 = open("commsim.csv", 'w')
  out4.write('%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' % ('tick', 'time_s', 'work_longest', 'agents_total', 'agents_connected', 'broker_len', 'local_len', 'mixed_len', 'android_len', 'localMixed', 'local2Mixed', 'localMixedAndroid'))
  print_commsim_ticks(out4, commsimTicks, workerTicks)



if __name__ == "__main__":
  if len(sys.argv) < 2:
    print ('Usage:\n' , sys.argv[0] , '<profile_trace.txt>')
    sys.exit(0)

  run_main(sys.argv[1])
  print("Done")

