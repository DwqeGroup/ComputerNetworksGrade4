//package com.imooc;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Scanner;


public class Client {
	public static void main(String[] args) {
		try {
			Socket client = new Socket("localhost",8888);
			while(1 == 1){
				OutputStream out=client.getOutputStream();
				DataOutputStream dos = new DataOutputStream(out);

				System.out.println("Please input request time: ");
				Scanner scanner = new Scanner(System.in);
				String request = scanner.nextLine();
				dos.writeUTF(request);
				//dos.writeUTF("hello");
				InputStream in = client.getInputStream();
				DataInputStream dis = new DataInputStream(in);
				System.out.println("info from server: " + dis.readUTF());
			}
		
		} catch (UnknownHostException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}