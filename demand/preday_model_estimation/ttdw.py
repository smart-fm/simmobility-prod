from math import pi,sin,cos 
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
choiceset=range(1,1177)

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
    exec("DUR_%s = Beta('DUR_%s',0,-50,50,0)" % (i,i))

TT1 = Beta('TT1',0,-100,100,1)
TT2 = Beta('TT2',0,-100,100,1)
C = Beta('C',0,-100,100,1)

####to define covariates######
#nonftemp=DefineVariable("nonftemp",PersonTypeIndex!=1)
#worktype=DefineVariable("worktype",occupation_id!=1&occupation_id!=2&occupation_id!=3)
#omrt=DefineVariable("omrt",num_remaining_tours>=1)
#omsp=DefineVariable("omsp",num_stop_purpose>=1)


####trigonometric functions for schedule disutility###
for i in range(1,n+1):
    arrtobeexec="def sarr_%s(t):\n\treturn " % i
    for j in range(1,k+1):
        arrtobeexec+="ARR_%s_%s * sin(%s*pi*t/24.) + ARR_%s_%s * cos(%s*pi*t/24.)" % (i,j,2*j,i,k+j,2*j)
        if j!=k: arrtobeexec+='+'
    exec(arrtobeexec)

    deptobeexec="def sdep_%s(t):\n\treturn " % i
    for j in range(1,k+1):
        deptobeexec+="DEP_%s_%s * sin(%s*pi*t/24.) + DEP_%s_%s * cos(%s*pi*t/24.)" % (i,j,2*j,i,k+j,2*j)
        if j!=k: deptobeexec+='+'
    exec(deptobeexec)

V={}
for count,comb in enumerate(combs):
    arrid=int(comb.split("_")[0])
    depid=int(comb.split("_")[1])
    arr=arrmidpoint[arrid-1]
    dep=depmidpoint[depid-1]
    dur=dep-arr
    #see if in peak hour
    if (arr<9.5)&(arr>7.5):
        arr_am,arr_pm,arr_op=1,0,0
    elif (arr<19.5)&(arr>17.5):
        arr_am,arr_pm,arr_op=0,1,0
    else:
        arr_am,arr_pm,arr_op=0,0,1
    if (dep<9.5)&(dep>7.5):
        dep_am,dep_pm,dep_op=1,0,0
    elif (dep<19.5)&(dep>17.5):
        dep_am,dep_pm,dep_op=0,1,0
    else:
        dep_am,dep_pm,dep_op=0,0,1
    V[count+1]=eval("sarr_1(%s) + sdep_1(%s) + (PersonTypeIndex!=1) * (sarr_2(%s) + sdep_2(%s)) + gender * (sarr_3(%s)+sdep_3(%s)) + (worktime==2) * (sarr_4(%s)+sdep_4(%s)) + DUR_1 * %s + DUR_2 * %s**2 + DUR_3 * %s**3 " % (arr,dep,arr,dep,arr,dep,arr,dep,dur,dur,dur) + \
"+ TT1 * TT_HT1_%s + TT2 * TT_HT2_%s" % (arrid,depid) + \
    "+ C * (cost_HT1_am * %s + cost_HT1_pm * %s + cost_HT1_op * %s + cost_HT2_am * %s + cost_HT2_pm * %s + cost_HT2_op * %s)\n" %(arr_am,arr_pm,arr_op,dep_am,dep_pm,dep_op))
#     
av=dict(zip(choiceset,[eval('avail%s' %i) for i in choiceset]))
#av=dict(zip(choiceset,[1 for i in choiceset]))
prob = bioLogit(V,av,UniqueIndex)
rowIterator('obsIter')
BIOGEME_OBJECT.ESTIMATE = Sum(log(prob),'obsIter')
exclude = (( PrimaryActivityIndex != 1)*( PrimaryActivityIndex != 2) + (AggregateModeIndex_HT1==2)*(ArrivalPeriodIndex<5) + (AggregateModeIndex_HT2==2)*(DeparturePeriodIndex>46))>0 
BIOGEME_OBJECT.EXCLUDE = exclude
BIOGEME_OBJECT.PARAMETERS['numberOfThreads'] = '8'
BIOGEME_OBJECT.PARAMETERS['optimizationAlgorithm'] = 'CFSQP'
BIOGEME_OBJECT.PARAMETERS['checkDerivatives'] = '0'
BIOGEME_OBJECT.PARAMETERS['moreRobustToNumericalIssues'] = '0'
