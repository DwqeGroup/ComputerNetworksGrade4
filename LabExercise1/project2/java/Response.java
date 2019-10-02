import java.io.OutputStream;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;

public class Response {
    private static final int BUFFER_SIZE = 1024;
    OutputStream outputstream = null;

    // use outputstream to initialize response
    public Response(OutputStream outputstream) {
        this.outputstream = outputstream;
    }

    // the path of request resource
    public void responseResource(String uri) {
        //byte[] when reading the file 
        byte[] resourcetemp = new byte[BUFFER_SIZE];
        // the input stream
        FileInputStream fileinputstream = null;
        if(uri == null)
            return;
        try {
            // establish the file 
            File resource = new File(Server.WEB_ROOT,uri);
            // judge whether file exists
            if(resource.exists()) {

                System.out.println("resource that is requested: " + resource.getName());

                fileinputstream = new FileInputStream(resource);
                int length = 0;

                String responsehead = "HTTP/1.1 200 OK\r\n" +  "\r\n";
                outputstream.write(responsehead.getBytes());

                while((length = fileinputstream.read(resourcetemp)) > 0) {
                    outputstream.write(resourcetemp, 0, length);
                }
            }
            else {
                String errorPage = "HTTP/1.1 404 File Not Found\r\n" + "Content-Type: text/html\r\n" + "Content-Length: 23\r\n" + "\r\n" + "<h1>File Not Found</h1>";
                outputstream.write(errorPage.getBytes());
            }

            outputstream.close();
        }catch(IOException e){
            e.printStackTrace();
        }finally {
            outputstream = null;
            fileinputstream = null;
        }
    }
}