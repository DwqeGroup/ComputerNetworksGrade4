import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Scanner;
import java.net.Socket;

import javax.sound.midi.Receiver;

import sun.misc.BASE64Encoder;

public class SMTPclient {

	public static void main(String[] args) {

		//Scanner scanner = new Scanner(System.in);
		//String sender = "qzlovetby@163.com";
		//String receiver = "hongyan9872@163.com";
		/*String password = "qz701731tby";
		String user = new BASE64Encoder().encode(sender.substring(0,
				sender.indexOf("@")).getBytes());
		String pass = new BASE64Encoder().encode(password.getBytes());*/

		try {
			Socket socket = new Socket("smtp.163.com", 25);
			InputStream inputStream = socket.getInputStream();
			OutputStream outputStream = socket.getOutputStream();
			BufferedReader reader = new BufferedReader(new InputStreamReader(
					inputStream));
			PrintWriter writter = new PrintWriter(outputStream, true); //the true is quite important

			System.out.println(reader.readLine());

			// HELO
			writter.println("HELO huan");
			System.out.println(reader.readLine());

			// AUTH LOGIN
			writter.println("auth login");
			System.out.println(reader.readLine());
			String user = getUsername();
			writter.println(user);
			System.out.println(reader.readLine());
			String pass = getPassword();
			writter.println(pass);
			System.out.println(reader.readLine());
			// Above Authentication successful

			// Set "mail from" and "rcpt to"
		    String sender = getSender();
			writter.println("mail from:<" + sender + ">");
			System.out.println(reader.readLine());
			String receiver = getReceiver();
			writter.println("rcpt to:<" + receiver + ">");
			System.out.println(reader.readLine());

			// Set "data"
			writter.println("data");
			System.out.println(reader.readLine());

			String subject = getSubject();
			writter.println("subject: " + subject);
			writter.println("from:" + sender);
			writter.println("to:" + receiver);
			writter.println("Content-Type: text/plain;charset=\"gb2312\"");
			writter.println();

			String content = getContent();
			writter.println(content);
			writter.println(".");
			writter.println("");
			System.out.println(reader.readLine());

			// after sending, say goodbye to the server
			writter.println("rset");
			System.out.println(reader.readLine());

			writter.println("quit");
			System.out.println(reader.readLine());

			//scanner.close();
			socket.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public static String getUsername(){
		System.out.println("Please input your username: ");
		Scanner scanner = new Scanner(System.in);
		String username = scanner.nextLine();
		String encrypt_username = new BASE64Encoder().encode(username.substring(0,
		username.indexOf("@")).getBytes());
		return encrypt_username;
	}

	public static String getPassword(){
		System.out.println("Please input your password: ");
		Scanner scanner = new Scanner(System.in);
		String password = scanner.nextLine();
		String encrypt_password = new BASE64Encoder().encode(password.getBytes());
		return encrypt_password;
	}

	public static String getSender(){
		System.out.println("Please input where you send from (your own username): ");
		Scanner scanner = new Scanner(System.in);
		String Sender = scanner.nextLine();
		return Sender;
	}

	public static String getReceiver(){
		System.out.println("Please input where you want to send (target username): ");
		Scanner scanner = new Scanner(System.in);
		String Receiver = scanner.nextLine();
		return Receiver;
	}

	public static String getSubject(){
		System.out.println("Please input your e-mail's subject: ");
		Scanner scanner = new Scanner(System.in);
		String Subject = scanner.nextLine();
		return Subject;
	}

	public static String getContent(){
		System.out.println("Please input your e-mail's content: ");
		Scanner scanner = new Scanner(System.in);
		String Content = scanner.nextLine();
		return Content;
	}
}