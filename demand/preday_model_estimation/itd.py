from math import sin,cos,pi
from biogeme import * 
from headers import * 
from loglikelihood import * 
from statistics import * 
###define k for trigonometric function, n for #covariates and ps for power series of dur####
k=4
n=4
ps=3

begin=range(1,49) 
end=range(1,49) 
choiceset=range(1,49)

arrmidpoint=[(i*0.5+2.75) for i in begin]
depmidpoint=[(i*0.5+2.75) for i in end]

###combination of arrival-departure
combs=[] 
for i in begin: 
    for j in end: 
        if j>=i: 
            combs.append(str(i)+'_'+str(j))

for i in range(1,n+1):
    for j in range(1,k+1):
        exec("ARR_%s_%s = Beta('ARR_%s_sin%spi',0,-100,100,0)" % (i,j,i,2*j))
        exec("ARR_%s_%s = Beta('ARR_%s_cos%spi',0,-100,100,0)" % (i,k+j,i,2*j))
        exec("DEP_%s_%s = Beta('DEP_%s_sin%spi',0,-100,100,0)" % (i,j,i,2*j))
        exec("DEP_%s_%s = Beta('DEP_%s_cos%spi',0,-100,100,0)" % (i,k+j,i,2*j))
for i in range(1,ps+1):
    exec("DUR_%s_work = Beta('DUR_%s_work',0,-50,50,0)" % (i,i))
    exec("DUR_%s_edu = Beta('DUR_%s_edu',0,-50,50,0)" % (i,i))
    exec("DUR_%s_shopping = Beta('DUR_%s_shopping',0,-50,50,0)" % (i,i))
    exec("DUR_%s_other = Beta('DUR_%s_other',0,-50,50,0)" % (i,i))
TT = Beta('TT',0,-100,100,0)
C_1 = Beta('Cost over income',0,-100,100,0)
C_2=  Beta('Cost with missing income',0,-100,100,0)
####to define covariates######
#nonftemp=DefineVariable("nonftemp",PersonTypeIndex!=1)
#worktype=DefineVariable("worktype",occupation_id!=1&occupation_id!=2&occupation_id!=3)
#omrt=DefineVariable("omrt",num_remaining_tours>=1)
#omsp=DefineVariable("omsp",num_stop_purpose>=1)


####trigonometric functions for schedule disutility###
for i in range(1,n+1):
    arrtobeexec="def sarr_%s(t):\n\treturn " % i
    for j in range(1,k+1):
        arrtobeexec+="first_bound*ARR_%s_%s * sin(%s*pi*t/24.) + first_bound*ARR_%s_%s * cos(%s*pi*t/24.)" % (i,j,2*j,i,k+j,2*j)
        if j!=k: arrtobeexec+='+'
    exec(arrtobeexec)

    deptobeexec="def sdep_%s(t):\n\treturn " % i
    for j in range(1,k+1):
        deptobeexec+="second_bound*DEP_%s_%s * sin(%s*pi*t/24.) + second_bound*DEP_%s_%s * cos(%s*pi*t/24.)" % (i,j,2*j,i,k+j,2*j)
        if j!=k: deptobeexec+='+'
    exec(deptobeexec)

V={}
work_stop_dummy=1*(stop_type==1)
edu_stop_dummy=1*(stop_type==2)
shopping_stop_dummy=1*(stop_type==3)
other_stop_dummy=1*(stop_type==4)

for count in range(1,49):
    arr=arrmidpoint[count-1]
    dep=depmidpoint[count-1]
    dur=first_bound*(high_tod-count+1)+second_bound*(count-low_tod+1)
    dur=0.25+(dur-1)/2.
    #see if in peak hour
    V[count]=eval("sarr_1(%s) + sdep_1(%s) + work_stop_dummy*(DUR_1_work * %s + DUR_2_work * %s**2 + DUR_3_work * %s**3)+ edu_stop_dummy*(DUR_1_edu * %s + DUR_2_edu * %s**2 + DUR_3_edu * %s**3)+shopping_stop_dummy*(DUR_1_shopping * %s + DUR_2_shopping * %s**2 + DUR_3_shopping * %s**3)+other_stop_dummy*(DUR_1_other * %s + DUR_2_other * %s**2 + DUR_3_other * %s**3)" % (arr,dep,dur,dur,dur,dur,dur,dur,dur,dur,dur,dur,dur,dur) + "+ TT * TT_%s" %count + "+ (1-missing_income_dummy)*C_1 * cost_%s/(0.5+income_mid)+missing_income_dummy*C_2*cost_%s/n" %(count,count))

av=dict(zip(choiceset,[eval('avail_%s' %i) for i in choiceset]))
#av=dict(zip(choiceset,[1 for i in choiceset]))
prob = bioLogit(V,av,choice_tod)
rowIterator('obsIter')
BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')
exclude = ((choice_tod==0)and (tod_violation_after==1))>0 
BIOGEME_OBJECT.EXCLUDE = exclude
BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = '4'
BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = 'CFSQP'
BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = '0'
BIOGEME_OBJECT.PARAMETERS['moreRobustToNumericalIssues'] = '0'
