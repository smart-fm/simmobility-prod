
function use_servicecontroller(params,time) 
               
if time  == "08:00:00" then
--[[
local return_table = {}
return_table[1]="NE3_1"
return_table[2]="NE5_2"
params:update_platformlist(1,return_table,"NE_1")

--]]
--params:reset_holdingTime_atstation("NE5_1",100,1,"NE_1")
--params:terminate_singletrainservice(5,"NE_1")
--params:terminate_trainservice("NE_1")

--params:restrict_passengers("NE4_1",1,"NE_1",0)
--params:insert_unscheduledtraintrip("NE_1","07:34:00","NE7")

--params:get_nextPlatform(3,"NE_1")
--params:get_distancetoNextPlatform("NE_1",5)
--params:get_dwelltime(18,"NE_2","NE13")
end

--params:reset_safeoperation_distance("20",1,"NE_1")


--if time  == "07:45:00" then

--params:reset_safeheadway_sec(200,5,"NE_1")
--end

--[[  commented section


   params:reset_speedlimit(30,"CC29/NE1","NE4","NE_1","07:30:00","08:00:00")
   if size ==0 then
   end
   --]]
 
if time =="09:00:00" then
--unset disrupted platforms
--unset Uturn flag
--unset force alight flag
--unset force alight status
--unset ignore safe distance and safe headway
 params:clear_disruption("NE_1")
 params:clear_disruption("NE_2")
 --local disruptedPlatformsSize_firstLine=params:get_disruptedplatformssize("NE_1")
 --local disruptedPlatformsSize_secLine=params:get_disruptedplatformssize("NE_2")
 local size=params:get_active_trains_size("NE_1")
 if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_active_train_by_index(i,"NE_1")
		  params:set_uturn(trainId,"NE_1",false)
		  params:force_release_passenegers(trainId,"NE_1",false)
		  params:set_force_alight_status(trainId,"NE_1",false)
		  params:set_ignore_safe_distance(trainId,"NE_1",false)
                  params:set_ignore_safe_headway(trainId,"NE_1",false)
		  if params:is_stranded_during_disruption(trainId,"NE_1") == true then
			params:set_subsequent_next_requested(trainId,"NE_1",5)
		  elseif params:get_disrupted_state(trainId,"NE_1") == true then
			params:set_subsequent_next_requested(trainId,"NE_1",1)
		  end
	   end
 end

size=params:get_active_trains_size("NE_2")
if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_active_train_by_index(i,"NE_2")
		  params:set_uturn(trainId,"NE_2",false)
		  params:force_release_passenegers(trainId,"NE_2",false)
		  params:set_force_alight_status(trainId,"NE_2",false)
		  params:set_ignore_safe_distance(trainId,"NE_2",false)
                  params:set_ignore_safe_headway(trainId,"NE_2",false)
	          if params:is_stranded_during_disruption(trainId,"NE_2") == true then
			params:set_subsequent_next_requested(trainId,"NE_2",5)
		  elseif params:get_disrupted_state(trainId,"NE_2") == true then
			params:set_subsequent_next_requested(trainId,"NE_2",1)
		  end
	   end
 end



elseif time  >= "08:00:00" and time<"09:00:00" then 
   local trainIds = {} 
   if time == "08:00:00" then
	   params:set_disrupted_platforms("NE10","NE13","NE_1")	   
	   params:set_disrupted_platforms("NE13","NE10","NE_2")
	   --trigger rerouting of passengers
          --[[
	   local disruptedPlatformsSize_firstLine=params:get_disruptedplatformssize("NE_1")
           local disruptedPlatformsSize_secLine=params:get_disruptedplatformssize("NE_2")
           for j=0,disruptedPlatformsSize_firstLine-1 do
	       disruptedPlatform=params:get_disruptedplatforms("NE_1",j)
               params:setRerouting_Passengers(disruptedPlatform)
          --prePlatform=params:get_preplatform(params:get_disruptedplatforms("NE_1",j))
          --params:setRerouting_Passengers(prePlatform)
          --]]   
       	  	   
   end
   local disruptedPlatformsSize_firstLine=params:get_disrupted_platforms_size("NE_1")
   local disruptedPlatformsSize_secLine=params:get_disrupted_platforms_size("NE_2")
   local size=params:get_active_trains_size("NE_1")
   if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_active_train_by_index(i,"NE_1")

		   local comingPlatform=params:get_next_platform(trainId,"NE_1")
		   local nextPlatform=params:get_platform_by_offset(trainId,"NE_1",1)

		   
		   for j=0,disruptedPlatformsSize_firstLine-1 do
		        if comingPlatform == params:get_disruptedplatforms("NE_1",j) then
				
				local forcereleasestatus=params:get_force_alight_status(trainId,"NE_1")				
				if forcereleasestatus==false then--if already force alighted then no need to ,need to unset force alight status first    
					params:force_release_passenegers(trainId,"NE_1",true)
				end
			   
			-- do something ignore safe distane bring trains as close if stranded between platform
                          if params:get_next_requested(trainId,"NE_1") ==5 then

                           local nextTrainId=params:get_trainid_train_ahead(trainId,"NE_1")
			
			   if nextTrainId~=-1 then
				local nextRequested=params:get_next_requested(nextTrainId,"NE_1")
				if nextRequested==1 or nextRequested==2 then
				   if params:get_nextPlatform(nextTrainId,"NE_1") == comingPlatform then
					--ignore safe distance and safe headway
				      params:set_ignore_safe_distance(trainId,"NE_1",true)
				      params:set_ignore_safe_headway(trainId,"NE_1",true)
				   end 
				end 			
			   end
			

			end

			elseif nextPlatform == params:get_disrupted_platforms("NE_1",j) then
			       if params:get_force_alight_status(trainId,"NE_1")==false then    			
			         params:force_release_passenegers(trainId,"NE_1",true)
			       end
			       params:set_uturn(trainId,"NE_1",true)
			  -- do something take Uturn
						
			end
		  		  
		 end
		   

		  -- trainIds[i]=trainId
 
end
end
   size=params:get_active_trains_size("NE_2")
   if size>0 then

 	   local i=0
	   for i = 0,size-1 do 	  
		  local trainId=params:get_active_train_by_index(i,"NE_2")

		   local comingPlatform=params:get_next_platform(trainId,"NE_2")
		   local nextPlatform=params:get_platform_by_offset(trainId,"NE_2",1)
		   for j=0,disruptedPlatformsSize_secLine-1 do
			local tobreak = false
		        if comingPlatform == params:get_disrupted_platforms("NE_2",j) then
				tobreak=true
				local forcereleasestatus=params:get_force_alight_status(trainId,"NE_2")				
				if forcereleasestatus==false then--if already force alighted then no need to ,need to unset force alight status first    
					params:force_release_passenegers(trainId,"NE_2",true)
				end
			   
			-- do something ignore safe distane bring trains as close if stranded between platform
                          if params:get_next_requested(trainId,"NE_2") ==5 then

                           local nextTrainId=params:get_trainId_train_ahead(trainId,"NE_2")
			
			   if nextTrainId~=-1 then
				local nextRequested=params:get_next_requested(nextTrainId,"NE_2")
				if nextRequested==1 or nextRequested==2 then
				   if params:get_next_platform(nextTrainId,"NE_2") == comingPlatform then
					--ignore safe distance and safe headway
				      params:set_ignore_safe_distance(trainId,"NE_2",true)
				      params:set_ignore_safe_headway(trainId,"NE_2",true)
				   end 
				end 			
			   end
			

			end

			elseif nextPlatform == params:get_disrupted_platforms("NE_2",j) then
			       tobreak=true
			       if params:get_force_alight_status(trainId,"NE_2")==false then    			
			         params:force_release_passenegers(trainId,"NE_2",true)
			       end
			       params:set_uturn(trainId,"NE_2",true)
			  -- do something take Uturn
						
			end
			if tobreak == true then
			    break;
			end
		  		  
		 end
		   

		  -- trainIds[i]=trainId
 
end
end

end
end
