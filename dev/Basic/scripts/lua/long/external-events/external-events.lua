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

addEvent(67, EventType.NEW_JOB, 1)
addEvent(67, EventType.NEW_JOB_LOCATION, 2)
addEvent(300, EventType.NEW_CHILD, 3)
addEvent(300, EventType.NEW_SCHOOL_LOCATION, 4)

--[[
    Gets all external events for given day.

    @param day to of the events.
    @return ExternalEvent list.
]]
function getExternalEvents (day)
    if events[day] ~= nil then
        return events[day];
    end 
end