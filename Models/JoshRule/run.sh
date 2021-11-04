
myhost=$(hostname -s)
d=2021-05-13-15-05-01_model_comparison_data
f=c001_2.txt

############ Baseline ###############

# Run baseline on all 100 tasks
# ls $d/*.txt | xargs -n 1 -x  basename | xargs -n 1 -I {} -P 40 sh -c "./main --propsim_ll 0 --largeAlphabet=1 --chains=5 --threads=1 --time=10m --input=$d/{} --output=results/out-small-baseline/{} 2>&1 | tee -a results/out-small-baseline/{}.log"
# ls $d/*.txt | xargs -n 1 -x  basename | xargs -n 1 -I {} -P 0 ./main --propsim_ll 1 --largeAlphabet=1 --chains=5 --threads=1 --time=1m --input=$d/{} --output=out-small-prop/{} > out-small-prop/{}.log

# Run baseline failed tasks
# cat out-small-baseline/tasks_failed.txt | xargs -n 1 -x basename | xargs -n 1 -I {} -P 40 sh -c "./main --propsim_ll 0 --largeAlphabet=1 --chains=5 --threads=1 --time=10m --input=$d/{} --output=out-small-baseline/{} 2>&1 | tee -a out-small-baseline/{}.log"


############ PropSim ###############


# Run propsim on all 100 tasks
ls $d/*.txt | xargs -n 1 -x  basename | xargs -n 1 -I {} -P 40 sh -c "./main --propsim_ll 1 --largeAlphabet=1 --chains=5 --threads=1 --time=10m --input=$d/{} --output=results/out-small-prop-all/{} 2>&1 | tee -a results/out-small-prop-all/{}.log"




############ Other ################

# original command (works locally but not on openmind)
# ls $d/*.txt | xargs -n 1 basename | parallel --jobs=48 "./main --largeAlphabet=0 --chains=5 --threads=1 --time=10m --input=$d/{1} --output=out-small/{1} > out-small/{1}.log" 
 
# test command for single task
# ./main --largeAlphabet=0 --chains=5 --threads=1 --time=2s --input=$d/$f --output=out-small/$f
# ./main --largeAlphabet=0 --chains=5 --threads=1 --time=1ms --input=2021-05-13-15-05-01_model_comparison_data/c001_2.txt --output=out-small/c001_2.txt > out-small/c001_2.txt.log

## The below is for running A*
# 
# {
# ls txt-smallAlphabet/*.txt | xargs -n 1 basename | parallel --jobs=2 "./main --doAstar --chains=5 --threads=1 --time=10m --input=txt-smallAlphabet/{1} --output=out-astar/{1} > out-astar/{1}.log" 
# } &
# 
# {
# ls txt-largeAlphabet/*.txt | xargs -n 1 basename | parallel --jobs=2 "./main --doAstar --largeAlphabet=1 --chains=5 --threads=1 --time=10m --input=txt-largeAlphabet/{1} --output=out-astar/{1} > out-astar/{1}.log" 
# } &
