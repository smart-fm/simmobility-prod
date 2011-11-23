package sim_mob.conf;

import java.awt.Color;
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
				System.out.println("-------------------------");
				LoadSingleFile(csi, f);
				System.out.println("-------------------------");
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
	private static Color ReadBackground(LexicalUnit value) {
		if (value.getLexicalUnitType()==LexicalUnit.SAC_IDENT) {
			return ReadColorIdentity(value);
		}
	}
	
	private static Color ReadColorIdentity(LexicalUnit value) {
		String ident = value.getStringValue();
		if (!IdentityColors.containsKey(ident.toLowerCase())) {
			throw new RuntimeException("Invalid color identity: " + ident;);
		}
		return IdentityColors.get(ident);
	}
	
	
	
	//Helper class
	private static class MyDocumentHandler implements DocumentHandler {
		private CSS_Interface res;
		public MyDocumentHandler(CSS_Interface res) { this.res = res; }
		
		
		
		public void property(String name, LexicalUnit value, boolean important) throws CSSException {
			System.out.print("  " + name + " : ");
			if (value.getLexicalUnitType()==LexicalUnit.SAC_IDENT) {
				System.out.print(value.getStringValue());
			} else if (value.getLexicalUnitType()==LexicalUnit.SAC_PIXEL) {
				System.out.print(value.getFloatValue());
				
				//border?
				LexicalUnit curr = value.getNextLexicalUnit();
				while (curr!=null) {
					if (curr.getLexicalUnitType()==LexicalUnit.SAC_IDENT) {
						System.out.print(" (" + curr.getStringValue() + ")");
					} else if (curr.getLexicalUnitType()==LexicalUnit.SAC_RGBCOLOR) {
						LexicalUnit curr2 = curr.getParameters();;
						
						System.out.print(" (");
						while (curr2!=null) {
							if (curr2.getLexicalUnitType()==LexicalUnit.SAC_INTEGER) {
								System.out.print(curr2.getIntegerValue());
							}
							if (curr2.getLexicalUnitType()==LexicalUnit.SAC_OPERATOR_COMMA) {
								System.out.print(",");
							}
							curr2=curr2.getNextLexicalUnit();
						}
						System.out.print(")");
					} else {
						System.out.print(" <" + curr.getLexicalUnitType() + ">");
					}
					curr = curr.getNextLexicalUnit();
				}
			} else if (value.getLexicalUnitType()==LexicalUnit.SAC_RGBCOLOR) {
				LexicalUnit curr = value.getParameters();;
				
				while (curr!=null) {
					if (curr.getLexicalUnitType()==LexicalUnit.SAC_INTEGER) {
						System.out.print(curr.getIntegerValue());
					}
					if (curr.getLexicalUnitType()==LexicalUnit.SAC_OPERATOR_COMMA) {
						System.out.print(",");
					}
					curr=curr.getNextLexicalUnit();
				}
			} else {
				System.out.print("<" + value.getLexicalUnitType() + ">");
			}
			System.out.println();
		}
		
		public void startSelector(SelectorList selectors) throws CSSException {
			Selector first = selectors.item(0);
			System.out.println(first.toString() + " {");
		}
		
		public void endSelector(SelectorList selectors) throws CSSException {
			System.out.println("}");
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
