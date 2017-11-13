#! /usr/bin/python3

import sys, getopt
import paho.mqtt.client as mqtt
from datetime import datetime
import json
import ast
import time

# client = mqtt.Client(transport="websockets")
client = mqtt.Client()
flag = False

def onConnected(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    print('Ket noi thanh cong')
    client.subscribe('CSR5ccf7fd16259/slave')

def onSubscribe(client, userdata, mid, granted_qos):
    global flag
    print('Da subscribe toi CSR5ccf7fd16259/slave')
    flag = True

def deviceControl(device, value):
    global flag
    tx = {}
    log = 'CMD-CONTROL: Device '
    if device == '0101':
        log = log + '1 '
    elif device == '0102':
        log = log + '2 '
    if value == '1':
        log = log + 'ON'
    elif value == '0':
        log = log + 'OFF'
    print(log)
    tx['USER'] = 'HgN37'
    tx['FUNC'] = '001'
    tx['ADDR'] = device
    tx['DATA'] = value
    client.publish('CSR5ccf7fd16259/master', json.dumps(tx))
    flag = False
    start = datetime.now()
    while flag is False:
        client.loop()
        timedelta = (datetime.now() - start).microseconds/1000
    return timedelta

def onMessage(client, userdata, msg):
    global flag
    if msg.topic != 'CSR5ccf7fd16259/slave':
        return
    message = ast.literal_eval(str(msg.payload.decode('utf-8')))
    if message['FUNC'] == '002':
        if message['ADDR'] == '0101':
            if message['DATA'] == '1':
                print('RES-STATUS: Device 1 ON')
            elif message['DATA'] == '0':
                print('RES-STATUS: Device 1 OFF')
        if message['ADDR'] == '0102':
            if message['DATA'] == '1':
                print('RES-STATUS: Device 2 ON')
            elif message['DATA'] == '0':
                print('RES-STATUS: Device 2 OFF')
        if message['ADDR'] == '0201':
            print('RES-STATUS: Temperature ' + message['DATA'])
        if message['ADDR'] == '0401':
            if message['DATA'] == '1':
                print('RES-STATUS: Water higher than threshold')
            elif message['DATA'] == '0':
                print('RES-STATUS: Water lower than threshold')
    if message['FUNC'] == '001':
        if message['ADDR'] == '0101':
            if message['DATA'] == '1':
                print('RES-CONTROL: Device 1 ON')
            elif message['DATA'] == '0':
                print('RES-CONTROL: Device 1 OFF')
        if message['ADDR'] == '0102':
            if message['DATA'] == '1':
                print('RES-CONTROL: Device 2 ON')
            elif message['DATA'] == '0':
                print('RES-CONTROL: Device 2 OFF')
        flag = True
    

def main(argv):
    global flag
    try:
        opts, args = getopt.getopt(argv, 'h', 'help')
    except getopt.GetoptError:
        print('Sai tham so')
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            print('Day la help')
            sys.exit()
    
    client.on_connect = onConnected
    client.on_subscribe = onSubscribe
    client.on_message = onMessage
    client.connect("cretacam.ddns.net", 1889, 60)
    while flag is False:
        client.loop()
    flag = False
    count = 0
    summ = 0
    respond = 0
    while True:
        try:
            summ = summ + deviceControl('0101', '1')
            count = count + 1
            summ = summ + deviceControl('0102', '1')
            count = count + 1
            summ = summ + deviceControl('0101', '0')
            count = count + 1
            summ = summ + deviceControl('0102', '0')
            count = count + 1
        except KeyboardInterrupt:
            print(str(summ/count))
            sys.exit()
    client.loop_forever()



if __name__ == '__main__':
    main(sys.argv[1:])