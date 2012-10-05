package sim_mob.vis.util;

import java.util.Random;

public class FastLineParser {
	private String line;
	private int currPos;
	private int endPos; //lastValidChar +1
	
	//TEMP
	private static Random rand = new Random();
	
	public FastLineParser() {
	}
	
	
	
	public Utility.ParseResults getResults(String line) {
		Utility.ParseResults res = null;
		if (!reset(line)) {
			res = new Utility.ParseResults();
			res.errorMsg = "Log lines must be enclosed in parentheses.";
		} else {
			res = parseLogLine();
		}
		return res;
	}
	
	private boolean reset(String line) {
		this.line = line;
		currPos = 0;
		endPos = 0;
		
		if (rand.nextInt(1000)<10) {
			//System.out.println("Line: " + line);
		}
		
		return determineBounds();
	}
	
	//Find the left and right parentheses.
	private boolean determineBounds() {
		//Left, +1
		for (currPos=0; currPos<line.length(); currPos++) {
			if (line.charAt(currPos)=='(') {
				currPos++;
				break;
			}
		}
		
		//Right +0
		for (endPos=line.length()-1; endPos>0; endPos--) {
			if (line.charAt(endPos)==')') {
				break;
			}
		}
		
		//Check
		if ((currPos>=line.length()-1) || (endPos<=1) || (currPos>=endPos)) {
			currPos = endPos = 0;
			return false;
		}
		
		return true;
	}
	
	
	private Utility.ParseResults parseLogLine() {
		Utility.ParseResults res = new Utility.ParseResults();
		
		//Proceed linearly from left to right. Avoid recursive descent when possible.
		if (parseType(res) && parseFrame(res) && parseID(res)) {
			parseProps(res);
		}
		
		return res;
	}
	
	private boolean parseType(Utility.ParseResults res) {
		parseWS(res);
		res.type = parseQuotedString(res);
		parseCharacter(res, ',');
		return !res.isError();
	}
	
	private boolean parseFrame(Utility.ParseResults res) {
		res.frame = (int)parseLongOrHex(res);
		parseCharacter(res, ',');
		return !res.isError();
	}
	
	private boolean parseID(Utility.ParseResults res) {
		res.objID = parseLongOrHex(res);
		parseCharacter(res, ',');
		return !res.isError();
	}
	
	private void parseProps(Utility.ParseResults res) {
		//Advance/backup currPos and endPos; check brackets
		//Note: endPos was already >1, so we can decrement it without checking.
		if (line.charAt(currPos)=='{' && line.charAt(endPos-1)=='}') { 
			currPos++;
			endPos--;
			checkIndices(res);
			
			//Now actually parse pairs of properties
			while (currPos!=endPos) {
				String key = parseQuotedString(res);
				parseCharacter(res, ':');
				String value = parseQuotedString(res);
				if (currPos<endPos) {
					parseCharacter(res, ',', false);
				}
				res.properties.put(key, value);
			}
		} else {
			res.errorMsg = "Log lines must enclose property lists in curly braces.";
		}
	}
	
	private void checkIndices(Utility.ParseResults res) {
		if (currPos<endPos && endPos>=1) {
			return;
		}
		res.errorMsg = "Index out of bounds parsing log line [" + currPos + "," + endPos + "] => " + line.charAt(currPos); 
	}
	
	private char advance(Utility.ParseResults res) {
		checkIndices(res);
		return line.charAt(currPos++);
	}
	
	private void undo() {
		currPos--;
	}
	
	private void parseWS(Utility.ParseResults res) {
		//"end of input" counts as whitespace, so don't crash if this happens.
		if (currPos<endPos) {
			char currChar = advance(res);
			while (currChar==' ' || currChar=='\t' || currChar=='\r') {
				currChar = advance(res);
			}
			undo();
		}
	}
	
	private void parseCharacter(Utility.ParseResults res, char letter) {
		parseCharacter(res, letter, true);
	}
	private void parseCharacter(Utility.ParseResults res, char letter, boolean required) {
		parseWS(res);
		if (line.charAt(currPos)==letter) {
			advance(res);
			parseWS(res);
		} else if (required) {
			res.errorMsg = "Line parser error, expected: " + letter;
		}
	}
	
	private String parseQuotedString(Utility.ParseResults res) {
		char quoteChar = advance(res);
		char currChar = '\0';
		StringBuffer sb = new StringBuffer();
		if (quoteChar=='"' || quoteChar=='\'') {
			while ((currChar=advance(res))!=quoteChar) {
				sb.append(currChar);
			}
		} else {
			res.errorMsg = "Unexpected quote character for string: " + quoteChar;
		}
		return sb.toString();
	}
	
	private long parseLongOrHex(Utility.ParseResults res) {
		int radix = 10;
		StringBuffer sb = new StringBuffer();
		
		//Handle the first character (special case if zero; we might be building up to an 0x)
		int rewindTo = currPos;
		char currChar = advance(res);
		if (currChar=='0') {
			char nextChar = advance(res);
			if (Character.toLowerCase(nextChar)=='x') {
				radix = 16;
				rewindTo = -1;
			}
		}
		
		//If we didn't find "0x", then rewind the parser. Breaks our rule of always advancing, but
		//  doesn't lead to recursive descent either.
		if (rewindTo != -1) {
			currPos = rewindTo;
		}
		
		//Now process as normal.
		for (;;) {
			currChar = Character.toLowerCase(advance(res));
			if (currChar>='0' && currChar<='9') {
				sb.append(currChar);
			} else if (radix==16 && currChar>='a' && currChar<='f') {
				sb.append(currChar);
			} else {
				undo();
				break;
			}			
		}

		return Long.parseLong(sb.toString(), radix);
	}

}












