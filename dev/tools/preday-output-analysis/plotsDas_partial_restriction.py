import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
from matplotlib import dates
import matplotlib as mpl
import os,pdb,math,csv
import numpy as np

#FONT = 'serif'
FONT = 'Latin Modern Roman'
mpl.rc('font',family=FONT)


outFolder = "/home/neeraj/Kakali/DAS_Check/output_partial_retr_Jurong"
totalPopulation = 6000000
modeList = ['Car','Car_Sharing_2','Car_Sharing_3','MRT','BusTravel','Walk','SMS','SMS_Pool','Rail_SMS','PrivateBus','Taxi','Motorcycle','Bike','Rail_SMS_Pool','Rail_AMOD_Pool','AMOD_Pool','AMOD','Rail_AMOD']
stopList = ['Work','Education','Shop','Other','Home']
activityList = ['Work','Education','Shop','Recreation','Personal','Escort','Other']


timeOfDayList = [str(item) for item in list(np.arange(3.25,27,0.5))]


#--------------------------------------------------------------------------------------------------

def barPlots(adict,labelList,xlabel=None,ylabel=None,title=None,xlim=None,ylim=None,lineStyles=None,colors=None,fname=None):
    '''
    '''
    # Plotting commences
    fig,ax1 = plt.subplots()
    
    barwidth = 0.7
    # Plotting
    vals = [adict[item] for item in labelList]
    positions = np.arange(len(adict))
    #positions = np.linspace(0,len(adict)/2,num=len(adict))
    ax1.bar(positions, vals, width = barwidth,align='center', alpha=0.5)
    # label ticks
    ax1.xaxis.set_ticks(positions)
    ax1.set_xticklabels(labelList,rotation=25)
    

    # x and y labels
    if xlabel:
        ax1.set_xlabel(xlabel,fontsize=20)
    if ylabel:
        ax1.set_ylabel(ylabel,fontsize=20)    

    # Removing top and right box-lines
    ax1.spines['right'].set_visible(False)
    ax1.spines['top'].set_visible(False)
    ax1.yaxis.set_ticks_position('left')
    ax1.xaxis.set_ticks_position('bottom')

    # Setting tick's fontname and size
    for label in (ax1.get_xticklabels() + ax1.get_yticklabels()):
        label.set_fontsize(12)

    if xlim:
        ax1.set_xlim([xlim[0],xlim[1]])
    if ylim:
        ax1.set_ylim([ylim[0],ylim[1]])
    if title:
        ax1.set_title(title,fontsize=22)
    
    ax1.autoscale(enable=True, axis='both', tight=True)
        
    fig.savefig(os.path.join(outFolder,fname+".png"),bbox_inches='tight',dpi=400)
    fig.clf()     
    

#--------------------------------------------------------------------------------------------------
    
def linePlot(x,y,label=None,xlabel=None,ylabel=None,title=None,xlim=None,ylim=None,colors=None,fname=None):
    
    # Plotting commences
    fig,ax1 = plt.subplots()
    
    # Plotting
    # checking if the input is alist of lists
    if any(isinstance(i, list) for i in y):
        for i in range(len(y)):
            if colors:
                ax1.plot(x,y[i],linestyle='-',label=label[i],color=colors[i])
            else:
                ax1.plot(x,y[i],linestyle='-',label=label[i])
    else:
        if label:
            ax1.plot(x,y,linestyle='-',label=label)
        else:
            ax1.plot(x,y,linestyle='-')
    
    
    #Legend properties
    if label:
        fontP = FontProperties(FONT)
        fontP.set_size(16)
        ax1.legend(loc='upper right',prop=fontP,frameon=False,ncol=1,numpoints=1)

    # x and y labels
    if xlabel:
        ax1.set_xlabel(xlabel,fontsize=20)
    if ylabel:
        ax1.set_ylabel(ylabel,fontsize=20)    

    # Removing top and right box-lines
    ax1.spines['right'].set_visible(False)
    ax1.spines['top'].set_visible(False)
    ax1.yaxis.set_ticks_position('left')
    ax1.xaxis.set_ticks_position('bottom')

    # Setting tick's fontname and size
    for label in (ax1.get_xticklabels() + ax1.get_yticklabels()):
        label.set_fontsize(16)

    if xlim:
        ax1.set_xlim([xlim[0],xlim[1]])
    if ylim:
        ax1.set_ylim([ylim[0],ylim[1]])
    if title:
        ax1.set_title(title,fontsize=22)
    
   
    
    fig.savefig(os.path.join(outFolder,fname+".png"),bbox_inches='tight',dpi=400)
    fig.clf() 

#--------------------------------------------------------------------------------------------------

def plotModeActivityShares(tripModeCount,tripStopCount,tripTodActiCount,tripTodModeCount,tripDistCount,tourModeCount,tourActivityCount,tourDistCount,totalNum):
    '''
    '''
    
    # Getting only the shares that are non-zero
    tripModeShare = {k:100*float(v)/totalNum['trips'] for k,v in tripModeCount.iteritems() if v>0}
    tripStopShare = {k:100*float(v)/totalNum['trips'] for k,v in tripStopCount.iteritems() if v>0}
    tourModeShare = {k:100*float(v)/totalNum['tours'] for k,v in tourModeCount.iteritems() if v>0}
    tourActivityShare = {k:100*float(v)/totalNum['tours'] for k,v in tourActivityCount.iteritems() if v>0}
    
    
    # Plotting mode and activity shares
    barPlots(tripModeShare,tripModeShare.keys(),xlabel="Modes",ylabel="'%' share of Total Trips",title=None,xlim=None,ylim=None,fname='TripModeShares')
    barPlots(tripStopShare,tripStopShare.keys(),xlabel="Stop Type",ylabel="'%' share of Total Trips",title=None,xlim=None,ylim=None,fname='TripStopShares')
    barPlots(tourModeShare,tourModeShare.keys(),xlabel="Modes",ylabel="'%' share of Total Tours",title=None,xlim=None,ylim=None,fname='TourModeShares')
    barPlots(tourActivityShare,tourActivityShare.keys(),xlabel="Activity Type",ylabel="'%' share of Total Tours",title=None,xlim=None,ylim=None,fname='TourActivityShares')    
    
    
    # Plotting number of trip and tour distribution
    tripcount = [i for i in xrange(0,max(tripDistCount.keys()))]
    tripsPerPerson = {i:tripDistCount[i] if i in tripDistCount else 0 for i in tripcount}
    barPlots(tripsPerPerson,tripcount,xlabel="Number of trips per person",ylabel=" '%' of total population",title=None,xlim=None,ylim=None,fname='Tripnumdist')

    tourcount = [i for i in xrange(0,max(tourDistCount.keys()))]
    toursPerPerson = {i:tourDistCount[i] if i in tourDistCount else 0 for i in tourcount}
    barPlots(toursPerPerson,tourcount,xlabel="Number of tours per person",ylabel=" '%' of total population",title=None,xlim=None,ylim=None,fname='Tournumdist')    
    
    
    # Plotting trip TOD by total and tour purpose
    totaltodTrips = [tripTodActiCount['total'][item] for item in timeOfDayList]
    linePlot(timeOfDayList,totaltodTrips,label=None,xlabel="Time of Day",ylabel="Number of Trips",title=None,xlim=None,ylim=None,colors=None,fname="TodTrips_depTime_total")
    
    # Filter the acitvity Types that are zero through out the day
    activityListWithTrips = [tour_activity for tour_activity in activityList if sum([tripTodActiCount[tour_activity][item] for item in timeOfDayList])>0]
    todActiTrips = [[tripTodActiCount[tour_activity][item] for item in timeOfDayList] for tour_activity in activityListWithTrips]
    linePlot(timeOfDayList,todActiTrips,label=activityListWithTrips,xlabel="Time of Day",ylabel="Number of Trips by tour activity",title=None,xlim=None,ylim=None,colors=None,fname="TodTrips_depTime_activity")
    
    # Filtering the modes without any trips
    modeListWithTrips = [stop_mode for stop_mode in modeList if sum([tripTodModeCount[stop_mode][item] for item in timeOfDayList])>0]
    todModeTrips = [[tripTodModeCount[stop_mode][item] for item in timeOfDayList] for stop_mode in modeListWithTrips]
    linePlot(timeOfDayList,todModeTrips,label=modeListWithTrips,xlabel="Time of Day",ylabel="Number of Trips by trip mode",title=None,xlim=None,ylim=None,colors=None,fname="TodTrips_depTime_mode")

    
    #pdb.set_trace()
    
    
    
    
    
    
    
    
    
    
    
