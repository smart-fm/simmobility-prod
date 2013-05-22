package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"sort"
	"strings"
	"bytes"
	"math"
	"errors"
	"encoding/binary"
    "container/list"
	"./simmob"
)

func processLine(line *string) (*simmob.DriverTick, *simmob.SimSettings, error) {
	//Sample lines will be something like this:
	//("TYPE",frame_id,agent_id,{"k1":"v1","k2":"v2",...})
	typeStr, frameId, agentId, propsStr, err := simmob.ParseLine(line)
	if err!=nil {
		return nil, nil, err
	}
	
	//Dispatch based on type.
	if typeStr=="driver" {
		//Simple properties.
		res := new(simmob.DriverTick)
		res.AgentId = agentId
		res.Frame = frameId

		//Dispatch construction of the propStr
		err = simmob.ParseDriverProps(res, propsStr)
		return res, nil, err
	} else if typeStr=="simulation" {
		//Dispatch construction
		res := new(simmob.SimSettings)
		err = simmob.ParseSimulationSettings(res, propsStr)
		return nil, res, err
	}

	//Failsafe
	return nil, nil, nil
}

func parseFile(f *os.File) (map[int] *list.List, float32, error) {  //float is tick-length in seconds
	ticks := make(map[int] *list.List)
	scanner := bufio.NewScanner(f)
	var tick_len float32
	isErr := false
	for scanner.Scan() {
		//Only process lines that begin and end with "(",")"
		line := strings.Trim(scanner.Text(), " \t\r\n")
		if len(line) >= 2 && strings.HasPrefix(line,"(") && strings.HasSuffix(line,")") {
			drivTick, simTick, err := processLine(&line)
			if err != nil {
				fmt.Println("Error parsing tick:\n" , err)
				isErr = true
				continue
			}

			//We might still have a nil results; e.g., it might be a pedestrian.
			if (drivTick != nil) {
				if _,exists := ticks[drivTick.Frame]; !exists {
					ticks[drivTick.Frame] = list.New()
				}
				ticks[drivTick.Frame].PushBack(drivTick)
			}

			//Also...
			if (simTick != nil) {
				tick_len = float32(simTick.FrameTickMs) / 1000.0 //Convert to seconds
			}
		}
	}

	//We *need* a tick_len
	if tick_len==0 {
		return nil, 0, errors.New("Simulation output file does not list \"frame-time-ms\" (in the \"simulation\" tag).")
	}

	//Final check
	if isErr {
		return nil, 0, errors.New("Parsing of ticks did not complete without errors.")
	}

	return ticks, tick_len, scanner.Err()
}

func sort_keys(source map[int] *list.List) (res []int) {
	res = make([]int, len(source))
    i := 0
    for k,_ := range source {
        res[i] = k
        i++
    }
	sort.Ints(res)
	return
}

func calc_bounds(ticks map[int] *list.List, boundary float64) (minX,minY,maxX,maxY float64) {
	first := true
	for _,val := range ticks {
		for e:=val.Front(); e!=nil; e=e.Next() { 
			drv := e.Value.(*simmob.DriverTick)
			//First; just save all.
			if first {
				minX = drv.XPos
				maxX = minX
				minY = drv.YPos
				maxY = minY
				first = false
			} else {
				minX = math.Min(minX, drv.XPos)
				maxX = math.Max(maxX, drv.XPos)
				minY = math.Min(minY, drv.YPos)
				maxY = math.Max(maxY, drv.YPos)
			}
		}
	}

	//Apply the border
	minX -= boundary
	maxX += boundary
	minY -= boundary
	maxY += boundary
	return
}

//Convert an integer value to a 4-byte array.
func intTo4Bytes(value int32) ([]byte) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, value)
	if err != nil {
		log.Fatal("Can't convert int to 4 bytes:", err)
	}
	return buf.Bytes()
}

//Convert a floating-point value to a 4-byte array.
func floatTo4Bytes(value float32) ([]byte) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, value)
	if err != nil {
		log.Fatal("Can't convert float to 4 bytes:", err)
	}
	return buf.Bytes()
}

func main() {
	//Param 1 = input file.
	fileName := "out.txt"
	if len(os.Args) >= 2 {
		fileName = os.Args[1]
	}

	//Param 2 = output file.
	outName := "out.trj"
	if len(os.Args) >= 3 {
		outName = os.Args[2]
	}

	//Make sure input file exists.
	f, err := os.Open(fileName)
	if err != nil {
		log.Fatal("Error, file \"", fileName, "\" couldn't be opened:\n", err)
	}

	//Make sure output file is reachable.
	out,err := os.Create(outName)
	if err != nil {
		log.Fatal("Error, file \"", outName, "\" won't work for output:\n", err)
	}

	//Parse it.
	var ticks map[int] *list.List
	var tick_len float32
	ticks, tick_len, err = parseFile(f)
	if err != nil {
		log.Fatal("Error parsing:\n", err)
	}
	f.Close()

	//Print the format record
	out.Write([]byte{0, 'L'}) //Little-endian
	out.Write(floatTo4Bytes(1.04))

	//Print the dimensions record. (X increases right; Y increases up)
	minX,minY,maxX,maxY := calc_bounds(ticks, 100) //10m added to each side.
	out.Write([]byte{1, 1}) //Metric
	out.Write(floatTo4Bytes(1.0)) //1 meter per unit
	out.Write(intTo4Bytes(int32(minX))) //Left
	out.Write(intTo4Bytes(int32(minY))) //Bottom
	out.Write(intTo4Bytes(int32(maxX))) //Right
	out.Write(intTo4Bytes(int32(maxY))) //Top

	//Print each TIMESTEP record (which triggers VEHICLE) records
	keys := sort_keys(ticks)
	for _,tt := range keys {
		out.Write([]byte{2})
		out.Write(floatTo4Bytes(float32(tt)*tick_len))

		//Print each VEHICLE record in this tick.
		val := ticks[tt]
		for e:=val.Front(); e!=nil; e=e.Next() { 
			drv := e.Value.(*simmob.DriverTick)
			out.Write([]byte{3})
			out.Write(intTo4Bytes(int32(drv.AgentId)))

			//TODO: out.txt now contains "curr-segment", which can be used to retrieve the Link ID. Lanes are tougher, since their IDs won't match the
			//      output (SM uses lane mid-lines; out.txt uses lane edge-lines). Either way, we should eventually list both Link and Lane IDs for Drivers.
			out.Write(intTo4Bytes(0))
			out.Write([]byte{0})

			//Middle-front bumper (x,y)
			out.Write(floatTo4Bytes(0))
			out.Write(floatTo4Bytes(0))

			//Middle-rear bumper (x,y)
			out.Write(floatTo4Bytes(0))
			out.Write(floatTo4Bytes(0))

			//Vehicle length/width
			out.Write(floatTo4Bytes(drv.Length))
			out.Write(floatTo4Bytes(drv.Width))

			//Forward speed, accell
			out.Write(floatTo4Bytes(drv.FwdSpeed))
			out.Write(floatTo4Bytes(drv.FwdAccel))
		}
	}

	out.Close()
	fmt.Println("Warning: Link and Lane IDs are currently not set (they are optional).")
	fmt.Println("Done")
}

