import requests

url = "http://192.168.178.59/post"  # Replace <IP_Address> with the actual IP address of your server

payload = {
    'type': "Siddhi",                       #siddhi or metadata
    'message':  "define stream StockStream (symbol string, price int, volume int); " +
                "@info(name = 'query1') " +
                "from StockStream[volume < 150] " +
                "select symbol, price " +
                "insert into OutputStream;",
    'events': "Gyro, Touchscreen",          #Touchscreen,  joystick, Accel, Humidity
    'values': "{}, {}",                     # has to be special for every sensor
    'timeframe': "2",                       #in seconds
    'operations': "&"
}

"""
payload = {
    'type': "time",                         #siddhi, event or time
    'timestamp': ""
}
"""

"""
payload = {
    'type': "event",                        #siddhi, event or time
    'events': "Gyro, Touchscreen",          #Touchscreen,  joystick, Accel, Humidity
    'values': "{}, {}",                     # has to be special for every sensor
    'timestamp': "2"                        #in seconds
}
"""


try:
    response = requests.post(url, data=payload)
    response.raise_for_status()  # Raise an exception for 4xx or 5xx status codes

    print("HTTP Request Successful")
    print("Response:", response.text)
except requests.exceptions.RequestException as e:
    print("HTTP Request Failed:", e)