/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package com.hazzard.gui;

/**
 *
 * @author Hao
 */
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

    RaceInfo raceInfo;
    PlayerInfo playerInfo;
    
    /**
     * PiNet constructor. Initializes constants, allows for extension using
     * different ports, or interface
     */
    public PiNet() {
        this.raceInfo = new RaceInfo();
        this.playerInfo = new PlayerInfo();
        this.port = 9001;
        this.ttl = 1;
        this.group = "224.0.0.1";
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
        if (whatData.equals("raceInfo")) {
            return raceInfo;
        } else if(whatData.equals("playerInfo")){
            return playerInfo;
        }else{
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
                        if (current.getName().equals("raceInfo")) {
                            raceInfo.time = Integer.parseInt(document.selectSingleNode("/data/raceInfo/time").getText());
                            raceInfo.laps = Integer.parseInt(document.selectSingleNode("/data/raceInfo/laps").getText());
                            raceInfo.joinedPlayers = Integer.parseInt(document.selectSingleNode("/data/raceInfo/joinedPlayers").getText());
                        }
                    }
                }
            }
                if (document.getRootElement().getName().equals("data")) {
                    List data;
                    data = document.selectNodes("/data/*");
                    Iterator<Element> iterator = data.iterator();
                    while (iterator.hasNext()) {
                        Element current = iterator.next();
                        if (current.getName().equals("playerInfo")) {
                            playerInfo.currentLap = Integer.parseInt(document.selectSingleNode("/data/playerInfo/currentLap").getText());
                            playerInfo.relativeTime = Integer.parseInt(document.selectSingleNode("/data/playerInfo/relativeTime").getText());
                            playerInfo.speed = Integer.parseInt(document.selectSingleNode("/data/playerInfo/speed").getText());
                            playerInfo.turn = Integer.parseInt(document.selectSingleNode("/data/playerInfo/turn").getText());
                            playerInfo.position = Integer.parseInt(document.selectSingleNode("/data/playerInfo/position").getText());
                            playerInfo.flag = Integer.parseInt(document.selectSignleNode("/data/plaerInfo/flag").getText());
                        }
                    }
                }
}
