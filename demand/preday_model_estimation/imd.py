from biogeme import *
from headers import *
from loglikelihood import *
from statistics import *
from nested import *

#beta_cost_bus_mrt_1= Beta('bus/mrt cost over income',0,-10,10,0)
#beta_cost_mrt= Beta('MRT cost',0,-10,10,0)
#beta_cost_private_bus_1 =Beta('private bus cost over income',0,-10,10,0)
#beta_cost_drive1_1= Beta('drive1 cost over income',0,-10,10,0)
#beta_cost_share2_1= Beta('shared ride 2 cost over income',0,-10,10,0)
#beta_cost_share3_1= Beta('shared ride 3 cost over income',0,-10,10,0)
#beta_cost_motor_1 = Beta('motor cost over income',0,-10,10,0)
#beta_cost_taxi_1 = Beta('taxi cost over income',0,-10,10,0)

beta_cost_bus_mrt_2= Beta('bus/mrt cost',0,-10,10,0)
#beta_cost_mrt= Beta('MRT cost',0,-10,10,0)
beta_cost_private_bus_2 =Beta('private bus cost',0,-10,10,0)

beta_cost_drive1_2_first= Beta('drive1 cost first',0,-10,10,0)
beta_cost_drive1_2_second= Beta('drive1 cost second',0,-10,10,0)

beta_cost_share2_2_first= Beta('shared ride 2 cost first',0,-10,10,1)
beta_cost_share2_2_second= Beta('shared ride 2 cost second',0,-10,10,1)

beta_cost_share3_2_first= Beta('shared ride 3 cost first',0,-10,10,1)
beta_cost_share3_2_second = Beta('shared ride 3 cost second',0,-10,10,1)

beta_cost_motor_2 = Beta('motor cost',0,-10,10,0)

beta_cost_taxi_2_first = Beta('taxi cost first',0,-10,10,1)
beta_cost_taxi_2_second = Beta('taxi cost second',0,-10,10,1)



beta_tt_bus_mrt = Beta('bus/mrt travel time',0,-10,10,0)
#beta_tt_mrt= Beta('MRT travel time',0,-10,10,0)
beta_tt_private_bus= Beta('private bus travel time',0,-10,10,1)
beta_tt_drive1_first = Beta('drive1 travel time first',0,-10,10,0)
beta_tt_drive1_second = Beta('drive1 travel time second',0,-10,10,0)
beta_tt_share2= Beta('shared ride 2 travel time',0,-10,10,1)
beta_tt_share3= Beta('shared ride 3 travel time',0,-10,10,1)
beta_tt_motor=Beta('motor travel time',0,-10,10,1)
beta_tt_walk=Beta('walk travel time',0,-10,10,1)
beta_tt_taxi_first= Beta('taxi travel time first',0,-10,10,0)
beta_tt_taxi_second= Beta('taxi travel time second',0,-10,10,0)

beta_work = Beta('work beta',0,-10,10,0)
beta_shop = Beta('shop beta',0,-10,10,0)


beta_central_bus_mrt = Beta('central dummy beta for bus/mrt',0,-10,10,0)
beta_central_private_bus=Beta('central dummy beta for private bus',0,-10,10,0)
beta_central_drive1=Beta('central dummy beta for drive1',0,-10,10,1)
beta_central_share2=Beta('central dummy beta for shared ride 2',0,-10,10,0)
beta_central_share3=Beta('central dummy beta for shared ride 3',0,-10,10,0)
beta_central_motor=Beta('central dummy beta for motor',0,-10,10,0)
beta_central_walk=Beta('central dummy beta for walk',0,-10,10,1)
beta_central_taxi=Beta('central dummy beta for taxi',0,-10,10,0)

beta_distance_bus_mrt = Beta('distance beta for bus/mrt',0,-10,10,0)
beta_distance_private_bus=Beta('distance beta for private bus',0,-10,10,0)
beta_distance_drive1=Beta('distance beta for drive1',0,-10,10,1)
beta_distance_share2=Beta('distance beta for shared ride 2',0,-10,10,0)
beta_distance_share3=Beta('distance beta for shared ride 3',0,-10,10,0)
beta_distance_motor=Beta('distance beta for motor',0,-10,10,0)
beta_distance_walk=Beta('distance beta for walk',0,-10,10,1)
beta_distance_taxi=Beta('distance beta for taxi',0,-10,10,0)


beta_cons_bus = Beta('constant for bus',0,-10,10,0)
beta_cons_mrt = Beta('constant for mrt',0,-10,10,0)
beta_cons_private_bus = Beta('constant for private bus',0,-10,10,0)
beta_cons_drive1 = Beta('constant for drive1',0,-10,10,1)
beta_cons_share2 = Beta('constant for shared ride 2',0,-10,10,0)
beta_cons_share3 = Beta('constant for shared ride 3',0,-10,10,0)
beta_cons_motor = Beta('constant for motor',0,-10,10,0)
beta_cons_walk = Beta('constant for walk',-30,-100,10,1)
beta_cons_taxi = Beta('constant for taxi',0,-10,10,0)

beta_zero_drive1=Beta('zero cars in drive1',0,-10,10,1)
beta_oneplus_drive1=Beta('one plus cars in drive1',0,-10,10,0)
beta_twoplus_drive1=Beta('two plus cars in drive1',0,-10,10,0)
beta_threeplus_drive1=Beta('three plus cars in drive1',0,-10,30,1)

beta_zero_share2=Beta('zero cars in share2',0,-10,10,1)
beta_oneplus_share2=Beta('one plus cars in share2',0,-10,10,0)
beta_twoplus_share2=Beta('two plus cars in share2',0,-10,10,1)
beta_threeplus_share2=Beta('three plus cars in share2',0,-10,10,1)

beta_zero_share3=Beta('zero cars in share3 plus',0,-10,10,1)
beta_oneplus_share3=Beta('one plus cars in share3 plus',0,-10,10,0)
beta_twoplus_share3=Beta('two plus cars in share3 plus',0,-10,10,1)
beta_threeplus_share3=Beta('three plus cars in share3 plus',0,-30,10,1)


beta_zero_motor=Beta('zero motors in motor',0,-10,10,1)
beta_oneplus_motor=Beta('one plus motors in motor',0,-10,10,0)
beta_twoplus_motor=Beta('two plus motors in motor',0,-10,10,1)
beta_threeplus_motor=Beta('three plus motors in motor',0,-10,10,1)

beta_female_bus=Beta('female dummy in bus',0,-10,10,0)
beta_female_mrt=Beta('female dummy in mrt',0,-10,10,0)
beta_female_private_bus=Beta('female dummy in privatebus',0,-10,10,0)
beta_female_drive1=Beta('female dummy in drive1',0,-10,10,1)
beta_female_share2=Beta('female dummy in share2',0,-10,10,0)
beta_female_share3=Beta('female dummy in share3 plus',0,-10,10,0)
beta_female_motor=Beta('female dummy in motor',0,-10,10,0)
beta_female_taxi=Beta('female dummy in taxi',0,-10,10,0)
beta_female_walk=Beta('female dummy in walk',0,-50,10,1)


MU1 = Beta('MU for all destinations by cars',1,1,100,1)
MU2 = Beta('MU for all destinations by pt', 2.0,1,100,1)
#define cost and travel time

for i in range(1,1093):
	exec("cost_bus_%s = cost_public_%s" % (i,i))
	exec("cost_mrt_%s = cost_public_%s" % (i,i))
	exec("cost_private_bus_%s = cost_public_%s" % (i,i))
	exec("cost_drive1_%s=cost_car_ERP_%s+cost_car_OP_%s+cost_car_parking_%s" % (i,i,i,i))
	exec("cost_share2_%s=(cost_car_ERP_%s+cost_car_OP_%s+cost_car_parking_%s)/2.0" % (i,i,i,i))
	exec("cost_share3_%s=(cost_car_ERP_%s+cost_car_OP_%s+cost_car_parking_%s)/3.0" % (i,i,i,i))
	exec("cost_motor_%s=0.5*cost_car_ERP_%s+0.5*cost_car_OP_%s+0.65*cost_car_parking_%s" % (i,i,i,i))
	exec("cost_taxi_%s=(6.8+cost_car_ERP_%s+6*central_dummy_%s+((walk_distance_first_%s*(walk_distance_first_%s>10)-10*(walk_distance_first_%s>10))/0.35+(walk_distance_first_%s*(walk_distance_first_%s<=10)+10*(walk_distance_first_%s>10))/0.4)*0.22+((walk_distance_second_%s*(walk_distance_second_%s>10)-10*(walk_distance_second_%s>10))/0.35+(walk_distance_second_%s*(walk_distance_second_%s<=10)+10*(walk_distance_second_%s>10))/0.4)*0.22)/2" % (i,i,i,i,i,i,i,i,i,i,i,i,i,i,i))
	exec("cost_over_income_bus_%s=30*cost_bus_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_mrt_%s=30*cost_mrt_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_private_bus_%s=30*cost_private_bus_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_drive1_%s=30*cost_drive1_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_share2_%s=30*cost_share2_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_share3_%s=30*cost_share3_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_motor_%s=30*cost_motor_%s/(0.5+income_mid)" % (i,i))
	exec("cost_over_income_taxi_%s=30*cost_taxi_%s/(0.5+income_mid)" % (i,i))

for i in range(1,1093):
	exec("tt_bus_%s = tt_public_ivt_%s +tt_public_out_%s" % (i,i,i))
	exec("tt_mrt_%s = tt_public_ivt_%s +tt_public_out_%s" % (i,i,i))
	exec("tt_private_bus_%s = tt_car_ivt_%s" % (i,i))
	exec("tt_drive1_%s = tt_car_ivt_%s +1.0/12" % (i,i))
	exec("tt_share2_%s = tt_car_ivt_%s + 1.0/12" % (i,i))
	exec("tt_share3_%s = tt_car_ivt_%s + 1.0/12" % (i,i))
	exec("tt_motor_%s = tt_car_ivt_%s+ 1.0/12" % (i,i))
	exec("tt_walk_%s = (walk_distance_first_%s+walk_distance_second_%s)/5/2" % (i,i,i))
	exec("tt_taxi_%s = tt_car_ivt_%s +1.0/12" % (i,i))


mode=['0','bus','mrt','private_bus','drive1','share2','share3','motor','walk','taxi']
group=['0','bus_mrt','bus_mrt','private_bus','drive1','share2','share3','motor','walk','taxi']

#beta4_1_cost * cost_over_income_bus * (1-missing_income_dummy) + beta4_2_cost * cost_bus *missing_income_dummy

work_stop_dummy=1*(stop_type==1)
edu_stop_dummy=1*(stop_type==2)
shop_stop_dummy=1*(stop_type==3)
other_stop_dummy=1*(stop_type==4)

#beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy

V_counter=0
#bus
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_bus + cost_bus_%s * beta_cost_bus_mrt_2 + tt_bus_%s * beta_tt_bus_mrt+beta_central_bus_mrt * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_bus_mrt + beta_female_bus * female_dummy" % (V_counter,j,j,j,j,j,j,j)) 

#mrt
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_mrt + cost_mrt_%s * beta_cost_bus_mrt_2 + tt_mrt_%s * beta_tt_bus_mrt+beta_central_bus_mrt * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_bus_mrt + beta_female_mrt * female_dummy" % (V_counter,j,j,j,j,j,j,j)) 

#private_bus
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_private_bus + cost_private_bus_%s * beta_cost_private_bus_2 + tt_private_bus_%s * beta_tt_bus_mrt+beta_central_private_bus * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_private_bus + beta_female_private_bus * female_dummy" % (V_counter,j,j,j,j,j,j,j)) 

#drive1
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_drive1 + first_bound * cost_drive1_%s * beta_cost_drive1_2_first + second_bound * cost_drive1_%s * beta_cost_drive1_2_second + first_bound * tt_drive1_%s * beta_tt_drive1_first + second_bound * tt_drive1_%s * beta_tt_drive1_second +beta_central_drive1 * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_drive1 + beta_zero_drive1 * zero_car + beta_oneplus_drive1 * one_plus_car + beta_twoplus_drive1 * two_plus_car + beta_threeplus_drive1 * three_plus_car +  beta_female_drive1 * female_dummy" % (V_counter,j,j,j,j,j,j,j,j,j))

#share2
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_share2 + first_bound * cost_share2_%s * beta_cost_drive1_2_first + second_bound * cost_share2_%s * beta_cost_drive1_2_second + first_bound * tt_share2_%s * beta_tt_drive1_first + second_bound * tt_share2_%s * beta_tt_drive1_second  +  beta_central_share2 * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_share2 + beta_zero_share2 * zero_car + beta_oneplus_share2 * one_plus_car + beta_twoplus_share2 * two_plus_car + beta_threeplus_share2 * three_plus_car + beta_female_share2 * female_dummy" % (V_counter,j,j,j,j,j,j,j,j,j))

#share3
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_share3 + first_bound * cost_share3_%s * beta_cost_drive1_2_first + second_bound * cost_share3_%s * beta_cost_drive1_2_second + first_bound * tt_share3_%s * beta_tt_drive1_first + second_bound * tt_share3_%s * beta_tt_drive1_second +  beta_central_share3 * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_share3 + beta_zero_share3 * zero_car + beta_oneplus_share3 * one_plus_car + beta_twoplus_share3 * two_plus_car + beta_threeplus_share3 * three_plus_car + beta_female_share3 * female_dummy" % (V_counter,j,j,j,j,j,j,j,j,j))

#motor
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_motor + cost_motor_%s * beta_cost_motor_2 + first_bound * tt_motor_%s * beta_tt_drive1_first + second_bound * tt_motor_%s * beta_tt_drive1_second +beta_central_motor * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_motor + beta_zero_motor * zero_motor + beta_oneplus_motor * one_plus_motor + beta_twoplus_motor * two_plus_motor + beta_threeplus_motor * three_plus_motor + beta_female_motor * female_dummy" % (V_counter,j,j,j,j,j,j,j,j))


#walk
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_walk+tt_walk_%s*beta_tt_walk +beta_central_walk * central_dummy_%s+ beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_walk + beta_female_walk * female_dummy" % (V_counter,j,j,j,j,j,j))

#taxi	
for j in range(1,1093):
	V_counter=V_counter+1
	exec("V_%s = beta_cons_taxi + first_bound * cost_taxi_%s * beta_cost_drive1_2_first + second_bound * cost_taxi_%s * beta_cost_drive1_2_second + first_bound * tt_taxi_%s * beta_tt_taxi_first + second_bound * tt_taxi_%s * beta_tt_taxi_second  +  beta_central_taxi * central_dummy_%s + beta_shop * log(1+shop_%s)*shop_stop_dummy+beta_work * log(1+employment_%s)*work_stop_dummy + (walk_distance_first_%s+walk_distance_second_%s)*beta_distance_taxi + beta_female_taxi * female_dummy" % (V_counter,j,j,j,j,j,j,j,j,j)) 



V =dict(zip(range(1,9*1092+1),[eval('V_%s' %i) for i in range(1,9*1092+1)]))
av=dict(zip(range(1,9*1092+1),[eval('avail_%s' %i) for i in range(1,9*1092+1)]))

#a=range(1+1092*0,3*1092+1)
#b=range(3*1092+1,6*1092+1)
#c=range(6*1092+1,9*1092+1)

#car = MU1,b
#PT =  MU2,a 
#other = 1.0,c
#nests = car,PT,other

# The choice model is a nested logit, with availability conditions
#prob = nested(V,av,nests,combined_choice_new)
prob = bioLogit(V,av,combine_choice)


rowIterator('obsIter') 
BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')


exclude = ((combine_choice==0)+(combined_violation==1)+(stop_type==0)) > 0

BIOGEME_OBJECT.EXCLUDE = exclude
#nullLoglikelihood(av,'obsIter')
#cteLoglikelihood(choiceSet,choice,'obsIter')
availabilityStatistics(av,'obsIter')
BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = "CFSQP"
BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = "1"
BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = "6"
