

--[[
    Gets all external events for given day.

    @param day to of the events.
    @return ExternalEvent list.
]]
function getExternalEvents (day)
    local events = {}
    for i=1,100 do
        entry = ExternalEvent()
        entry.day = day;
        entry.householdId = i;
        entry.type = 1;
        events[i] = entry;
    end
    return events;
end