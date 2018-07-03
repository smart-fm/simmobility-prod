#!/bin/bash -l

#### set num of cores (make it equal to 
#### num of threads in your xml input)
#$ -pe smp 21

#### batch job's name
#$ -N supply_latest

#### set the queue
#$ -q deadline

#### join stderr and stdout as a single file
#$ -j y

#### set the shell
#$ -S /bin/bash
#$ -V

#### set current dir as working dir
#$ -cwd

#### execute your command/application
#Release/SimMobility_Medium data/simulation.xml data/simrun_MidTerm.xml
Debug/SimMobility_Medium data/simulation.xml data/simrun_MidTerm.xml
echo "Simulation Done"
mkdir 2_2
mv controller.log 2_2/controller.log
mv warn.log 2_2/warn.log
Debug/SimMobility_Medium data/simulation.xml data/simrun_MidTerm.xml
echo "Simulation Done"
mkdir 3_2
mv controller.log 3_2/controller.log
mv warn.log 3_2/warn.log
Debug/SimMobility_Medium data/simulation.xml data/simrun_MidTerm.xml
echo "Simulation Done"
mkdir 3_1
mv controller.log 3_1/controller.log
mv warn.log 3_1/warn.log
Debug/SimMobility_Medium data/simulation.xml data/simrun_MidTerm.xml
echo "Simulation Done
mkdir 4_1
mv controller.log 4_1/controller.log
mv warn.log 4_1/warn.log
###### How to submit job ############
##### qsub jobscript.sh ############
##### NOTE: make sure the folder where the jobscript found is writable by hpcusers group #######
##### To change mode: chmod g+rw .
