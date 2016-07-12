
function Use_Service_Controller(params,time) 
               
if time  == "07:47:00" then
--[[
local return_table = {}
return_table[1]="NE3_1"
return_table[2]="NE5_2"
params:update_platformList(1,return_table,"NE_1")

--]]
--params:reset_HoldingTime_AtStation("NE5_1",100,1,"NE_1")
--params:terminate_SingleTrainService(5,"NE_1")
--params:terminate_TrainService("NE_1")

--params:restrict_Passengers("NE4_1",1,"NE_1",0)
--params:insert_UnscheduledTrainTrip("NE_1","07:34:00","NE7")

--params:get_nextPlatform(3,"NE_1")
--params:get_distancetoNextPlatform("NE_1",5)
--params:get_DwellTime(18,"NE_2","NE13")
end

--params:reset_SafeOperation_Distance("20",1,"NE_1")


--if time  == "07:45:00" then

--params:reset_SafeHeadway_Sec(200,5,"NE_1")
--end

--[[  commented section


   params:reset_SpeedLimit(30,"CC29/NE1","NE4","NE_1","07:30:00","08:00:00")
   if size ==0 then
   end
   --]]
 
if time  == "07:47:00" then 
   local trainIds = {} 
   params:set_DisruptedPlatforms("NE5","NE8","NE_1")
   local disruptedPlatformsSize_firstLine=params:get_DisruptedPlatformsSize("NE_1")
   params:set_DisruptedPlatforms("NE8","NE5","NE_2")
   local disruptedPlatformsSize_secLine=params:get_DisruptedPlatformsSize("NE_2")
   local size=params:get_ActiveTrainsSize("NE_1")
   if size>1 then
	   for i = 0,size-1 do 
		   local trainId=params:get_ActiveTrainByIndex(i,"NE_1")
		   local comingPlatform=params:get_nextPlatform(trainId,"NE_1")
		   local nextPlatform=params:get_Platform_By_Offset(1)
		   for j=0,disruptedPlatformsSize_firstLine-1 do
		        if comingPlatform == params:GetDisruptedPlatformByIndex("NE_1",j) then
			     params:force_Release_Passenegers(trainId,"NE_1")
			   -- do something ignore safe distane bring trains as close if stranded between platform
                           if params:get_NextRequested(trainId) ==5 then
                           local nextTrainId=params:get_TrainId_TrainAhead(trainId)
			   if nextTrainId!=-1 then
				local nextRequested=params:get_NextRequested(nextTrainId)
				if nextRequested==1 or nextRequested==2 then
				   if params:get_nextPlatform(nextTrainId,"NE_1") == comingPlatform then
					--ignore safe distance 
				      params:set_IgnoreSafeDistance(trainId,"NE_1",true)
				   end 
				end 			
			   end
			end
			elseif nextPlatform == params:GetDisruptedPlatformByIndex("NE_1",j) then
			       params:force_Release_Passenegers(trainId,"NE_1")
			       params:make_Train_Uturn(trainId,"NE_1")
			  -- do something take Uturn
			end
		   trainIds[i]=trainId
	   end
--[[
	   for i=0,size-1 do

		local trainId=trainIds[i]
		params:get_nextPlatform(trainId,
	   end   
--]]
   end 
   
end
