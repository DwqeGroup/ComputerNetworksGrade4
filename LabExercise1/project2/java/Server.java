import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;
import java.util.LinkedList;

public class Server {
    //define the storing place of html page 
    public static final String WEB_ROOT = System.getProperty("user.dir") + File.separator + "web_resource";
    public static final int port = 8080;
    //list to store each connection
    private List<Thread> connectlist = null;

    //main function
    public static void main(String[] args) {
        //webserver start
        Server server = new Server();
        server.start();
    }

    public void start() {
        ServerSocket serversocket = null;
        try {
            serversocket = new ServerSocket(port, 1, InetAddress.getByName("127.0.0.1"));
            System.out.println("Server is running!!");

        }catch(IOException e) {
            e.printStackTrace();
        }

        //record the number of request
        int count = 0;

        //list of storing each process
        connectlist = new LinkedList<Thread>();
        while(true) {
            Socket socket = null;
            try {
                //establish the connection for this request
                socket = serversocket.accept();

                System.out.println("Connection"+ count +" has been established!!");

                //establish multi-thread for this socket and add this thread into list
                ConnectionThread connectionthread = new ConnectionThread(socket);
                Thread thread = new Thread(connectionthread);
                thread.start();
                connectlist.add(thread);

                System.out.println("Connection"+ count++ +" thread has been added into the queue!!");

            }catch(Exception e) {
                e.printStackTrace();
                break;
            }
        }
    }
}