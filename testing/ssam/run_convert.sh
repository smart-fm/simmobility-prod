#/bin/bash

#If you're comfortable with Go, just build the code yourself. If not, you can use this:
#  ./run_convert.sh 
#...to convert out.txt to out.trj
#Note that this will rebuild convert.go every time.

#In case the user did a bleeding-edge install of go.
export PATH=$PATH:/usr/local/go/bin

#Figure out which version of the compiler we're using.
if  (go tool -n 6g >/dev/null 2>&1)
then
    echo "Found 64-bit Go compiler"
    GoCompiler="6g"
    GoLinker="6l"
	GoSuffix="6"
elif  (go tool -n 8g >/dev/null 2>&1)
then
    echo "Found 32-bit Go compiler"
    GoCompiler="8g"
    GoLinker="8l"
	GoSuffix="8"
else
    echo "Couldn't find a valid install of Go."
    exit 1
fi

#Build both object files (just use a default ".o" suffix notation) and link
if  ! (go tool "${GoCompiler}" simmob.go)
then
    exit 1
fi
if  ! (go tool "${GoCompiler}" convert.go)
then
    exit 1
fi
if  ! (go tool "${GoLinker}" -o convert  convert.${GoSuffix})
then
    exit 1
fi

#Clean up
rm *.${GoSuffix}

#Run it
echo "Program built successfully; now running..."
if  ! (./convert)
then
    exit 1
fi


