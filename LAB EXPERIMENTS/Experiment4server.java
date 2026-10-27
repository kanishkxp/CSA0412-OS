import java.io.*;
import java.net.*;

public class Experiment4server {
    public static void main(String[] args) {
        int port = 9090;
        try (DatagramSocket serverSocket = new DatagramSocket(port)) {
            System.out.println("UDP Server is ready to receive on port " + port + "...");

            byte[] receiveBuffer = new byte[1024];
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length);
            
            // Receive datagram from client
            serverSocket.receive(receivePacket);
            String message = new String(receivePacket.getData(), 0, receivePacket.getLength());
            InetAddress clientAddress = receivePacket.getAddress();
            int clientPort = receivePacket.getPort();
            
            System.out.println("Received message from " + clientAddress + ":" + clientPort + " -> " + message);

            // Send response back to client
            String responseStr = "Hello from UDP Server! Datagram received.";
            byte[] sendBuffer = responseStr.getBytes();
            DatagramPacket sendPacket = new DatagramPacket(sendBuffer, sendBuffer.length, clientAddress, clientPort);
            
            serverSocket.send(sendPacket);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}