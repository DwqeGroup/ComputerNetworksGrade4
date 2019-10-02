import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.io.IOException;

public class ConnectionThread implements Runnable{

    Socket socket = null;
    InputStream inputstream = null;
    OutputStream outputstream = null;

    // according to socket, initialize socket multi-thread class
    public ConnectionThread(Socket socket) {
        this.socket = socket;
    }

    /* (non-Javadoc)
     * @see java.lang.Runnable#run()
     */
    @Override
    public void run() {
        try {
            // acquire the link of input and output stream
            inputstream = socket.getInputStream();
            outputstream = socket.getOutputStream();
            //establish the connection for request, request read the http request message from the inputstream
            Request request = new Request(inputstream);
            //get requested resource uri from the request
            String uri = request.getUri();

            // set up a response for the request, response according to outputstream and uri, set up the route from resource file to outputstream
            Response response = new Response(outputstream);
            // response reply with the requested resource
            response.responseResource(uri);

        }catch(IOException e) {
            e.printStackTrace();
        }finally {

        }
    }

}
