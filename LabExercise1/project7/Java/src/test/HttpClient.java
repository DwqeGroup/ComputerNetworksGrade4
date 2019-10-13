package test;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.util.Scanner;

public class HttpClient extends Thread{
	Socket socket = null;
	private String html = "";
	public HttpClient(String proxy, int port) {
		try {
			socket = new Socket(proxy, port);
		} catch (IOException e) {
			// TODO: handle exception
		}
	}
	
	
	
	@Override
	public void run() {
		// TODO Auto-generated method stub
		
		super.run();
		try {
			sendMessage();
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		int len = 0;
		try {
			InputStream s  = null;
			
			s = socket.getInputStream();
			InputStreamReader sr = new InputStreamReader(s);
			BufferedReader br = new BufferedReader(sr);
			
			
			
			html = br.readLine();
			
			System.out.println(html);
			System.out.println(html.length());
		} catch (IOException e) {
			// TODO: handle exception
		}
	}
	
	private void sendMessage() throws IOException{
		
			Scanner scanner = null;
			OutputStream os = socket.getOutputStream();
			OutputStreamWriter osw = new OutputStreamWriter(os);
			BufferedWriter bw = new BufferedWriter(osw);
			String in = "";
			try {
				scanner = new Scanner(System.in);
				
				
				
				in = scanner.nextLine();
				bw.write(in+"\r\n\r\n");
				bw.flush();
				
			}catch(IOException e) {
				e.printStackTrace();
			}
			scanner.close();
		
	}



	public static void main(String[] args) {
		// TODO Auto-generated method stub
		HttpClient client = new HttpClient("127.0.0.1", 1234);
		client.start();
	}

}
