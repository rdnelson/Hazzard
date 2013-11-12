package PiNet;

import java.net.*;
import java.io.IOException;
import org.dom4j.*;
import org.dom4j.io.*;
import java.util.*;
import java.io.Writer;
import java.io.StringWriter;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * The PiNet multi-casting library implementation in Java. This program is for
 * use in conjunction with the PiNet library in python
 *
 * @version 10.01.2013
 * @author Darren
 */
public class PiNet {

    MulticastSocket sock_send;
    MulticastSocket sock_receive;
    // Which port should we send to
    int port;
    // The time to live
    int ttl;
    // The multicast group
    String group;
    // Data about the song
    SongData songData;

    /**
     * PiNet constructor. Initializes constants, allows for extension using
     * different ports, or interface
     */
    public PiNet() {
        this.songData = new SongData();
        this.port = 9001;
        this.ttl = 1;
        this.group = "224.0.0.1";
    }

    /**
     * SongData class. Includes data about the current song, as received from
     * the synth.
     */
    public class SongData {

        public String songName = "";
        public int volume = -1;
        public Time time = new Time();

        public class Time {

            public int frame = -1;
            public int percent = -1;
            public int second = -1;
        }
    }

    public void initialize()
            throws IOException {
        // Configure the socket
        sock_receive = new MulticastSocket(new InetSocketAddress("0.0.0.0", port));
        sock_receive.joinGroup(InetAddress.getByName(group));
        sock_receive.setTimeToLive(ttl);
        sock_receive.setSoTimeout(1000);

        // Configure the socket
        sock_send = new MulticastSocket(new InetSocketAddress(InetAddress.getLocalHost().getHostName(), port));
        sock_send.joinGroup(InetAddress.getByName(group));
        sock_send.setTimeToLive(ttl);
        sock_send.setSoTimeout(1000);

        Receiver receiver = new Receiver();
        receiver.start();
    }

    public Object getData(String whatData) {
        if (whatData.equals("song")) {
            return songData;
        } else {
            return null;
        }
    }

    public void sendCommand(String command, String data)
            throws Exception {
        Document document;
        document = DocumentHelper.createDocument();
        Element root = document.addElement("commands");
        Element commandElem = root.addElement("command");
        commandElem.setText(command);

        commandElem.addAttribute("data", data);

        final Writer writer = new StringWriter();
        new XMLWriter(writer).write(root);
        String text = writer.toString().trim();

        DatagramPacket pack = new DatagramPacket(text.getBytes(), text.length(), InetAddress.getByName(group), port);
        sock_send.send(pack);
    }

    public void sendCommand(String command)
            throws Exception {
        sendCommand(command, "");
    }

    private class Receiver extends Thread {

        @Override
        public void run() {

            while (true) {
                try {
                    byte[] buf = new byte[1024];
                    DatagramPacket pack = new DatagramPacket(buf, buf.length);
                    sock_receive.receive(pack);
                    parse(new String(pack.getData()));
                } catch (IOException | DocumentException ex) {
                    if (ex.getClass().equals(IOException.class)) {
                        // Do nothing, this is expected to timeout
                    } else if (ex.getClass().equals(DocumentException.class)) {
                        Logger.getLogger(PiNet.class.getName()).log(Level.WARNING, null, ex);
                        System.err.println("Failed to parse received data");
                    }
                }
            }
        }
    }

    public void parse(String text)
            throws DocumentException {
        Document document = DocumentHelper.parseText(text.trim());
        
        System.out.println(text);
        
        if (document.getRootElement().getName().equals("data")) {
            List data;
            data = document.selectNodes("/data/*");
            Iterator<Element> iterator = data.iterator();
            while (iterator.hasNext()) {
                Element current = iterator.next();
                if (current.getName().equals("songData")) {
                    songData.songName = document.selectSingleNode("/data/songData/songName").getText();
                    songData.volume = Integer.parseInt(document.selectSingleNode("/data/songData/volume").getText());
                    songData.time.frame = Integer.parseInt(document.selectSingleNode("/data/songData/time/frame").getText());
                    songData.time.percent = Integer.parseInt(document.selectSingleNode("/data/songData/time/percent").getText());
                    songData.time.second = Integer.parseInt(document.selectSingleNode("/data/songData/time/second").getText());
                }
            }
        }
    }
}
