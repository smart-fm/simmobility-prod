#!/usr/bin/python2

import re

#Declare our regexes
LL_Key = '\"([^"]+)\"'
LL_Value = '\"([^"]+)\"'
LL_Property = " *#{LL_Key} *: *#{LL_Value},? *"
LL_Line = "{(?:#{LL_Property})+}"
LogPropRegex = re.compile(LL_Property)
LogLineRegex = re.compile(LL_Line)

#For parsing times:
#(sec,X),(nano,Y)
LogTimeRegex = re.compile('\( *sec *, *([0-9]+) *\) *, *\( *nano *, *([0-9]+) *\)')




def dispatch_line(props);
  #Make sure we have some key properties
  if not 'action' in props:
    raise Exception("Can't parse a line without an 'action'.")
  if not 'real-time' in props:
    raise Exception("Can't parse a line without a 'real-time'.")

  #The rest we dispatch based on the action-type
  act = props['action']
  if act.startsWith('worker-update-'):
    dispatch_worker_update_action(act, props)
  elif act.startsWith('query-'):
    dispatch_query_action(act, props)
  elif act.startsWith('auramgr-update-'):
    dispatch_auramgr_update_action(act, props)
  else:
    print ("Unknown action: %s" % act)


def run_main(inFilePath):
  for line in open(inFilePath):
    #Skip empty lines, comments
    line = line.strip()
    if (len(line)==0) or line.startsWith('#'):
      continue

    #Parse it.
    if LogLineRegex.match(line):
      props = {}
      pos = 0
      while mr = LogPropRegex.search(line, pos):
        key = pos.group(1)
        val = pos.group(2)
        if key in props:
          raise Exception('Duplicate property: %s => %s' % (key, val))
        props[key] = val
        pos = mr.pos+1
    else:
      print ('Skipped: %s' % line)
      continue

    #Dispatch it
    dispatch_line(props)
  }



if __name__ == "__main__":
  if len(sys.argv) < 2:
    print ('Usage:\n' , sys.argv[0] , '<profile_trace.txt>')
    sys.exit(0)

  run_main(sys.argv[1])
  print("Done")

