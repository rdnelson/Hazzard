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
 *
 * NOTE: This code was written as a last minute workaround to get past Jython. Do not take this code for a final product, even though it "shipped".
 */
public class PiNet {

    MulticastSocket sock_receive;
    // Which port should we send to
    int port;
    // The time to live
    int ttl;
    // The multicast group
    String group;

    RaceInfo raceInfo;
    PlayerInfo playerInfo1;
    PlayerInfo playerInfo2;
    
    /**
     * PiNet constructor. Initializes constants, allows for extension using
     * different ports, or interface
     */
    public PiNet() {
        this.raceInfo = new RaceInfo();
        this.playerInfo1 = new PlayerInfo();
        this.playerInfo2 = new PlayerInfo();
        this.port = 9001;
        this.ttl = 1;
        this.group = "224.0.0.1";
    }

    //Initialize, creates the sockets, and starts the receiver thread
    public void initialize()
            throws IOException {
        // Configure the socket
        sock_receive = new MulticastSocket(new InetSocketAddress("0.0.0.0", port));
        sock_receive.joinGroup(InetAddress.getByName(group));
        sock_receive.setTimeToLive(ttl);
        sock_receive.setSoTimeout(1000);

        Receiver receiver = new Receiver();
        receiver.start();
    }

    //Get either Race info, or the appropriate PlayerInfo
    public Object getData(String whatData) {
        if (whatData.equals("RaceInfo")) {
            return raceInfo;
        } else if(whatData.equals("PlayerInfo1")){
            return playerInfo1;
        } else if(whatData.equals("PlayerInfo2")){
            return playerInfo2;
        }else{
            return null;
        }
    }
    
    //Receiver thread, sends data to parse
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

    //Parses an XML input string. Not enough time to do dynamic class creation
    //in Java, so everything is hardcoded. I do not like this, not one bit.
    public void parse(String text)
                    throws DocumentException {
                Document document = DocumentHelper.parseText(text.trim());

                if (document.getRootElement().getName().equals("syncData")) {
                    List data;
                    data = document.selectNodes("/syncData/*");
                    Iterator<Element> iterator = data.iterator();
                    while (iterator.hasNext()) {
                        Element current = iterator.next();
                        if (current.getName().equals("RaceInfo")) {
                            raceInfo.time = Integer.parseInt(document.selectSingleNode("/syncData/RaceInfo/Time").getText());
                            raceInfo.laps = Integer.parseInt(document.selectSingleNode("/syncData/RaceInfo/Laps").getText());
                            raceInfo.joinedPlayers = Integer.parseInt(document.selectSingleNode("/syncData/RaceInfo/JoinedPlayers").getText());                            
                        }else if(current.getName().equals("PlayerInfo1")) {
                            playerInfo1.currentLap = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/CurrentLap").getText());
                            playerInfo1.relativeTime = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/RelativeTime").getText());
                            playerInfo1.speed = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/Speed").getText());
                            playerInfo1.turn = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/Turn").getText());
                            playerInfo1.position = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/Position").getText());
                            playerInfo1.finished = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo1/Finished").getText());
                        } else if(current.getName().equals("PlayerInfo2")) {                          
                            playerInfo2.currentLap = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/CurrentLap").getText());
                            playerInfo2.relativeTime = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/RelativeTime").getText());
                            playerInfo2.speed = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/speed").getText());
                            playerInfo2.turn = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/Turn").getText());
                            playerInfo2.position = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/Position").getText());
                            playerInfo2.finished = Integer.parseInt(document.selectSingleNode("/syncData/PlayerInfo2/Finished").getText());
                        }
                    }
                }
    }
}
