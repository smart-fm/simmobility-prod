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
	Length  int
	Width   int
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
var lineRegex *regexp.Regexp = nil
var keyvalRegex *regexp.Regexp = nil


func ParseDriverProps(drv *DriverTick, propsStr string) (err error) {
	//The first time this is called, compile the regex.
	if keyvalRegex == nil {
		keyvalRegex, err = regexp.Compile(propKeyVal)
		if err != nil {
			return err
		}
	}

	//Keep matching properties, throwing them into a dictionary.
	props := make(map[string] string)
	propsRaw := keyvalRegex.FindAllStringSubmatch(propsStr, -1)
	for _,arr := range propsRaw {
		props[strings.ToLower(arr[1])] = arr[2]
	}

    //Check that we have required properties
	if props["xpos"]=="" || props["ypos"]=="" || props["angle"]=="" || props["length"]=="" || props["width"]=="" {
		return errors.New("Missing (or empty) required property in propsStr: \"" + propsStr + "\"")
	}

	//Set them
	var x,y int
	x,err = parseIntOrPass(props["xpos"], 10, err)
	y,err = parseIntOrPass(props["ypos"], 10, err)
	drv.Angle,err = parseFloatOrPass(props["angle"], err)
	drv.Length,err = parseIntOrPass(props["length"], 10, err)
	drv.Width,err = parseIntOrPass(props["width"], 10, err)
	if (err != nil) {
		return err
	}

	//Tidy up x/y
	drv.XPos = float64(x) / 100
	drv.YPos = float64(y) / 100

	return nil
}


func ParseLine(line *string) (typeStr string, frame, ag_id int, propsStr string, err error) {
	//The first time this is called, compile the regex.
	if lineRegex == nil {
		lineRegex, err = regexp.Compile("^"+lPar+ qStr+cSep + iStr+cSep + iStr+cSep + lBrc + propsGeneral + rBrc +rPar+"$")
		if err != nil {
			return
		}
	}

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



