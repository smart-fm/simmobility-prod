package sim_mob.conf;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Stroke;
import java.io.BufferedReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Hashtable;

import org.apache.batik.css.parser.Parser;
import org.w3c.css.sac.CSSException;
import org.w3c.css.sac.DocumentHandler;
import org.w3c.css.sac.InputSource;
import org.w3c.css.sac.LexicalUnit;
import org.w3c.css.sac.SACMediaList;
import org.w3c.css.sac.Selector;
import org.w3c.css.sac.SelectorList;

/**
 * Load a CSS_Interface via the Batik library.
 * 
 * @author sethhetu
 */
public class BatikCSS_Loader {
	//List of colors by ID
	private static Hashtable<String, Color> IdentityColors;
	
	//Load each file in order, overwriting settings as you go.
	public static CSS_Interface LoadCSS_Interface(ArrayList<BufferedReader> files) {
		if (IdentityColors==null) {
			IdentityColors = new Hashtable<String, Color>();
			CssColorIdentifiers.LoadIdentityColors(IdentityColors);
		}
		
		CSS_Interface csi = new CSS_Interface();
		for (BufferedReader f : files) {
			try {
				LoadSingleFile(csi, f);
			} catch (Exception ex) {
				System.out.println("Can't parse colors file");
				throw new RuntimeException(ex); 
			}
		}
		return csi;
	}
	
	private static void LoadSingleFile(CSS_Interface res, BufferedReader f) throws InstantiationException, IOException, IllegalAccessException, ClassNotFoundException {
		Parser p = new Parser();
		p.setDocumentHandler(new MyDocumentHandler(res));
		p.parseStyleSheet(new InputSource(f));
	}
	
	
	
	//Processing nodes & helpers
	private static float ReadGeneralFloat(LexicalUnit value) {
		if (value.getLexicalUnitType()==LexicalUnit.SAC_PIXEL) {
			return value.getFloatValue();
		}
		throw new RuntimeException("Unknown lexical type for general float (pixel currently supported): " + value.getLexicalUnitType());
	}
	private static int ReadGeneralStyle(LexicalUnit value) {
		if (value.getLexicalUnitType()==LexicalUnit.SAC_IDENT) {
			if (value.getStringValue().equals("solid")) {
				return 0;
			}
			throw new RuntimeException("Un-supported border-style: " + value.getStringValue());
		}
		throw new RuntimeException("Unknown border style lexical type: " + value.getLexicalUnitType());
	}
	private static Color ReadGeneralColor(LexicalUnit value) {
		if (value.getLexicalUnitType()==LexicalUnit.SAC_IDENT) {
			return ReadColorIdentity(value);
		} else if (value.getLexicalUnitType()==LexicalUnit.SAC_RGBCOLOR) {
			return ReadColorRgb(value);
		}
		throw new RuntimeException("Unknown lexical type for general color: " + value.getLexicalUnitType());
	}
	private static Object[] ReadAndMakeColor(LexicalUnit value) { //Return [Color,Stroke]
		//Width
		float width = ReadGeneralFloat(value);
		
		//Style
		value = value.getNextLexicalUnit();
		if (value==null) {
			throw new RuntimeException("Stroke shortcut is missing properties.");
		}
		/*int style =*/ ReadGeneralStyle(value);
		
		//Color
		value = value.getNextLexicalUnit();
		if (value==null) {
			throw new RuntimeException("Stroke shortcut is missing properties.");
		}
		Color clr = ReadGeneralColor(value);
		
		//Done!
		return new Object[] {
			clr,
			new BasicStroke(width)
		};
	}
	
	private static Color ReadColorIdentity(LexicalUnit value) {
		Color res = IdentityColors.get(value.getStringValue().toLowerCase());
		if (res==null) {
			throw new RuntimeException("Invalid color identity: " + value.getStringValue());
		}
		return res;
	}
	
	private static Color ReadColorRgb(LexicalUnit value) {
		ArrayList<Integer> rgb = new ArrayList<Integer>();
		LexicalUnit currPair = value.getParameters();;
		while (currPair!=null) {
			if (currPair.getLexicalUnitType()==LexicalUnit.SAC_INTEGER) {
				rgb.add(currPair.getIntegerValue());
			}
			currPair = currPair.getNextLexicalUnit();
			if (currPair==null || currPair.getLexicalUnitType()!=LexicalUnit.SAC_OPERATOR_COMMA) {
				break;
			}
			currPair = currPair.getNextLexicalUnit();
		}
		
		//Generate the RGB equivalent.
		if (rgb.size()!=3) {
			throw new RuntimeException("Invalid RGB string.");
		}
		
		return new Color(rgb.get(0), rgb.get(1), rgb.get(2));
	}
	
	
	
	//Helper class
	private static class MyDocumentHandler implements DocumentHandler {
		private CSS_Interface res;
		private String currSelectorName; //What we're currently working on.
		
		private float currStrokeWidth; //Partial building.
		private int currStrokeStyle;   //Partial building. For now, only "solid" (0)
		public MyDocumentHandler(CSS_Interface res) { this.res = res; }
		
		
		
		public void property(String name, LexicalUnit value, boolean important) throws CSSException {
			if (currSelectorName==null) {
				return;
			}			
			
			//Handle the value based on the name.
			name = name.trim();
			if (name.equals("background")) {
				res.backgroundColors.put(currSelectorName, ReadGeneralColor(value));
			} else if (name.equals("border-color")) {
				res.lineColors.put(currSelectorName, ReadGeneralColor(value));
			} else if (name.equals("border-width")) {
				currStrokeWidth = ReadGeneralFloat(value);
				
				//Build it?
				if (currStrokeWidth>0 && currStrokeStyle==0) {
					res.lineStrokes.put(currSelectorName, new BasicStroke(currStrokeWidth));
					currStrokeWidth = -1;
					currStrokeStyle = -1;
				}
			} else if (name.equals("border-style")) {
				currStrokeStyle = ReadGeneralStyle(value);
				
				//Build it?
				if (currStrokeWidth>0 && currStrokeStyle==0) {
					res.lineStrokes.put(currSelectorName, new BasicStroke(currStrokeWidth));
					currStrokeWidth = -1;
					currStrokeStyle = -1;
				}
			} else if (name.equals("border")) {
				//Shortcut declaration: width, style, color
			} else {
				System.out.println("Skipping CSS property: " + name);
				Object[] objs = ReadAndMakeColor(value);
				res.lineColors.put(currSelectorName, (Color)objs[0]);
				res.lineStrokes.put(currSelectorName, (Stroke)objs[1]);
			}
		}
		
		public void startSelector(SelectorList selectors) throws CSSException {
			if (selectors.getLength()!=1) {
				throw new RuntimeException("Unknown: multiple selectors.");
			}
			
			//Save for later.
			//Trim, remove the *. before the class name.
			Selector currSelector = selectors.item(0);
			currSelectorName = currSelector.toString().trim();
			if (currSelectorName.startsWith("*.")) {
				currSelectorName = currSelectorName.substring(2);
			} else {
				System.out.println("Skipping CSS class name: " + currSelectorName);
				return;
			}

		}
		
		public void endSelector(SelectorList selectors) throws CSSException {
			currSelectorName = null;
			currStrokeWidth = -1;
			currStrokeStyle = -1;
		}
		
		public void startDocument(InputSource source) throws CSSException {}
		public void endDocument(InputSource source) throws CSSException {}
		public void comment(String text) throws CSSException {}
		public void ignorableAtRule(String atRule) throws CSSException {}
		public void namespaceDeclaration(String prefix, String uri) throws CSSException {}
		public void importStyle(String uri, SACMediaList media, String defaultNamespaceURI) throws CSSException {}
		public void startMedia(SACMediaList media) throws CSSException {}
		public void endMedia(SACMediaList media) throws CSSException {}
		public void startPage(String name, String pseudo_page) throws CSSException {}
		public void endPage(String name, String pseudo_page) throws CSSException {}
		public void startFontFace() throws CSSException {}
		public void endFontFace() throws CSSException {}
	}

}
