package org.spacehq.sockfile;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.Socket;

public class Main {
    public static void main(String args[]) {
        if(args.length < 2) {
            System.err.println("Usage: java -jar sockfile.jar <ip> <file>");
            return;
        }

        File file = new File(args[1]);
        if(!file.exists()) {
            System.err.println("File \"" + args[1] + "\" does not exist.");
            return;
        }

        Socket socket = null;
        DataOutputStream out = null;
        try {
            socket = new Socket(args[0], 5000);
            out = new DataOutputStream(socket.getOutputStream());
        } catch(IOException e) {
            System.err.println("Failed to open socket.");
            e.printStackTrace();
            return;
        }

        FileInputStream in = null;
        try {
            in = new FileInputStream(file);
        } catch(IOException e) {
            System.err.println("Failed to open file stream.");
            e.printStackTrace();
            return;
        }

        try {
            System.out.println("Sending info...");
            out.writeLong(file.length());

            System.out.println("Sending file...");
            byte buffer[] = new byte[1024 * 128];
            int length = 0;
            while((length = in.read(buffer)) != -1) {
                out.write(buffer, 0, length);
            }

            System.out.println("File sent successfully.");
        } catch(IOException e) {
            System.err.println("Failed to send data.");
            e.printStackTrace();
            return;
        } finally {
            try {
                in.close();
                out.close();
                socket.close();
            } catch(IOException e) {
            }
        }
    }
}
