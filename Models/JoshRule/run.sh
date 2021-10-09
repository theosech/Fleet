
myhost=$(hostname -s)
d=2021-05-13-15-05-01_model_comparison_data
f=c001_2.txt

# ls $d/*.txt | xargs -n 1 basename | parallel --jobs=48 "./main --largeAlphabet=1 --chains=5 --threads=1 --time=10m --input=$d/{1} --output=out-large/{1} > out-large/{1}.log" 

# ls $d/*.txt | xargs -n 1 basename | parallel --jobs=48 "./main --largeAlphabet=0 --chains=5 --threads=1 --time=10m --input=$d/{1} --output=out-small/{1} > out-small/{1}.log" 
 
# test command for single task
./main --largeAlphabet=0 --chains=5 --threads=1 --time=2000ms --input=$d/$f --output=out-small/$f > out-small/$f.log
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
