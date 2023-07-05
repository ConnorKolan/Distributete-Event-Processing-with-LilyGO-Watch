import requests

url = "http://192.168.178.59/ip"  # Replace <IP_Address> with the actual IP address of your server

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


"""payload = {                         
    'ip': "10.11.251.33",
    'device': "pi1"           #either "watch" or "pi"
}"""


"""
payload = {                      #siddhi, event or time
    'event': "Gyro",                            #Touchscreen,  joystick, Accel, Humidity
    'values': "True",                           # has to be special for every sensor
    'timestamp': "42069"                        #in seconds
}
"""


try:
    response = requests.post(url, data=payload)
    response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

    print("HTTP Request Successful")
    print("Response:", response.text)
except requests.exceptions.RequestException as e:
    print("HTTP Request Failed:", e)
