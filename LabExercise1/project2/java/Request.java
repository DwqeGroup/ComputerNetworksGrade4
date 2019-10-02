import java.io.InputStream;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Request {

    private InputStream inputstream = null;
    private String uri = null;

    // initialize request according to inputstream 
    public Request(InputStream inputstream) {
        this.inputstream = inputstream;
        parseUri();
    }

    // read inputstream and turn it into string
    @SuppressWarnings("finally")
    private String requestToString() {
        String requestString = null;
        // turn the byte string into char string 
        BufferedReader bfreader = new BufferedReader(new InputStreamReader(inputstream));
        StringBuffer buffer = new StringBuffer();
        char[] temp = new char[2048];
        int length = 0;
        try {
            length = bfreader.read(temp);
            buffer.append(temp,0,length);   
            requestString = buffer.toString();

        }catch(IOException e) {
            e.printStackTrace();
        }finally {
            // output request
            System.out.println("request is :");
            System.out.println(requestString);

            return requestString;
        }
    }

    // according to the characteristic of request message, requested uri is between the 1st and 2nd blank.
    public void parseUri() {
        String request = requestToString();
        if(request != null) {
            int space1 = -1;
            int space2 = -1;
            space1 = request.indexOf(' ');
            if(space1 != -1) {
                space2 = request.indexOf(' ',space1 + 1);
            }
            if(space2 > space1) {
                // cut out the string between the 1st and 2nd blank, the requested uri
                uri = request.substring(space1 + 1, space2);
            }
        }
    }

    // return the uri
    public String getUri() {
        return uri;
    }
}