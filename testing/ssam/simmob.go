package simmob

import (
	"regexp"
	"errors"
	"strconv"
	"strings"
	"fmt"
)

type DriverTick struct {
	AgentId int
	Frame   int
	XPos    float64 //meters
	YPos    float64
	Angle   float32 //degrees
	Length  float32 //meters
	Width   float32 //meters
	FwdSpeed float32 //m/s
	FwdAccel float32 //m/s^2
}

type SimSettings struct {
	FrameTickMs int
}

var qStr string = "\"([^\"]*)\""
var iStr string = "(-?[x0-9a-fA-F]+)"
var cSep string = " *,? *"
var scSep string = " *:? *"
var lPar string = "\\("
var rPar string = "\\)"
var lBrc string = "\\{"
var rBrc string = "\\}"
var propsGeneral string = "([^}]+)"
var propKeyVal string = qStr + scSep + qStr
var lineStr string = "^"+lPar+ qStr+cSep + iStr+cSep + iStr+cSep + lBrc + propsGeneral + rBrc +rPar+"$"
var lineRegex *regexp.Regexp = nil
var keyvalRegex *regexp.Regexp = nil


func compileRegex(src *string, dest *regexp.Regexp) (res *regexp.Regexp, err error) {
	if dest == nil {
		res, err = regexp.Compile(*src)
	} else {
		res = dest
	}
	return
}

func parsePropMap(str string) (props map[string] string, err error) {
	//The first time this is called, compile the regex.
	keyvalRegex,err = compileRegex(&propKeyVal, keyvalRegex)
	props = make(map[string] string)

	//Keep matching properties, throwing them into a dictionary.
	//props := make(map[string] string)
	propsRaw := keyvalRegex.FindAllStringSubmatch(str, -1)
	for _,arr := range propsRaw {
		props[strings.ToLower(arr[1])] = arr[2]
	}
	return
}

func ParseSimulationSettings(simSet *SimSettings, propsStr string) (err error) {
	//Retrieve the properties list
	props,err := parsePropMap(propsStr)

    //Check that we have required properties
	if props["frame-time-ms"]=="" {
		return errors.New("Missing (or empty) required property in propsStr: \"" + propsStr + "\"")
	}

	//Set it
	simSet.FrameTickMs,err = parseIntOrPass(props["frame-time-ms"], 10, err)
	if (err != nil) {
		fmt.Println("Mising frame tick in Simulation tag") //Note: Leave this comment in; I dislike commenting out fmt all the time.
		return err
	}

	return nil
}


func ParseDriverProps(drv *DriverTick, propsStr string) (err error) {
	//Retrieve the properties list
	props,err := parsePropMap(propsStr)

    //Check that we have required properties
	if props["xpos"]=="" || props["ypos"]=="" || props["angle"]=="" || props["length"]=="" || props["width"]=="" || props["fwd-speed"]=="" || props["fwd-accel"]=="" {
		return errors.New("Missing (or empty) required property in propsStr: \"" + propsStr + "\"")
	}

	//Set them
	var x,y,l,w int
	var ac,sp float32
	x,err = parseIntOrPass(props["xpos"], 10, err)
	y,err = parseIntOrPass(props["ypos"], 10, err)
	drv.Angle,err = parseFloatOrPass(props["angle"], err)
	l,err = parseIntOrPass(props["length"], 10, err)
	w,err = parseIntOrPass(props["width"], 10, err)
	sp,err = parseFloatOrPass(props["fwd-speed"], err)
	ac,err = parseFloatOrPass(props["fwd-accel"], err)
	if (err != nil) {
		return err
	}

	//Tidy up x/y/l/w/sp/ac
	drv.XPos = float64(x) / 100
	drv.YPos = float64(y) / 100
	drv.Length = float32(l) / 100
	drv.Width = float32(w) / 100
	drv.FwdSpeed = float32(sp) / 100
	drv.FwdAccel = float32(ac) / 100

	return nil
}


func ParseLine(line *string) (typeStr string, frame, ag_id int, propsStr string, err error) {
	//The first time this is called, compile the regex.
	lineRegex,err = compileRegex(&lineStr, lineRegex)

	//Match
	matches := lineRegex.FindStringSubmatch(*line)
	if matches == nil {
		err = errors.New("Line does not match regex: \"" + *line + "\"")
		return
	}

	//Retrieve capture groups. The "or pass" function will hand off errors without further processing.
	typeStr = strings.ToLower(matches[1])
	frame,err = parseIntOrPass(matches[2], 10, err)
	ag_id,err = parseIntOrPass(matches[3], 10, err)
	if (err != nil) {
		return
	}

	//Save the props for later.
	propsStr = matches[4]
	return
}



//Helper for parsing an integer; simply "passes" on existing errors (by returning them verbatim)
func parseIntOrPass(str string, radix int, err error) (int, error) {
	if (err != nil) {
		return 0,nil
	}

	//Parse a res64 first
	res,err := strconv.ParseInt(str, radix, 0)
	if (err!=nil) {
		return 0,nil
	}
	return int(res),nil
}

//Helper for parsing a 32-bit float; simply "passes" on existing errors (by returning them verbatim)
func parseFloatOrPass(str string, err error) (float32, error) {
	if (err != nil) {
		return 0,nil
	}

	//Parse a res64 first
	res,err := strconv.ParseFloat(str, 32)
	if (err!=nil) {
		return 0,nil
	}
	return float32(res),nil
}



