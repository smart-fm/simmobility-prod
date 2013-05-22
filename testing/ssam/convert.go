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
	"encoding/binary"
    "container/list"
	"./simmob"
)

func processLine(line *string) (*simmob.DriverTick, error) {
	//Sample lines will be something like this:
	//("TYPE",frame_id,agent_id,{"k1":"v1","k2":"v2",...})
	typeStr, frameId, agentId, propsStr, err := simmob.ParseLine(line)
	if err!=nil {
		return nil, err
	}
	
	//Dispatch based on type.
	if typeStr=="driver" {
		//Simple properties.
		res := new(simmob.DriverTick)
		res.AgentId = agentId
		res.Frame = frameId

		//Dispatch construction of the propStr
		err = simmob.ParseDriverProps(res, propsStr)
		return res, err
	}

	//Failsafe
	return nil, nil
}

func parseFile(f *os.File) (map[int] *list.List, error) {
	ticks := make(map[int] *list.List)
	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		//Only process lines that begin and end with "(",")"
		line := strings.Trim(scanner.Text(), " \t\r\n")
		if len(line) >= 2 && strings.HasPrefix(line,"(") && strings.HasSuffix(line,")") {
			drivTick, err := processLine(&line)
			if err != nil {
				fmt.Println("Error parsing tick:\n" , err)
				continue
			}

			//We might still have a nil results; e.g., it might be a pedestrian.
			if (drivTick != nil) {
				if _,exists := ticks[drivTick.Frame]; !exists {
					ticks[drivTick.Frame] = list.New()
				}
				ticks[drivTick.Frame].PushBack(drivTick)
			}
		}
	}

	return ticks, scanner.Err()
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
	ticks, err = parseFile(f)
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

	//Now convert each tick.
	keys := sort_keys(ticks)
	for _,key := range keys {
		fmt.Println("Tick: ", key)
	}

	out.Close()
	fmt.Println("Done")
}

