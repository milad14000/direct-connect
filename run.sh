#!/bin/bash

#exit on any failure


num_TOR="32"
num_run="1"
tdm="perm"
load="10 15 20 25 30 35 40 45"
log_dir="check"
work_dir="direct-connect"
demand="pattern"
# format of the files should be ${tdm}{# of tors}_{run}
tdm_dir="traffic_demands"    
topology="direct"
te="vl2"
scheduler="NONE"
weights="optimal"
num_flows=20000

options="o:s:t:d:l:r:n:w:f:i:"
while getopts $options option
do
  case $option in
    o) log_dir=$OPTARG ;;
    s) scheduler=$OPTARG ;;
    t) te=$OPTARG ;;
    d) tdm=$OPTARG ;;
    l) load=$OPTARG ;;
    r) num_run=$OPTARG ;;
    n) num_TOR=$OPTARG ;;
    w) weights=$OPTARG ;;
    f) num_flows=$OPTARG ;;
    i) topology=$OPTARG ;;
  esac
done

if [ ! -d "${log_dir}" ]; then                                                                        
  mkdir ${log_dir}                                                                                    
fi                                                                                                    
sub_dir=${log_dir}/${tdm}_${num_TOR}                                                                  
if [ ! -d "$sub_dir" ]; then                                                                          
  mkdir ./$sub_dir                                                                                    
fi      

if [ $tdm == "unif" ]; then
  demand="uniform"
else
  demand="pattern"
fi

case $num_TOR in
  8)	
    # empirical thresholds for 8-TOR
    T=([10]=1 [15]=1 [20]=1 [25]=1 [30]=2 [35]=2 [40]=3 [45]=3 [50]=5 [60]=6 [70]=7 [80]=8 [90]=9) ;;
  16)
    # empirical thresholds for 16-TOR
    T=([10]=1 [15]=1 [20]=1 [25]=2 [30]=2 [35]=2 [40]=3 [45]=4 [50]=5 [60]=6 [70]=7 [80]=8 [90]=9) ;;
  32)  
    # empirical thresholds for 32-TOR
    # Unif
    T=([10]=1 [15]=2 [20]=2 [25]=3 [30]=3 [35]=3 [40]=4 [45]=4 [50]=5 [55]=5 [60]=6 [70]=7 [80]=8 [90]=9) ;;
esac


for run in $num_run; do
  if [ $demand == "pattern" ]; then
    cp ./${tdm_dir}/${tdm}${num_TOR}_${run} pattern
  fi
  sleep 1
  echo "run $run ..."
 
  if [ $te == 'IEWF' -o $te == 'PS' ]; then 
    result_dir=./${sub_dir}/${topology}_${te}_${weights}_${scheduler}
  else
    result_dir=./${sub_dir}/${topology}_${te}_${scheduler}           
  fi  
    
  if [ ! -d "${result_dir}" ]; then
    mkdir $result_dir
  fi
  if [ ! -d "${result_dir}/${run}" ]; then
    mkdir $result_dir/${run}
  fi

  for l in $load; do
    if [ $weights == 'optimal' ] && ([ $te == 'IEWF' -o  $te == 'PS' ]); then
      ./approx_optimal/a.out -n ${num_TOR} -l $l >> /dev/null
    fi	      

    ./main --load $l --demand ${demand} -o ${result_dir}/${run}/flow${l}.log -t ${te} -p ${weights} -s ${scheduler} -T ${T[$l]} -i ${topology} -n ${num_TOR} -f ${num_flows}
  done
done

