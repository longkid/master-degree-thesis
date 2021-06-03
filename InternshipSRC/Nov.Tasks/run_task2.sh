textDir=$HOME/Documents/AnnotatingResult
imgDir=$HOME/Documents/WMBER/LaBRI/videos
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
	$buildPath/task2 $imgDir/$dirName $textFile $buildPath/$movieID.txt $method
    done
}

cd $imgDir
movieID=SRC01
process
movieID=SRC07
process
movieID=SRC13
process