from biogeme import *
from headers import *
from loglikelihood import *
from statistics import *
from nested import *
# rowid	H1_HHID	H1_Pcode	Pax_ID	day_pattern_code	In_day_pattern_choice_set	modified_code	begin_in_tour_table	End_in_tour_table	simple_day_pattern	universitystudent	person_type_id	age_id	income_id	incmid	missingincome	fixedworkplace	missingworkplace	female_dummy	HH_with_under_4	HH_with_under_15	HH_all_adults	HH_all_workers	work_at_home_dummy	hh_car_avail	hh_motor_avail	1_tour_purpose	2_tour_purpose	3_tour_purpose	Work	Edu	Shopping	Others		

# Day Pattern ID #28 is No Tours.

# Base characteristics for subjects are as follows:
# Employed Full Time - person_type ID = 1
# Male
# No Children under age 15
# Family Household - All adults and All workers
# Age 36-50


#Person type
fulltime = (person_type_id == 1) #Base
parttime = (person_type_id == 2)
selfemployed = (person_type_id == 3)
universitystudent = (person_type_id == 4) * (universitystudent == 1)
homemaker = (person_type_id == 5)
retired = (person_type_id == 6)
unemployed = (person_type_id == 7)
nationalservice = (person_type_id == 8)
voluntary =  (person_type_id == 9)
domestic =  (person_type_id == 10)
otherworker = (person_type_id == 12)
student16 = (person_type_id == 4) * (age_id == 3)
student515 = (person_type_id == 4) * ((age_id == 1) + (age_id == 2))
child4 = (age_id == 0) #Under 4 years old

#Adult age group
age20 = (age_id < 4) #Do not include
age2025 = (age_id == 4)
age2635 = (age_id == 5) + (age_id == 6)
age3650 = (age_id == 7) + (age_id == 8) + (age_id == 9) #Base 
age5165 = (age_id == 10) + (age_id == 11) + (age_id == 12)
age65 = (age_id > 12)

#Adult gender/children
#HH_with_under_4 is the number of children under 4 in household
#However, HH_with_under_15 is a binary variable
maleage4 = (female_dummy == 0) * (HH_with_under_4 >= 1)
maleage515 = (female_dummy == 0) * (HH_with_under_15 ==1) * (HH_with_under_4 == 0)
malenone = (female_dummy == 0) * (HH_all_adults == 1) #Base
femalenone = (female_dummy == 1) * (HH_all_adults == 1)
femaleage4 = (female_dummy == 1) * (HH_with_under_4 >= 1)
femaleage515 = (female_dummy == 1) *(HH_with_under_15 == 1) * (HH_with_under_4 == 0) 

#Household composition
onlyadults = (HH_all_adults == 1)
onlyworkers = (HH_all_workers == 1)
onlyadultsworkers = (HH_all_adults == 1) * (HH_all_workers == 1) #Base
#non-family 2+ person HH is not possible

#Personal Income
income = incmid * (1 - missingincome)

#Others
workathome = (work_at_home_dummy == 1)
caravail = (hh_car_avail >= 1)
motoravail = (hh_motor_avail >= 1)
	


#Parameters
bound=100
#Person type
beta_parttime_other_2 = Beta('beta_parttime_other_2',0,-bound,bound,0)
beta_parttime_other_3 = Beta('beta_parttime_other_3',0,-bound,bound,0)

beta_selfemployed_other_2 = Beta('beta_selfemployed_other_2',0,-bound,bound,0)
beta_selfemployed_other_3 = Beta('beta_selfemployed_other_3',0,-bound,bound,0)

beta_universitystudent_other_2 = Beta('beta_universitystudent_other_2',0,-bound,bound,1)
beta_universitystudent_other_3 = Beta('beta_universitystudent_other_3',0,-bound,bound,1)

beta_homemaker_other_2 = Beta('beta_homemaker_other_2',0,-bound,bound,0)
beta_homemaker_other_3 = Beta('beta_homemaker_other_3',0,-bound,bound,1)

beta_retired_other_2 = Beta('beta_retired_other_2',0,-bound,bound,0)
beta_retired_other_3 = Beta('beta_retired_other_3',0,-bound,bound,0)

beta_unemployed_other_2 = Beta('beta_unemployed_other_2',0,-bound,bound,0)
beta_unemployed_other_3 = Beta('beta_unemployed_other_3',0,-bound,bound,1)

beta_nationalservice_other_2 = Beta('beta_nationalservice_other_2',0,-bound,bound,1)
beta_nationalservice_other_3 = Beta('beta_nationalservice_other_3',0,-bound,bound,1)

beta_voluntary_other_2 = Beta('beta_voluntary_other_2',0,-bound,bound,1)
beta_voluntary_other_3 = Beta('beta_voluntary_other_3',0,-bound,bound,1)

beta_domestic_other_2 = Beta('beta_domestic_other_2',0,-bound,bound,1)
beta_domestic_other_3 = Beta('beta_domestic_other_3',0,-bound,bound,1)

beta_otherworker_other_2 = Beta('beta_otherworker_other_2',0,-bound,bound,1)
beta_otherworker_other_3 = Beta('beta_otherworker_other_3',0,-bound,bound,1)

beta_student16_other_2 = Beta('beta_student16_other_2',0,-bound,bound,1)
beta_student16_other_3 = Beta('beta_student16_other_3',0,-bound,bound,1)

beta_student515_other_2 = Beta('beta_student515_other_2',0,-bound,bound,1)
beta_student515_other_3 = Beta('beta_student515_other_3',0,-bound,bound,1)

beta_child4_other_2 = Beta('beta_child4_other_2',0,-bound,bound,1)
beta_child4_other_3 = Beta('beta_child4_other_3',0,-bound,bound,1)

#Adult age group

beta_age2025_other_2 = Beta('beta_age2025_other_2',0,-bound,bound,1)
beta_age2025_other_3 = Beta('beta_age2025_other_3',0,-bound,bound,1)

beta_age2635_other_2 = Beta('beta_age2635_other_2',0,-bound,bound,1)
beta_age2635_other_3 = Beta('beta_age2635_other_3',0,-bound,bound,1)


beta_age5165_other_2 = Beta('beta_age5165_other_2',0,-bound,bound,1)
beta_age5165_other_3 = Beta('beta_age5165_other_3',0,-bound,bound,1)

#Adult gender/children

beta_maleage4_other_2 = Beta('beta_maleage4_other_2',0,-bound,bound,0)
beta_maleage4_other_3 = Beta('beta_maleage4_other_3',0,-bound,bound,0)

beta_maleage515_other_2 = Beta('beta_maleage515_other_2',0,-bound,bound,0)
beta_maleage515_other_3 = Beta('beta_maleage515_other_3',0,-bound,bound,0)

beta_femalenone_other_2 = Beta('beta_femalenone_other_2',0,-bound,bound,0)
beta_femalenone_other_3 = Beta('beta_femalenone_other_3',0,-bound,bound,1)

beta_femaleage4_other_2 = Beta('beta_femaleage4_other_2',0,-bound,bound,0)
beta_femaleage4_other_3 = Beta('beta_femaleage4_other_3',0,-bound,bound,1)

beta_femaleage515_other_2 = Beta('beta_femaleage515_other_2',0,-bound,bound,0)
beta_femaleage515_other_3 = Beta('beta_femaleage515_other_3',0,-bound,bound,1)

#Household composition
beta_onlyadults_other_2 = Beta('beta_onlyadults_other_2',0,-bound,bound,0)
beta_onlyadults_other_3 = Beta('beta_onlyadults_other_3',0,-bound,bound,1)

beta_onlyworkers_other_2 = Beta('beta_onlyworkers_other_2',0,-bound,bound,0)
beta_onlyworkers_other_3 = Beta('beta_onlyworkers_other_3',0,-bound,bound,1)

#Personal income
beta_income_other_2 = Beta('beta_income_other_2',0,-bound,bound,1)
beta_income_other_3 = Beta('beta_income_other_3',0,-bound,bound,1)


#Others
beta_workathome_other_2 = Beta('beta_workathome_other_2',0,-bound,bound,1)
beta_workathome_other_3 = Beta('beta_workathome_other_3',0,-bound,bound,1)

beta_caravail_other_2 = Beta('beta_caravail_other_2',0,-bound,bound,0)
beta_caravail_other_3 = Beta('beta_caravail_other_3',0,-bound,bound,1)

beta_motoravail_other_2 = Beta('beta_motoravail_other_2',0,-bound,bound,0)
beta_motoravail_other_3 = Beta('beta_motoravail_other_3',0,-bound,bound,1)

beta_logsum_other_2=Beta('beta_logsum_other_2',0,-bound,bound,0)
beta_logsum_other_3=Beta('beta_logsum_other_3',0,-bound,bound,0)

beta_cons_other_2=Beta('beta_cons_other_2',0,-bound,bound,0)
beta_cons_other_3=Beta('beta_cons_other_3',0,-bound,bound,0)

#Choice set
counter = 0
choiceset = range(1,4)

#MU1= Beta('MU for 1 tour',1,0,100,1)
#MU2plus= Beta('MU for 2+ tour',1,0,100,1)


counter = counter + 1
exec("V_%s = 0" % (counter))

for i in range(2,4):
    counter = counter + 1
    exec("V_%s =  beta_cons_other_%s+beta_parttime_other_%s * parttime + beta_selfemployed_other_%s * selfemployed +\
        beta_universitystudent_other_%s * universitystudent + beta_homemaker_other_%s * homemaker +\
        beta_retired_other_%s * retired + beta_unemployed_other_%s * unemployed +\
        beta_nationalservice_other_%s * nationalservice + beta_voluntary_other_%s * voluntary +\
        beta_domestic_other_%s * domestic + beta_otherworker_other_%s * otherworker +\
        beta_student16_other_%s * student16 + beta_student515_other_%s * student515 +\
        beta_child4_other_%s * child4 + beta_age2025_other_%s * age2025 +\
        beta_age2635_other_%s * age2635 + beta_age5165_other_%s * age5165 +\
        beta_maleage4_other_%s * maleage4 + beta_maleage515_other_%s * maleage515 +\
        beta_femalenone_other_%s * femalenone + beta_femaleage4_other_%s * femaleage4 +\
        beta_femaleage515_other_%s * femaleage515 + beta_onlyadults_other_%s * onlyadults +\
        beta_onlyworkers_other_%s * onlyworkers + beta_income_other_%s * income +\
        beta_workathome_other_%s * workathome +\
        beta_caravail_other_%s * caravail +\
        beta_motoravail_other_%s * motoravail + beta_logsum_other_%s*new_otherlogsum" % ((counter,)*30))
V =dict(zip(range(1,4),[eval('V_%s' %i) for i in choiceset]))
av={1:1,2:1,3:1}

#one=MU1,[1]
#twoplus=MU2plus, [2,3]
#nests=one,twoplus

prob = bioLogit(V,av,other_tour)
#prob=nested(V,av,nests,other_tour)

rowIterator('obsIter')
BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')
exclude = ((oldpattern == 0) + (other_tour<=0)+(other_tour>3))
BIOGEME_OBJECT.EXCLUDE = exclude
BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = '4'
BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = 'CFSQP'
BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = '0'
BIOGEME_OBJECT.PARAMETERS['moreRobustToNumericalIssues'] = '0'



