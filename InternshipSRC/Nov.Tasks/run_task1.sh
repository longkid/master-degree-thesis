textDir=$HOME/Documents/AnnotatingResult
imgDir=$HOME/Documents/Maps/BGUMaps/BGU
buildPath=$HOME/tools/Nov.Tasks/build

process ()
{
    for dirName in $movieID*
    do
	echo "$imgDir/$dirName"
	textFile=$(ls $textDir/$movieID*/$dirName*.txt)
	echo "$textFile"
    # Call the program
	$buildPath/task1 $imgDir/$dirName $textFile $buildPath/$movieID.txt
    done
}

cd $imgDir
movieID=SRC01
process
#movieID=SRC07
#process
#movieID=SRC13
#process