ExpectationEntry = {}
ExpectationEntry.__index = ExpectationEntry

setmetatable(ExpectationEntry, {
  __call = function (cls, ...)
    return cls.new(...)
  end,
})

function ExpectationEntry.new(init)
  local self = setmetatable({}, ExpectationEntry)
  self.value = init
  return self
end 
