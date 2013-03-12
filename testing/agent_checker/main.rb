require 'Qt4'
require_relative 'mainwindow_ui'

#Determines how many ticks we load in at once when viewing agent updates.
# Anything much over 500 will probably hang the system.
$MaxTick = 200

#Regexes for parsing log lines, which are built like so:
#  {"key":"value",...}
LL_Key = '\"([^"]+)\"'
LL_Value = '\"([^"]+)\"'
LL_Property = " *#{LL_Key} *: *#{LL_Value},? *"
LL_Line = "{(?:#{LL_Property})+}"
LogPropRegex = Regexp.new(LL_Property)
LogLineRegex = Regexp.new(LL_Line)

#For parsing times:
#(sec,X),(nano,Y)
LogTimeRegex = /\( *sec *, *([0-9]+) *\) *, *\( *nano *, *([0-9]+) *\)/


#Colors used to paint generic messages
GenColors = [Qt::Color.new(0x33, 0x99, 0xEE), Qt::Color.new(0x44, 0xBB, 0x55), Qt::Color.new(0xDD, 0x11, 0x11)]


#####################
# Data-only classes.
#####################

class Simulation
  attr_accessor :agents
  attr_accessor :generics
  attr_accessor :ticks
  attr_accessor :knownWorkerIDs

  def initialize()
    @agents = {} #agentID => Agent
    @generics = {} #group => {caption => GenCaption}
    @ticks = {} #tickID => FrameTick
    @knownWorkerIDs = [] #string IDs, sorted
  end
end


class GenCaption
  attr_reader :caption
  attr_accessor :startTime
  attr_accessor :endTime

  def initialize(caption)
    @caption = caption
    @startTime = nil
    @endTime = nil
  end
end


class Agent
  attr_accessor :constTime
  attr_accessor :destTime
  attr_accessor :errorMsg
  attr_reader   :agentID

  def initialize(id)
    @agentID = id
    @constTime = nil
    @destTime = nil
    @errorMsg = nil
  end
end


class FrameTick
  attr_reader   :tickID
  attr_accessor :minStartTime
  attr_accessor :maxEndTime
  attr_accessor :workers

  def initialize(id)
    @tickID = id

    #Defines the frame's boundaries
    @minStartTime = nil
    @maxEndTime = nil
    @workers = {}   #workerID => Worker
  end
end


class Worker
  attr_reader   :workerID
  attr_accessor :minStartTime
  attr_accessor :maxEndTime
  attr_accessor :agentUpdates

  def initialize(id)
    @workerID = id
    @minStartTime = nil   #A Worker's min/max start/end time can extend past its Agents' updates, but Agents cannot go outside these bounds.
    @maxEndTime = nil
    @agentUpdates = {}  #agentID => AgentUpdate
  end
end


class AgentUpdate
  attr_reader   :agent
  attr_accessor :updateStartTime
  attr_accessor :updateEndTime

  def initialize(ag)
    @agent = ag #Ref. to the actual agent
    @updateStartTime = nil
    @updateEndTime = nil
  end
end


#Helper function to deal with ms without losing (too much) accuracy.
def conv_subsec_to_ms(subsec)
  #This should work for both Rationals and FixNums
  return (subsec * 1000).to_f
end

#Helper function to calculate the difference between two time values in
#  milliseconds. We were losing too much accuracy with endTime - startTime (which returns seconds)
def time_diff_ms(startTime, endTime)
  res = (endTime.tv_sec - startTime.tv_sec) * 1000.0          #Convert seconds
  res += (conv_subsec_to_ms(endTime.subsec) - conv_subsec_to_ms(startTime.subsec)) #Convert sub-seconds
  return res
end


#Visual representation of an Agent's lifecycle (creation/destruction + ID)
class AgentLifecycleItem < Qt::GraphicsItem
  #Agents are scaled to a given width/height. Adjust these if the view looks funny.
  @@SecondsW =  50  #1 second is X pixels
  @@SecondsH =  40  #Each agent (recorded in seconds) is X pixels in height
  def self.getSecondsW()
    return @@SecondsW
  end
  def self.getSecondsH()
    return @@SecondsH
  end

  def initialize(ag, earliestTime, agOffsets)
    super()

    #NOTE: Some of this can be cleaned up; it's a result of previous attempts to share a 
    #      common subclass between our visualization components.
    @errorMsg = ag.errorMsg
    @agentID = ag.agentID

    #Convert start/end times to offsets (s)
    @startTime = time_diff_ms(earliestTime, ag.constTime)
    @liveDuration = time_diff_ms(ag.constTime, ag.destTime)

    #Figure out a decent horizontal offset
    if agOffsets
      hOffset = -1
      agOffsets.each_index() {|i|
        if agOffsets[i] < ag.constTime
          #Pick the farthest one back (TODO: Is this really necessary?)
          if (hOffset==-1) or (agOffsets[i] < agOffsets[hOffset])
            hOffset = i
          end
        end
      }

      #Add one if we didn't find anything
      if hOffset == -1
        agOffsets.push(0)
        hOffset = agOffsets.length - 1
      end

      #Save it
      agOffsets[hOffset] = ag.destTime

      #Set its initial position
      setPos((@startTime*@@SecondsW)/1000, hOffset*@@SecondsH)
    end
  end

  def boundingRect()
    #Local (0,0) is centered on the start time
    return Qt::RectF.new(0, 0, (@liveDuration*@@SecondsW)/1000, @@SecondsH)
  end

  def paint(painter, option, widget)
    if @errorMsg
      painter.brush = Qt::Brush.new(Qt::Color.new(0xFF, 0x00, 0x00, 0x88))
    else
      painter.brush = Qt::Brush.new(Qt::Color.new(0x00, 0x00, 0xFF, 0x88))
    end

    painter.pen = Qt::Pen.new(Qt::Color.new(0x33, 0x33, 0x33))
    painter.drawRoundedRect(boundingRect(), 5, 5)

    painter.pen = Qt::Pen.new(Qt::Color.new(0xFF, 0xFF, 0xFF))
    painter.font = Qt::Font.new("Arial", 12)
    painter.drawText(5, 5+painter.fontMetrics.ascent(), @agentID.to_s)
  end

end



#Visual representation of a single frame update for a single worker thread.
class WorkerFrameTickItem < Qt::GraphicsItem
  #Used for adding meta-data. Adjust if things don't look right
  @@BorderW =  0  #In retrospect, we shouldn't clutter a timescale with anything not relevant to that scale.
  @@BorderH =  32

  #SecondsW at least needs to be MUCH higher than it was for the AgentLifecycle (or you won't see anything).
  @@SecondsW =  1000*250  #1 second is X pixels
  @@SecondsH =  40  #Each agent (recorded in seconds) is X pixels in height

  def initialize(frameTick, workerTick, earliestTime, tickColumn, workerRow)
    super()

    #Save frameTick for later
    @frame = frameTick
    @worker = workerTick
    @workerID = workerRow+1   #Looks better than a pointer

    #Convert start time, duration
    @startTime = time_diff_ms(earliestTime, frameTick.minStartTime)
    @liveDuration = time_diff_ms(frameTick.minStartTime, frameTick.maxEndTime)

    #Now we need to determine its position. This depends on both the @@Seconds and @@Border fields
    #We start with the "naive" values, and add in a border for each item
    naiveX = (@startTime * @@SecondsW) / 1000
    naiveY = workerRow * @@SecondsH
    setPos(naiveX + tickColumn*@@BorderW, naiveY + workerRow*@@BorderH)
  end

  def boundingRect()
    #Local (0,0) is centered on the start time. Again, we use the concept of a "naive" size here.
    naiveW = (@liveDuration * @@SecondsW) / 1000
    naiveH = @@SecondsH
    return Qt::RectF.new(0, 0, naiveW + @@BorderW, naiveH + @@BorderH)
  end

  def paint(painter, option, widget)
    #First, paint the background rectangle (it's white)
    painter.brush = Qt::Brush.new(Qt::Color.new(0xFF, 0xFF, 0xFF))
    painter.pen = Qt::Pen.new(Qt::Color.new(0x33, 0x33, 0x33))
    bounds = boundingRect()
    painter.drawRect(bounds)

    #The @@Border square in the upper-left tells us if any Agents were actually processed in this time tick.
    if @@BorderW > 0
      if @frame.workers.empty?
        painter.brush = Qt::Brush.new(Qt::Color.new(0x99, 0x99, 0x99))
      else
        painter.brush = Qt::Brush.new(Qt::Color.new(0x63, 0x72, 0xFF))
      end
      painter.pen = Qt::Pen.new(Qt::Color.new(0x00, 0x00, 0x00))
      painter.drawRect(0, 0, @@BorderW, @@BorderH)
    end

    #Now label the frame tick and worker id, centered and underneath each other.
    painter.pen = Qt::Pen.new(Qt::Color.new(0x33, 0x33, 0x33))
    painter.font = Qt::Font.new("Arial", 9)  #TODO: Can we apply an inverse transform here so that the text never looks stretched?
    painter.drawText(Qt::RectF.new(0, 0, bounds.width, @@BorderH/2), Qt::AlignCenter, "Th.#{@workerID}")
    painter.drawText(Qt::RectF.new(0, @@BorderH/2, bounds.width, @@BorderH/2), Qt::AlignCenter, @frame.tickID)

    #Draw the "Agents" area.
    painter.brush = Qt::Brush.new(Qt::Color.new(0x99, 0x99, 0x99))
    painter.drawRect(@@BorderW, @@BorderH, bounds.width-@@BorderW, bounds.height-@@BorderH)

    #Now draw each Agent update time. Do this in order, just in case.
    blue = false
    @worker.agentUpdates.values.sort{ |a1, a2| a1.updateStartTime <=> a2.updateStartTime }.each{|agent|
      #Alternate between blue and green
      blue = !blue
      if blue
        painter.brush = Qt::Brush.new(Qt::Color.new(0x30, 0x50, 0x90))
      else
        painter.brush = Qt::Brush.new(Qt::Color.new(0x30, 0x70, 0x30))
      end

      #No border
      painter.pen = Qt::NoPen

      #Try to get more accurate ms readings...
      startXOff = time_diff_ms(@frame.minStartTime, agent.updateStartTime)
      endXOff = time_diff_ms(@frame.minStartTime, agent.updateEndTime)

      #Offset for the @@Border
      startX = @@BorderW + (startXOff*@@SecondsW)/1000
      endX = @@BorderW + (endXOff*@@SecondsW)/1000
      agRect = Qt::RectF.new(startX, @@BorderH, endX-startX, bounds.height-@@BorderH)
      painter.drawRect(agRect)

      #Attempt to draw the Agent ID
      painter.font = Qt::Font.new("Arial", 8)
      painter.pen = Qt::Pen.new(Qt::Color.new(0xFF, 0xFF, 0xFF))
      painter.drawText(agRect, Qt::AlignCenter, agent.agent.agentID)
    }
  end
end



#Visual representation of a generic message
class GenericMessageItem < Qt::GraphicsItem
  #Note: We should really generalize these view classes. For example, the AgentLifecycleItem might extend this.
  @@SecondsW =  1000  #1 second is X pixels
  @@SecondsH =  40  #Each agent (recorded in seconds) is X pixels in height
  def self.getSecondsW()
    return @@SecondsW
  end
  def self.getSecondsH()
    return @@SecondsH
  end

  def initialize(capt, earliestTime, currRow, currColor)
    super()

    @caption = capt.caption
    @color = currColor

    #Convert start/end times to offsets (s)
    @startTime = time_diff_ms(earliestTime, capt.startTime)
    @liveDuration = time_diff_ms(capt.startTime, capt.endTime)

    #Set its initial position
    setPos((@startTime*@@SecondsW)/1000, currRow*@@SecondsH)
  end

  def boundingRect()
    #Local (0,0) is centered on the start time
    return Qt::RectF.new(0, 0, (@liveDuration*@@SecondsW)/1000, @@SecondsH)
  end

  def paint(painter, option, widget)
    painter.brush = Qt::Brush.new(@color)

    painter.pen = Qt::Pen.new(Qt::Color.new(0x33, 0x33, 0x33))
    painter.drawRoundedRect(boundingRect(), 5, 5)

    painter.pen = Qt::Pen.new(Qt::Color.new(0x00, 0x00, 0x00))
    painter.font = Qt::Font.new("Arial", 12)
    painter.drawText(5, 5+painter.fontMetrics.ascent(), @caption)
  end

end




#A custom MainWindow class for holding all our components.
#  Currently does to much; need to modularize it.
class MyWindow < Qt::MainWindow
  #Qt slots
  slots('open_trace_file()', 'read_trace_file()', 'switch_view()', 'update_view()')

  def initialize()
    super()

    #Setup and save the basic ui (from Qt Designer). This will connect a few signals, such as Quit.
    @ui = Ui_Main_window.new()
    @ui.setup_ui(self)

    #Cleaner drawing
    @ui.agViewCanvas.setRenderHints(Qt::Painter.Antialiasing | Qt::Painter.SmoothPixmapTransform)

    #Set up a button group
    @toolbarGroup = Qt::ButtonGroup.new(@ui.centralwidget)
    @toolbarGroup.addButton(@ui.viewCreateDestroy)
    @toolbarGroup.addButton(@ui.viewUpdates)
    @toolbarGroup.addButton(@ui.viewGeneral)

    #More component/variable initialization
    @ui.fileProgress.setVisible(false)
    @simRes = nil
    @miscDrawings = nil
    @currScaleX = 1.0

    #Now connect our own signals.
    Qt::Object.connect(@ui.menuOpenTraceFile, SIGNAL('activated()'), self, SLOT('open_trace_file()'))
    Qt::Object.connect(@toolbarGroup, SIGNAL('buttonClicked(QAbstractButton *)'), self, SLOT('switch_view()'))
    Qt::Object.connect(@ui.agTicksCmb, SIGNAL('currentIndexChanged(int)'), self, SLOT('update_view()'))
  end

############################################
# Parsing-relevant functions.
# These should definitely go into their own class.
############################################
  def get_property(props, key, req)
    return props[key] if props.has_key? key
    return nil unless req
    raise "Error: Log line missing mandatory property: #{key}"
  end

  def parse_time(timeStr)
    m = LogTimeRegex.match(timeStr)
    raise "Invalid time: #{timeStr}" unless m
    return Time.at(m[1].to_i, m[2].to_i/1000)
  end

  def dispatch_line(timeticks, knownWorkerIDs, agents, generics, properties, unknown_types)
    type = get_property(properties, "action", true)
    time = get_property(properties, "real-time", true)

    agRequired = ['constructed', 'destructed', 'exception', 'update-begin', 'update-end']
    agID = get_property(properties, "agent", agRequired.include?(type))

    if type=="constructed"
      dispatch_construction(agents, properties, agID, time)
    elsif type=="destructed"
      dispatch_destruction(agents, properties, agID, time)
    elsif type=="exception"
      dispatch_error(agents, properties, agID, time)
    elsif type=="update-begin"
      dispatch_startupdate(timeticks, agents, properties, agID, time, knownWorkerIDs)
    elsif type=="update-end"
      dispatch_endupdate(timeticks, agents, properties, agID, time, knownWorkerIDs)
    elsif type=="worker-update-begin"
      dispatch_worker_startupdate(timeticks, properties, time, knownWorkerIDs)
    elsif type=="worker-update-end"
      dispatch_worker_endupdate(timeticks, properties, time, knownWorkerIDs)
    elsif type=="generic-start"
      dispatch_generic_start(generics, properties, time)
    elsif type=="generic-end"
      dispatch_generic_end(generics, properties, time)
    else
      unknown_types[type]=0 unless unknown_types.has_key? type
      unknown_types[type] += 1
    end
  end


  def dispatch_generic_start(generics, properties, time)
    #Retrieve group, caption. Make and check as needed.
    group = get_property(properties, "group", true)
    caption = get_property(properties, "caption", true)
    generics[group] = {} unless generics.has_key? group
    raise "Double-construction for Caption #{group}:#{caption}" if generics[group].has_key? caption

    #Make, set
    generics[group][caption] = GenCaption.new(caption)
    generics[group][caption].startTime = parse_time(time)

    #Update minGenericTime
    time = generics[group][caption].startTime
    @minGenericTime = time unless @minGenericTime
    @minGenericTime = time if time < @minGenericTime
  end

  def dispatch_generic_end(generics, properties, time)
    #Retrieve group, caption. Make and check as needed.
    group = get_property(properties, "group", true)
    caption = get_property(properties, "caption", true)
    raise "Unknown generic group: #{group}" unless generics.has_key? group
    raise "Unknown generic caption: #{group}:#{caption}" unless generics[group].has_key? caption
    raise "Double end-time for caption: #{group}:#{caption}" if generics[group][caption].endTime

    #Set
    generics[group][caption].endTime = parse_time(time)
  end

  def dispatch_construction(agents, properties, agID, time)
    raise "Double-construction for Agent #{agID}" if agents.has_key? agID
    agents[agID] = Agent.new(agID)
    agents[agID].constTime = parse_time(time)

    #Update minTime
    time = agents[agID].constTime
    @minStartTime = time unless @minStartTime
    @minStartTime = time if time < @minStartTime
  end

  def dispatch_destruction(agents, properties, agID, time)
    raise "Agent does not exist: #{agID}" unless agents.has_key? agID
    raise "Double-destruction for Agent #{agID}" if agents[agID].destTime
    agents[agID].destTime = parse_time(time)
  end

  def dispatch_error(agents, properties, agID, time)
    raise "Agent does not exist: #{agID}" unless agents.has_key? agID
    raise "Double-exception set for Agent #{agID}" if agents[agID].errorMsg
    agents[agID].errorMsg = get_property(properties, "message", true)
  end

  def dispatch_startupdate(timeticks, agents, properties, agID, time, knownWorkerIDs)
    raise "Agent does not exist: #{agID}" unless agents.has_key? agID
    time = parse_time(time)
    worker = create_path_to_worker_and_update_bounds(properties, timeticks, time, knownWorkerIDs)

    #Add agent.
    raise "Double-startup on Agent: #{agID}" if worker.agentUpdates.has_key? agID
    worker.agentUpdates[agID] = AgentUpdate.new(agents[agID])
    worker.agentUpdates[agID].updateStartTime = time

    #Update the Worker's time boundaries too.
    worker.minStartTime = [time, worker.minStartTime].compact.min
    worker.maxEndTime = [time, worker.minStartTime].compact.max
  end

  def dispatch_endupdate(timeticks, agents, properties, agID, time, knownWorkerIDs)
    raise "Agent does not exist: #{agID}" unless agents.has_key? agID
    time = parse_time(time)
    worker = create_path_to_worker_and_update_bounds(properties, timeticks, time, knownWorkerIDs)

    #Update agent
    raise "Non-existent Agent Update for Agent: #{agID}" unless worker.agentUpdates.has_key? agID
    worker.agentUpdates[agID].updateEndTime = time

    #Update the Worker's time boundaries too.
    worker.minStartTime = [time, worker.minStartTime].compact.min
    worker.maxEndTime = [time, worker.minStartTime].compact.max
  end

  def dispatch_worker_startupdate(timeticks, properties, time, knownWorkerIDs)
    time = parse_time(time)
    worker = create_path_to_worker_and_update_bounds(properties, timeticks, time, knownWorkerIDs)
    worker.minStartTime = [time, worker.minStartTime].compact.min
    worker.maxEndTime = [time, worker.minStartTime].compact.max
  end

  def dispatch_worker_endupdate(timeticks, properties, time, knownWorkerIDs)
    time = parse_time(time)
    worker = create_path_to_worker_and_update_bounds(properties, timeticks, time, knownWorkerIDs)
    worker.minStartTime = [time, worker.minStartTime].compact.min
    worker.maxEndTime = [time, worker.minStartTime].compact.max
  end

  def create_path_to_worker_and_update_bounds(properties, timeticks, time, knownWorkerIDs)
    worker = get_property(properties, "worker", true)
    tick = get_property(properties, "tick", true)

    #Add this to our list of worker IDs
    knownWorkerIDs.push(worker) unless knownWorkerIDs.include? worker

    #Expand the bounds of the timetick (also, add it)
    timeticks[tick] = FrameTick.new(tick) unless timeticks.has_key? tick
    tick = timeticks[tick]
    tick.minStartTime = [time, tick.minStartTime].compact.min
    tick.maxEndTime = [time, tick.minStartTime].compact.max

    #Add worker, return
    tick.workers[worker] = Worker.new(worker) unless tick.workers.has_key? worker
    return tick.workers[worker]
  end


############################################
# General reader. This function is called from 
#  a Qt Timer, so it is allowed to update components.
############################################

  def read_trace_file()
    #Open our file, set the progress bar's maximum to its size in bytes
    f = File.open(@fileName)
    @ui.fileProgress.setMaximum(f.size)
    @ui.fileProgress.setValue(0)
    @ui.fileProgress.setVisible(true)

    start_time = Time.new
    unknown_types = {}
    @simRes = Simulation.new()
    bytesLoaded = 0
    @minStartTime = nil
    @minGenericTime = nil
    lastBytesLoaded = 0
    f.each { |line|
      #Update progress bar
      bytesLoaded += line.length
      if (bytesLoaded - lastBytesLoaded) > 1024*200 #Every few kb, update
        #TODO: We could "yield" here (kind of) if we want to repaint the rest of the UI.
        #      If you choose to do this, make sure to change the "singleShot" call in main.
        @ui.fileProgress.setValue(bytesLoaded)
        lastBytesLoaded = bytesLoaded
      end

      #Skip comments, empty lines
      line.strip!
      next if line.empty? or line.start_with? '#'

      #Parse
      if line =~ LogLineRegex
        props = {}
        line.scan(LogPropRegex) {|propRes|
          key = propRes[0]
          val = propRes[1]
          raise "Duplicate property: #{key}=>#{val}" if props.has_key? key
          props[key] = val
        }
        dispatch_line(@simRes.ticks, @simRes.knownWorkerIDs, @simRes.agents, @simRes.generics, props, unknown_types)
      else
        puts "Skipped: #{line}"
      end
    }

    #Break the agent update ticks into discrete chunks.
    #Convert keys to integer values here so we avoid sorting issues like 12 > 100
    @ui.agTicksCmb.clear()
    sorted_keys = @simRes.ticks.keys.sort{|t1, t2| t1.to_i <=> t2.to_i }
    sorted_keys.each_slice($MaxTick){|chunk|
      @ui.agTicksCmb.addItem("Ticks (#{chunk[0]}-#{chunk[-1]})")
    }

    #Sort the knownWorkerIDs array
    @simRes.knownWorkerIDs.sort! #Worker IDs are written as 0xXX; no need for to_i
    puts "Known worker IDs: #{@simRes.knownWorkerIDs}"

    #Print some diagnostics.
    puts "First time tick: #{@minStartTime}"
    puts "Warning: unknown \"action\" types: #{unknown_types}" unless unknown_types.empty?
    end_time = Time.new
    puts "File loaded in: #{end_time-start_time}"

    #Trigger adding agents by pressing the relevant button
    forceTriggerButton(@toolbarGroup, @ui.viewCreateDestroy)

    #Done
    @ui.fileProgress.setVisible(false)
  end

  #Force a button to trigger its event even if it is already pressed.
  def forceTriggerButton(toolbarGroup, buttonToTrigger)
    #TODO: How to actually trigger the button press event?
    buttonToTrigger.setChecked(true)
    switch_view()
  end

  #Prompt for a file to open; start a new Qt Timer object to load it.
  def open_trace_file()
    @fileName = Qt::FileDialog.getOpenFileName(self, "Open Trace File", "agent_update_trace.txt", "Trace Files (*.txt)")
    return unless @fileName

    #Start a new thread to do this (it's a time-consuming task)
    Qt::Timer.singleShot(0, self) { self.read_trace_file() }
  end


  #Switch between our "Lifecycle" and "Update" views, re-creating all visible objects each time (it's fast)
  def switch_view()
    #New scene
    #NOTE: The Ruby interpreter is good about reference counting; however, something's 
    #      not quite right with Qt<=>Ruby interaction. If you don't 'save' a Qt-specific variable
    #      (like the scene) into something that Ruby is aware of (e.g., @scene) then it will be GC'd at 
    #      some point in the future and you will get weird errors. 
    @scene = Qt::GraphicsScene.new()
    @ui.agViewCanvas.scene = @scene
    @ui.agTicksCmb.enabled = false

    if @toolbarGroup.checkedButton() == @ui.viewCreateDestroy
      make_create_destroy_objects() if @simRes
    elsif @toolbarGroup.checkedButton() == @ui.viewUpdates
      if @simRes
        @ui.agTicksCmb.enabled = true
        make_agent_update_objects()
      end
    elsif @toolbarGroup.checkedButton() == @ui.viewGeneral
      make_generic_objects() if @simRes
    end

    #Take the focus, for key events
    @ui.agViewCanvas.setFocus()

    #Zoom to default
    @currScaleX = 1.0
    @ui.agViewCanvas.resetTransform()
  end


  def update_view()
    if @toolbarGroup.checkedButton() == @ui.viewCreateDestroy
      #Nothing to update
    elsif @toolbarGroup.checkedButton() == @ui.viewUpdates
      if @simRes
        @scene = Qt::GraphicsScene.new()
        @ui.agViewCanvas.scene = @scene
        make_agent_update_objects()
      end
    elsif @toolbarGroup.checkedButton() == @ui.viewGeneral
      #Nothing to update
    end
  end


  #Create all objects which aren't specific to agents. Group by category vertically.
  def make_generic_objects()
    @miscDrawings = []
    @maxEndTime = nil

    #Generics are sorted (vertically) by their id. This id is not shown, but
    # could easily be toggled (later) in some dialog.
    currRow = 0
    @simRes.generics.keys.sort.each{|groupID|
      grp = @simRes.generics[groupID]

      #Now retrieve each caption. Cycle through colors as required.
      colorID = 0
      grp.keys.sort.each{|captID|
        capt = grp[captID]

        #Update end time
        @maxEndTime = capt.endTime unless @maxEndTime 
        @maxEndTime = capt.endTime if capt.endTime > @maxEndTime

        #Add a new graphics item
        genMsg = GenericMessageItem.new(capt, @minGenericTime, currRow, GenColors[colorID])
        @miscDrawings.push(genMsg)
        @ui.agViewCanvas.scene().addItem(genMsg)

        colorID = (colorID+1)%GenColors.length
      }
      currRow += 1
    }

    #"No data" is still valid.
    return unless @maxEndTime

    #Add seconds markers
    (0...(@maxEndTime-@minGenericTime)).step(1){|secs|
      xPos = secs*GenericMessageItem.getSecondsW
      yPos = currRow*GenericMessageItem.getSecondsH
          
      #Make a line
      line = Qt::GraphicsLineItem.new(xPos, 0, xPos, yPos)
      @miscDrawings.push(line)
      @ui.agViewCanvas.scene().addItem(line)

      #Make a label
      label = Qt::GraphicsSimpleTextItem.new("#{secs}s")
      label.setFont(Qt::Font.new("Arial", 10))
      label.setPos(xPos+2, yPos-10-2) #10 is just a guess based on font size.
      @miscDrawings.push(label)
      @ui.agViewCanvas.scene().addItem(label)
    }


  end



  #Create all objects relevant to the Agent Lifecycle view.
  def make_create_destroy_objects()
    @miscDrawings = []
    agOffsets = [] #"EndPos" for each row. New rows are added as needed
    @maxEndTime = nil
     
    #Now, add components. (Note that we can't add them earlier, since they may be specified out of order)
    #Sort by start time, so that we end up with fewer rows.
    @simRes.agents.values.sort { |a1, a2| a1.constTime <=> a2.constTime }.each{|agent|
      if agent.constTime and agent.destTime
        @maxEndTime = agent.destTime unless @maxEndTime 
        @maxEndTime = agent.destTime if agent.destTime > @maxEndTime
        lifecycle = AgentLifecycleItem.new(agent, @minStartTime, agOffsets)
        @miscDrawings.push(lifecycle)
        @ui.agViewCanvas.scene().addItem(lifecycle)
      else
        puts "Warning: Skipping agent #{agID}; no construction/destruction time."
      end
    }

    #"No data" is still valid.
    return unless @maxEndTime

    #Add seconds markers
    (0...(@maxEndTime-@minStartTime)).step(5){|secs|
      xPos = secs*AgentLifecycleItem.getSecondsW
      yPos = agOffsets.length*AgentLifecycleItem.getSecondsH
          
      #Make a line
      line = Qt::GraphicsLineItem.new(xPos, 0, xPos, yPos)
      @miscDrawings.push(line)
      @ui.agViewCanvas.scene().addItem(line)

      #Make a label
      label = Qt::GraphicsSimpleTextItem.new("#{secs}s")
      label.setFont(Qt::Font.new("Arial", 10))
      label.setPos(xPos+2, yPos-10-2) #10 is just a guess based on font size.
      @miscDrawings.push(label)
      @ui.agViewCanvas.scene().addItem(label)
    }
  end


  #Create all objects relevant to the Worker Tick view
  def make_agent_update_objects()
    @miscDrawings = []

    #Determine our min/max ticks. This is a little hackish at the moment, since 
    # it seems QtRuby doesn't preserve the "userData" field of "addItem"
    return unless m = @ui.agTicksCmb.currentText.match(/[(]([0-9]+)[-]+([0-9]+)[)]/)
    minVal = m[1]
    maxVal = m[2]

    #Step 1: Add rectangles over each of these describing the worker ID and tick ID. 
    tickColumn = 0
    (minVal..maxVal).each {|tickI|
      tick = @simRes.ticks[tickI.to_s]

      workerRow = 0
      @simRes.knownWorkerIDs.each{|workerID|
        next unless tick.workers.has_key? workerID #Some ticks don't perform any processing
        worker = tick.workers[workerID]
        wrk = WorkerFrameTickItem.new(tick, worker, @minStartTime, tickColumn, workerRow)
        @miscDrawings.push(wrk)
        @ui.agViewCanvas.scene().addItem(wrk)

        workerRow += 1
      }
      tickColumn += 1
    }
    puts 'Tick update objects added.'

    #Step 2: TODO: For each Agent, if there's an Exception, add a dotted red box around the potential area.
  end


  #Monitor key press events; use this to zoom in/out.
  #Currently we only zoom the x-axis; zooming y wouldn't make much sense.
  def keyPressEvent(event)
    #Handle
    if event.modifiers == Qt::ControlModifier
      if event.key == Qt::Key_Equal
        @currScaleX *= 1.1
        @ui.agViewCanvas.scale(1.1, 1.0)
      elsif event.key == Qt::Key_Minus
        @currScaleX *= 0.9
        @ui.agViewCanvas.scale(0.9, 1.0)
      elsif event.key == Qt::Key_0
        #Zoom to default
        @currScaleX = 1.0
        @ui.agViewCanvas.resetTransform()
      end
    end

    #Dispatch up
    super
  end

end


#Start up the Qt app
app = Qt::Application.new(ARGV)
window = MyWindow.new()
window.show()
app.exec()



