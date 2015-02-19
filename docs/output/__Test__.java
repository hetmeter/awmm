import java.io.*;
import org.antlr.runtime.*;
import org.antlr.runtime.debug.DebugEventSocketProxy;


public class __Test__ {

    public static void main(String args[]) throws Exception {
        rma_01Lexer lex = new rma_01Lexer(new ANTLRFileStream("C:\\Users\\Aldhissla\\Source\\Repos\\awmm\\docs\\ex_prog_1", "UTF8"));
        CommonTokenStream tokens = new CommonTokenStream(lex);

        rma_01Parser g = new rma_01Parser(tokens, 49100, null);
        try {
            g.programBlock();
        } catch (RecognitionException e) {
            e.printStackTrace();
        }
    }
}