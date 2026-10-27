import java.io.*;
import java.net.*;

public class Experiment4client {
    public static void main(String[] args) {
        String serverAddress = "127.0.0.1";
        int port = 9090;

        try (DatagramSocket clientSocket = new DatagramSocket()) {
            InetAddress address = InetAddress.getByName(serverAddress);
            
            // Send message to server
            String message = "Hello UDP Server, sending datagram.";
            byte[] sendBuffer = message.getBytes();
            DatagramPacket sendPacket = new DatagramPacket(sendBuffer, sendBuffer.length, address, port);
            clientSocket.send(sendPacket);

            // Receive response from server
            byte[] receiveBuffer = new byte[1024];
            DatagramPacket receivePacket = new DatagramPacket(receiveBuffer, receiveBuffer.length);
            clientSocket.receive(receivePacket);

            String response = new String(receivePacket.getData(), 0, receivePacket.getLength());
            System.out.println("Received from server: " + response);

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
