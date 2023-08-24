import requests
import json

url = "http://192.168.178.60/network" #ce <IP_Address> with the actual IP address of your server

"""
"define stream StockStream (symbol string, price int, volume int); " +
                "@info(name = 'query1') " +
                "from StockStream[volume < 150] " +
                "select symbol, price " +
                "insert into OutputStream;"
"""

"""payload = {                     #siddhi or metadata 
    'events': "Gyro, Touchscreen, joystick",                        #Touchscreen,  joystick, Accel, Humidity
    'values': "",                                                   # has to be special for every sensor
    'timeframe': "2, 2, 2",                                         #in seconds
    'operations': "&, &",
    'origins': ""                                                   #device name
    'destination': ""                                               #device name
}"""

"""
payload = {                         #siddhi, event or time
    'timestamp': "63800006400"
}
"""

"""
payload = {
    'ip': "10.11.251.33",
    'data': "true",
    'device': "pi1"           #either "watch" or "pi"
}
"""
#for switching networks
"""payload = {}"""

"""payload = {                      #siddhi, event or time
    'Event': "Touch",                            #Touchscreen,  joystick, Accel, Humidity
    'Touch': "True",                           # has to be special for every sensor
    'Timestamp': "42069",                        #in seconds
    'Origin': "3"
}"""

"""payload = {
    'type': "gyro",
    'payload': "3"

    #'type': "timeframe",
    #'payload': "10"

    #'type': "port",
    #'payload': "80"
}"""

payload = {
    
}
def setTime():
    payload = {
        'timestamp': "0"
    }
    url = "http://192.168.178.61/time"  # Replace <IP_Address> with the actual IP address of your server

    try:
        response = requests.post(url, data=payload)
        response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

        print("HTTP Request Successful")
        print("Response:", response.text)
    except requests.exceptions.RequestException as e:
        print("HTTP Request Failed:", e)

    url = "http://192.168.178.60/time"  # Replace <IP_Address> with the actual IP address of your server

    try:
        response = requests.post(url, data=payload)
        response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

        print("HTTP Request Successful")
        print("Response:", response.text)
    except requests.exceptions.RequestException as e:
        print("HTTP Request Failed:", e)

    url = "http://192.168.178.59/time"  # Replace <IP_Address> with the actual IP address of your server

    try:
        response = requests.post(url, data=payload)
        response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

        print("HTTP Request Successful")
        print("Response:", response.text)
    except requests.exceptions.RequestException as e:
        print("HTTP Request Failed:", e)

#setTime()

try:
    headers = {
        "Content-Type": "application/json"
    }

    response = requests.post(url, data=payload, headers=headers)

    response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

    print("HTTP Request Successful")
    print("Response:", response.text)
except requests.exceptions.RequestException as e:
    print("HTTP Request Failed:", e)
