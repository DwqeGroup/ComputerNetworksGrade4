//package com.imooc;

import java.io.FileReader;
import java.io.File;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.ObjectInputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.DataOutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.io.DataInputStream;

public class Server {
	public static void main(String[] args) {
		try {
			//1.create ServerSocket
			ServerSocket serverSocket = new ServerSocket(8888);
			//2.call accept(), waiting for the client connection
			System.out.println("***Server is starting, waiting for client's connection***");
			Socket socket=serverSocket.accept();
			while(1 == 1){
				InputStream in = socket.getInputStream();
				DataInputStream dis = new DataInputStream(in);
				String str = dis.readUTF();
				System.out.println("Server read from client: " + str);
				
				OutputStream out = socket.getOutputStream();
				//dataOutputSream
				DataOutputStream dos = new DataOutputStream(out);
				String tmp_data = getTempData(str);
				dos.writeUTF(tmp_data);
			}		
			
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	public static String getTempData(String str) throws IOException{
		int date = Integer.parseInt(str);
		String Temperature_filename = "Temperature.dat";
		String Humidity_filename = "Humidity.dat";
		String Light_filename = "Light.dat";

		ArrayList Temperature = getDatFile(Temperature_filename);
		ArrayList Humidity = getDatFile(Humidity_filename);
		ArrayList Light = getDatFile(Light_filename);

		String outcome = "TEMPERATURE = "+ Temperature.get(date - 1) + " HUMIDITY = " + Humidity.get(date - 1) +  " LIGHT = " + Light.get(date - 1);
		return outcome;
	}

	public static ArrayList getDatFile(String filename) throws IOException{
		File file=new File("./" + filename);
		if(!file.exists()||file.isDirectory())
			throw new FileNotFoundException();
		BufferedReader br=new BufferedReader(new FileReader(file));
		ArrayList<String> data = new ArrayList<String>();
		String temp=null;
		temp=br.readLine();
		while(temp!=null){
			data.add(temp);
			temp=br.readLine();
		}
		return data;
	}

}