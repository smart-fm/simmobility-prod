
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
 
if time =="07:56:00" then
--unset disrupted platforms
--unset Uturn flag
--unset force alight flag
--unset force alight status
--unset ignore safe distance and safe headway
 params:clear_Disruption("NE_1")
 params:clear_Disruption("NE_2")
 --local disruptedPlatformsSize_firstLine=params:get_DisruptedPlatformsSize("NE_1")
 --local disruptedPlatformsSize_secLine=params:get_DisruptedPlatformsSize("NE_2")
 local size=params:get_ActiveTrainsSize("NE_1")
 if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_ActiveTrainByIndex(i,"NE_1")
		  params:setUnset_Uturn(trainId,"NE_1",false)
		  params:force_Release_Passenegers(trainId,"NE_1",false)
		  params:setUnset_ForceAlightStatus(trainId,"NE_1",false)
		  params:set_IgnoreSafeDistance(trainId,"NE_1",false)
                  params:set_IgnoreSafeHeadway(trainId,"NE_1",false)
		  if params:isStranded_DuringDisruption(trainId,"NE_1") == true then
			params:set_SubsequentNextRequested(trainId,"NE_1",5)
		  elseif params:get_DisruptedState(trainId,"NE_1") == true then
			params:set_SubsequentNextRequested(trainId,"NE_1",1)
		  end
	   end
 end

size=params:get_ActiveTrainsSize("NE_2")
if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_ActiveTrainByIndex(i,"NE_2")
		  params:setUnset_Uturn(trainId,"NE_2",false)
		  params:force_Release_Passenegers(trainId,"NE_2",false)
		  params:setUnset_ForceAlightStatus(trainId,"NE_2",false)
		  params:set_IgnoreSafeDistance(trainId,"NE_2",false)
                  params:set_IgnoreSafeHeadway(trainId,"NE_2",false)
	          if params:isStranded_DuringDisruption(trainId,"NE_2") == true then
			params:set_SubsequentNextRequested(trainId,"NE_2",5)
		  elseif params:get_DisruptedState(trainId,"NE_2") == true then
			params:set_SubsequentNextRequested(trainId,"NE_2",1)
		  end
	   end
 end



elseif time  >= "07:47:00" and time<="07:56:00" then 
   local trainIds = {} 
   if time == "07:47:00" then
	   params:set_DisruptedPlatforms("NE5","NE8","NE_1")	   
	   params:set_DisruptedPlatforms("NE8","NE5","NE_2")
	   
   end
   local disruptedPlatformsSize_firstLine=params:get_DisruptedPlatformsSize("NE_1")
   local disruptedPlatformsSize_secLine=params:get_DisruptedPlatformsSize("NE_2")
   local size=params:get_ActiveTrainsSize("NE_1")
   if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_ActiveTrainByIndex(i,"NE_1")

		   local comingPlatform=params:get_nextPlatform(trainId,"NE_1")
		   local nextPlatform=params:get_Platform_By_Offset(trainId,"NE_1",1)

		   
		   for j=0,disruptedPlatformsSize_firstLine-1 do
		        if comingPlatform == params:get_DisruptedPlatforms("NE_1",j) then
				
				local forcereleasestatus=params:get_ForceAlightStatus(trainId,"NE_1")				
				if forcereleasestatus==false then--if already force alighted then no need to ,need to unset force alight status first    
					params:force_Release_Passenegers(trainId,"NE_1",true)
				end
			   
			-- do something ignore safe distane bring trains as close if stranded between platform
                          if params:get_NextRequested(trainId,"NE_1") ==5 then

                           local nextTrainId=params:get_TrainId_TrainAhead(trainId,"NE_1")
			
			   if nextTrainId~=-1 then
				local nextRequested=params:get_NextRequested(nextTrainId,"NE_1")
				if nextRequested==1 or nextRequested==2 then
				   if params:get_nextPlatform(nextTrainId,"NE_1") == comingPlatform then
					--ignore safe distance and safe headway
				      params:set_IgnoreSafeDistance(trainId,"NE_1",true)
				      params:set_IgnoreSafeHeadway(trainId,"NE_1",true)
				   end 
				end 			
			   end
			

			end

			elseif nextPlatform == params:get_DisruptedPlatforms("NE_1",j) then
			       if params:get_ForceAlightStatus(trainId,"NE_1")==false then    			
			         params:force_Release_Passenegers(trainId,"NE_1",true)
			       end
			       params:setUnset_Uturn(trainId,"NE_1",true)
			  -- do something take Uturn
						
			end
		  		  
		 end
		   

		  -- trainIds[i]=trainId
 
end
end
   size=params:get_ActiveTrainsSize("NE_2")
   if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_ActiveTrainByIndex(i,"NE_2")

		   local comingPlatform=params:get_nextPlatform(trainId,"NE_2")
		   local nextPlatform=params:get_Platform_By_Offset(trainId,"NE_2",1)

		   
		   for j=0,disruptedPlatformsSize_secLine-1 do
		        if comingPlatform == params:get_DisruptedPlatforms("NE_2",j) then
				
				local forcereleasestatus=params:get_ForceAlightStatus(trainId,"NE_2")				
				if forcereleasestatus==false then--if already force alighted then no need to ,need to unset force alight status first    
					params:force_Release_Passenegers(trainId,"NE_2",true)
				end
			   
			-- do something ignore safe distane bring trains as close if stranded between platform
                          if params:get_NextRequested(trainId,"NE_2") ==5 then

                           local nextTrainId=params:get_TrainId_TrainAhead(trainId,"NE_2")
			
			   if nextTrainId~=-1 then
				local nextRequested=params:get_NextRequested(nextTrainId,"NE_2")
				if nextRequested==1 or nextRequested==2 then
				   if params:get_nextPlatform(nextTrainId,"NE_2") == comingPlatform then
					--ignore safe distance and safe headway
				      params:set_IgnoreSafeDistance(trainId,"NE_2",true)
				      params:set_IgnoreSafeHeadway(trainId,"NE_2",true)
				   end 
				end 			
			   end
			

			end

			elseif nextPlatform == params:get_DisruptedPlatforms("NE_2",j) then
			       if params:get_ForceAlightStatus(trainId,"NE_2")==false then    			
			         params:force_Release_Passenegers(trainId,"NE_2",true)
			       end
			       params:setUnset_Uturn(trainId,"NE_2",true)
			  -- do something take Uturn
						
			end
		  		  
		 end
		   

		  -- trainIds[i]=trainId
 
end
end

end
end
