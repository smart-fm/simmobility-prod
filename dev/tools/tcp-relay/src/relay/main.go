//Allows forwarding of tcp connections between Sim Mobility and minimega apps.
package main

import (
	"net"
	"fmt"
	"os"
	"sync"
	"flag"
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
	//Deub flag?
	debug_flag bool

	//What addresses are we listening on?
	listenAddr string
	remoteAddr string

	//Lines to be sent to the server.
	outgoing chan string

	//Lines to be sent to clients (any/all).
	incoming chan []byte

	//A list of pending client connections. These are clients who
	//have connected, but who have not yet sent their ID.
	pending_clients chan Connection

	//A lookup of client connections by ID.
	//Since this is global, there's also a mutex.
	clients_by_id map[string]Connection
	clients_lock sync.RWMutex

	//The same remote connection is handed off multiple times using these channels.
	//Push something onto the first and then read from the second.
	getRemote chan bool
	remote chan Connection
}


//TODO: Describe how our message flow works. Essentially, this code is very dependant on how Sim Mobility sends its 
//      messages, but this is unavoidable when multiplexing. We should chart it out, anyway, especially
//      since it is four-way decoupled using Go channels.
//TODO: This code desperately needs some organizing structs, but I have to read more about Go's memory management first.
func main() {
	//Get our data structure (partly) going
	params := NetData{
		listenAddr : "192.168.0.102:6799",
		remoteAddr : "192.168.0.102:6745",
		outgoing : make(chan string),
		incoming : make(chan []byte),
		pending_clients : make(chan Connection),
		clients_by_id : make(map[string]Connection),
		getRemote : make(chan bool),
		remote : make(chan Connection),
	}

	//Parse our command line flags.
	flag.BoolVar(&params.debug_flag, "debug", false, "Turns on detailed tracing of messages (may want to pipe into a file).")
	flag.Parse()

	//Listen/connect on the local address.
	sock_serve, err := net.Listen("tcp", params.listenAddr) 
	if err != nil { 
		fmt.Println("cannot listen: %v", err)
		os.Exit(2)
	} 

	fmt.Println("Params loaded; listening for client connections. Debug is: " , params.debug_flag)

	//Listen forever for new connections. 
	totalConn := 0
	firstTime := true
	for { 
		var localconn Connection;
		localconn.conn, err = sock_serve.Accept() 
		if err != nil { 
			fmt.Println("accept failed: %v", err) 
		}
		totalConn += 1

		//Connect in a different goroutine, so we can get back to handling connections.
		go handle_connection(&params, localconn, firstTime, totalConn)
		firstTime = false
	} 
}


func connect_and_distribute(params *NetData) {
	//Connect
	conn, err := net.Dial("tcp", params.remoteAddr) 
	if err != nil { 
		fmt.Println("remote dial failed: %v", err)
		os.Exit(2)
	}

	//Create our readers/writers
	var res Connection
	res.conn = conn
	res.read = bufio.NewReader(res.conn)
	res.write = bufio.NewWriter(res.conn)

	//Start our server threads here
	go receive_remote(params)
	go forward_to_client(params)
	go forward_to_server(params)

	//Now just spin and hand out connections as they are requested.
	for {
		_ = <-params.getRemote
		params.remote <- res
	}
}


func get_remote(params* NetData) Connection {
	params.getRemote <- true
	res := <-params.remote
	return res
}


func handle_connection(params *NetData, localconn Connection, firstTime bool, totalConn int) {
	//The first time, connect to the server.
	if firstTime {
		fmt.Println("Opening remote connection.")
		go connect_and_distribute(params)
	} else {
		//Inject a "new_client" message here, which instructs the server to send a WHOAREYOU message.
		fmt.Println("New local connection on existing remote connection: " , totalConn) 
		new_cli_msg := "\x01\x01\x01\x01\x00\x00\x00\x1E" + "00" + "\x00\x00\x19" + "{\"msg_type\":\"new_client\"}"  //v1 format
		params.outgoing <- new_cli_msg
	}

	//Make a reader/writer.
	localconn.read = bufio.NewReader(localconn.conn)
	localconn.write = bufio.NewWriter(localconn.conn)

	//Pend this client until later.
	params.pending_clients <- localconn

	//...but start its reader/writer loops now.
	go receive_client(params, localconn)
}


//Receive a message from the server and push it to "incoming".
//Should be FAST.
func receive_remote(params *NetData) {
	//Our buffer. Must be at least as long as the longest message
	rec_buff := make([]byte, 20480+8)
	rec_ind := 0

	//Keep reading from the server, and pushing it to the client.
	remote := get_remote(params)
	for {
		//Read the 8-byte header.
		for rec_ind < 8 {
			len,err := remote.read.Read(rec_buff[rec_ind:8])
			rec_ind += len
			if (err != nil) {
				fmt.Println("remote read failed: %v", err)
				os.Exit(2)
			}
		}

		//Sanity check.
		if (rec_ind>8) { fmt.Println("ERRR: read more than 8.") ; os.Exit(2) }

		//Compute the remaining message length (header[4,5,6,7])
		data_len := ((int(rec_buff[0])&0xFF)<<24) | ((int(rec_buff[1])&0xFF)<<16) | ((int(rec_buff[2])&0xFF)<<8) | (int(rec_buff[3])&0xFF)
		data_len += rec_ind  //+8
		if data_len > len(rec_buff) {
			fmt.Println("ERRR: message exceeds max size.")
			os.Exit(2) 
		}

		//Read it.
		for rec_ind < data_len {
			len,err := remote.read.Read(rec_buff[rec_ind:data_len])
			rec_ind += len
			if (err != nil) {
				fmt.Println("remote read failed: %v", err)
				os.Exit(2)
			}
		}

		//Sanity check.
		if (rec_ind>data_len) { fmt.Println("ERRR: read more than DATA.") ; os.Exit(2) }

		if params.debug_flag {
			fmt.Print("Reading data from the server ###" , rec_buff[0:rec_ind] , "###\n")
		}

		//Pass a copy of this buffer off to the client.
		line := make([]byte, rec_ind)
		copy(line, rec_buff[0:rec_ind])
		params.incoming <- line
    }
}

//Pull lines from incoming and forward to the actual destination agent.
func forward_to_client(params *NetData) {
	var line []byte
	for {
		line = <-params.incoming

		//Retrieve the destination ID (sendIDLength in byte 2, destIDLength in byte 3, sendID starts after byte 8, and destID after sendID)
		destIDStart := 8 + (int(line[1])&0xFF)
		destIDLen := int(line[2])&0xFF
		destId := string(line[destIDStart:destIDStart+destIDLen])

		//Route correctly
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

		if params.debug_flag {
			fmt.Print("Writing to client with id [" , destId , "], data: ###" , line , "###\n")
		}
		_,err := destConn.write.Write(line)
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
	remote := get_remote(params)
	for {
		line = <-params.outgoing
		if params.debug_flag {
			fmt.Print("Writing to server: ###" , line , "###\n")
		}
		_,err := remote.write.WriteString(line)
		if err != nil {
			fmt.Println("remote write failed: %v", err)
			os.Exit(2)
		}
		remote.write.Flush()
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
	if params.debug_flag {
		fmt.Print("Reading (first time) from client with id [" , sendId , "], data: ###" , line , "###\n")
	}
	params.outgoing <- line

	//Now, just keep reading from the client, and pushing this information to the server.
	for {
		line,err = localconn.read.ReadString('\n')
		if err != nil { 
			fmt.Println("local read failed: %v", err)
			os.Exit(2)
		}

		if params.debug_flag {
			fmt.Print("Reading from client with id [" , sendId , "], data: ###" , line , "###\n")
		}
		params.outgoing <- line
    }
}





