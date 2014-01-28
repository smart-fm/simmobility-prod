//Allows forwarding of tcp connections between Sim Mobility and minimega apps.
package main

import (
	"net"
	"fmt"
	"os"
	"sync"
	"bufio"
	"regexp"
)

//Holds a connection, reader, and writer. Used for connected clients. Some params can be nil.
type Connection struct {
	conn net.Conn
	read *bufio.Reader
	write *bufio.Writer
}


//Holds all our network-related data.
type NetData struct {
	listenAddr string
	remoteAddr string

	//Lines to be sent to the server.
	outgoing chan string

	//Lines to be sent to clients (any/all).
	incoming chan string

	//A list of pending client connections. These are clients who
	//have connected, but who have not yet sent their ID.
	pending_clients chan Connection

	//A lookup of client connections by ID.
	//Since this is global, there's also a mutex.
	clients_by_id map[string]Connection
	clients_lock sync.RWMutex

	//Finally, the remote connection. This remains null until the first client connection attempt.
	remote Connection
}


//TODO: Describe how our message flow works. Essentially, this code is very dependant on how Sim Mobility sends its 
//      messages, but this is unavoidable when multiplexing. We should chart it out, anyway, especially
//      since it is four-way decoupled using Go channels.
//TODO: This code desperately needs some organizing structs, but I have to read more about Go's memory management first.
func main() {
	//Get our data structure (partly) going
	params := NetData{
		listenAddr : "128.30.87.128:6799",
		remoteAddr : "128.30.87.128:6745",
		outgoing : make(chan string),
		incoming : make(chan string),
		pending_clients : make(chan Connection),
		clients_by_id : make(map[string]Connection),
		//clients_lock : make(sync.RWMutex),
	}

	//Listen/connect on the local address.
	sock_serve, err := net.Listen("tcp", params.listenAddr) 
	if err != nil { 
		fmt.Println("cannot listen: %v", err)
		os.Exit(2)
	} 

	fmt.Println("Params loaded; listening for client connections.")

	//Listen forever for new connections. 
	for { 
		var localconn Connection;
		localconn.conn, err = sock_serve.Accept() 
		if err != nil { 
			fmt.Println("accept failed: %v", err) 
		}

		//The first time, connect to the server.
		if params.remote.read == nil {
			fmt.Println("Opening remote connection.") 
			params.remote.conn, err = net.Dial("tcp", params.remoteAddr) 
			if err != nil { 
				fmt.Println("remote dial failed: %v", err)
				os.Exit(2)
			}

			//Create our readers/writers
			params.remote.read = bufio.NewReader(params.remote.conn)
			params.remote.write = bufio.NewWriter(params.remote.conn)

			//Start our server threads here
			go receive_remote(&params)
			go forward_to_client(&params)
			go forward_to_server(&params)
		}

		//Make a reader/writer.
		localconn.read = bufio.NewReader(localconn.conn)
		localconn.write = bufio.NewWriter(localconn.conn)

		//TODO: We may need to inject a "NEWCLIENT" message here.

		//Pend this client until later.
		params.pending_clients <- localconn

		//...but start its reader/writer loops now.
		go receive_client(&params, localconn)
	} 
}

//Receive a message from the server and push it to "incoming".
//Should be FAST.
func receive_remote(params *NetData) {
	//Keep reading from the server, and pushing it to the client.
	for {
		line,err := params.remote.read.ReadString('\n')
		if err != nil { 
			fmt.Println("remote read failed: %v", err)
			os.Exit(2)
		}

		//TODO: Use conn (maybe wrapped).
		fmt.Println("Reading data from the server..")
		params.incoming <- line
    }
}

//Pull lines from incoming and forward to the actual destination agent.
func forward_to_client(params *NetData) {
	//Easy matching: "DEST_AGENT":"123456789012"
	destRegex := regexp.MustCompile("\"DEST_AGENT\" *: *\"([^\"]*)\"")

	var line string
	for {
		line = <-params.incoming

		//TEMP
		matches := destRegex.FindStringSubmatch(line)
		if matches == nil {
			fmt.Println("forward to client failed -- no dest_id")
			os.Exit(2)
		}

		//Route correctly
		destId := matches[1]
		var destConn Connection

		if destId == "0" {
			//Else, just pull a pending client; this will only work for ONE message, but 
			//that's all Sim Mobility expects.
			destConn = <-params.pending_clients //TODO: This will wait forever; maybe we want a list?
		} else {
			var ok bool
			params.clients_lock.RLock()
			destConn,ok = params.clients_by_id[destId]
			params.clients_lock.RUnlock()
			if !ok {
				fmt.Println("Error: no client connection known with Id: %s", destId)
				os.Exit(2)
			}
		}

		fmt.Println("Writing to client.")
		_,err := destConn.write.WriteString(line)
		if err != nil {
			fmt.Println("local write failed: %v", err)
			os.Exit(2)
		}
		destConn.write.Flush()
	}
}

//Pull lines from outgoing and forward to the server
func forward_to_server(params *NetData) {
	var line string
	for {
		line = <-params.outgoing
		fmt.Println("Writing to server.")
		_,err := params.remote.write.WriteString(line)
		if err != nil {
			fmt.Println("remote write failed: %v", err)
			os.Exit(2)
		}
		params.remote.write.Flush()
	}
}



//Receive a message from the client and push it to outgoing.
//Also update the agent's ID on the first message.
//Should be FAST.
func receive_client(params *NetData, localconn Connection) {
	//Easy matching: "SENDER":"123456789012"
	sendRegex := regexp.MustCompile("\"SENDER\" *: *\"([^\"]*)\"")

	//The first line read is special.
	line,err := localconn.read.ReadString('\n')
	if err != nil { 
		fmt.Println("FIRST local read failed: %v", err)
		os.Exit(2)
	}

	//Extract the sender's ID.
	matches := sendRegex.FindStringSubmatch(line)
	if matches == nil {
		fmt.Println("client receive failed -- no sender id")
		os.Exit(2)
	}

	//Now add it.
	sendId := matches[1]
	params.clients_lock.Lock()
	params.clients_by_id[sendId] = localconn
	params.clients_lock.Unlock()

	//Forward this message.
	fmt.Println("Reading first line of data from the client.")
	params.outgoing <- line

	//Now, just keep reading from the client, and pushing this information to the server.
	for {
		line,err = localconn.read.ReadString('\n')
		if err != nil { 
			fmt.Println("local read failed: %v", err)
			os.Exit(2)
		}

		fmt.Println("Reading data from the client.")
		params.outgoing <- line
    }
}





