from biogeme import *
from headers import *
from nested import *
from loglikelihood import *
from statistics import *
#import random

cons_usual = Beta('cons for usual',0,-10,10,0)
cons_unusual = Beta('cons for unusual',0,-10,10,1)

beta_fixedlocation_usual=Beta('Beta for fixed location dummy in usual',0,-10,10,1)
beta_fixedlocation_unusual=Beta('Beta for fixed location dummy in unusual',0,-10,10,1)

beta_fixedtime_usual=Beta('Beta for fixed time dummy in usual',0,-10,10,0)
beta_fixedtime_unusual=Beta('Beta for fixed time dummy in unusual',0,-10,10,1)

beta_female_usual=Beta('Beta for female dummy in usual',0,-10,10,0)
beta_female_unusual=Beta('Beta for female dummy in unusual',0,-10,10,1)

beta_under3000_usual=Beta('Beta for low income in usual',0,-10,10,1)
beta_under3000_unusual=Beta('Beta for low income in unusual',0,-10,10,1)

beta_distance_log_usual=Beta('Beta for log(distance) in usual',0,-10,10,0)
beta_distance1_usual=Beta('Beta for distance in usual',0,-10,10,1)
beta_distance2_usual=Beta('Beta for distance^2 in usual',0,-10,10,1)
beta_distance3_usual=Beta('Beta for distance^3 in usual',0,-10,10,1)

beta_distance_log_unusual=Beta('Beta for log(distance) in unusual',0,-10,10,1)
beta_distance1_unusual=Beta('Beta for distance in unusual',0,-10,10,1)
beta_distance2_unusual=Beta('Beta for distance^2 in unusual',0,-10,10,1)
beta_distance3_unusual=Beta('Beta for distance^3 in unusual',0,-10,10,1)

beta_employment_full_usual=Beta('Beta for log(1+employment)*full in usual',0,-10,10,0)
beta_employment_full_unusual=Beta('Beta for log(1+employment)*full in unusual',0,-10,10,1)

beta_employment_part_usual=Beta('Beta for log(1+employment)*part in usual',0,-10,10,0)
beta_employment_part_unusual=Beta('Beta for log(1+employment)*part in unusual',0,-10,10,1)

beta_employment_self_usual=Beta('Beta for log(1+employment)*self in usual',0,-10,10,0)
beta_employment_self_unusual=Beta('Beta for log(1+employment)*self in unusual',0,-10,10,1)


beta_work_home_usual=Beta('Beta for work from home dummy in usual',0,-10,10,0)
beta_work_home_unusual=Beta('Beta for work from home dummy in unusual',0,-10,10,1)

beta_first_work_usual=Beta('Beta for first of multiple work tours dummy in usual', 0,-10,10,0)
beta_first_work_unusual=Beta('Beta for first of multiple work tours dummy in unusual', 0,-10,10,1)

beta_sub_work_usual=Beta('Beta for sub of multiple work tours dummy in usual', 0,-10,10,0)
beta_sub_work_unusual=Beta('Beta for first of multiple work tours dummy in unusual', 0,-10,10,1)


binary_choice=go_to_primary_work_location+1 # 1 for unusual 2 for usual
low_income_dummy=1*(IncomeIndex<=5)

distance=max(walk_distance1+walk_distance2,0.1)
employment=log(1+work_op)

full_time_dummy =1 *(person_type_id==1)
part_time_dummy= 1*(person_type_id==2)
self_employed_dummy= 1*(person_type_id==3)

#V1 for unusual
V1 = cons_unusual + beta_fixedlocation_unusual * fixed_place + beta_fixedtime_unusual * fixed_work_hour + beta_female_unusual*Female_dummy+ beta_under3000_unusual*low_income_dummy +beta_distance_log_unusual * log(distance)+beta_distance1_unusual * distance + beta_distance2_unusual * pow(distance,2)+beta_distance3_unusual * pow(distance,3)+ beta_employment_full_unusual * employment *full_time_dummy +  beta_employment_part_unusual * employment *part_time_dummy +beta_employment_self_unusual * employment*self_employed_dummy + beta_work_home_unusual * work_from_home_dummy + beta_first_work_unusual * first_of_multiple + beta_sub_work_unusual * subsequent_of_multiple

#V2 for usual
V2 = cons_usual + beta_fixedlocation_usual * fixed_place + beta_fixedtime_usual * fixed_work_hour + beta_female_usual*Female_dummy + beta_under3000_usual * low_income_dummy +  beta_distance_log_usual * log(distance)+beta_distance1_usual * distance + beta_distance2_usual * pow(distance,2)+beta_distance3_usual * pow(distance,3) +  beta_employment_full_usual * employment *full_time_dummy +  beta_employment_part_usual * employment *part_time_dummy +beta_employment_self_usual * employment*self_employed_dummy + beta_work_home_usual * work_from_home_dummy + beta_first_work_usual * first_of_multiple + beta_sub_work_usual * subsequent_of_multiple


V = {1:V1,2: V2}
av= {1:motor_avail_dummy_all,2:motor_avail_dummy_all}

prob = bioLogit(V,av,binary_choice)

rowIterator('obsIter') 
BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')


exclude = ((choice==0)+(PrimaryActivityIndex!=1)+(fixed_place==0)+(avail_violation==1)+(IncomeIndex==12)) > 0


BIOGEME_OBJECT.EXCLUDE = exclude
nullLoglikelihood(av,'obsIter')
choiceSet = [1,2]
cteLoglikelihood(choiceSet,binary_choice,'obsIter')
availabilityStatistics(av,'obsIter')
BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = "CFSQP"
BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = "1"
BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = "4"
