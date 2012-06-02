package sim_mob.vis.util;

public class FastLineParser {
	private String line;
	private int currPos;
	private int endPos; //lastValidChar +1
	
	//Parsed result
	Utility.ParseResults res;
	
	public FastLineParser(String line) {
		this.line = line;
		if (!determineBounds()) {
			res = new Utility.ParseResults();
			res.errorMsg = "Log lines must be enclosed in parentheses.";
		}
	}	
	
	public Utility.ParseResults getResults() {
		if (res==null) {
			parseLogLine();
		}
		return res;
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
	
	
	private void parseLogLine() {
		res = new Utility.ParseResults();
		
		//Proceed linearly from left to right. Avoid recursive descent when possible.
		if (parseType() && parseFrame() && parseID()) {
			parseProps();
		}
	}
	
	private boolean parseType() {
		parseWS();
		res.type = parseQuotedString();
		parseComma();
		return !res.isError();
	}
	
	private boolean parseFrame() {
		res.objID = parseIntOrHexInt();
		parseComma();
		return !res.isError();
	}
	
	private boolean parseID() {
		res.objID = parseIntOrHexInt();
		parseComma();
		return !res.isError();
	}
	
	private void parseProps() {
		//Advance/backup currPos and endPos; check brackets
		//Note: endPos was already >1, so we can decrement it without checking.
		if (line.charAt(currPos)=='{' && line.charAt(endPos-1)=='}') { 
			currPos++;
			endPos--;
			
			checkIndices();
		} else {
			res.errorMsg = "Log lines must enclose property lists in curly braces.";
		}
	}
	
	private void checkIndices() {
		if (currPos<endPos && endPos>=1) {
			return;
		}
		res.errorMsg = "Index out of bounds parsing log line."; 
	}
	
	private char advance() {
		char res = line.charAt(currPos++);
		checkIndices();
		return res;
	}
	
	private void undo() {
		currPos--;
	}
	
	private void parseWS() {
		char currChar = advance();
		while (currChar==' ' || currChar=='\t' || currChar=='\r') {
			currChar = advance();
		}
		undo();
	}
	
	private void parseComma() {
		parseWS();
		if (line.charAt(currPos)==',') {
			advance();
			parseWS();
		} else {
			res.errorMsg = "Line parser error: expected comma.";
		}
	}
	
	private String parseQuotedString() {
		char quoteChar = advance();
		char currChar = '\0';
		StringBuffer sb = new StringBuffer();
		if (quoteChar=='"' || quoteChar=='\'') {
			while ((currChar=advance())!=quoteChar) {
				sb.append(currChar);
			}
		} else {
			res.errorMsg = "Unexpected quote character for string: " + quoteChar;
		}
		return sb.toString();
	}
	
	private int parseIntOrHexInt() {
		int radix = 10;
		StringBuffer sb = new StringBuffer();
		
		//Handle the first character (special case if zero; we might be building up to an 0x)
		int rewindTo = currPos;
		char currChar = advance();
		if (currChar=='0') {
			char nextChar = advance();
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
			currChar = Character.toLowerCase(advance());
			if (currChar>='0' && currChar<='9') {
				sb.append(currChar);
			} else if (radix==16 && currChar>='a' && currChar<='f') {
				sb.append(currChar);
			} else {
				undo();
				break;
			}			
		}

		return Integer.parseInt(sb.toString(), radix);
	}

}












