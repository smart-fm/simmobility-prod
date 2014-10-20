from biogeme import *

from headers import *

from loglikelihood import *

from statistics import *

from nested import *

#import random





cons_work= Beta('cons for work', 0,-10,10,0)

cons_edu = Beta('cons for education',0,-50,10,0)

cons_shopping = Beta('cons for shopping',0,-10,10,0)

cons_other = Beta('cons for other',0,-10,10,0)

cons_Q = Beta('cons for quit',0,-10,10,1)



first_stop_inbound= Beta('dummy for first stop of inbound half tour', 0,-10,10,1)

second_stop_inbound= Beta('dummy for second stop of inbound half tour',0,-10,10,0)

threeplus_stop_inbound=Beta('dummy for 3+ stop of inbound half tour',0,-10,10,0)

first_stop_outbound= Beta('dummy for first stop of outbound half tour', 0,-10,10,0)

second_stop_outbound= Beta('dummy for second stop of outbound half tour',0,-10,10,0)

threeplus_stop_outbound=Beta('dummy for 3+ stop of outbound half tour',0,-10,10,0)



work_tour_dummy_Q=Beta('work tour dummy in quit',0,-10,10,1)

edu_tour_dummy_Q=Beta('edu tour dummy in quit',0,-10,10,1)

shopping_tour_dummy_Q=Beta('shopping tour dummy in quit',0,-10,10,1)

other_tour_dummy_Q=Beta('other tour dummy in quit',0,-10,10,1)



first_tour_dummy_Q=Beta('first tour dummy in quit',0,-10,10,0)



sub_tour_dummy_Q=Beta('has subtour dummy in quit',0,-10,10,0)



zero_tour_remain_Q=Beta('zero tour remain dummy',0,-10,10,1)

one_tour_remain_Q=Beta('one tour remain dummy',0,-10,10,0)

twoplus_tour_remain_Q=Beta('2+ tour remain dummy',0,-10,10,1)





work_tour_dummy_W=Beta('work tour dummy in work',0,-10,10,1)

edu_tour_dummy_W=Beta('edu tour dummy in work',0,-10,10,1)

shopping_tour_dummy_W=Beta('shopping tour dummy in work',0,-10,10,1)

other_tour_dummy_W=Beta('other tour dummy in work',0,-10,10,1)

female_dummy_W=Beta('female dummy in work',0,-10,10,0)

student_dummy_W=Beta('student dummy in work',0,-10,10,1)

worker_dummy_W=Beta('worker dummy in work',0,-10,10,1)

driver_dummy_W=Beta('driver dummy in work',0,-10,10,0)

passenger_dummy_W=Beta('passenger dummy in work',0,-10,10,0)

public_dummy_W=Beta('PT dummy in work',0,-10,10,0)



work_tour_dummy_E=Beta('work tour dummy in edu',0,-10,10,1)

edu_tour_dummy_E=Beta('edu tour dummy in edu',0,-10,10,1)

shopping_tour_dummy_E=Beta('shopping tour dummy in edu',0,-10,10,1)

other_tour_dummy_E=Beta('other tour dummy in edu',0,-10,10,1)

female_dummy_E=Beta('female dummy in edu',0,-10,10,0)

student_dummy_E=Beta('student dummy in edu',0,-10,10,1)

worker_dummy_E=Beta('worker dummy in edu',0,-10,10,1)

driver_dummy_E=Beta('driver dummy in edu',0,-10,10,0)

passenger_dummy_E=Beta('passenger dummy in edu',0,-10,10,0)

public_dummy_E=Beta('PT dummy in edu',0,-10,10,0)





work_tour_dummy_S=Beta('work tour dummy in shopping',0,-10,10,1)

edu_tour_dummy_S=Beta('edu tour dummy in shopping',0,-10,10,1)

shopping_tour_dummy_S=Beta('shopping tour dummy in shopping',0,-10,10,1)

other_tour_dummy_S=Beta('other tour dummy in shopping',0,-10,10,0)

female_dummy_S=Beta('female dummy in shopping',0,-10,10,0)

student_dummy_S=Beta('student dummy in shopping',0,-10,10,1)

worker_dummy_S=Beta('worker dummy in shopping',0,-10,10,0)

driver_dummy_S=Beta('driver dummy in shopping',0,-10,10,0)

passenger_dummy_S=Beta('passenger dummy in shopping',0,-10,10,0)

public_dummy_S=Beta('PT dummy in shopping',0,-10,10,0)







work_tour_dummy_O=Beta('work tour dummy in other',0,-10,10,0)

edu_tour_dummy_O=Beta('edu tour dummy in other',0,-10,10,0)

shopping_tour_dummy_O=Beta('shopping tour dummy in other',0,-10,10,0)

other_tour_dummy_O=Beta('other tour dummy in other',0,-10,10,1)

female_dummy_O=Beta('female dummy in other',0,-10,10,0)

student_dummy_O=Beta('student dummy in other',0,-10,10,0)

worker_dummy_O=Beta('worker dummy in other',0,-10,10,0)

driver_dummy_O=Beta('driver dummy in other',0,-10,10,0)

passenger_dummy_O=Beta('passenger dummy in other',0,-10,10,0)

public_dummy_O=Beta('PT dummy in other',0,-10,10,0)



work_logsum=Beta('work logsum in work',0,-10,10,1)

edu_logsum=Beta('edu logsum in edu',0,-10,10,1)

shop_logsum=Beta('shop logsum in shop',0,-10,10,1)

other_logsum=Beta('other logsum in other',0,-10,10,1)





time_window_work=Beta('time available in work',0,-10,10,1)

time_window_edu= Beta('time available in edu',0,-10,10,1)

time_window_shopping= Beta('time available in shopping',0,-10,10,1)

time_window_other= Beta('time available in other',0,-10,10,1)



tour_distance_work= Beta('log tour distance in work',0,-10,10,0)

tour_distance_edu= Beta('log tour distance in edu',0,-10,10,0)

tour_distance_shopping= Beta('log tour distance in shopping',0,-10,10,0)

tour_distance_other=Beta('log tour distance in other',0,-10,10,0)



a700_a930_work= Beta('period 7am to 9:30am in work',0,-10,10,0)

a930_a1200_work=Beta('period 9:30am to 12pm in work',0,-10,10,0)

p300_p530_work=Beta('period 3pm to 5:30pm in work',0,-10,10,0)

p530_p730_work=Beta('period 5:30pm to 7:30 pm in work',0,-10,10,0)

p730_p1000_work=Beta('period 7:30pm to 10pm in work',0,-10,10,0)

p1000_a700_work=Beta('period 10pm to 7am in work',0,-10,10,0)



a700_a930_edu= Beta('period 7am to 9:30am in edu',0,-10,10,0)

a930_a1200_edu=Beta('period 9:30am to 12pm in edu',0,-10,10,0)

p300_p530_edu=Beta('period 3pm to 5:30pm in edu',0,-10,10,0)

p530_p730_edu=Beta('period 5:30pm to 7:30 pm in edu',0,-10,10,0)

p730_p1000_edu=Beta('period 7:30pm to 10pm in edu',0,-10,10,0)

p1000_a700_edu=Beta('period 10pm to 7am in edu',0,-10,10,0)



a700_a930_shopping= Beta('period 7am to 9:30am in shopping',0,-10,10,0)

a930_a1200_shopping=Beta('period 9:30am to 12pm in shopping',0,-10,10,0)

p300_p530_shopping=Beta('period 3pm to 5:30pm in shopping',0,-10,10,0)

p530_p730_shopping=Beta('period 5:30pm to 7:30 pm in shopping',0,-10,10,0)

p730_p1000_shopping=Beta('period 7:30pm to 10pm in shopping',0,-10,10,0)

p1000_a700_shopping=Beta('period 10pm to 7am in shopping',0,-10,10,0)



a700_a930_other= Beta('period 7am to 9:30am in other',0,-10,10,0)

a930_a1200_other=Beta('period 9:30am to 12pm in other',0,-10,10,0)

p300_p530_other=Beta('period 3pm to 5:30pm in other',0,-10,10,0)

p530_p730_other=Beta('period 5:30pm to 7:30 pm in other',0,-10,10,0)

p730_p1000_other=Beta('period 7:30pm to 10pm in other',0,-10,10,0)

p1000_a700_other=Beta('period 10pm to 7am in other',0,-10,10,0)



MU1 = Beta('MU for quit',1,0,100,1)

MU2 = Beta('MU for non-quit', 1.0,0,100,1)



#V for work

V_work= cons_work+\
work_tour_dummy_W*1*(tour_type==1)+\
edu_tour_dummy_W*1*(tour_type==2)+\
shopping_tour_dummy_W*1*(tour_type==3)+\
other_tour_dummy_W*1*(tour_type==4)+\
female_dummy_W*female_dummy+\
student_dummy_W*student_dummy+\
worker_dummy_W*worker_dummy+\
driver_dummy_W*driver_dummy+\
passenger_dummy_W*passenger_dummy+\
public_dummy_W*public_dummy+\
work_logsum * worklogsum+\
time_window_work*time_window_h+\
tour_distance_work*log(1+distance)+\
a700_a930_work*p_700a_930a+\
a930_a1200_work*p_930a_1200a+\
p300_p530_work*p_300p_530p+\
p530_p730_work*p_530p_730p+\
p730_p1000_work*p_730p_1000p+\
p1000_a700_work*p_1000p_700a





#V for education

V_edu = cons_edu+\
work_tour_dummy_E*1*(tour_type==1)+\
edu_tour_dummy_E*1*(tour_type==2)+\
shopping_tour_dummy_E*1*(tour_type==3)+\
other_tour_dummy_E*1*(tour_type==4)+\
female_dummy_E*female_dummy+\
student_dummy_E*student_dummy+\
worker_dummy_E*worker_dummy+\
driver_dummy_E*driver_dummy+\
passenger_dummy_E*passenger_dummy+\
public_dummy_E*public_dummy+\
edu_logsum * edulogsum+\
time_window_edu*time_window_h+\
tour_distance_edu*log(1+distance)+\
a700_a930_edu*p_700a_930a+\
a930_a1200_edu*p_930a_1200a+\
p300_p530_edu*p_300p_530p+\
p530_p730_edu*p_530p_730p+\
p730_p1000_edu*p_730p_1000p+\
p1000_a700_edu*p_1000p_700a


#V for shopping

V_shopping = cons_shopping+\
work_tour_dummy_S*1*(tour_type==1)+\
edu_tour_dummy_S*1*(tour_type==2)+\
shopping_tour_dummy_S*1*(tour_type==3)+\
other_tour_dummy_S*1*(tour_type==4)+\
female_dummy_S*female_dummy+\
student_dummy_S*student_dummy+\
worker_dummy_S*worker_dummy+\
driver_dummy_S*driver_dummy+\
passenger_dummy_S*passenger_dummy+\
public_dummy_S*public_dummy+\
shop_logsum * shoplogsum+\
time_window_shopping*time_window_h+\
tour_distance_shopping*log(1+distance)+\
a700_a930_shopping*p_700a_930a+\
a930_a1200_shopping*p_930a_1200a+\
p300_p530_shopping*p_300p_530p+\
p530_p730_shopping*p_530p_730p+\
p730_p1000_shopping*p_730p_1000p+\
p1000_a700_shopping*p_1000p_700a

#V for other

V_other=cons_other+\
work_tour_dummy_O*1*(tour_type==1)+\
edu_tour_dummy_O*1*(tour_type==2)+\
shopping_tour_dummy_O*1*(tour_type==3)+\
other_tour_dummy_O*1*(tour_type==4)+\
female_dummy_O*female_dummy+\
student_dummy_O*student_dummy+\
worker_dummy_O*worker_dummy+\
driver_dummy_O*driver_dummy+\
passenger_dummy_O*passenger_dummy+\
public_dummy_O*public_dummy+\
other_logsum * otherlogsum+\
time_window_other*time_window_h+\
tour_distance_other*log(1+distance)+\
a700_a930_other*p_700a_930a+\
a930_a1200_other*p_930a_1200a+\
p300_p530_other*p_300p_530p+\
p530_p730_other*p_530p_730p+\
p730_p1000_other*p_730p_1000p+\
p1000_a700_other*p_1000p_700a



#V for quit

V_quit= cons_Q+first_stop_inbound*first_stop*first_bound+\
second_stop_inbound*second_stop*first_bound+\
threeplus_stop_inbound*three_plus_stop*first_bound+\
first_stop_outbound*first_stop*second_bound+\
second_stop_outbound*second_stop*second_bound+\
threeplus_stop_outbound*three_plus_stop*second_bound+\
work_tour_dummy_Q*1*(tour_type==1)+\
edu_tour_dummy_Q*1*(tour_type==2)+\
shopping_tour_dummy_Q*1*(tour_type==3)+\
other_tour_dummy_Q*1*(tour_type==4)+\
first_tour_dummy_Q*first_tour_dummy+\
sub_tour_dummy_Q*has_subtour+zero_tour_remain_Q*1*(tour_remain==0)+\
one_tour_remain_Q*1*(tour_remain==1)+twoplus_tour_remain_Q*1*(tour_remain>=2)

V = {0:V_quit,1: V_work,2:V_edu,3:V_shopping,4:V_other}

av= {0:avail_quit,1:avail_workstop,2:avail_edustop,3:avail_shopstop,4:avail_otherstop}



nest_quit = MU1 , [0]

nest_nonquit = MU2 , [1,2,3,4]

nests=nest_quit,nest_nonquit





prob = nested(V,av,nests,stop_type)

#prob = bioLogit(V,av,stop_type)

rowIterator('obsIter') 

BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')



exclude = ((avail_violation==1)+(origin_mtz==0)+(destination_mtz==0)+(time_window_h>=10)) > 0



BIOGEME_OBJECT.EXCLUDE = exclude

nullLoglikelihood(av,'obsIter')

choiceSet = [0,1,2,3,4]

cteLoglikelihood(choiceSet,stop_type,'obsIter')

availabilityStatistics(av,'obsIter')

BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = "CFSQP"

BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = "1"

BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = "6"