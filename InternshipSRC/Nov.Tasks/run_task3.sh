textDir=$HOME/Documents/AnnotatingResult
errorMapDir=$HOME/Documents/WMBER/LaBRI/videos
eyeTrackerMapDir=$HOME/Documents/BGUMaps/BGU
buildPath=$HOME/tools/Nov.Tasks/build

if [ $# > 0 ]
then
    method=$1
else
    method=1
fi

echo "Compute method $method"

process ()
{
    for dirName in $movieID*
    do
	echo "$imgDir/$dirName"
	textFile=$(ls $textDir/$movieID*/$dirName*.txt)
	echo "$textFile"
    # Call the program
	$buildPath/task3 $errorMapDir/$dirName $eyeTrackerMapDir/$dirName $textFile $buildPath/$movieID.txt $method
    done
}

cd $errorMapDir
movieID=SRC01
process
movieID=SRC07
process
movieID=SRC13
process