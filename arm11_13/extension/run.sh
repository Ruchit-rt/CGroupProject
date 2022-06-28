make;
./trans_Analyser $1;
python3 plot.py; 
make clean;
echo "\n==========DONE==========\n";