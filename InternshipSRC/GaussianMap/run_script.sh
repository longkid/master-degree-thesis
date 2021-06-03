#
# This script file is used to run GaussianMap program with many annotating results.
#

echo "Please input the directory containing the annotating results:"
read inputPath
#inputPath=$HOME/tools/BBBox/build
echo "Running GaussianMap program\n"

buildPath=$HOME/tools/GaussianMap_v2/build
cd $inputPath
for dirName in SRC*
do
    echo "$inputPath/$dirName"
    mkdir $buildPath/$dirName
    # Iterate all text file
    cd $dirName
    for textFile in *.txt
    do
	mkdir $buildPath/$dirName/$textFile
	cd $buildPath/$dirName/$textFile
	echo "$inputPath/$dirName/$textFile"
	../../GaussianMap $inputPath/$dirName/$textFile 1920 1080
	cd $inputPath/$dirName
	done
    cd $inputPath
done