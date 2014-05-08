events = {}
EventType = { 
    NEW_JOB = 0, 
    LOST_JOB = 1, 
    NEW_JOB_LOCATION = 2, 
    NEW_CHILD = 3,
    NEW_SCHOOL_LOCATION = 4}
    
function addEvent (day, type, householdId)
    event = ExternalEvent()
    event.day = day;
    event.householdId = householdId;
    event.type = type;
    if events[day] == nil then
        events[day] = {}
    end
    table.insert(events[day], event);
end

--addEvent(67, EventType.NEW_JOB, 1)
--addEvent(67, EventType.NEW_JOB_LOCATION, 2)
--addEvent(300, EventType.NEW_CHILD, 3)
--addEvent(300, EventType.NEW_SCHOOL_LOCATION, 4)
--addEvent(69, EventType.NEW_JOB, 51841)
--addEvent(77, EventType.NEW_JOB_LOCATION, 51842)
--addEvent(160, EventType.NEW_CHILD, 51848)
--addEvent(200, EventType.NEW_SCHOOL_LOCATION, 51850)

--[[
    Gets all external events for given day.

    @param day to of the events.
    @return ExternalEvent list.
]]
excluded={}
function getExternalEvents (day)
    --if events[day] ~= nil then
    --    return events[day];
    --end
    if day == 1 then     
        for i= 1, 100 do
           local household = math.random(1,1146054)
           if excluded[household] == nil then
              addEvent(day, EventType.NEW_SCHOOL_LOCATION, household)
              excluded[household]={}
           end
           --addEvent(1, EventType.NEW_SCHOOL_LOCATION, i)
        end
    end 
    return events[day]
end