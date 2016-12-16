#!/bin/bash

###---------------------------------------------------
##  Macro for processing directory containing
##  RawHits_*_*.dat files.
##  Runs CSA on each file and saves clusters in
##  RawHits_*_*_tree.root files. Afterwards performs
##  an anlysis on the combined data.
##  
##  Written by Miljenko Suljic, m.suljic@cern.ch
##  Following srs-software concept.
##  (thanks to Felix & Jacobus)
###---------------------------------------------------

if [ "$#" -lt 1 ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
    echo "-------------------------------------------------------"
    echo "Either number of arguments is not correct"
    echo " or you have requested help... "
    echo "Required arugments:"
    echo "  1) path to dir with raw files"
    echo "Optional:"
    echo "  2) mask file name (in dir with raw files, 0 for none)"
    echo "  3) find interesting events? 0 = no (default), 1 = yes"
    echo "  4) recreate cluster trees?  0 = no (default), 1 = yes"
    echo "-------------------------------------------------------"
    exit 1
fi

# variables
CROWN=7

# paths
DIR_ANALYSIS=${BASH_SOURCE%/*}
DIR_MACROS=$(readlink -f $DIR_ANALYSIS/clustering)/
DIR_CLASSES=$(readlink -f $DIR_ANALYSIS/classes)/
DIR_RAW=$(readlink -f $1)/
DIR_RESULTS=$(readlink -f $DIR_RAW/results)_cr${CROWN}/
DIR_LOGS=$(readlink -f $DIR_RAW/logs)/

# parallel processing
MAXJOBS=3

if [ "$#" -ge 2 ] 
then
    if [ "$2" == "NULL" ] || [ "$2" == "0" ]
    then
        FILE_MASK=""
    else
        FILE_MASK=$DIR_RAW/$2
    fi
else
    FILE_MASK=""
fi

if [ "$#" -ge 3 ]
then
    FLAG_INTERESTING=$3
else
    FLAG_INTERESTING=0
fi

if [ "$#" -ge 4 ]
then
    FLAG_RECREATE=$4
else
    FLAG_RECREATE=0
fi

mkdir -p $DIR_RESULTS
mkdir -p $DIR_LOGS

echo "---------------------------------"
echo "Target directory:  $DIR_RAW"
echo "Mask filename:     $FILE_MASK"
echo "Results directory: $DIR_RESULTS"
echo "Dir with macros:   $DIR_MACROS"
echo "Dir with classes:  $DIR_CLASSES"
echo "---------------------------------"

# compile classes
cd $DIR_CLASSES
#rm *.so *.d
root -l -b -q "compile_classes.C"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIR_CLASSES

# compile macros
cd $DIR_MACROS
#rm *.so *.d
root -l -b <<EOF
.x $DIR_CLASSES/load_classes.C
.L csa.C+
.L interesting_events.C+
.L analysis_basic.C+
.L analysis_ibhic.C+
.L analysis_chip7.C+
.q
EOF

njobs=0
for FILE_RAW in `ls -1 $DIR_RAW | egrep -x 'NoiseOccupancy_[0-9]+_[0-9]+_Chip[0-9].dat'`
do
    echo $FILE_RAW
    FILE_ROOT=${FILE_RAW%.dat}_tree.root
    FILE_LOG=${FILE_RAW%.dat}_csa.log
    [ -f "$DIR_RAW/crown" ] && LAST_CSA_CROWN=`cat $DIR_RAW/crown` || LAST_CSA_CROWN=0
	if [ ! -f "$DIR_RAW/$FILE_ROOT" ] || [ "$FLAG_RECREATE" -eq 1 ] || [ "$LAST_CSA_CROWN" != "$CROWN" ]
	then
        echo "Processing $FILE_RAW"
        time root -l -b -q "$DIR_CLASSES/load_classes.C" "csa.C+(\"$DIR_RAW/$FILE_RAW\", \"$DIR_RAW/$FILE_ROOT\", \"$FILE_MASK\", $CROWN)" > $DIR_LOGS/$FILE_LOG 2>&1 &
        let njobs=njobs+1
        if [ $njobs -ge $MAXJOBS ]
        then
		    echo "maximum job number spawned, waiting until they're finished"
		    wait
		    njobs=0
        fi
	fi
done
wait

echo $CROWN > $DIR_RAW/crown

if [ "$FLAG_INTERESTING" -eq 1 ]; then
    for i in `seq 0 8`
    do
        root -l -b -q "$DIR_CLASSES/load_classes.C" "interesting_events.C+(\"$DIR_RAW/NoiseOccupancy_*_Chip${i}_tree.root\", \"$DIR_RESULTS/interesting_events_Chip${i}.root\")" | tee $DIR_LOGS/interesting_events_chip${i}.log 2>&1
    done
else
    echo "Not searching for interesting events!"
fi

for i in `seq 0 8`
do
    #root -l -b -q "$DIR_CLASSES/load_classes.C" "analysis_basic.C+(\"$DIR_RAW/NoiseOccupancy_*_Chip${i}_tree.root\", \"$DIR_RESULTS\", \"Chip${i}\")" | tee $DIR_LOGS/analysis_basic_chip${i}.log
    root -l -b -q "$DIR_CLASSES/load_classes.C" "analysis_chip7.C+(\"$DIR_RAW/NoiseOccupancy_*_Chip${i}_tree.root\", \"$DIR_RESULTS\", \"Chip${i}\")" | tee $DIR_LOGS/analysis_basic_chip${i}.log
done


root -l "$DIR_CLASSES/load_classes.C" "analysis_ibhic.C+(\"$DIR_RESULTS/\")" | tee $DIR_LOGS/analysis_ibhic.log


echo "Finished processing directory!"
