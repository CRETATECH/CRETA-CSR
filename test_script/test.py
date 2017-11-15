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
    print("SUCCESS: Connected with result code "+str(rc))
    client.subscribe('CSR5ccf7fd16259/slave')

def onSubscribe(client, userdata, mid, granted_qos):
    global flag
    print('SUCCESS: subscribed to CSR5ccf7fd16259/slave')
    flag = True

def deviceControl(device, value):
    global flag
    global flagAuto
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

def deviceRead(device):
    global flag
    global flagAuto
    tx = {}
    log = 'CMD-DATA: '
    if device == '0101':
        log = log + 'Device 1 '
    elif device == '0102':
        log = log + 'Device 2 '
    elif device == '0201':
        log = log + 'Temperature'
    elif device == '0401':
        log = log + 'Water level'
    print(log)
    tx['USER'] = 'HgN37'
    tx['FUNC'] = '002'
    tx['ADDR'] = device
    tx['DATA'] = ''
    client.publish('CSR5ccf7fd16259/master', json.dumps(tx))
    flag = False
    start = datetime.now()
    while flag is False:
        client.loop()
        timedelta = (datetime.now() - start).microseconds/1000
    return timedelta

def onMessage(client, userdata, msg):
    global flag
    global flagAuto
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
    if message['FUNC'] == '003':
        print(message)
        flag = True
    

def main(argv):
    global flag
    mode = 'default'
    device = ''
    function = ''
    value = ''
    period = 0
    try:
        opts, args = getopt.getopt(argv, 'hd:f:v:p:', ['help', 'function=', 'device=', 'value=', 'period='])
    except getopt.GetoptError:
        print('Wrong parameters, use -h or --help for more information')
    for opt, arg in opts:
        if opt in ('-h', '--help'):
            print('Usage')
            print('Default mode: send commands continously')
            print('    -p        Set period (in second)')
            print('')
            print('Manual mode: send one single command')
            print('    -f        Set function')
            print('    -d        Set device')
            print('    -v        Set value')
            sys.exit()
        if opt in ('-d', '--device'):
            mode = 'manual'
            device = arg
        if opt in ('-f', '--function'):
            mode = 'manual'
            function = arg
        if opt in ('-v', '--value'):
            mode = 'manual'
            value = arg
        if opt in ('-p', '--period'):
            period = float(arg)

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
    if mode == 'default':
        while True:
            try:
                summ = summ + deviceControl('0101', '1')
                count = count + 1
                time.sleep(period)
                summ = summ + deviceControl('0102', '1')
                count = count + 1
                time.sleep(period)
                summ = summ + deviceControl('0101', '0')
                count = count + 1
                time.sleep(period)
                summ = summ + deviceControl('0102', '0')
                count = count + 1
                time.sleep(period)
            except KeyboardInterrupt:
                print('\nAverate respond time: ' + str(summ/count))
                sys.exit()
    elif mode == 'manual':
        if function == '':
            print('Error: Need function in manual mode')
            sys.exit()
        if device == '':
            print('Error: Need device in manual mode')
            sys.exit()
        if (function == '001') and (value == ''):
            print('Error: Control function need value')
            sys.exit()
        res = 0
        if function == '001':
            res = deviceControl(device, value)
        elif function == '002':
            res = deviceRead(device)
        print('\nRespond time: ' + str(res) + ' milisecond') 



if __name__ == '__main__':
    main(sys.argv[1:])