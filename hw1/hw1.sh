#!/bin/bash

option="0"

while [ $option -ne "4" ]
  do
  	echo "Input 1,2,3 or 4 to pick an option"
  	echo "1 - Print process tree for current process"
  	echo "2 - Output which users are online"
  	echo "3 - Output processes a given user is running"
  	echo "4 - Exit"

  	read option

  	if [ $option -eq "1" ]
  	then
  		echo "The process tree for current process is: "
  		curProc=$$
  		echo ""
  		echo $curProc
  		try="1"
  		while [ $curProc -ne "1" ]
  		do
  			echo "  |"
  			curProc=$(ps -ef | grep $curProc |  awk 'NR=="'"$try"'"; START{print}' | awk '{print $3}' )
  			while [ $curProc -eq $$ ]
  			do
  				$try=$(( $try+1 ))
  				curProc=$(ps -ef | grep $curProc |  awk 'NR=="'"$try"'"; START{print}' | awk '{print $3}' )
  			done
  			echo $curProc
  			try="1"
  		done
  		echo ""
    fi

    if [ $option -eq "2" ]
  	then
  		echo ""
  	    who | awk '{print $1}' | uniq
    	echo ""
    fi

    if [ $option -eq "3" ]
  	then
  	    echo "Select an online user and view his processes"
  	    count=0
    	for i in $(who | awk '{print $1}' | uniq) 
    	do
    		echo ""
    		echo $count") " $i  
    		echo ""
    		count=$(($count+1))
    	done
    	read user
    	user=$(($user+1))
    	theUser=$(who | awk '{print $1}' | uniq | awk 'NR=="'"$user"'"; START{print}')
		echo ""
		ps -ef | awk '$1 ~ "'"$theUser"'" {print}'    	
    	echo ""
    fi
  done