#!/usr/bin/python
# -*- coding: utf-8 -*-

import socket
import sys
from speech_recognition import Recognizer, Microphone
recognizer = Recognizer()
    
text = ""

SERVER_ADDRESS = "192.168.131.80" # To check !
SERVER_PORT = 5000
ESP_ADDRESS = "192.168.131.100"
ESP_PORT = 4000

socketServer = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def send(arg):
    cmd = ""
    socketESP = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        socketESP.connect((ESP_ADDRESS, ESP_PORT))
    except socket.error:
        print("Connection failed...")
        sys.exit()    
            
    print("Connection established !")
    print("Sending command...")
    
    arg += "\r"
    socketESP.send(arg.encode())
    socketESP.close()
    print("Connection finished")


try:
    socketServer.bind((SERVER_ADDRESS, SERVER_PORT))
except:
    print("Linking failed...")
    sys.exit()

while(1):
    print("Listening for connections...")
    socketServer.listen(1)
    
    connection, address = socketServer.accept()
    print("Client connected, address : %s:%s" % (address[0], address[1]))
    
    msgReceived = connection.recv(1024)
    print(msgReceived)
    if(str(msgReceived).find("Sunday") != -1):
        with Microphone(14) as source:
            print("Detecting ambient noise... Please wait...")
            recognizer.adjust_for_ambient_noise(source)
            print("Start talking...")
            recorded_audio = recognizer.listen(source)
            print("Record finished, sending to server...")
            try:
                text = recognizer.recognize_google(recorded_audio, language="fr-FR")
                print("You said : {}".format(text))
            except Exception as ex:
                print(ex)
    
        if(text == "allume la lumière"):
            send("light:1")
    
        elif(text == "éteins la lumière"):
            send("light:0")
            
        elif(text.find("allume le chauffage") != -1):
            stringList = text.split()
            for s in stringList:
                if s[0] in "0123456789":
                    send("heat:" + s)
            
        elif(text == "éteins le chauffage"):
            send("heat:0")
            
        else:
            print("No Result...")
        
sys.exit()
