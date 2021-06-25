make
echo "> ./makeFileSystem 4 mySystem.data"
./makeFileSystem 4 mySystem.data
echo "> ./fileSystemOper mySystem.data mkdir '\usr'"
./fileSystemOper mySystem.data mkdir '\usr'
echo "> ./fileSystemOper mySystem.data mkdir '\usr\ysa'"
./fileSystemOper mySystem.data mkdir '\usr\ysa'
echo "> ./fileSystemOper mySystem.data mkdir '\bin\ysa'"
./fileSystemOper mySystem.data mkdir '\bin\ysa'
echo "> echo "contents of linuxFile.data" > linuxFile.data"
echo "contents of linuxFile.data" > linuxFile.data
./fileSystemOper mySystem.data write '\usr\ysa\file1' linuxFile.data
echo "> ./fileSystemOper mySystem.data write '\usr\file2' linuxFile.data"
./fileSystemOper mySystem.data write '\usr\file2' linuxFile.data
echo "> ./fileSystemOper mySystem.data write '\file3' linuxFile.data"
./fileSystemOper mySystem.data write '\file3' linuxFile.data
echo "> ./fileSystemOper mySystem.data dir '\'"
./fileSystemOper mySystem.data dir '\'
echo "> ./fileSystemOper mySystem.data del '\usr\ysa\file1'"
./fileSystemOper mySystem.data del '\usr\ysa\file1'
echo "> ./fileSystemOper mySystem.data dump2fs"
./fileSystemOper mySystem.data dumpe2fs
echo "> ./fileSystemOper mySystem.data read '\usr\file2' linuxFile2.data"
./fileSystemOper mySystem.data read '\usr\file2' linuxFile2.data
echo "> cmp linuxFile2.data linuxFile.data"
cmp linuxFile2.data linuxFile.data
