1) For the capacitated location routing problem, under a linux operating system, please enter floder "code", and run the makefile to generate the executable pragram 'ilrp', where the makefile has been included in code floder. 
The program 'ilrp' contains eight input parameters which are listed in each script file 'run.sh', 
where ./instances/set_S/100-5-1c.json is the number of input instance, ./instances/depot configurations/S/depot_configuratons_100-5-1c.txt denotes the input depot configurations, results/10051c_0.txt is the output file, 134516 is the best results found in literature, 0 is used to indicate whether this instance has been solved optimally, 60000 is the time limit (in seconds) for each run of program, 300000 is the maximum iterations for each run of program, 0 is the radom seed

————————————————————————————————————————
./code/ilrp ./instances/set_S/100-5-1c.json ./instances/depot configurations/S/depot_configuratons_100-5-1c.txt results/10051c_0.txt 134516 0 60000 300000 0
————————————————————————————————————————
One notices that parameter './instances/depot configurations/S/depot_configuratons_100-5-1c.txt' can default.


2) Submission of jobs
The parameters of program can be located in a script file named 'run.sh'.
Under a linux operating system, the job 'instance.sh' can be submitted as follows:
-----------------------------------------
chmod 777 ilrp
chmod 777 run.sh 
sbatch run.sh
-----------------------------------------
 If you want to get scripts used to submit jobs by slurm, I can provide all necessary shell files.
