//Allows forwarding of tcp connections between Sim Mobility and minimega apps.
package main

import (
	"net"
	"fmt"
	"os"
	"bufio"
	"regexp"
)


func main() {
	listenAddr := "128.30.87.128:6799"
   remoteAddr := "128.30.87.128:6745"
	remoteInit := false

	//Outgoing data.
	outgoing := make(chan string)

	//Incoming data.
	//TODO: This will have to be one channel per agent thread.
	incoming := make(chan string)

	//Listen/connect on the local address.
	local, err := net.Listen("tcp", listenAddr) 
	if err != nil { 
		fmt.Println("cannot listen: %v", err)
		os.Exit(2)
	} 

	//Listen forever for new connections. 
	for { 
		conn, err := local.Accept() 
		if err != nil { 
			fmt.Println("accept failed: %v", err) 
		}

		//The first time, connect to the server.
		if remoteInit == false {
			remoteInit = true

			fmt.Println("Opening remote connection.") 
			remote, err := net.Dial("tcp", remoteAddr) 
			if err != nil { 
				fmt.Println("remote dial failed: %v", err)
				os.Exit(2)
			}

			go receive_remote(bufio.NewReader(remote), incoming)
			go forward_to_server(bufio.NewWriter(remote), outgoing)
		}

		//Now, capture and forward all messages.
		go receive_client(bufio.NewReader(conn), outgoing)
		go forward_to_client(bufio.NewWriter(conn), incoming)
	} 
}

func receive_remote(remote *bufio.Reader, incoming chan string) {
	//Easy matching: "DEST_AGENT":"123456789012"
	destRegex := regexp.MustCompile("\"DEST_AGENT\" *: *\"([^\"]*)\"")

	//Keep reading from the server, and pushing it to the client.
	for {
		line,err := remote.ReadString('\n')
		if err != nil { 
			fmt.Println("remote read failed: %v", err)
			os.Exit(2)
		}

		//TEMP
		matches := destRegex.FindStringSubmatch(line)
		if matches == nil {
			fmt.Println("remote route failed -- no dest_id: %v", err)
			os.Exit(2)
		}

		//TODO: This is how we route data.
		//destId := matches[1]


		fmt.Println("Reading data from the server..")
		incoming <- line
    }
}

func receive_client(local *bufio.Reader, outgoing chan string) {
	//TODO: We may need to inject a "NEWCLIENT" message here.

	//Now, just keep reading from the client, and pushing this information to the server.
	for {
		line,err := local.ReadString('\n')
		if err != nil { 
			fmt.Println("local read failed: %v", err)
			os.Exit(2)
		}

		fmt.Println("Reading data from the client.")
		outgoing <- line
    }
}

func forward_to_server(remote *bufio.Writer, outgoing chan string) {
	var line string
	for {
		line = <-outgoing
		fmt.Println("Writing to server.")
		_,err := remote.WriteString(line)
		if err != nil {
			fmt.Println("remote write failed: %v", err)
			os.Exit(2)
		}
		remote.Flush()
	}
}


func forward_to_client(local *bufio.Writer, incoming chan string) {
	var line string
	for {
		line = <-incoming
		fmt.Println("Writing to client.")
		_,err := local.WriteString(line)
		if err != nil {
			fmt.Println("local write failed: %v", err)
			os.Exit(2)
		}
		local.Flush()
	}
}






