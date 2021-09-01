#
# Make write dirs structure and copy files to SRAKIT and then read files from
# srakit and compare these files
#

## connection settings ##
MSCPATH=/media/bubenik/57C4-063B

ncycles=0

# create rd-rootfs
rm -rf rootfs-wr
rm -rf rootfs-rd
mkdir -p rootfs-wr
cp -R rootfs-wr rootfs-rd



function write_file {

    dirname=$1
    filename=$2
    numfiles=$3
    filesize=$4

    count=0
    until [ $count -ge $numfiles ]; do

        src_fname=rootfs-wr$dirname$filename$count
	dst_fname=$MSCPATH$dirname$filename$count

        dd if=/dev/urandom of=$src_fname bs=$filesize count=1 2>/dev/null
	echo "   $dst_fname ($filesize bytes)"
        
	cp $src_fname $dst_fname

	let count=count+1
        let total_size=$total_size+$filesize
        let total_files=$total_files+1
    done 

    echo
}

function compare_dir {

    dirname=$1

    for filename in `ls -l rootfs-wr$dirname | grep ^- | awk '{print $9}'`; do
        src_fname=rootfs-rd$dirname$filename
	dst_fname=$MSCPATH$dirname$filename

        echo "   Read file: "$dst_fname
	cp $dst_fname $src_fname

        dst_fname=rootfs-wr$dirname$filename
        diff -s $src_fname $dst_fname
        if [ $? -ne 0 ]; then
            echo "Test aborted !!, ncycles = "$ncycles
            exit 1
        fi
    done

    echo
}

# start infinite loop
while true
do

total_files=0
total_size=0

echo "Write files ..."

# root
write_file "/" "file4K" 10 4096
write_file "/" "cfg_3k" 10 3072
write_file "/" "fw_4k" 10 4096
write_file "/" "fw_16k" 10 16384
#write_file "/" "lang_10k" 10 10240
write_file "/" "lic" 10 1024
#write_file "/" "log" 5 102400
#usr
#write_file "/usr/voice/" "voice_16k" 10 16384
#write_file "/usr/voice/" "voice_160k" 10 163840
# voice
#write_file "/voice/" "voice_3M" 2 3145728

echo
echo ">>> FS write OK, total  files="$total_files"   size="$total_size " <<<" 
echo


echo
echo "Compare files ..."
echo

compare_dir "/" 
#compare_dir "/cfg/" 
#compare_dir "/fw/" 
#compare_dir "/lang/" 
#compare_dir "/lic/" 
#compare_dir "/log/" 
#compare_dir "/usr/" 
#compare_dir "/usr/voice/" 
#compare_dir "/voice/" 

let ncycles=$ncycles+1

echo "*********************************"
echo "*** Test done, ncycles = "$ncycles
echo "*********************************"
echo

sleep 1

done
