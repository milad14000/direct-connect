#!/bin/bash

#exit on any failure

# Number of servers for simulation 
servers=(l2 l3 l4 l5 l6)
#servers=(l3)
num_instances=${#servers[@]}

# Simulation Parameters
num_TOR="16"
num_run="1"
tdm="unif"
#load="10 15 20 25 30 35 40 45 50"

#load="10 20 30 40 50 60 70"
option="exponential"
num_flow=200000
#log_dir="log_16TOR"
log_dir="test_PS"

work_dir="/home/msharif/direct_connect"
if [ ! -d "${log_dir}" ]; then
  mkdir ${log_dir}
fi
sub_dir=${log_dir}/${tdm}_${num_TOR}
if [ ! -d "$sub_dir" ]; then
  mkdir $sub_dir
fi

#te=(vl2 ECMP ECMP vl2 ECMP)
#scheduler=(NONE NONE CA_FLLB Hedera NONE)
#weights=(equal perm_optimal equal equal equal)
#topology=(direct direct direct direct bipartite)

#te=(ECMP)
#scheduler=(CA_FLLB)
#weights=(equal)
#topology=(bipartite)

#te=(IEWF)
#scheduler=(NONE)
#weights=(perm_optimal)
#topology=(bipartite)

te=(ECMP PS)
scheduler=(CA_FLLB NONE)
weights=(equal equal)
topology=(direct direct)

update() {
  for s in ${servers[@]}; do
    echo "Updating $s ..."
    scp "main" "$s:${work_dir}/" 
    #> /dev/null
    ssh $s "sh -c 'cd $word_dir'" 
  done
}

killAll() {
  for s in ${servers[@]}; do
    ssh $s "killall main" 
  done
}

cleanAll() {
  for s in ${servers[@]}; do
    echo "Cleaning up $s ..."
    ssh $s "sh -c 'cd ${work_dir}; rm out_*; rm -rf tempLog'"
  done
}

run() {
  size=`expr ${#te[@]} - 1`
  # Run the simulation for different load/scheduler/
  for run in $num_run; do
    echo "run $run ..."
    for s in `seq 0 ${size}`; do
      ind=$((${s} % ${num_instances}))
      server=${servers[$ind]}
      echo "Running  (${te[$s]},${scheduler[$s]}) on ${server}"
      for l in $load; do  
        cmd="./run.sh -f ${num_flow} -d ${tdm} -t ${te[$s]} -s ${scheduler[$s]} -i ${topology[$s]} -l ${l} -n ${num_TOR} -r ${run} -w ${weights[$s]} -o tempLog"
	ssh ${server} "sh -c 'cd $work_dir; nohup $cmd > out_l${l}_${tdm} 2>&1 &'"
      done
    done
  done
}

copy() {
  # Copy the logs
  for s in ${servers[@]}; do
    scp -r "$s:${work_dir}/tempLog/${tdm}_${num_TOR}/*" "./${sub_dir}"
    #ssh $s "rm -rf ${work_dir}/tempLog"
  done
}

# Update workers
#update
#killAll
cleanAll
#run
#copy

# Check for finished simulation
finished=0
while [ $finished -ne 1 ]; do
  finished=1	
  for s in ${servers[@]}; do
    num_proc=$(ssh $s ps ax | grep "./main" | wc -l)
    if [ $num_proc -gt 0 ]; then
      finished=0
      echo $num_proc on $s
    fi
  done
  sleep 60
done

echo "Done."




